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

#include "bench.h"

typedef bench_real R;
typedef bench_complex C;

void arand(C *a, int n);
void mkreal(C *A, int n);
void mkhermitian(C *A, int rank, const bench_iodim *dim);
void aadd(C *c, C *a, C *b, int n);
void asub(C *c, C *a, C *b, int n);
void arol(C *b, C *a, int n, int nb, int na);
void aphase_shift(C *b, C *a, int n, int nb, int na, double sign);
void ascale(C *a, C alpha, int n);
double acmp(C *a, C *b, int n, const char *test, double tol);
double mydrand(void);
void impulse(dofft_closure *k,
	     int n, int vecn, 
	     C *inA, C *inB, C *inC,
	     C *outA, C *outB, C *outC,
	     C *tmp, int rounds, double tol);
void linear(dofft_closure *k, int realp,
	    int n, C *inA, C *inB, C *inC, C *outA,
	    C *outB, C *outC, C *tmp, int rounds, double tol);

enum { TIME_SHIFT, FREQ_SHIFT };
void tf_shift(dofft_closure *k, int realp, const bench_tensor *sz,
	      int n, int vecn, double sign,
	      C *inA, C *inB, C *outA, C *outB, C *tmp,
	      int rounds, double tol, int which_shift);

