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

/* $Id: hc2hc-dit.c,v 1.7 2002-09-12 20:10:05 athena Exp $ */

/* decimation in time Cooley-Tukey */
#include "rdft.h"
#include "hc2hc.h"

static void apply(plan *ego_, R *I, R *O)
{
     plan_hc2hc *ego = (plan_hc2hc *) ego_;

     /* two-dimensional r x vl sub-transform: */
     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, I, O);
     }

     {
          plan_rdft *cld0 = (plan_rdft *) ego->cld0;
          plan_rdft *cldm = (plan_rdft *) ego->cldm;
          uint i, r = ego->r, m = ego->m, vl = ego->vl;
          int os = ego->os, ovs = ego->ovs;

          for (i = 0; i < vl; ++i, O += ovs) {
	       cld0->apply((plan *) cld0, O, O);
               ego->k(O + os, O + (r * m - 1) * os, ego->W, ego->ios, m, os);
	       cldm->apply((plan *) cldm, O + os*(m/2), O + os*(m/2));
	  }
     }
}

static int applicable(const solver_hc2hc *ego, const problem *p_,
		      const planner *plnr)
{
     UNUSED(plnr);
     if (X(rdft_hc2hc_applicable)(ego, p_)) {
	  int ivs, ovs;
	  uint vl;
          const hc2hc_desc *e = ego->desc;
          const problem_rdft *p = (const problem_rdft *) p_;
          iodim *d = p->sz.dims;
	  uint m = d[0].n / e->radix;
	  X(tensor_tornk1)(&p->vecsz, &vl, &ivs, &ovs);
          return (1
		  && (e->genus->okp(e, p->O + d[0].os,
				    p->O + (e->radix * m - 1) * d[0].os, 
				    (int)m * d[0].os, 0, m, d[0].os))
		  && (e->genus->okp(e, p->O + ovs + d[0].os,
				    p->O + ovs + (e->radix * m - 1) * d[0].os, 
				    (int)m * d[0].os, 0, m, d[0].os))
	       );
     }
     return 0;
}

static void finish(plan_hc2hc *ego)
{
     const hc2hc_desc *d = ego->slv->desc;
     ego->ios = X(mkstride)(ego->r, ego->m * ego->os);
     ego->super.super.ops =
          X(ops_add)(X(ops_add)(ego->cld->ops,
				X(ops_mul)(ego->vl,
					   X(ops_add)(ego->cld0->ops,
						      ego->cldm->ops))),
		     X(ops_mul)(ego->vl * ((ego->m - 1)/2) / d->genus->vl,
				d->ops));
}

static int score(const solver *ego_, const problem *p_, const planner *plnr)
{
     const solver_hc2hc *ego = (const solver_hc2hc *) ego_;
     const problem_rdft *p;
     uint n;

     if (!applicable(ego, p_, plnr))
          return BAD;

     p = (const problem_rdft *) p_;

     /* emulate fftw2 behavior */
     if ((p->vecsz.rnk > 0) && NO_VRECURSE(plnr))
	  return BAD;

     n = p->sz.dims[0].n;
     if (0
	 || n <= 16 
	 || n / ego->desc->radix <= 4
	  )
          return UGLY;

     if ((plnr->planner_flags & IMPATIENT) && plnr->nthr > 1)
	  return UGLY; /* prefer threaded version */

     return GOOD;
}


static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     static const hc2hcadt adt = {
	  sizeof(plan_hc2hc), 
	  X(rdft_mkcldrn_dit), finish, applicable, apply
     };
     return X(mkplan_rdft_hc2hc)((const solver_hc2hc *) ego, p, plnr, &adt);
}


solver *X(mksolver_rdft_hc2hc_dit)(khc2hc codelet, const hc2hc_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     static const char name[] = "rdft-dit";

     return X(mksolver_rdft_hc2hc)(codelet, desc, name, &sadt);
}
