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

/* $Id: direct.c,v 1.9 2002-06-09 11:52:22 athena Exp $ */

/* direct DFT solver, if we have a codelet */

#include "dft.h"

#define OPTIMAL_SIZE 32		/* for estimator */

typedef struct {
     solver super;
     const kdft_desc *desc;
     kdft k;
} S;

typedef struct {
     plan_dft super;

     stride is, os;
     uint vl;
     int ivs, ovs;
     kdft k;
     const S *slv;
} P;

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     ego->k(ri, ii, ro, io, ego->is, ego->os, ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     fftw_stride_destroy(ego->is);
     fftw_stride_destroy(ego->os);
     fftw_free(ego);
}

static void print(plan *ego_, plan_printf prntf)
{
     P *ego = (P *) ego_;
     const S *s = ego->slv;
     const kdft_desc *d = s->desc;

     prntf("(dft-direct-%u", d->sz);
     if (d->is || d->os)
	  prntf("/is=%d/os=%d", d->is, d->os);
     if (ego->vl > 1)
	  prntf("-x%u", ego->vl);
     prntf(")");
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (DFTP(p_)) {
	  const S *ego = (const S *) ego_;
	  const problem_dft *p = (const problem_dft *) p_;
	  const kdft_desc *d = ego->desc;

	  return (
	       1
	       && p->sz.rnk == 1 && p->vecsz.rnk <= 1
	       && p->sz.dims[0].n == d->sz
	       && (!d->is || d->is == p->sz.dims[0].is)
	       && (!d->os || d->os == p->sz.dims[0].os)
	       && (0 

		   /* can always operate out-of-place */
		   || p->ri != p->ro 

		   /*
		    * can always compute one transform in-place, no
		    * matter what the strides are.
		    */
		   || p->vecsz.rnk == 0


		   /* can operate in-place as long as strides are the same */
		   || (fftw_tensor_inplace_strides(p->sz) &&
		       fftw_tensor_inplace_strides(p->vecsz))
		    )
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p)
{
     return (applicable(ego, p)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *planner)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_dft *p;
     iodim *d, *vd;
     const kdft_desc *e = ego->desc;

     static const plan_adt padt = { 
	  fftw_dft_solve, fftw_null_awake, print, destroy 
     };

     UNUSED(planner);

     if (!applicable(ego_, p_))
	  return (plan *)0;

     p = (const problem_dft *) p_;

     pln = MKPLAN_DFT(P, &padt, apply);

     d = p->sz.dims;
     vd = p->vecsz.dims;

     pln->k = ego->k;
     pln->is = fftw_mkstride(e->sz, d[0].is);
     pln->os = fftw_mkstride(e->sz, d[0].os);

     if (p->vecsz.rnk == 0) {
	  pln->vl = 1;
	  pln->ivs = pln->ovs = 0;
     } else {
	  pln->vl = vd[0].n;
	  pln->ivs = vd[0].is;
	  pln->ovs = vd[0].os;
     }

     pln->slv = ego;
     pln->super.super.cost = 1.0 + 0.1 * fftw_square(e->sz - OPTIMAL_SIZE);
     pln->super.super.flops = fftw_flops_mul(pln->vl, e->flops);

     return &(pln->super.super);
}

/* constructor */
solver *fftw_mksolver_dft_direct(kdft k, const kdft_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = k;
     slv->desc = desc;
     return &(slv->super);
}
