/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

/* $Id: alloc.c,v 1.11 2002-07-29 01:19:35 stevenj Exp $ */

#include "ifftw.h"

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#define real_free free /* memalign and malloc use ordinary free */

static inline void *real_malloc(size_t n)
{
     void *p;

#if defined(HAVE_MEMALIGN)
     p = memalign(MIN_ALIGNMENT, n);
#elif defined(__ICC) || defined(__INTEL_COMPILER) || defined(HAVE__MM_MALLOC)
     /* Intel's C compiler defines _mm_malloc and _mm_free intrinsics */
     p = (void *) _mm_malloc(n, MIN_ALIGNMENT);
#    undef real_free
#    define real_free _mm_free
#else
     p = malloc(n);
#endif
     return p;
}

/**********************************************************
 *   DEBUGGING CODE
 **********************************************************/
#ifdef FFTW_DEBUG
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

struct mstat {
     int siz;
     int maxsiz;
     int cnt;
     int maxcnt;
};

static struct mstat mstat[MALLOC_WHAT_LAST];

#define MAGIC ((size_t)0xABadCafe)
#define PAD_FACTOR 2
#define SZ_HEADER (4 * sizeof(size_t))

struct minfo {
     const char *file;
     int line;
     size_t n;
     void *p;
     struct minfo *next;
};

static struct minfo *minfo = 0;

void *X(malloc_debug)(size_t n, enum fftw_malloc_what what,
                      const char *file, int line)
{
     char *p;
     size_t i;
     struct minfo *info;
     struct mstat *stat = mstat + what;
     struct mstat *estat = mstat + EVERYTHING;

     if (n == 0)
          n = 1;

     stat->siz += n;
     if (stat->siz > stat->maxsiz)
          stat->maxsiz = stat->siz;
     estat->siz += n;
     if (estat->siz > estat->maxsiz)
          estat->maxsiz = estat->siz;

     p = (char *) real_malloc(PAD_FACTOR * n + SZ_HEADER);
     A(p);

     /* store the sz in a known position */
     ((size_t *) p)[0] = n;
     ((size_t *) p)[1] = MAGIC;
     ((size_t *) p)[2] = what;

     /* fill with junk */
     for (i = 0; i < PAD_FACTOR * n; i++)
          p[i + SZ_HEADER] = (char) (i ^ 0xEF);

     ++stat->cnt;
     ++estat->cnt;

     if (stat->cnt > stat->maxcnt)
          stat->maxcnt = stat->cnt;
     if (estat->cnt > estat->maxcnt)
          estat->maxcnt = estat->cnt;


     /* skip the info we stored previously */
     p = p + SZ_HEADER;

     /* record allocation in allocation list */
     info = (struct minfo *) malloc(sizeof(struct minfo));
     info->n = n;
     info->file = file;
     info->line = line;
     info->p = p;
     info->next = minfo;
     minfo = info;

     return (void *) p;
}

void X(free)(void *p)
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

          stat->siz -= n;
          A(stat->siz >= 0);
          estat->siz -= n;
          A(estat->siz >= 0);

          /* check for writing past end of array: */
          for (i = n; i < PAD_FACTOR * n; ++i)
               if (q[i + SZ_HEADER] != (char) (i ^ 0xEF)) {
                    A(0 /* array bounds overwritten */ );
               }
          for (i = 0; i < PAD_FACTOR * n; ++i)
               q[i + SZ_HEADER] = (char) (i ^ 0xAD);

          --stat->cnt;
          --estat->cnt;

          A(stat->cnt >= 0);
          A((stat->cnt == 0 && stat->siz == 0) ||
            (stat->cnt > 0 && stat->siz > 0));
          A(estat->cnt >= 0);
          A((estat->cnt == 0 && estat->siz == 0) ||
            (estat->cnt > 0 && estat->siz > 0));

          real_free(q);
     }

     {
          /* delete minfo entry */
          struct minfo **i;

          for (i = &minfo; *i; i = &((*i)->next)) {
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

void X(malloc_print_minfo)(void)
{
     struct minfo *info;
     int what;

     static const char *names[MALLOC_WHAT_LAST] = {
	  "EVERYTHING",
	  "PLANS", "SOLVERS", "PROBLEMS", "BUFFERS",
	  "HASHT", "TENSORS", "PLANNERS", "PAIRS", "TWIDDLES",
	  "OTHER"
     };

     printf("%12s %8s %8s %10s %10s\n",
            "what", "cnt", "maxcnt", "siz", "maxsiz");

     for (what = 0; what < MALLOC_WHAT_LAST; ++what) {
          struct mstat *stat = mstat + what;
          printf("%12s %8d %8d %10d %10d\n",
                 names[what], stat->cnt, stat->maxcnt,
                 stat->siz, stat->maxsiz);
     }

     if (minfo)
          printf("\nUnfreed allocations:\n");

     for (info = minfo; info; info = info->next) {
          printf("%s:%d:  %u bytes at %p\n",
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
     return p;
}

void X(free)(void *p)
{
     real_free(p);
}

#endif
