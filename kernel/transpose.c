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

/* in place square transposition, iterative */
#include "ifftw.h"

void X(transpose)(R *I, int n, int s0, int s1, int vl)
{
     int i0, i1, v;

     switch (vl) {
	 case 1:
	      for (i1 = 1; i1 < n; ++i1) {
		   for (i0 = 0; i0 < i1; ++i0) {
			R *p0 = I + i1 * s0 + i0 * s1;
			R *p1 = I + i0 * s0 + i1 * s1;
			R t0 = p0[0];
			R r0 = p1[0];
			p0[0] = r0;
			p1[0] = t0;
		   }
	      }
	      break;
	 case 2:
	      for (i1 = 1; i1 < n; ++i1) {
		   for (i0 = 0; i0 < i1; ++i0) {
			R *p0 = I + i1 * s0 + i0 * s1;
			R *p1 = I + i0 * s0 + i1 * s1;
			R t0 = p0[0];
			R t1 = p0[1];
			R r0 = p1[0];
			R r1 = p1[1];
			p0[0] = r0;
			p0[1] = r1;
			p1[0] = t0;
			p1[1] = t1;
		   }
	      }
	      break;
	 default:
	      for (i1 = 1; i1 < n; ++i1) {
		   for (i0 = 0; i0 < i1; ++i0) {
			R *p0 = I + i1 * s0 + i0 * s1;
			R *p1 = I + i0 * s0 + i1 * s1;
			for (v = vl; v > 0; --v, ++p0, ++p1) {
			     R t0 = p0[0];
			     R r0 = p1[0];
			     p0[0] = r0;
			     p1[0] = t0;
			}
		   }
	      }
	      break;
     }
}


static void dotile(R *X, R *Y, int i, int j, int s0, int s1,
		   int vl, R *buf0, R *buf1)
{
     X(cpy2d_ci)(X, buf0, i, s0, vl, j, s1, vl * i, vl);
     X(cpy2d_ci)(Y, buf1, i, s1, vl, j, s0, vl * i, vl);
     X(cpy2d_co)(buf1, X, i, vl, s0, j, vl * i, s1, vl);
     X(cpy2d_co)(buf0, Y, i, vl, s1, j, vl * i, s0, vl);
}
		   

static void transpose_and_swap(R *I, int i0, int i1, int j0, int j1,
			       int s0, int s1, int vl)
{
     R buf0[CACHESIZE/2];
     R buf1[CACHESIZE/2];
     int tilesz = X(isqrt)((CACHESIZE / 2 / sizeof(R)) / vl);
     int i, j;

     for (i = i0; i < i1 - tilesz; i += tilesz) {
	  for (j = j0; j < j1 - tilesz; j += tilesz) {
	       dotile(I + i * s0 + j * s1, 
		      I + i * s1 + j * s0,
		      tilesz, tilesz, s0, s1,
		      vl, buf0, buf1);
	  }
	  dotile(I + i * s0 + j * s1, 
		 I + i * s1 + j * s0,
		 tilesz, j1 - j, s0, s1,
		 vl, buf0, buf1);
     }
     for (j = j0; j < j1 - tilesz; j += tilesz) {
	  dotile(I + i * s0 + j * s1, 
		 I + i * s1 + j * s0,
		 i1 - i, tilesz, s0, s1,
		 vl, buf0, buf1);
     }
     dotile(I + i * s0 + j * s1, 
	    I + i * s1 + j * s0,
	    i1 - i, j1 - j, s0, s1,
	    vl, buf0, buf1);
}

void X(transpose_tiled)(R *I, int n, int s0, int s1, int vl) 
{
   tail:
     if (n > 1) {
	  int n2 = n / 2;
	  transpose_and_swap(I, 0, n2, n2, n, s0, s1, vl);
	  X(transpose_tiled)(I, n2, s0, s1, vl);
	  I += n2 * (s0 + s1); n -= n2; goto tail;
     }
}
