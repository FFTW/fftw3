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

/* in place square transposition, recursive, with and without buffer */
#include "ifftw.h"

#define CUTOFF 8

static void transpose_and_swap(R *I, int i0, int i1, int j0, int j1,
			       int s0, int s1, int vl)
{
     int di, dj;

   tail:
     di = i1 - i0;
     dj = j1 - j0;

     if (di >= dj && di > CUTOFF) {
	  int im = (i0 + i1) / 2;
	  transpose_and_swap(I, i0, im, j0, j1, s0, s1, vl);
	  i0 = im; goto tail;
     } else if (dj >= di && dj > CUTOFF) {
	  int jm = (j0 + j1) / 2;
	  transpose_and_swap(I, i0, i1, j0, jm, s0, s1, vl);
	  j0 = jm; goto tail;
     } else {
	  int i, j, v;
	  switch (vl) {
	      case 1:
		   for (i = i0; i < i1; ++i)
			for (j = j0; j < j1; ++j) {
			     R a = I[i * s0 + j * s1];
			     R b = I[j * s0 + i * s1];
			     I[j * s0 + i * s1] = a;
			     I[i * s0 + j * s1] = b;
			}
		   break;
	      case 2:
		   for (i = i0; i < i1; ++i)
			for (j = j0; j < j1; ++j) {
			     R a0 = I[i * s0 + j * s1];
			     R a1 = I[i * s0 + j * s1 + 1];
			     R b0 = I[j * s0 + i * s1];
			     R b1 = I[j * s0 + i * s1 + 1];
			     I[j * s0 + i * s1] = a0;
			     I[j * s0 + i * s1 + 1] = a1;
			     I[i * s0 + j * s1] = b0;
			     I[i * s0 + j * s1 + 1] = b1;
			}
		   break;
	      default:
		   for (i = i0; i < i1; ++i)
			for (j = j0; j < j1; ++j)
			     for (v = 0; v < vl; ++v) {
				  R a = I[i * s0 + j * s1 + v];
				  R b = I[j * s0 + i * s1 + v];
				  I[j * s0 + i * s1 + v] = a;
				  I[i * s0 + j * s1 + v] = b;
			     }
		   break;
	  }
     }
}

void X(transpose_rec)(R *I, int n, int s0, int s1, int vl) 
{
   tail:
     if (n > 1) {
	  int n2 = n / 2;
	  transpose_and_swap(I, 0, n2, n2, n, s0, s1, vl);
	  X(transpose_rec)(I, n2, s0, s1, vl);
	  I += n2 * (s0 + s1); n -= n2; goto tail;
     }
}

static void transpose_and_swapbuf(R *I, int i0, int i1, int j0, int j1,
				  int s0, int s1, int vl)
{
     int di, dj;

   tail:
     di = i1 - i0;
     dj = j1 - j0;

     if (di >= dj && di > CUTOFF) {
	  int im = (i0 + i1) / 2;
	  transpose_and_swapbuf(I, i0, im, j0, j1, s0, s1, vl);
	  i0 = im; goto tail;
     } else if (dj >= di && dj > CUTOFF) {
	  int jm = (j0 + j1) / 2;
	  transpose_and_swapbuf(I, i0, i1, j0, jm, s0, s1, vl);
	  j0 = jm; goto tail;
     } else {
	  R buf0[CUTOFF * CUTOFF * 2];
	  R buf1[CUTOFF * CUTOFF * 2];

	  /* copy I[(i0..i1)*s0+(j0..j1)*s1] into buf0 */
	  X(cpy2d_ci) (I + i0 * s0 + j0 * s1, buf0,
		       i1 - i0, s0, 2, j1 - j0, s1, 2 * CUTOFF, vl);

	  /* copy I[(i0..i1)*s1+(j0..j1)*s0] into buf1 */
	  X(cpy2d_ci) (I + i0 * s1 + j0 * s0, buf1,
		       i1 - i0, s1, 2, j1 - j0, s0, 2 * CUTOFF, vl);

	  /* copy buf1 into I[(i0..i1)*s0+(j0..j1)*s1] */
	  X(cpy2d_co) (buf1, I + i0 * s0 + j0 * s1,
		       i1 - i0, 2, s0, j1 - j0, 2 * CUTOFF, s1, vl);

	  /* copy buf0 into I[(i0..i1)*s1+(j0..j1)*s0] */
	  X(cpy2d_co) (buf0, I + i0 * s1 + j0 * s0,
		       i1 - i0, 2, s1, j1 - j0, 2 * CUTOFF, s0, vl);

     }
}

void X(transpose_recbuf)(R *I, int n, int s0, int s1, int vl) 
{
     A(vl <= 2);
   tail:
     if (n > 1) {
	  int n2 = n / 2;
	  transpose_and_swapbuf(I, 0, n2, n2, n, s0, s1, vl);
	  X(transpose_recbuf)(I, n2, s0, s1, vl);
	  I += n2 * (s0 + s1); n -= n2; goto tail;
     }
}
