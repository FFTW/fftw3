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

/* $Id: directwsq.c,v 1.1 2003-05-15 23:09:07 athena Exp $ */

/* direct DFTW transposed solver */

#include "dft.h"

typedef struct {
     solver super;
     const ct_desc *desc;
     kdftwsq k;
     int dec;
} S;

typedef struct {
     plan_dft super;
     kdftwsq k;
     twid *td;
     stride ios, vs;
     int r, m, dist;
     const S *slv;
} P;

static void apply(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     ego->k(rio, iio, ego->td->W, ego->ios, ego->vs, ego->m, ego->dist);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     X(twiddle_awake)(flg, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, ego->m);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->ios);
     X(stride_destroy)(ego->vs);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const ct_desc *e = slv->desc;

     p->print(p, "(dft-directwsq-%d/%d \"%s\")",
              ego->r, X(twiddle_length)(ego->r, e->tw), e->nam);
}

static int applicable(const solver *ego_, const problem *p_,
		       const planner *plnr)
{
     if (DFTWP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_dftw *p = (const problem_dftw *) p_;
          const ct_desc *e = ego->desc;

          return (
	       1

	       && p->dec == ego->dec
	       && p->r == e->radix

	       /* transposed only */
	       && p->vl == p->r
	       && p->ws == p->vs
	       && p->wvs == p->m * p->s

	       /* SIMD strides etc. */
	       && (e->genus->okp(e, p->rio, p->iio, 
				 p->wvs /* == p->m * p->s */, p->ws, 
				 p->m, p->s /* dist */, plnr))
	       );
     }

     return 0;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_dftw *p;
     const ct_desc *e = ego->desc;

     static const plan_adt padt = {
	  X(dftw_solve), awake, print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_, plnr))
          return (plan *)0;

     p = (const problem_dftw *) p_;

     pln = MKPLAN_DFTW(P, &padt, apply);

     pln->k = ego->k;
     pln->ios = X(mkstride)(e->radix, p->wvs /* == p->m * p->s */);
     pln->vs = X(mkstride)(e->radix, p->vs);
     pln->td = 0;
     pln->r = p->r;
     pln->m = p->m;
     pln->dist = p->s;
     pln->slv = ego;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(pln->m / e->genus->vl, &e->ops, &pln->super.super.ops);

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_dft_directwsq)(kdftwsq codelet,
				  const ct_desc *desc, int dec)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = codelet;
     slv->desc = desc;
     slv->dec = dec;
     return &(slv->super);
}
