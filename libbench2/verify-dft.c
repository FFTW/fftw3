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

/* $Id: verify-dft.c,v 1.2 2003-01-18 20:41:18 athena Exp $ */

#include "verify.h"

void verify_dft(dofft_closure *k, tensor *sz, tensor *vecsz, int sign,
		int rounds, double tol)
{
     C *inA, *inB, *inC, *outA, *outB, *outC, *tmp;
     int n, vecn, N;

     if (rounds == 0)
	  rounds = 20;  /* default value */

     n = tensor_sz(sz);
     vecn = tensor_sz(vecsz);
     N = n * vecn;

     inA = (C *) bench_malloc(N * sizeof(C));
     inB = (C *) bench_malloc(N * sizeof(C));
     inC = (C *) bench_malloc(N * sizeof(C));
     outA = (C *) bench_malloc(N * sizeof(C));
     outB = (C *) bench_malloc(N * sizeof(C));
     outC = (C *) bench_malloc(N * sizeof(C));
     tmp = (C *) bench_malloc(N * sizeof(C));

     impulse(k, n, vecn, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     linear(k, 0, N, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);

     tf_shift(k, 0, sz, n, vecn, sign, inA, inB, outA, outB, tmp, 
	      rounds, tol, TIME_SHIFT);
     tf_shift(k, 0, sz, n, vecn, sign, inA, inB, outA, outB, tmp, 
	      rounds, tol, FREQ_SHIFT);

     bench_free(tmp);
     bench_free(outC);
     bench_free(outB);
     bench_free(outA);
     bench_free(inC);
     bench_free(inB);
     bench_free(inA);
}

