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

/* out of place 2D copy routines */
#include "ifftw.h"

void X(cpy2d)(R *I, R *O,
	      INT n0, INT is0, INT os0,
	      INT n1, INT is1, INT os1, 
	      INT vl)
{
     INT i0, i1, v;

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

/* like cpy2d, but read input contiguously if possible */
void X(cpy2d_ci)(R *I, R *O,
		 INT n0, INT is0, INT os0,
		 INT n1, INT is1, INT os1, 
		 INT vl)
{
     if (IABS(is0) < IABS(is1))	/* inner loop is for n0 */
	  X(cpy2d) (I, O, n0, is0, os0, n1, is1, os1, vl);
     else
	  X(cpy2d) (I, O, n1, is1, os1, n0, is0, os0, vl);
}

/* like cpy2d, but write output contiguously if possible */
void X(cpy2d_co)(R *I, R *O,
		 INT n0, INT is0, INT os0,
		 INT n1, INT is1, INT os1, 
		 INT vl)
{
     if (IABS(os0) < IABS(os1))	/* inner loop is for n0 */
	  X(cpy2d) (I, O, n0, is0, os0, n1, is1, os1, vl);
     else
	  X(cpy2d) (I, O, n1, is1, os1, n0, is0, os0, vl);
}


/* tiled copy routines */
struct cpy2d_closure {
     R *I, *O;
     INT is0, os0, is1, os1, vl;
     R *buf; 
};

static void dotile(INT n0l, INT n0u, INT n1l, INT n1u, void *args)
{
     struct cpy2d_closure *k = (struct cpy2d_closure *)args;
     X(cpy2d)(k->I + n0l * k->is0 + n1l * k->is1,
	      k->O + n0l * k->os0 + n1l * k->os1,
	      n0u - n0l, k->is0, k->os0,
	      n1u - n1l, k->is1, k->os1,
	      k->vl);
}

static void dotile_buf(INT n0l, INT n0u, INT n1l, INT n1u, void *args)
{
     struct cpy2d_closure *k = (struct cpy2d_closure *)args;

     /* copy from I to buf */
     X(cpy2d_ci)(k->I + n0l * k->is0 + n1l * k->is1,
		 k->buf,
		 n0u - n0l, k->is0, k->vl,
		 n1u - n1l, k->is1, k->vl * (n0u - n0l),
		 k->vl);

     /* copy from buf to O */
     X(cpy2d_co)(k->buf,
		 k->O + n0l * k->os0 + n1l * k->os1,		 
		 n0u - n0l, k->vl, k->os0,
		 n1u - n1l, k->vl * (n0u - n0l), k->os1,
		 k->vl);
}


void X(cpy2d_tiled)(R *I, R *O,
		    INT n0, INT is0, INT os0,
		    INT n1, INT is1, INT os1, INT vl) 
{
     INT tilesz = X(compute_tilesz)(vl, 
				    1 /* input array */
				    + 1 /* ouput array */);
     struct cpy2d_closure k;
     k.I = I;
     k.O = O;
     k.is0 = is0;
     k.os0 = os0;
     k.is1 = is1;
     k.os1 = os1;
     k.vl = vl;
     k.buf = 0; /* unused */
     X(tile2d)(0, n0, 0, n1, tilesz, dotile, &k);
}

void X(cpy2d_tiledbuf)(R *I, R *O,
		       INT n0, INT is0, INT os0,
		       INT n1, INT is1, INT os1, INT vl) 
{
     R buf[CACHESIZE / (2 * sizeof(R))];
     /* input and buffer in cache, or
	output and buffer in cache */
     INT tilesz = X(compute_tilesz)(vl, 2);
     struct cpy2d_closure k;
     k.I = I;
     k.O = O;
     k.is0 = is0;
     k.os0 = os0;
     k.is1 = is1;
     k.os1 = os1;
     k.vl = vl;
     k.buf = buf;
     A(tilesz * tilesz * vl * sizeof(R) <= sizeof(buf));
     X(tile2d)(0, n0, 0, n1, tilesz, dotile_buf, &k);
}

