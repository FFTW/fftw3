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

/* $Id: align.c,v 1.20 2003-04-02 02:11:31 athena Exp $ */

#include "ifftw.h"

#if HAVE_3DNOW
#  define ALGN 8
#elif HAVE_SIMD
#  define ALGN 16
#elif HAVE_K7
#  define ALGN 8
#else
#  define ALGN (sizeof(R))
#endif

/* NONPORTABLE */
int X(alignment_of)(R *p)
{
     return (int)(((uintptr_t) p) % ALGN);
}

/* NONPORTABLE */
R *X(most_unaligned)(R *p1, R *p2)
{
     uintptr_t a1 = (uintptr_t)p1;
     uintptr_t a2 = (uintptr_t)p2;
     
     if (p1 == p2) return p1;

     for (;;) {
	  if (a1 & 1) return p1;
	  if (a2 & 1) return p2;
	  a1 >>= 1;
	  a2 >>= 1;
     }
}

void X(most_unaligned_complex)(R *r, R *i, R **rp, R **ip, int s)
{
     R *p;
     if (i == r + 1) {
	  /* forward complex format.  Choose the worst alignment
	     for r, adjust i consequently */
	  *rp = p = X(most_unaligned)(r, r + s);
	  *ip = i + (p - r);
     } else if (r == i + 1) {
	  /* backward complex format.  Choose the worst alignment
	     for i, adjust r consequently */
	  *ip = p = X(most_unaligned)(i, i + s);
	  *rp = r + (p - i);
     } else {
	  /* split format.  pick worst of r/i, adjust other accordingly. */
	  *rp = X(most_unaligned)(r, r + s);
	  *ip = X(most_unaligned)(i, i + s);
	  if (*rp == X(most_unaligned)(*rp, *ip))
               *ip = i + (*rp - r);
	  else
               *rp = r + (*ip - i);
     }
}


void X(with_aligned_stack)(void (*f)(void *), void *p)
{
#if defined(__GNUC__) && defined(__i386__)
     /*
      * horrible hack to align the stack to a 16-byte boundary.
      *
      * We assume a gcc version >= 2.95 so that
      * -mpreferred-stack-boundary works.  Otherwise, all bets are
      * off.  However, -mpreferred-stack-boundary does not create a
      * stack alignment, but it only preserves it.  Unfortunately,
      * many versions of libc on linux call main() with the wrong
      * initial stack alignment, with the result that the code is now
      * pessimally aligned instead of having a 50% chance of being
      * correct.
      */
     {
	  /*
	   * Use alloca to allocate some memory on the stack.
	   * This alerts gcc that something funny is going
	   * on, so that it does not omit the frame pointer
	   * etc.
	   */
	  (void)__builtin_alloca(16); 

	  /*
	   * Now align the stack pointer
	   */
	  __asm__ __volatile__ ("andl $-16, %esp");
     }
#endif

#ifdef __ICC /* Intel's compiler for ia32 */
     {
	  /*
	   * Simply calling alloca seems to do the right thing. 
	   * The size of the allocated block seems to be irrelevant.
	   */
	  _alloca(16);
     }
#endif

     f(p);
}
