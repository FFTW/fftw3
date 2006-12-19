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


/* direct RDFT solver, using r2c codelets */

#include "rdft.h"

typedef struct {
     solver super;
     const kr2c_desc *desc;
     kr2c k;
     INT sz;
     rdft_kind kind;
     const char *nam;
} S;

typedef struct {
     plan_rdft super;

     INT ioffset;
     INT vl, rs0, ivs, ovs;
     stride rs, csr, csi;
     kr2c k;
     const S *slv;
} P;

static void apply_r2hc(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     ASSERT_ALIGNED_DOUBLE;
     ego->k(I, I + ego->rs0, O, O + ego->ioffset, 
	    ego->rs, ego->csr, ego->csi,
	    ego->vl, ego->ivs, ego->ovs);
}

static void apply_hc2r(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     ASSERT_ALIGNED_DOUBLE;
     ego->k(O, O + ego->rs0, I, I + ego->ioffset, 
	    ego->rs, ego->csr, ego->csi,
	    ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->rs);
     X(stride_destroy)(ego->csr);
     X(stride_destroy)(ego->csi);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *s = ego->slv;

     p->print(p, "(rdft-%s-direct-r2c-%D%v \"%s\")", 
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
     INT vl, ivs, ovs, rs, cs;
     R *R0, *C0;

     return (
	  1
	  && p->sz->rnk == 1
	  && p->vecsz->rnk <= 1
	  && p->sz->dims[0].n == ego->sz
	  && p->kind[0] == ego->kind

	  /* check strides etc */
	  && X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs)

	  && ((R2HC_KINDP(ego->kind) ?
	       (R0 = p->I, C0 = p->O, 
		rs = p->sz->dims[0].is, cs = p->sz->dims[0].os)
	       :
	       (R0 = p->O, C0 = p->I,
		rs = p->sz->dims[0].os, cs = p->sz->dims[0].is)),
	      1)

	  && ego->desc->genus->okp(ego->desc, 
				   R0, R0 + rs,
				   C0, C0 + ioffset(ego->kind, ego->sz, cs),
				   rs, cs, -cs,
				   vl, ivs, ovs)
	       
	  && (0
	      /* can operate out-of-place */
	      || p->I != p->O

	      /* computing one transform */
	      || vl == 1

	      /* can operate in-place as long as strides are the same */
	      || X(tensor_inplace_strides2)(p->sz, p->vecsz)
	       )
	  );
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_rdft *p;
     iodim *d;
     INT rs, cs;
     R *R0, *C0;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     if (R2HC_KINDP(ego->kind)) {
	  R0 = p->I; C0 = p->O;
	  rs = p->sz->dims[0].is; cs = p->sz->dims[0].os;
     } else {
	  R0 = p->O; C0 = p->I;
	  rs = p->sz->dims[0].os; cs = p->sz->dims[0].is;
     }

     pln = MKPLAN_RDFT(P, &padt, 
		       R2HC_KINDP(ego->kind) ? apply_r2hc : apply_hc2r);

     d = p->sz->dims;

     pln->k = ego->k;
     pln->ioffset = ioffset(ego->kind, d[0].n, cs);

     pln->rs0 = rs;
     pln->rs = X(mkstride)(ego->sz, 2 * rs);
     pln->csr = X(mkstride)(ego->sz, cs);
     pln->csi = X(mkstride)(ego->sz, -cs);

     X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);

     pln->slv = ego;
     X(ops_zero)(&pln->super.super.ops);

     X(ops_madd2)(pln->vl / ego->desc->genus->vl,
		  &ego->desc->ops,
		  &pln->super.super.ops);

     pln->super.super.could_prune_now_p = 1;

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_rdft_r2c_direct)(kr2c k, const kr2c_desc *desc)
{
     static const solver_adt sadt = { PROBLEM_RDFT, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = k;
     slv->desc = desc;
     slv->sz = desc->sz;
     slv->nam = desc->nam;
     slv->kind = desc->genus->kind;
     return &(slv->super);
}
