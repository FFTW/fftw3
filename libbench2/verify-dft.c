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

/* $Id: verify-dft.c,v 1.4 2003-01-19 01:31:22 athena Exp $ */

#include "verify.h"

void verify_dft(dofft_closure *k, bench_tensor *sz, bench_tensor *vecsz, int sign,
		int rounds, double tol)
{
     C *inA, *inB, *inC, *outA, *outB, *outC, *tmp;
     int n, vecn, N;
     double el, ei, es = 0.0;

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

     ei = impulse(k, n, vecn, inA, inB, inC, outA, outB, outC, 
		  tmp, rounds, tol);
     el = linear(k, 0, N, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);

     es = dmax(es, tf_shift(k, 0, sz, n, vecn, sign, inA, inB, outA, outB, 
			    tmp, rounds, tol, TIME_SHIFT));
     es = dmax(es, tf_shift(k, 0, sz, n, vecn, sign, inA, inB, outA, outB, 
			    tmp, rounds, tol, FREQ_SHIFT));

     if (verbose)
	  ovtpvt("%g %g %g\n", el, ei, es);

     bench_free(tmp);
     bench_free(outC);
     bench_free(outB);
     bench_free(outA);
     bench_free(inC);
     bench_free(inB);
     bench_free(inA);
}

