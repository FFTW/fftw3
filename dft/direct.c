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

/* $Id: direct.c,v 1.48 2006-01-05 03:04:26 stevenj Exp $ */

/* direct DFT solver, if we have a codelet */

#include "dft.h"

typedef struct {
     solver super;
     const kdft_desc *desc;
     kdft k;
     int bufferedp;
} S;

typedef struct {
     plan_dft super;

     stride is, os, bufstride;
     INT n, vl, ivs, ovs;
     kdft k;
     const S *slv;
} P;

static void dobatch(kdft k, 
		    R *ri, R *ii, R *ro, R *io,
		    INT n, stride is, stride os,
		    INT vl, INT ivs, INT ovs, 
		    R *buf, stride bufstride)
{
     X(cpy2d_pair_ci)(ri, ii, buf, buf+1,
		      n, WS(is, 1), WS(bufstride, 1),
		      vl, ivs, 2);
     
     if (IABS(WS(os, 1)) < IABS(ovs)) {
	  /* transform directly to output */
	  k(buf, buf+1, ro, io, bufstride, os, vl, 2, ovs);
     } else {
	  /* transform to buffer and copy back */
	  k(buf, buf+1, buf, buf+1, bufstride, bufstride, vl, 2, 2);
	  X(cpy2d_pair_co)(buf, buf+1, ro, io,
			   n, WS(bufstride, 1), WS(os, 1), 
			   vl, 2, ovs);
     }
}

static INT compute_batchsize(INT n)
{
     /* round up to multiple of 4 */
     n += 3;
     n &= -4;

     return (n + 2);
}

static void apply_buf(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     R *buf;
     INT vl = ego->vl;
     INT n = ego->n;
     INT i;
     INT batchsz = compute_batchsize(n);

     STACK_MALLOC(R *, buf, n * batchsz * 2 * sizeof(R));

     for (i = 0; i < vl - batchsz; i += batchsz) {
	  dobatch(ego->k, ri, ii, ro, io,
		  n, ego->is, ego->os,
		  batchsz, ego->ivs, ego->ovs,
		  buf, ego->bufstride);
	  ri += batchsz * ego->ivs;
	  ii += batchsz * ego->ivs;
	  ro += batchsz * ego->ovs;
	  io += batchsz * ego->ovs;
     }
     dobatch(ego->k, ri, ii, ro, io,
	     n, ego->is, ego->os,
	     vl - i, ego->ivs, ego->ovs,
	     buf, ego->bufstride);

     STACK_FREE(buf);
}

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     ASSERT_ALIGNED_DOUBLE;
     ego->k(ri, ii, ro, io, ego->is, ego->os, ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->is);
     X(stride_destroy)(ego->os);
     X(stride_destroy)(ego->bufstride);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *s = ego->slv;
     const kdft_desc *d = s->desc;

     if (ego->slv->bufferedp)
	  p->print(p, "(dft-directbuf/%D-%D%v \"%s\")", 
		   compute_batchsize(d->sz), d->sz, ego->vl, d->nam);
     else
	  p->print(p, "(dft-direct-%D%v \"%s\")", d->sz, ego->vl, d->nam);
}

static int applicable_buf(const solver *ego_, const problem *p_,
			  const planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p = (const problem_dft *) p_;
     const kdft_desc *d = ego->desc;
     INT vl;
     INT ivs, ovs;
     INT batchsz;

     return (
	  1
	  && p->sz->rnk == 1
	  && p->vecsz->rnk == 1
	  && p->sz->dims[0].n == d->sz

	  /* check strides etc */
	  && X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs)

	  /* UGLY if IS <= IVS */
	  && !(NO_UGLYP(plnr) &&
	       X(iabs)(p->sz->dims[0].is) <= X(iabs)(ivs))

	  && (batchsz = compute_batchsize(d->sz), 1)
	  && (d->genus->okp(d, 0, ((const R *)0) + 1, p->ro, p->io,
			    2 * batchsz, p->sz->dims[0].os,
			    batchsz, 2, ovs, plnr))
	  && (d->genus->okp(d, 0, ((const R *)0) + 1, p->ro, p->io,
			    2 * batchsz, p->sz->dims[0].os,
			    vl % batchsz, 2, ovs, plnr))


	  && (0
	      /* can operate out-of-place */
	      || p->ri != p->ro

	      /* any in-place problem that fits in the buffer is ok */
	      || vl <= batchsz

	      /* can operate in-place as long as strides are the same */
	      || (X(tensor_inplace_strides2)(p->sz, p->vecsz))
	       )
	  );
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p = (const problem_dft *) p_;
     const kdft_desc *d = ego->desc;
     INT vl;
     INT ivs, ovs;

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

     if (ego->bufferedp) {
	  if (!applicable_buf(ego_, p_, plnr))
	       return (plan *)0;
     } else {
	  if (!applicable(ego_, p_, plnr))
	       return (plan *)0;
     }

     p = (const problem_dft *) p_;

     pln = MKPLAN_DFT(P, &padt, ego->bufferedp ? apply_buf : apply);

     d = p->sz->dims;

     pln->k = ego->k;
     pln->n = d[0].n;
     pln->is = X(mkstride)(pln->n, d[0].is);
     pln->os = X(mkstride)(pln->n, d[0].os);
     pln->bufstride = X(mkstride)(pln->n, 2 * compute_batchsize(pln->n));

     X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     pln->slv = ego;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(pln->vl / e->genus->vl, &e->ops, &pln->super.super.ops);

     if (ego->bufferedp) 
	  pln->super.super.ops.other += 4 * pln->n * pln->vl;

     pln->super.super.could_prune_now_p = !ego->bufferedp;
     return &(pln->super.super);
}

static solver *mksolver(kdft k, const kdft_desc *desc, int bufferedp)
{
     static const solver_adt sadt = { PROBLEM_DFT, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = k;
     slv->desc = desc;
     slv->bufferedp = bufferedp;
     return &(slv->super);
}

solver *X(mksolver_dft_direct)(kdft k, const kdft_desc *desc)
{
     return mksolver(k, desc, 0);
}

solver *X(mksolver_dft_directbuf)(kdft k, const kdft_desc *desc)
{
     return mksolver(k, desc, 1);
}
