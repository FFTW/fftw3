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

/* $Id: ct-ditbuf.c,v 1.30 2003-05-07 21:59:23 stevenj Exp $ */

/* decimation in time Cooley-Tukey.  Codelet operates on
   contiguous buffer rather than directly on the output array.  */

/* FIXME: find a way to use rank-0 transforms for this stuff */

#include "dft.h"
#include "ct.h"

/*
   Copy A -> B, where A and B are n0 x n1 complex matrices
   such that the (i0, i1) element has index (i0 * s0 + i1 * s1). 
*/
static void cpy(int n0, int n1, 
		const R *rA, const R *iA, int sa0, int sa1, 
		R *rB, R *iB, int sb0, int sb1)
{
     int i0, i1;
     int ima = iA - rA, imb = iB - rB;

     for (i0 = 0; i0 < n0; ++i0) {
	  const R *pa; 
	  R *pb;

	  pa = rA; rA += sa0;
	  pb = rB; rB += sb0;
	  for (i1 = 0; i1 < n1; ++i1) {
	       R xr = pa[0], xi = pa[ima];
	       pb[0] = xr; pb[imb] = xi; 
	       pa += sa1; pb += sb1;
	  }
     }
}

static const R *doit(kdft_dit k, R *rA, R *iA, const R *W, int ios, int dist, 
		     int r, int batchsz, R *buf, stride bufstride)
{
     cpy(r, batchsz, rA, iA, ios, dist, buf, buf + 1, 2, 2 * r);
     W = k(buf, buf + 1, W, bufstride, batchsz, 2 * r);
     cpy(r, batchsz, buf, buf + 1, 2, 2 * r, rA, iA, ios, dist);
     return W;
}

#define BATCHSZ 4 /* FIXME: parametrize? */

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const plan_ct *ego = (const plan_ct *) ego_;
     plan *cld0 = ego->cld;
     plan_dft *cld = (plan_dft *) cld0;

     /* two-dimensional r x vl sub-transform: */
     cld->apply(cld0, ri, ii, ro, io);

     {
          int i, j, m = ego->m, vl = ego->vl, r = ego->r;
          int os = ego->os, ovs = ego->ovs, ios = ego->iios;
	  R *buf;

	  STACK_MALLOC(R *, buf, r * BATCHSZ * 2 * sizeof(R));

          for (i = 0; i < vl; ++i) {
	       R *rA = ro + i * ovs, *iA = io + i * ovs;
	       const R *W = ego->td->W;

	       for (j = m; j >= BATCHSZ; j -= BATCHSZ) {
		    W = doit(ego->k.dit, rA, iA, W, ios, os, r, 
			     BATCHSZ, buf, ego->vs);
		    rA += os * (int)BATCHSZ;
		    iA += os * (int)BATCHSZ;
	       }

	       /* do remaining j calls, if any */
	       if (j > 0)
		    doit(ego->k.dit, rA, iA, W, ios, os, r, j, buf, ego->vs);

	  }

	  STACK_FREE(buf);
     }
}

static int applicable0(const solver_ct *ego, const problem *p_,
		       const planner *plnr)
{
     UNUSED(plnr);
     if (X(dft_ct_applicable)(ego, p_)) {
          const ct_desc *e = ego->desc;
          const problem_dft *p = (const problem_dft *) p_;
          iodim *d = p->sz->dims;
	  int m = d[0].n / e->radix;
          return (1

                  /* check both batch size and remainder */
		  && (m < BATCHSZ ||
		      (e->genus->okp(e, 0, ((const R *)0)+1, 2, 0, BATCHSZ,
				     2 * e->radix, plnr)))
		  && (e->genus->okp(e, 0, ((const R *)0)+1, 2, 0, m % BATCHSZ,
				    2 * e->radix, plnr))
	       );
     }
     return 0;
}

static int applicable(const solver_ct *ego, const problem *p_,
		      const planner *plnr)
{
     const problem_dft *p;

     if (!applicable0(ego, p_, plnr)) return 0;

     p = (const problem_dft *) p_;

     /* emulate fftw2 behavior */
     if (NO_VRECURSEP(plnr) && (p->vecsz->rnk > 0))  return 0;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(512, p->sz->dims[0].n,
				       ego->desc->radix))
	  return 0;

     return 1;
}


static void finish(plan_ct *ego)
{
     const ct_desc *d = ego->slv->desc;
     ego->iios = ego->m * ego->os;
     ego->vs = X(mkstride)(ego->r, 2);
     X(ops_madd)(ego->vl * ego->m / d->genus->vl, &d->ops, &ego->cld->ops,
		 &ego->super.super.ops);

     /* 4 load/stores * N * VL */
     ego->super.super.ops.other += 4 * ego->r * ego->m * ego->vl;
}

static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     static const ctadt adt = {
	  sizeof(plan_ct), X(dft_mkcld_dit), finish, applicable, apply
     };
     return X(mkplan_dft_ct)((const solver_ct *) ego, p, plnr, &adt);
}


solver *X(mksolver_dft_ct_ditbuf)(kdft_dit codelet, const ct_desc *desc)
{
     static const solver_adt sadt = { mkplan };
     static const char name[] = "dft-ditbuf";
     union kct k;
     k.dit = codelet;

     return X(mksolver_dft_ct)(k, desc, name, &sadt);
}
