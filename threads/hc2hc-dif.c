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

/* $Id: hc2hc-dif.c,v 1.8 2003-01-15 02:10:25 athena Exp $ */

/* decimation in frequency Cooley-Tukey, with codelet divided among threads */
#include "threads.h"
#include "hc2hc.h"

typedef struct {
     plan_hc2hc super;
     int nthr;
     int mloop;
     int sW;
} P;

typedef struct {
     R *ri, *ii;
     khc2hc k;
     R *W;
     int sW;
     stride ios;
     int is;
} PD;

static void *spawn_apply(spawn_data *d)
{
     PD *ego = (PD *) d->data;
     int min = d->min, max = d->max;
     int is = ego->is;

     ego->k(ego->ri + min * is, ego->ii - min * is,
	    ego->W + min * ego->sW, ego->ios, 2 * (max - min) + 1, is);
     return 0;
}

static void apply(plan *ego_, R *I, R *O)
{
     plan_hc2hc *ego = (plan_hc2hc *) ego_;

     {
	  plan_rdft *cld0 = (plan_rdft *) ego->cld0;
	  cld0->apply((plan *) cld0, I, I);
     }

     {
	  plan_rdft *cldm = (plan_rdft *) ego->cldm;
          int r = ego->r, m = ego->m;
          int is = ego->is;
	  P *ego_thr = (P *) ego_;
	  PD d;
	  
	  cldm->apply((plan *) cldm, I + is*(m/2), I + is*(m/2));

	  d.ri = I + is; d.ii = I + (r * m - 1) * is;
	  d.k = ego->k;
	  d.W = ego->W;
	  d.sW = ego_thr->sW;
	  d.ios = ego->ios;
	  d.is = ego->is;

	  X(spawn_loop)(ego_thr->mloop, ego_thr->nthr, spawn_apply, (void*)&d);
     }

     /* two-dimensional r x vl sub-transform: */
     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, I, O);
     }

}

static int applicable0(const solver_hc2hc *ego, const problem *p_,
		       const planner *plnr)
{
     if (plnr->nthr > 1 && X(rdft_hc2hc_applicable)(ego, p_)) {
          const hc2hc_desc *e = ego->desc;
          const problem_rdft *p = (const problem_rdft *) p_;
          iodim *d = p->sz->dims;
	  int m = d[0].n / e->radix;
          return (1
		  && p->vecsz->rnk == 0
		  && (p->I == p->O || DESTROY_INPUTP(plnr))
		  && (e->genus->okp(e, p->I + d[0].is,
				    p->I + (e->radix * m - 1) * d[0].is, 
				    (int)m * d[0].is, 0, m, d[0].is))
	       );
     }
     return 0;
}

static int applicable(const solver_hc2hc *ego, const problem *p_,
		      const planner *plnr)
{
     const problem_rdft *p;

     if (!applicable0(ego, p_, plnr)) return 0;

     p = (const problem_rdft *) p_;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(16, p->sz->dims[0].n, ego->desc->radix))
	  return 0;

     return 1;
}

static void finish(plan_hc2hc *ego)
{
     const hc2hc_desc *d = ego->slv->desc;
     opcnt t;

     ego->ios = X(mkstride)(ego->r, ego->m * ego->is);

     X(ops_add)(&ego->cld0->ops, &ego->cldm->ops, &t);
     X(ops_madd)(ego->vl, &t, &ego->cld->ops, &ego->super.super.ops);
     X(ops_madd2)(ego->vl * ((ego->m - 1)/2) / d->genus->vl, &d->ops,
		  &ego->super.super.ops);
}

static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     plan *pln;
     static const hc2hcadt adt = {
	  sizeof(P), 
	  X(rdft_mkcldrn_dif), finish, applicable, apply
     };
     pln = X(mkplan_rdft_hc2hc)((const solver_hc2hc *) ego, p, plnr, &adt);
     if (pln) {
	  P *pln_thr = (P *) pln;
	  pln_thr->nthr = plnr->nthr;
	  pln_thr->mloop = 
	       ((pln_thr->super.m-1)/2) / pln_thr->super.slv->desc->genus->vl;
	  pln_thr->sW = X(twiddle_length)(pln_thr->super.r, 
					  pln_thr->super.slv->desc->tw);
     }
     return pln;
}

solver *X(mksolver_rdft_hc2hc_dif_thr)(khc2hc codelet, const hc2hc_desc *desc)
{
     static const solver_adt sadt = { mkplan };
     static const char name[] = "rdft-dif-thr";

     return X(mksolver_rdft_hc2hc)(codelet, desc, name, &sadt);
}
