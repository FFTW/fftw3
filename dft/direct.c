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

/* $Id: direct.c,v 1.38 2003-03-16 20:00:03 stevenj Exp $ */

/* direct DFT solver, if we have a codelet */

#include "dft.h"

typedef struct {
     solver super;
     const kdft_desc *desc;
     kdft k;
} S;

typedef struct {
     plan_dft super;

     stride is, os;
     int vl;
     int ivs, ovs;
     kdft k;
     const S *slv;
} P;

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     ego->k(ri, ii, ro, io, ego->is, ego->os, ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->is);
     X(stride_destroy)(ego->os);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *s = ego->slv;
     const kdft_desc *d = s->desc;

     p->print(p, "(dft-direct-%d%v \"%s\")", d->sz, ego->vl, d->nam);
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     if (DFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_dft *p = (const problem_dft *) p_;
          const kdft_desc *d = ego->desc;
	  int vl;
	  int ivs, ovs;

          return (
	       1
	       && p->sz->rnk == 1
	       && p->vecsz->rnk <= 1
	       && p->sz->dims[0].n == d->sz

	       /* check strides etc */
	       && X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs)

	       && (d->genus->okp(d, p->ri, p->ii, p->ro, p->io,
				 p->sz->dims[0].is, p->sz->dims[0].os,
				 vl, ivs, ovs, plnr))

	       && (0
		   /* can operate out-of-place */
		   || p->ri != p->ro

		   /*
		    * can compute one transform in-place, no matter
		    * what the strides are.
		    */
		   || p->vecsz->rnk == 0

		   /* can operate in-place as long as strides are the same */
		   || (X(tensor_inplace_strides2)(p->sz, p->vecsz))
		    )
	       );
     }

     return 0;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_dft *p;
     iodim *d;
     const kdft_desc *e = ego->desc;

     static const plan_adt padt = {
	  X(dft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_, plnr))
          return (plan *)0;

     p = (const problem_dft *) p_;

     pln = MKPLAN_DFT(P, &padt, apply);

     d = p->sz->dims;

     pln->k = ego->k;
     pln->is = X(mkstride)(e->sz, d[0].is);
     pln->os = X(mkstride)(e->sz, d[0].os);

     X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     pln->slv = ego;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(pln->vl / e->genus->vl, &e->ops, &pln->super.super.ops);

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_dft_direct)(kdft k, const kdft_desc *desc)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = k;
     slv->desc = desc;
     return &(slv->super);
}
