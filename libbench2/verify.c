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

/* $Id: verify.c,v 1.8 2003-01-26 21:29:18 athena Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "verify.h"

/**************************************************************/
/* DFT code: */
/* copy A into B, using output stride of A and input stride of B */
typedef struct {
     dotens2_closure k;
     R *ra; R *ia;
     R *rb; R *ib;
     int scalea, scaleb;
} dftcpy_closure;

static void dftcpy0(dotens2_closure *k_, 
		    int indxa, int ondxa, int indxb, int ondxb)
{
     dftcpy_closure *k = (dftcpy_closure *)k_;
     k->rb[indxb * k->scaleb] = k->ra[ondxa * k->scalea];
     k->ib[indxb * k->scaleb] = k->ia[ondxa * k->scalea];
     UNUSED(indxa); UNUSED(ondxb);
}

static void dftcpy(R *ra, R *ia, const bench_tensor *sza, int scalea,
		   R *rb, R *ib, const bench_tensor *szb, int scaleb)
{
     dftcpy_closure k;
     k.k.apply = dftcpy0;
     k.ra = ra; k.ia = ia; k.rb = rb; k.ib = ib;
     k.scalea = scalea; k.scaleb = scaleb;
     bench_dotens2(sza, szb, &k.k);
}

typedef struct {
     dofft_closure k;
     bench_problem *p;
} dofft_dft_closure;

static void dft_apply(dofft_closure *k_, bench_complex *in, bench_complex *out)
{
     dofft_dft_closure *k = (dofft_dft_closure *)k_;
     bench_problem *p = k->p;
     bench_complex *pin = p->in, *pout = p->out;
     bench_tensor *totalsz, *pckdsz;
     bench_real *ri, *ii, *ro, *io;
     int n, totalscale;

     totalsz = tensor_append(p->vecsz, p->sz);
     pckdsz = verify_pack(totalsz, 2);
     n = tensor_sz(totalsz);
     ri = &c_re(pin[0]);
     ro = &c_re(pout[0]);

     /* confusion: the stride is the distance between complex elements
	when using interleaved format, but it is the distance between
	real elements when using split format */
     if (p->split) {
	  ii = ri + n;
	  io = ro + n;
	  totalscale = 1;
     } else {
	  ii = ri + 1;
	  io = ro + 1;
	  totalscale = 2;
     }

     dftcpy(&c_re(in[0]), &c_im(in[0]), pckdsz, 1,
	    ri, ii, totalsz, totalscale);
     doit(1, p);
     dftcpy(ro, io, totalsz, totalscale,
	    &c_re(out[0]), &c_im(out[0]), pckdsz, 1);

     tensor_destroy(totalsz);
     tensor_destroy(pckdsz);
}

static void dodft(bench_problem *p, int rounds, double tol, errors *e)
{
     dofft_dft_closure k;
     k.k.apply = dft_apply;
     k.p = p;
     verify_dft((dofft_closure *)&k, p->sz, p->vecsz, p->sign, rounds, tol, e);
}

/* end DFT code */
/**************************************************************/

void verify(const char *param, int rounds, double tol)
{
     bench_problem *p;
     errors e;

     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);

     switch (p->kind) {
	 case PROBLEM_COMPLEX: dodft(p, rounds, tol, &e); break;
	 case PROBLEM_REAL:	  /* TODO */
	 default: BENCH_ASSERT(0); 
     }

     if (verbose)
	  ovtpvt("%g %g %g\n", e.l, e.i, e.s);

     done(p);
     problem_destroy(p);
}

void accuracy(const char *param, int rounds)
{
     bench_problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);
     BENCH_ASSERT(0); /* TODO */
     done(p);
     problem_destroy(p);
}
