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

/* $Id: direct.c,v 1.3 2002-08-26 04:05:53 stevenj Exp $ */

/* direct RDFT R2HC/HC2R solver, if we have a codelet */

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
     plan_rdft super;

     stride is, ros, ios;
     int ioffset;
     uint vl;
     int ivs, ovs;
     kodelet k;
     const S *slv;
} P;

static void apply_r2hc(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     ego->k.r2hc(I, O, O + ego->ioffset, ego->is, ego->ros, ego->ios,
		 ego->vl, ego->ivs, ego->ovs);
}

static void apply_hc2r(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     ego->k.hc2r(I, I + ego->ioffset, O, ego->ros, ego->ios, ego->is,
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

     p->print(p, "(rdft-%s-direct-%u%v \"%s\")", 
	      X(rdft_kind_str)(s->kind), s->sz, ego->vl, s->nam);
}

static int ioffset(rdft_kind kind, uint sz, int s)
{
     return(s * ((kind == R2HC || kind == HC2R) ? sz : (sz - 1)));
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (RDFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
	  uint rnk = p->vecsz.rnk;
          iodim *vd = p->vecsz.dims;
	  uint vl = rnk == 1 ? vd[0].n : 1;
	  int ivs = rnk == 1 ? vd[0].is : 0;
	  int ovs = rnk == 1 ? vd[0].os : 0;

          return (
	       1
	       && p->sz.rnk == 1
	       && p->vecsz.rnk <= 1
	       && p->sz.dims[0].n == ego->sz
	       && p->kind[0] == ego->kind

	       /* check strides etc */
	       && (!R2HC_KINDP(ego->kind) ||
		   ego->desc.r2hc->genus->okp(ego->desc.r2hc, p->I, p->O, p->O
					      + ioffset(ego->kind, ego->sz, p->sz.dims[0].os),
					      p->sz.dims[0].is,
					      p->sz.dims[0].os, -p->sz.dims[0].os,
					      vl, ivs, ovs))
	       && (!HC2R_KINDP(ego->kind) ||
		   ego->desc.hc2r->genus->okp(ego->desc.hc2r, p->I, p->I
					      + ioffset(ego->kind, ego->sz, p->sz.dims[0].is), p->O,
					      p->sz.dims[0].is, -p->sz.dims[0].is,
					      p->sz.dims[0].os, 
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

static int score(const solver *ego, const problem *p, const planner *plnr)
{
     UNUSED(plnr);
     return (applicable(ego, p)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_rdft *p;
     iodim *d, *vd;
     uint nr;
     int r2hc_kindp;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     r2hc_kindp = R2HC_KINDP(ego->kind);
     A(r2hc_kindp || HC2R_KINDP(ego->kind));

     pln = MKPLAN_RDFT(P, &padt, r2hc_kindp ? apply_r2hc : apply_hc2r);

     d = p->sz.dims;
     vd = p->vecsz.dims;

     pln->k = ego->k;
     pln->ioffset = ioffset(ego->kind, d[0].n, r2hc_kindp ? d[0].os : d[0].is);

     nr = (ego->kind == R2HC || ego->kind == HC2R) ? (d[0].n + 2) / 2 : /* R2HCII */ (d[0].n + 1) / 2;
     pln->is = X(mkstride)(ego->sz, r2hc_kindp ? d[0].is : d[0].os);
     pln->ros = X(mkstride)(nr, r2hc_kindp ? d[0].os : d[0].is);
     pln->ios = X(mkstride)(ego->sz - nr + 1, r2hc_kindp ? -d[0].os : -d[0].is);

     if (p->vecsz.rnk == 0) {
          pln->vl = 1;
          pln->ivs = pln->ovs = 0;
     } else {
          pln->vl = vd[0].n;
          pln->ivs = vd[0].is;
          pln->ovs = vd[0].os;
     }

     pln->slv = ego;
     if (r2hc_kindp)
	  pln->super.super.ops = X(ops_mul)(pln->vl/ego->desc.r2hc->genus->vl, ego->desc.r2hc->ops);
     else
	  pln->super.super.ops = X(ops_mul)(pln->vl/ego->desc.hc2r->genus->vl, ego->desc.hc2r->ops);

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_rdft_r2hc_direct)(kr2hc k, const kr2hc_desc *desc)
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

solver *X(mksolver_rdft_hc2r_direct)(khc2r k, const khc2r_desc *desc)
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
