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

/* out of place 2D copy routines */
#include "ifftw.h"

void X(cpy2d)(R *I, R *O,
	      int n0, int is0, int os0,
	      int n1, int is1, int os1, 
	      int vl)
{
     int i0, i1, v;

     switch (vl) {
	 case 1:
	      for (i1 = 0; i1 < n1; ++i1)
		   for (i0 = 0; i0 < n0; ++i0) {
			R x0 = I[i0 * is0 + i1 * is1];
			O[i0 * os0 + i1 * os1] = x0;
		   }
	      break;
	 case 2:
	      for (i1 = 0; i1 < n1; ++i1)
		   for (i0 = 0; i0 < n0; ++i0) {
			R x0 = I[i0 * is0 + i1 * is1];
			R x1 = I[i0 * is0 + i1 * is1 + 1];
			O[i0 * os0 + i1 * os1] = x0;
			O[i0 * os0 + i1 * os1 + 1] = x1;
		   }
	      break;
	 default:
	      for (i1 = 0; i1 < n1; ++i1)
		   for (i0 = 0; i0 < n0; ++i0)
			for (v = 0; v < vl; ++v) {
			     R x0 = I[i0 * is0 + i1 * is1 + v];
			     O[i0 * os0 + i1 * os1 + v] = x0;
			}
	      break;
     }
}

#define IABS(x) (((x) < 0) ? (-(x)) : (x))

/* like cpy2d, but read input contiguously if possible */
void X(cpy2d_ci)(R *I, R *O,
		 int n0, int is0, int os0,
		 int n1, int is1, int os1, 
		 int vl)
{
     if (IABS(is0) < IABS(is1))	/* inner loop is for n0 */
	  X(cpy2d) (I, O, n0, is0, os0, n1, is1, os1, vl);
     else
	  X(cpy2d) (I, O, n1, is1, os1, n0, is0, os0, vl);
}

/* like cpy2d, but write output contiguously if possible */
void X(cpy2d_co)(R *I, R *O,
		 int n0, int is0, int os0,
		 int n1, int is1, int os1, int vl)
{
     if (IABS(os0) < IABS(os1))	/* inner loop is for n0 */
	  X(cpy2d) (I, O, n0, is0, os0, n1, is1, os1, vl);
     else
	  X(cpy2d) (I, O, n1, is1, os1, n0, is0, os0, vl);
}

/*
  Recursive 2-dimensional copy routines, useful e.g. for
  transpositions.

  In an ideal world where caches don't suck, the ``cache-oblivious''
  cpy2d_rec() routine would be sufficient and optimal.

  In a real world of caches with limited associativity, the extra
  buffering of cpy2d_recbuf() is necessary when stride=2^k (i.e., in the
  common case.) See
  
     K.S. Gatlin and Larry Carter, ``Memory Hierarchy Considerations
     for Fast Transpose and Bit-Reversals'', HPCA99, January 1999.

*/

/*
  Ideally:  max CUTOFF s.t.
    CUTOFF^2 * sizeof(complex double) <= cache size
    CUTOFF * sizeof(float) >= line size 

  CUTOFF = 16 implies cache size >= 4096, which is reasonable.
  CUTOFF = 16 fails to exploit 128 bytes lines vl = 1, R = float;
  however vl = 2 or R = double is ok.
*/
#define CUTOFF 16

void X(cpy2d_rec)(R *I, R *O,
		  int n0, int is0, int os0,
		  int n1, int is1, int os1,
		  int vl)
{
   tail:
     if (n0 >= n1 && n0 > CUTOFF) {
	  int nm = n0 / 2;
	  X(cpy2d_rec) (I, O, nm, is0, os0, n1, is1, os1, vl);
	  I += nm * is0; O += nm * os0; n0 -= nm; goto tail;
     } else if ( /* n1 >= n0 && */ n1 > CUTOFF) {
	  int nm = n1 / 2;
	  X(cpy2d_rec) (I, O, n0, is0, os0, nm, is1, os1, vl);
	  I += nm * is1; O += nm * os1; n1 -= nm; goto tail;
     } else {
	  X(cpy2d) (I, O, n0, is0, os0, n1, is1, os1, vl);
     }
}

void X(cpy2d_recbuf)(R *I, R *O,
		     int n0, int is0, int os0,
		     int n1, int is1, int os1, int vl) 
{
     A(vl <= 2);

   tail:
     if (n0 >= n1 && n0 > CUTOFF) {
	  int nm = n0 / 2;
	  X(cpy2d_recbuf) (I, O, nm, is0, os0, n1, is1, os1, vl);
	  I += nm * is0; O += nm * os0; n0 -= nm; goto tail;
     } else if ( /* n1 >= n0 && */ n1 > CUTOFF) {
	  int nm = n1 / 2;
	  X(cpy2d_recbuf) (I, O, n0, is0, os0, nm, is1, os1, vl);
	  I += nm * is1; O += nm * os1; n1 -= nm; goto tail;
     } else {
	  R buf[CUTOFF * CUTOFF * 2];

	  /* copy from I to buf */
	  X(cpy2d_ci) (I, buf, n0, is0, vl, n1, is1, vl * CUTOFF, vl);

	  /* copy from buf to O */
	  X(cpy2d_co) (buf, O, n0, vl, os0, n1, vl * CUTOFF, os1, vl);
     }
}
