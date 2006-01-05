/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* $Id: direct.c,v 1.24 2006-01-05 03:04:27 stevenj Exp $ */

/* direct RDFT R2HC/HC2R solver, if we have a codelet */

#include "rdft.h"

typedef union {
     kr2hc r2hc;
     khc2r hc2r;
     kr2r r2r;
} kodelet;

typedef struct {
     solver super;
     union {
	  const kr2hc_desc *r2hc;
	  const khc2r_desc *hc2r;
	  const kr2r_desc *r2r;
     } desc;
     kodelet k;
     INT sz;
     rdft_kind kind;
     const char *nam;
} S;

typedef struct {
     plan_rdft super;

     stride is, ros, ios;
     INT ioffset;
     INT vl;
     INT ivs, ovs;
     kodelet k;
     const S *slv;
} P;

static void apply_r2hc(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     ASSERT_ALIGNED_DOUBLE;
     ego->k.r2hc(I, O, O + ego->ioffset, ego->is, ego->ros, ego->ios,
		 ego->vl, ego->ivs, ego->ovs);
}

static void apply_hc2r(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     ASSERT_ALIGNED_DOUBLE;
     ego->k.hc2r(I, I + ego->ioffset, O, ego->ros, ego->ios, ego->is,
		 ego->vl, ego->ivs, ego->ovs);
}

static void apply_r2r(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     ASSERT_ALIGNED_DOUBLE;
     ego->k.r2r(I, O, ego->is, ego->ros, ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->is);
     X(stride_destroy)(ego->ros);
     if (!R2R_KINDP(ego->slv->kind))
	  X(stride_destroy)(ego->ios);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *s = ego->slv;

     p->print(p, "(rdft-%s-direct-%D%v \"%s\")", 
	      X(rdft_kind_str)(s->kind), s->sz, ego->vl, s->nam);
}

static INT ioffset(rdft_kind kind, INT sz, INT s)
{
     return(s * ((kind == R2HC || kind == HC2R) ? sz : (sz - 1)));
}

static int applicable(const solver *ego_, const problem *p_)
{
     const S *ego = (const S *) ego_;
     const problem_rdft *p = (const problem_rdft *) p_;
     INT vl;
     INT ivs, ovs;

     return (
	  1
	  && p->sz->rnk == 1
	  && p->vecsz->rnk <= 1
	  && p->sz->dims[0].n == ego->sz
	  && p->kind[0] == ego->kind

	  /* check strides etc */
	  && X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs)

	  && (!R2HC_KINDP(ego->kind) ||
	      ego->desc.r2hc->genus->okp(ego->desc.r2hc, p->I, p->O, p->O
					 + ioffset(ego->kind, ego->sz, p->sz->dims[0].os),
					 p->sz->dims[0].is,
					 p->sz->dims[0].os, 0-p->sz->dims[0].os,
					 vl, ivs, ovs))
	  && (!HC2R_KINDP(ego->kind) ||
	      ego->desc.hc2r->genus->okp(ego->desc.hc2r, p->I, p->I
					 + ioffset(ego->kind, ego->sz, p->sz->dims[0].is), p->O,
					 p->sz->dims[0].is, 0-p->sz->dims[0].is,
					 p->sz->dims[0].os, 
					 vl, ivs, ovs))
	       
	  && (!R2R_KINDP(ego->kind) ||
	      ego->desc.r2r->genus->okp(ego->desc.r2r, p->I, p->O,
					p->sz->dims[0].is,
					p->sz->dims[0].os, 
					vl, ivs, ovs))
	       
	  && (0
	      /* can operate out-of-place */
	      || p->I != p->O

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

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_rdft *p;
     iodim *d;
     int hc2r_kindp, r2r_kindp;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     hc2r_kindp = HC2R_KINDP(ego->kind);
     r2r_kindp = R2R_KINDP(ego->kind);

     pln = MKPLAN_RDFT(P, &padt, 
		       r2r_kindp ? apply_r2r :
		       (hc2r_kindp ? apply_hc2r : apply_r2hc));

     d = p->sz->dims;

     pln->k = ego->k;
     pln->ioffset = ioffset(ego->kind, d[0].n, hc2r_kindp ? d[0].is : d[0].os);

     pln->is = X(mkstride)(ego->sz, hc2r_kindp ? d[0].os : d[0].is);
     if (r2r_kindp) {
	  pln->ros = X(mkstride)(ego->sz, d[0].os);
	  pln->ios = 0;
     }
     else {  
	  INT nr = (ego->kind == R2HC || ego->kind == HC2R) 
	       ?(d[0].n + 2) / 2 : /* R2HCII */ (d[0].n + 1) / 2;
	  pln->ros = X(mkstride)(nr, hc2r_kindp ? d[0].is : d[0].os);
	  pln->ios = X(mkstride)(ego->sz - nr + 1, 
				 hc2r_kindp ? -d[0].is : -d[0].os);
     }

     X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);

     pln->slv = ego;
     X(ops_zero)(&pln->super.super.ops);
     if (r2r_kindp)
	  X(ops_madd2)(pln->vl / ego->desc.r2r->genus->vl,
		       &ego->desc.r2r->ops,
		       &pln->super.super.ops);
     else if (hc2r_kindp)
	  X(ops_madd2)(pln->vl / ego->desc.hc2r->genus->vl,
		       &ego->desc.hc2r->ops,
		       &pln->super.super.ops);
     else
	  X(ops_madd2)(pln->vl / ego->desc.r2hc->genus->vl,
		       &ego->desc.r2hc->ops,
		       &pln->super.super.ops);

     pln->super.super.could_prune_now_p = 1;

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_rdft_r2hc_direct)(kr2hc k, const kr2hc_desc *desc)
{
     static const solver_adt sadt = { PROBLEM_RDFT, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k.r2hc = k;
     slv->desc.r2hc = desc;
     slv->sz = desc->sz;
     slv->nam = desc->nam;
     slv->kind = desc->genus->kind;
     return &(slv->super);
}

solver *X(mksolver_rdft_hc2r_direct)(khc2r k, const khc2r_desc *desc)
{
     static const solver_adt sadt = { PROBLEM_RDFT, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k.hc2r = k;
     slv->desc.hc2r = desc;
     slv->sz = desc->sz;
     slv->nam = desc->nam;
     slv->kind = desc->genus->kind;
     return &(slv->super);
}

solver *X(mksolver_rdft_r2r_direct)(kr2r k, const kr2r_desc *desc)
{
     static const solver_adt sadt = { PROBLEM_RDFT, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k.r2r = k;
     slv->desc.r2r = desc;
     slv->sz = desc->sz;
     slv->nam = desc->nam;
     slv->kind = desc->kind;
     return &(slv->super);
}

