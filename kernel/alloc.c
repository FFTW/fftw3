/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* $Id: alloc.c,v 1.41 2003-03-15 20:29:43 stevenj Exp $ */

#include "ifftw.h"

#if defined(HAVE_DECL_MEMALIGN) && !HAVE_DECL_MEMALIGN
#  if defined(HAVE_MALLOC_H)
#    include <malloc.h>
#  else
extern void *memalign(size_t, size_t);
#  endif
#endif

#if defined(HAVE_DECL_POSIX_MEMALIGN) && !HAVE_DECL_POSIX_MEMALIGN
extern int posix_memalign(void **, size_t, size_t);
#endif

#if defined(macintosh) /* MacOS 9 */
#  include <Multiprocessing.h>
#endif

#define real_malloc X(malloc)
#define real_free free /* memalign and malloc use ordinary free */

#if defined(WITH_OUR_MALLOC16) && (MIN_ALIGNMENT == 16)
/* Our own 16-byte aligned malloc/free.  Assumes sizeof(void*) is a
   power of two <= 8 and that malloc is at least sizeof(void*)-aligned.

   The main reason for this routine is that, as of this writing,
   Windows does not include any aligned allocation routines in its
   system libraries, and instead provides an implementation with a
   Visual C++ "Processor Pack" that you have to statically link into
   your program.  We do not want to require users to have VC++
   (e.g. gcc/MinGW should be fine).  Our code should be at least as good
   as the MS _aligned_malloc, in any case, according to second-hand
   reports of the algorithm it employs (also based on plain malloc). */
static void *our_malloc16(size_t n)
{
     void *p0, *p;
     if (!(p0 = malloc(n + 16))) return (void *) 0;
     p = (void *) (((uintptr_t) p0 + 16) & (~((uintptr_t) 15)));
     *((void **) p - 1) = p0;
     return p;
}
static void our_free16(void *p)
{
     if (p) free(*((void **) p - 1));
}
#endif

/* part of user-callable API */
void *X(malloc)(size_t n)
{
     void *p;

#if defined(MIN_ALIGNMENT)

#  if defined(WITH_OUR_MALLOC16) && (MIN_ALIGNMENT == 16)
     p = our_malloc16(n);
#    undef real_free
#    define real_free our_free16

#  elif defined(HAVE_MEMALIGN)
     p = memalign(MIN_ALIGNMENT, n);

#  elif defined(HAVE_POSIX_MEMALIGN)
     /* note: posix_memalign is broken in glibc 2.2.5: it constrains
	the size, not the alignment, to be (power of two) * sizeof(void*).
        The bug seems to have been fixed as of glibc 2.3.1. */
     if (posix_memalign(&p, MIN_ALIGNMENT, n))
	  p = (void*) 0;

#  elif defined(__ICC) || defined(__INTEL_COMPILER) || defined(HAVE__MM_MALLOC)
     /* Intel's C compiler defines _mm_malloc and _mm_free intrinsics */
     p = (void *) _mm_malloc(n, MIN_ALIGNMENT);
#    undef real_free
#    define real_free _mm_free

#  elif defined(_MSC_VER)
     /* MS Visual C++ 6.0 with a "Processor Pack" supports SIMD
	and _aligned_malloc/free (uses malloc.h) */
     p = (void *) _aligned_malloc(n, MIN_ALIGNMENT);
#    undef real_free
#    define real_free _aligned_free

#  elif (defined(__MACOSX__) || defined(__APPLE__)) && (MIN_ALIGNMENT <= 16)
     /* MacOS X malloc is already 16-byte aligned */
     p = malloc(n);

#  elif defined(macintosh) /* MacOS 9 */
     p = (void *) MPAllocateAligned(n,
#    if MIN_ALIGNMENT == 8
				    kMPAllocate8ByteAligned,
#    elif MIN_ALIGNMENT == 16
				    kMPAllocate16ByteAligned,
#    elif MIN_ALIGNMENT == 32
				    kMPAllocate32ByteAligned,
#    else
#      error "Unknown alignment for MPAllocateAligned"
#    endif
				    0);
#    undef real_free
#    define real_free MPFree

#  else
     /* Add your machine here and send a patch to fftw@fftw.org 
        or (e.g. for Windows) configure --with-our-malloc16 */
#    error "Don't know how to malloc() aligned memory."
#  endif

#else /* !defined(MIN_ALIGMENT) */
     p = malloc(n);
#endif

     return p;
}

/* part of user-callable API */
void X(free)(void *p)
{
     real_free(p);
}

/**********************************************************
 *   DEBUGGING CODE
 **********************************************************/
#if defined(FFTW_DEBUG_MALLOC)

#include <stdio.h>

/*
  debugging malloc/free. 
 
  1) Initialize every malloced and freed area to random values, just
  to make sure we are not using uninitialized pointers.
 
  2) check for blocks freed twice.
 
  3) Check for writes past the ends of allocated blocks
 
  4) destroy contents of freed blocks in order to detect incorrect reuse.
 
  5) keep track of who allocates what and report memory leaks
 
  This code is a quick and dirty hack.  May be nonportable. 
  Use at your own risk.
 
*/

#define MAGIC ((size_t)0xABadCafe)
#define PAD_FACTOR 2
#define SZ_HEADER (4 * sizeof(size_t))
#define HASHSZ 1031

static unsigned int hashaddr(void *p)
{
     return ((unsigned long)p) % HASHSZ;
}

struct mstat {
     int siz;
     int maxsiz;
     int cnt;
     int maxcnt;
};

static struct mstat mstat[MALLOC_WHAT_LAST];

struct minfo {
     const char *file;
     int line;
     size_t n;
     void *p;
     struct minfo *next;
};

static struct minfo *minfo[HASHSZ] = {0};

#ifdef HAVE_THREADS
int X(in_thread) = 0;
#endif

void *X(malloc_debug)(size_t n, enum malloc_tag what,
                      const char *file, int line)
{
     char *p;
     size_t i;
     struct minfo *info;
     struct mstat *stat = mstat + what;
     struct mstat *estat = mstat + EVERYTHING;

     if (n == 0)
          n = 1;

     if (!IN_THREAD) {
	  stat->siz += n;
	  if (stat->siz > stat->maxsiz)
	       stat->maxsiz = stat->siz;
	  estat->siz += n;
	  if (estat->siz > estat->maxsiz)
	       estat->maxsiz = estat->siz;
     }

     p = (char *) real_malloc(PAD_FACTOR * n + SZ_HEADER);
     A(p);

     /* store the sz in a known position */
     ((size_t *) p)[0] = n;
     ((size_t *) p)[1] = MAGIC;
     ((size_t *) p)[2] = what;

     /* fill with junk */
     for (i = 0; i < PAD_FACTOR * n; i++)
          p[i + SZ_HEADER] = (char) (i ^ 0xEF);

     if (!IN_THREAD) {
	  ++stat->cnt;
	  ++estat->cnt;
	  
	  if (stat->cnt > stat->maxcnt)
	       stat->maxcnt = stat->cnt;
	  if (estat->cnt > estat->maxcnt)
	       estat->maxcnt = estat->cnt;
     }

     /* skip the info we stored previously */
     p = p + SZ_HEADER;

     if (!IN_THREAD) {
	  unsigned int h = hashaddr(p);
	  /* record allocation in allocation list */
	  info = (struct minfo *) malloc(sizeof(struct minfo));
	  info->n = n;
	  info->file = file;
	  info->line = line;
	  info->p = p;
	  info->next = minfo[h];
	  minfo[h] = info;
     }

     return (void *) p;
}

void X(ifree)(void *p)
{
     char *q;

     A(p);

     q = ((char *) p) - SZ_HEADER;
     A(q);

     {
          size_t n = ((size_t *) q)[0];
          size_t magic = ((size_t *) q)[1];
          int what = ((size_t *) q)[2];
          size_t i;
          struct mstat *stat = mstat + what;
          struct mstat *estat = mstat + EVERYTHING;

          /* set to zero to detect duplicate free's */
          ((size_t *) q)[0] = 0;

          A(magic == MAGIC);
          ((size_t *) q)[1] = ~MAGIC;

	  if (!IN_THREAD) {
	       stat->siz -= n;
	       A(stat->siz >= 0);
	       estat->siz -= n;
	       A(estat->siz >= 0);
	  }

          /* check for writing past end of array: */
          for (i = n; i < PAD_FACTOR * n; ++i)
               if (q[i + SZ_HEADER] != (char) (i ^ 0xEF)) {
                    A(0 /* array bounds overwritten */ );
               }
          for (i = 0; i < PAD_FACTOR * n; ++i)
               q[i + SZ_HEADER] = (char) (i ^ 0xAD);

	  if (!IN_THREAD) {
	       --stat->cnt;
	       --estat->cnt;
	       
	       A(stat->cnt >= 0);
	       A((stat->cnt == 0 && stat->siz == 0) ||
		 (stat->cnt > 0 && stat->siz > 0));
	       A(estat->cnt >= 0);
	       A((estat->cnt == 0 && estat->siz == 0) ||
		 (estat->cnt > 0 && estat->siz > 0));
	  }

          real_free(q);
     }

     if (!IN_THREAD) {
          /* delete minfo entry */
	  unsigned int h = hashaddr(p);
          struct minfo **i;

          for (i = minfo + h; *i; i = &((*i)->next)) {
               if ((*i)->p == p) {
                    struct minfo *i0 = (*i)->next;
                    free(*i);
                    *i = i0;
                    return;
               }
          }

          A(0 /* no entry in minfo list */ );
     }
}

void X(malloc_print_minfo)(int verbose)
{
     struct minfo *info;
     int what;
     unsigned int h;

     if (verbose) {
	  static const char *names[MALLOC_WHAT_LAST] = {
	       "EVERYTHING",
	       "PLANS", "SOLVERS", "PROBLEMS", "BUFFERS",
	       "HASHT", "TENSORS", "PLANNERS", "SLVDSC", "TWIDDLES",
	       "STRIDES", "OTHER"
	  };

	  printf("%12s %8s %8s %10s %10s\n",
		 "what", "cnt", "maxcnt", "siz", "maxsiz");

	  for (what = 0; what < MALLOC_WHAT_LAST; ++what) {
	       struct mstat *stat = mstat + what;
	       printf("%12s %8d %8d %10d %10d\n",
		      names[what], stat->cnt, stat->maxcnt,
		      stat->siz, stat->maxsiz);
	  }
     }

     for (h = 0; h < HASHSZ; ++h) 
	  if (minfo[h]) {
	       printf("\nUnfreed allocations:\n");
	       break;
	  }

     for (h = 0; h < HASHSZ; ++h) 
	  for (info = minfo[h]; info; info = info->next) {
	       printf("%s:%d:  %d bytes at %p\n",
		      info->file, info->line, info->n, info->p);
	  }
}

#else
/**********************************************************
 *   NON DEBUGGING CODE
 **********************************************************/
/* production version, no hacks */

void *X(malloc_plain)(size_t n)
{
     void *p;
     if (n == 0)
          n = 1;
     p = real_malloc(n);
     CK(p);

#ifdef MIN_ALIGMENT
     A((((uintptr_t)p) % MIN_ALIGNMENT) == 0);
#endif

     return p;
}

void X(ifree)(void *p)
{
     real_free(p);
}

#endif

void X(ifree0)(void *p)
{
     /* common pattern */
     if (p) X(ifree)(p);
}
