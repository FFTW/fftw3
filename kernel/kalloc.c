/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* $Id: kalloc.c,v 1.6 2006-01-25 23:00:04 stevenj Exp $ */

#include "ifftw.h"

/* ``kernel'' malloc(), with proper memory alignment */

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

void *X(kernel_malloc)(size_t n)
{
     void *p;

#if defined(MIN_ALIGNMENT)

#  if defined(WITH_OUR_MALLOC16) && (MIN_ALIGNMENT == 16)
     p = our_malloc16(n);
#    undef real_free
#    define real_free our_free16

#  elif defined(__FreeBSD__) && (MIN_ALIGNMENT <= 16)
     /* FreeBSD does not have memalign, but its malloc is 16-byte aligned. */
     p = malloc(n);

#  elif (defined(__MACOSX__) || defined(__APPLE__)) && (MIN_ALIGNMENT <= 16)
     /* MacOS X malloc is already 16-byte aligned */
     p = malloc(n);

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
#    error "Don't know how to malloc() aligned memory ... try configuring --with-our-malloc16"
#  endif

#else /* !defined(MIN_ALIGNMENT) */
     p = malloc(n);
#endif

     return p;
}

void X(kernel_free)(void *p)
{
     real_free(p);
}
