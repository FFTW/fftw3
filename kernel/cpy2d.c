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


static void dotile(R *I, R *O,
		   int n0, int is0, int os0,
		   int n1, int is1, int os1, 
		   int vl, R *buf) 
{
     /* copy from I to buf */
     X(cpy2d_ci) (I, buf, n0, is0, vl, n1, is1, vl * n0, vl);
     
     /* copy from buf to O */
     X(cpy2d_co) (buf, O, n0, vl, os0, n1, vl * n0, os1, vl);
}

void X(cpy2d_tiled)(R *I, R *O,
		    int n0, int is0, int os0,
		    int n1, int is1, int os1, int vl) 
{
     R buf[CACHESIZE];
     int tilesz = X(isqrt)((CACHESIZE / sizeof(R)) / vl);
     int i0, i1;

     for (i1 = 0; i1 < n1 - tilesz; i1 += tilesz) {
	  for (i0 = 0; i0 < n0 - tilesz; i0 += tilesz) 
	       dotile(I + i0 * is0 + i1 * is1,
		      O + i0 * os0 + i1 * os1,
		      tilesz, is0, os0,
		      tilesz, is1, os1,
		      vl, buf);

	  dotile(I + i0 * is0 + i1 * is1,
		 O + i0 * os0 + i1 * os1,
		 n0 - i0, is0, os0,
		 tilesz, is1, os1,
		 vl, buf);
     }

     for (i0 = 0; i0 < n0 - tilesz; i0 += tilesz) 
	  dotile(I + i0 * is0 + i1 * is1,
		 O + i0 * os0 + i1 * os1,
		 tilesz, is0, os0,
		 n1 - i1, is1, os1,
		 vl, buf);

     dotile(I + i0 * is0 + i1 * is1,
	    O + i0 * os0 + i1 * os1,
	    n0 - i0, is0, os0,
	    n1 - i1, is1, os1,
	    vl, buf);
}
