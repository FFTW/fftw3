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

/* $Id: ct-dit.c,v 1.28 2002-09-21 11:58:11 athena Exp $ */

/* decimation in time Cooley-Tukey */
#include "dft.h"
#include "ct.h"

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     plan_ct *ego = (plan_ct *) ego_;
     plan *cld0 = ego->cld;
     plan_dft *cld = (plan_dft *) cld0;

     /* two-dimensional r x vl sub-transform: */
     cld->apply(cld0, ri, ii, ro, io);

     {
          uint i, m = ego->m, vl = ego->vl;
          int os = ego->os, ovs = ego->ovs;

          for (i = 0; i < vl; ++i)
               ego->k.dit(ro + i * ovs, io + i * ovs, ego->td->W,
			  ego->ios, m, os);
     }
}

static int applicable0(const solver_ct *ego, const problem *p_,
		       const planner *plnr)
{
     UNUSED(plnr);
     if (X(dft_ct_applicable)(ego, p_)) {
	  int ivs, ovs;
	  uint vl;
          const ct_desc *e = ego->desc;
          const problem_dft *p = (const problem_dft *) p_;
          iodim *d = p->sz.dims;
	  uint m = d[0].n / e->radix;
	  X(tensor_tornk1)(&p->vecsz, &vl, &ivs, &ovs);
          return (1
		  && (e->genus->okp(e, p->ro, p->io, 
				    (int)m * d[0].os, 0, m, d[0].os, plnr))
		  && (e->genus->okp(e, p->ro + ovs, p->io + ovs, 
				    (int)m * d[0].os, 0, m, d[0].os, plnr))
	       );
     }
     return 0;
}

static int applicable(const solver_ct *ego, const problem *p_,
		      const planner *plnr)
{
     const problem_dft *p;

     if (!applicable0(ego, p_, plnr))
          return 0;

     p = (const problem_dft *) p_;

     /* emulate fftw2 behavior */
     if (NO_VRECURSEP(plnr) && (p->vecsz.rnk > 0))  return 0;

     if (NO_UGLYP(plnr)) {
	  if (X(ct_uglyp)(16, p->sz.dims[0].n, ego->desc->radix)) return 0;
	  if (NONTHREADED_ICKYP(plnr))
	       return 0; /* prefer threaded version */
     }

     return 1;
}


static void finish(plan_ct *ego)
{
     const ct_desc *d = ego->slv->desc;
     ego->ios = X(mkstride)(ego->r, ego->m * ego->os);
     ego->super.super.ops =
          X(ops_add)(ego->cld->ops,
		     X(ops_mul)(ego->vl * ego->m / d->genus->vl, d->ops));
}

static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     static const ctadt adt = {
	  sizeof(plan_ct), X(dft_mkcld_dit), finish, applicable, apply
     };
     return X(mkplan_dft_ct)((const solver_ct *) ego, p, plnr, &adt);
}


solver *X(mksolver_dft_ct_dit)(kdft_dit codelet, const ct_desc *desc)
{
     static const solver_adt sadt = { mkplan };
     static const char name[] = "dft-dit";
     union kct k;
     k.dit = codelet;

     return X(mksolver_dft_ct)(k, desc, name, &sadt);
}
