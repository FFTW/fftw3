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

/* $Id: directwbuf.c,v 1.3 2003-06-11 02:15:41 athena Exp $ */

/* direct DFTW solver, with buffer*/

#include "dft.h"

typedef struct {
     solver super;
     const ct_desc *desc;
     kdftw k;
     int dec;
} S;

typedef struct {
     plan_dft super;
     kdftw k;
     twid *td;
     int r, m, vl;
     int s, vs, ios;
     stride bufstride;
     const S *slv;
} P;

/*
   Copy A -> B, where A and B are n0 x n1 complex matrices
   such that the (i0, i1) element has index (i0 * s0 + i1 * s1). 
*/
static void cpy(int n0, int n1, 
		const R *rA, const R *iA, int sa0, int sa1, 
		R *rB, R *iB, int sb0, int sb1)
{
     int i0, i1;
     ptrdiff_t ima = iA - rA, imb = iB - rB;

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

static const R *doit(kdftw k, R *rA, R *iA, const R *W, int ios, int dist, 
		     int r, int batchsz, R *buf, stride bufstride)
{
     cpy(r, batchsz, rA, iA, ios, dist, buf, buf + 1, 2, 2 * r);
     W = k(buf, buf + 1, W, bufstride, batchsz, 2 * r);
     cpy(r, batchsz, buf, buf + 1, 2, 2 * r, rA, iA, ios, dist);
     return W;
}

#define BATCHSZ 4 /* FIXME: parametrize? */

static void apply(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     int i, j, m = ego->m, vl = ego->vl, r = ego->r;
     int s = ego->s, vs = ego->vs, ios = ego->ios;
     R *buf;

     STACK_MALLOC(R *, buf, r * BATCHSZ * 2 * sizeof(R));

     for (i = 0; i < vl; ++i) {
	  R *rA = rio + i * vs, *iA = iio + i * vs;
	  const R *W = ego->td->W;

	  for (j = m; j >= BATCHSZ; j -= BATCHSZ) {
	       W = doit(ego->k, rA, iA, W, ios, s, r, BATCHSZ, 
			buf, ego->bufstride);
	       rA += s * (int)BATCHSZ;
	       iA += s * (int)BATCHSZ;
	  }

	  /* do remaining j calls, if any */
	  if (j > 0)
	       doit(ego->k, rA, iA, W, ios, s, r, j, buf, ego->bufstride);

     }

     STACK_FREE(buf);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     X(twiddle_awake)(flg, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, ego->m);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->bufstride);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const ct_desc *e = slv->desc;

     p->print(p, "(dft-directw-buf-%d/%d%v \"%s\")",
              ego->r, X(twiddle_length)(ego->r, e->tw), ego->vl, e->nam);
}

static int applicable0(const solver *ego_, const problem *p_,
		       const planner *plnr)
{
     if (DFTWP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_dftw *p = (const problem_dftw *) p_;
          const ct_desc *e = ego->desc;

          return (
	       1

	       && p->dec == ego->dec
	       && p->r == e->radix

	       /* check for alignment/vector length restrictions,
		  both for batchsize and for the remainer */
	       && (p->m < BATCHSZ ||
		   (e->genus->okp(e, 0, ((const R *)0)+1, 2, 0, BATCHSZ,
				  2 * e->radix, plnr)))
	       && (e->genus->okp(e, 0, ((const R *)0)+1, 2, 0, p->m % BATCHSZ,
				 2 * e->radix, plnr))
	       );
     }

     return 0;
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const problem_dftw *p;

     if (!applicable0(ego_, p_, plnr))
          return 0;

     p = (const problem_dftw *) p_;

     if (NO_UGLYP(plnr)) {
	  if (X(ct_uglyp)(512, p->m * p->r, p->r)) return 0;
     }

     return 1;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_dftw *p;
     const ct_desc *e = ego->desc;

     static const plan_adt padt = {
	  X(dftw_solve), awake, print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_, plnr))
          return (plan *)0;

     p = (const problem_dftw *) p_;

     pln = MKPLAN_DFTW(P, &padt, apply);

     pln->k = ego->k;
     pln->ios = p->m * p->s;
     pln->td = 0;
     pln->r = p->r;
     pln->m = p->m;
     pln->s = p->s; /* == p->ws */
     pln->vl = p->vl;
     pln->vs = p->vs; /* == p->wvs */
     pln->slv = ego;
     pln->bufstride = X(mkstride)(e->radix, 2);

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(pln->m * (pln->vl / e->genus->vl),
		  &e->ops, &pln->super.super.ops);
     /* 4 load/stores * N * VL */
     pln->super.super.ops.other += 4 * pln->r * pln->m * pln->vl;

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_dft_directwbuf)(kdftw codelet, const ct_desc *desc, int dec)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = codelet;
     slv->desc = desc;
     slv->dec = dec;
     return &(slv->super);
}
