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

/* $Id: align.c,v 1.21 2003-04-02 10:25:56 athena Exp $ */

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
