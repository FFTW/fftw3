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

/* $Id: direct2.c,v 1.2 2002-08-04 21:03:45 stevenj Exp $ */

/* direct RDFT2 R2HC/HC2R solver, if we have a codelet */

#include "rdft.h"

typedef union {
     kr2hc r2hc;
     khc2r hc2r;
} kodelet;

typedef struct {
     solver super;
     union {
	  const kr2hc_desc *r2hc;
	  const khc2r_desc *hc2r;
     } desc;
     kodelet k;
     uint sz;
     rdft_kind kind;
     const char *nam;
} S;

typedef struct {
     plan_rdft2 super;

     stride is, os;
     uint vl;
     int ivs, ovs;
     kodelet k;
     const S *slv;
     int ilast;
} P;

static void apply_r2hc(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;
     ego->k.r2hc(r, rio, iio, ego->is, ego->os, ego->os,
		 ego->vl, ego->ivs, ego->ovs);
     iio[0] = iio[ego->ilast] = 0;
}

static void apply_hc2r(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;
     ego->k.hc2r(rio, iio, r, ego->os, ego->os, ego->is,
		 ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->is);
     X(stride_destroy)(ego->os);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     const S *s = ego->slv;

     p->print(p, "(rdft2-%s-direct-%u%v \"%s\")", 
	      X(rdft_kind_str)(s->kind), s->sz, ego->vl, s->nam);
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (RDFT2P(p_)) {
          const S *ego = (const S *) ego_;
          const problem_rdft2 *p = (const problem_rdft2 *) p_;
	  uint rnk = p->vecsz.rnk;
          iodim *vd = p->vecsz.dims;
	  uint vl = rnk == 1 ? vd[0].n : 1;
	  int ivs = rnk == 1 ? vd[0].is : 0;
	  int ovs = rnk == 1 ? vd[0].os : 0;

          return (
	       1
	       && p->vecsz.rnk <= 1
	       && p->sz.n == ego->sz
	       && p->kind == ego->kind

	       /* check strides etc */
	       && (!R2HC_KINDP(ego->kind) ||
		   ego->desc.r2hc->genus->okp(ego->desc.r2hc, 
					      p->r, p->rio, p->rio,
					      p->sz.is,
					      p->sz.os, p->sz.os,
					      vl, ivs, ovs))
	       && (!HC2R_KINDP(ego->kind) ||
		   ego->desc.hc2r->genus->okp(ego->desc.hc2r,
					      p->rio, p->rio, p->r,
					      p->sz.os, p->sz.os,
					      p->sz.is,
					      vl, ivs, ovs))
	       
	       && (0
		   /* can operate out-of-place */
		   || p->r != p->rio
		   || p->r != p->iio

		   /*
		    * can compute one transform in-place, no matter
		    * what the strides are.
		    */
		   || p->vecsz.rnk == 0

		   /* can operate in-place as long as strides are the same */
		   || X(rdft2_inplace_strides)(p, RNK_MINFTY)
		    )
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p, const planner *plnr)
{
     UNUSED(plnr);
     return (applicable(ego, p)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_rdft2 *p;
     iodim d, *vd;
     int r2hc_kindp;

     static const plan_adt padt = {
	  X(rdft2_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_rdft2 *) p_;

     r2hc_kindp = R2HC_KINDP(p->kind);
     A(r2hc_kindp || HC2R_KINDP(p->kind));

     pln = MKPLAN_RDFT2(P, &padt, r2hc_kindp ? apply_r2hc : apply_hc2r);

     d = p->sz;
     vd = p->vecsz.dims;

     pln->k = ego->k;

     pln->is = X(mkstride)(ego->sz, d.is);
     pln->os = X(mkstride)(d.n/2 + 1, d.os);

     if (p->vecsz.rnk == 0) {
          pln->vl = 1;
          pln->ivs = pln->ovs = 0;
     } else {
          pln->vl = vd[0].n;
          pln->ivs = vd[0].is;
          pln->ovs = vd[0].os;
     }

     pln->ilast = (d.n % 2) ? 0 : (d.n/2) * d.os; /* Nyquist freq., if any */

     pln->slv = ego;
     if (r2hc_kindp)
	  pln->super.super.ops = X(ops_mul)(pln->vl/ego->desc.r2hc->genus->vl, ego->desc.r2hc->ops);
     else {
	  pln->super.super.ops = X(ops_mul)(pln->vl/ego->desc.hc2r->genus->vl, ego->desc.hc2r->ops);
	  pln->super.super.ops.other += 2 * pln->vl; /* + 2 stores */
     }

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_rdft2_r2hc_direct)(kr2hc k, const kr2hc_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->k.r2hc = k;
     slv->desc.r2hc = desc;
     slv->sz = desc->sz;
     slv->nam = desc->nam;
     slv->kind = desc->genus->kind;
     return &(slv->super);
}

solver *X(mksolver_rdft2_hc2r_direct)(khc2r k, const khc2r_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->k.hc2r = k;
     slv->desc.hc2r = desc;
     slv->sz = desc->sz;
     slv->nam = desc->nam;
     slv->kind = desc->genus->kind;
     return &(slv->super);
}
