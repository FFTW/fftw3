/*
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
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

/* $Id: verify.c,v 1.3 2003-01-18 20:41:18 athena Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "bench.h"

/**************************************************************/
/* DFT code: */
/* copy A into B, using output stride of A and input stride of B */
typedef struct {
     dotens2_closure k;
     bench_complex *a, *b;
} dftcpy_closure;

static void dftcpy0(dotens2_closure *k_, 
		    int indxa, int ondxa, int indxb, int ondxb)
{
     dftcpy_closure *k = (dftcpy_closure *)k_;
     CASSIGN(k->b[indxb], k->a[ondxa]);
     UNUSED(indxa); UNUSED(ondxb);
}

static void dftcpy(bench_complex *a, const tensor *sza, 
		   bench_complex *b, const tensor *szb)
{
     dftcpy_closure k;
     k.k.apply = dftcpy0; 
     k.a = a; k.b = b;
     dotens2(sza, szb, &k.k);
}

typedef struct {
     dofft_closure k;
     problem *p;
} dofft_dft_closure;

static void dft_apply(dofft_closure *k_, bench_complex *in, bench_complex *out)
{
     dofft_dft_closure *k = (dofft_dft_closure *)k_;
     problem *p = k->p;
     bench_complex *pin = p->in, *pout = p->out;
     tensor *totalsz, *pckdsz;

     totalsz = tensor_append(p->vecsz, p->sz);
     pckdsz = verify_pack(totalsz, 1);

     dftcpy(in, pckdsz, pin, totalsz);
     doit(1, p);
     dftcpy(pout, totalsz, out, pckdsz);

     tensor_destroy(totalsz);
     tensor_destroy(pckdsz);
}

static void dodft(problem *p, int rounds, double tol)
{
     dofft_dft_closure k;
     k.k.apply = dft_apply;
     k.p = p;
     verify_dft((dofft_closure *)&k, p->sz, p->vecsz, p->sign, rounds, tol);
}

/* end DFT code */
/**************************************************************/

void verify(const char *param, int rounds, double tol)
{
     struct problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);

     switch (p->kind) {
	 case PROBLEM_COMPLEX: dodft(p, rounds, tol); break;
	 case PROBLEM_REAL:	  /* TODO */
	 default: BENCH_ASSERT(0); 
     }
     done(p);
     problem_destroy(p);
}

void accuracy(const char *param, int rounds)
{
     struct problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);
     BENCH_ASSERT(0); /* TODO */
     done(p);
     problem_destroy(p);
}
