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

/* $Id: dftw-directbuf.c,v 1.6 2005-02-27 01:06:49 athena Exp $ */

#include "ct.h"

typedef struct {
     ct_solver super;
     const ct_desc *desc;
     kdftw k;
} S;

typedef struct {
     plan_dftw super;
     kdftw k;
     int r, vl;
     int s, vs, ios;
     int mcount;
     stride bufstride;
     const R *tdW;
     int mstart, m;
     twid *td;
     const S *slv;
} P;

static const R *doit(kdftw k, R *rA, R *iA, const R *W, int ios, int dist, 
		     int r, int batchsz, R *buf, stride bufstride)
{
     X(cpy2d_pair_ci)(rA, iA, buf, buf + 1, 
		      r, ios, WS(bufstride, 1),
		      batchsz, dist, 2);
     W = k(buf, buf + 1, W, bufstride, batchsz, 2);
     X(cpy2d_pair_co)(buf, buf + 1, rA, iA,
		      r, WS(bufstride, 1), ios,
		      batchsz, 2, dist);
     return W;
}

/* must be even for SIMD alignment; should not be 2^k to avoid
   associativity conflicts */
static int compute_batchsize(int radix)
{
     /* round up to multiple of 4 */
     radix += 3;
     radix &= -4;

     return (radix + 2);
}

static void apply(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     int i, j, mcount = ego->mcount, vl = ego->vl, r = ego->r;
     int s = ego->s, vs = ego->vs, ios = ego->ios;
     int batchsz = compute_batchsize(r);
     R *buf;

     STACK_MALLOC(R *, buf, r * batchsz * 2 * sizeof(R));

     for (i = 0; i < vl; ++i) {
	  R *rA = rio + i * vs, *iA = iio + i * vs;
	  const R *W = ego->tdW;

	  for (j = 0; j < mcount - batchsz; j += batchsz) {
	       W = doit(ego->k, rA, iA, W, ios, s, r, batchsz, 
			buf, ego->bufstride);
	       rA += s * batchsz;
	       iA += s * batchsz;
	  }

	  doit(ego->k, rA, iA, W, ios, s, r, mcount - j, buf, ego->bufstride);

     }

     STACK_FREE(buf);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     X(twiddle_awake)(flg, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, ego->m);
     ego->tdW = X(twiddle_shift)(ego->td, ego->mstart);
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
     int batchsz = compute_batchsize(ego->r);

     p->print(p, "(dftw-directbuf/%d-%d/%d%v \"%s\")",
              batchsz, ego->r, X(twiddle_length)(ego->r, e->tw), 
	      ego->vl, e->nam);
}

static int applicable0(const S *ego, 
		       int dec, int r, int m, int s, int vl, int vs, 
		       R *rio, R *iio,
		       const planner *plnr)
{
     const ct_desc *e = ego->desc;
     int batchsz;
     UNUSED(vl); UNUSED(s); UNUSED(vs); UNUSED(rio); UNUSED(iio);

     return (
	  1

	  && dec == ego->super.dec
	  && r == e->radix

	  /* check for alignment/vector length restrictions, both for
	     batchsize and for the remainder */
	  && (batchsz = compute_batchsize(r), 1)
	  && (e->genus->okp(e, 0, ((const R *)0) + 1, 2 * batchsz, 0,
			    batchsz, 2, plnr))
	  && (e->genus->okp(e, 0, ((const R *)0) + 1, 2 * batchsz, 0, 
			    m % batchsz, 2, plnr))
	  );
}

static int applicable(const S *ego, 
		      int dec, int r, int m, int s, int vl, int vs, 
		      R *rio, R *iio,
		      const planner *plnr)
{
     if (!applicable0(ego, dec, r, m, s, vl, vs, rio, iio, plnr))
          return 0;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(512, m * r, r))
	  return 0;

     return 1;
}

static plan *mkcldw(const ct_solver *ego_, 
		    int dec, int r, int m, int s, int vl, int vs, 
		    int mstart, int mcount,
		    R *rio, R *iio,
		    planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const ct_desc *e = ego->desc;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     A(mstart >= 0 && mstart + mcount <= m);
     if (!applicable(ego, dec, r, m, s, vl, vs, rio, iio, plnr))
          return (plan *)0;

     pln = MKPLAN_DFTW(P, &padt, apply);

     pln->k = ego->k;
     pln->ios = m * s;
     pln->td = 0;
     pln->tdW = 0;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->vl = vl;
     pln->vs = vs;
     pln->mstart = mstart;
     pln->mcount = mcount;
     pln->slv = ego;
     pln->bufstride = X(mkstride)(r, 2 * compute_batchsize(r));

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(mcount * (vl/e->genus->vl), &e->ops, &pln->super.super.ops);
     /* 4 load/stores * N * VL */
     pln->super.super.ops.other += 4 * r * mcount * vl;

     return &(pln->super.super);
}

void X(regsolver_ct_directwbuf)(planner *plnr, kdftw codelet, 
				    const ct_desc *desc, int dec)
{
     S *slv = (S *)X(mksolver_ct)(sizeof(S), desc->radix, dec, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     REGISTER_SOLVER(plnr, &(slv->super.super));
     if (X(mksolver_ct_hook)) {
	  slv = (S *)X(mksolver_ct_hook)(sizeof(S), desc->radix,
					     dec, mkcldw);
	  slv->k = codelet;
	  slv->desc = desc;
	  REGISTER_SOLVER(plnr, &(slv->super.super));
     }
}
