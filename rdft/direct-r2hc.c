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

/* $Id: direct-r2hc.c,v 1.3 2002-07-22 05:02:38 stevenj Exp $ */

/* direct RDFT R2HC solver, if we have a codelet */

#include "rdft.h"

typedef struct {
     solver super;
     const kr2hc_desc *desc;
     kr2hc k;
} S;

typedef struct {
     plan_rdft super;

     stride is, ros, ios;
     int ioffset;
     uint vl;
     int ivs, ovs;
     kr2hc k;
     const S *slv;
} P;

static void apply(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     ego->k(I, O, O + ego->ioffset, ego->is, ego->ros, ego->ios,
	    ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->is);
     X(stride_destroy)(ego->ros);
     X(stride_destroy)(ego->ios);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     const S *s = ego->slv;
     const kr2hc_desc *d = s->desc;

     p->print(p, "(rdft-%s-direct-%u%v \"%s\")", 
	      X(rdft_kind_str)(s->desc->genus->kind), d->sz, ego->vl, d->nam);
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (RDFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
          const kr2hc_desc *d = ego->desc;
	  uint rnk = p->vecsz.rnk;
          iodim *vd = p->vecsz.dims;
	  uint vl = rnk == 1 ? vd[0].n : 1;
	  int ivs = rnk == 1 ? vd[0].is : 0;
	  int ovs = rnk == 1 ? vd[0].os : 0;

          return (
	       1
	       && p->sz.rnk == 1
	       && p->vecsz.rnk <= 1
	       && p->sz.dims[0].n == d->sz
	       && p->kind == d->genus->kind

	       /* check strides etc */
	       && (d->genus->okp(d, p->I, p->O, p->O + d->sz,
				 p->sz.dims[0].is,
				 p->sz.dims[0].os, -p->sz.dims[0].os,
				 vl, ivs, ovs))

	       && (0
		   /* can operate out-of-place */
		   || p->I != p->O

		   /*
		    * can compute one transform in-place, no matter
		    * what the strides are.
		    */
		   || p->vecsz.rnk == 0

		   /* can operate in-place as long as strides are the same */
		   || (X(tensor_inplace_strides)(p->sz) &&
		       X(tensor_inplace_strides)(p->vecsz))
		    )
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p, int flags)
{
     UNUSED(flags);
     return (applicable(ego, p)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_rdft *p;
     iodim *d, *vd;
     const kr2hc_desc *e = ego->desc;
     uint nr;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     pln = MKPLAN_RDFT(P, &padt, apply);

     d = p->sz.dims;
     vd = p->vecsz.dims;

     pln->k = ego->k;
     pln->ioffset = 
	  d[0].os * ((p->kind == R2HC) ? d[0].n : /* R2HCII */ (d[0].n - 1));

     nr = (p->kind == R2HC) ? (d[0].n + 2) / 2 : /* R2HCII */ (d[0].n + 1) / 2;
     pln->is = X(mkstride)(e->sz, d[0].is);
     pln->ros = X(mkstride)(nr, d[0].os);
     pln->ios = X(mkstride)(e->sz - nr + 1, -d[0].os);

     if (p->vecsz.rnk == 0) {
          pln->vl = 1;
          pln->ivs = pln->ovs = 0;
     } else {
          pln->vl = vd[0].n;
          pln->ivs = vd[0].is;
          pln->ovs = vd[0].os;
     }

     pln->slv = ego;
     pln->super.super.ops = X(ops_mul)(pln->vl / e->genus->vl, e->ops);

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_rdft_r2hc_direct)(kr2hc k, const kr2hc_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = k;
     slv->desc = desc;
     return &(slv->super);
}
