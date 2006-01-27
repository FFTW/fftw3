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

/* $Id: hc2hc-directbuf.c,v 1.9 2006-01-27 02:10:50 athena Exp $ */

#include "hc2hc.h"

typedef struct {
     hc2hc_solver super;
     const hc2hc_desc *desc;
     khc2hc k;
} S;

typedef struct {
     plan_hc2hc super;
     khc2hc k;
     plan *cld0, *cldm; /* children for 0th and middle butterflies */
     INT twlen;
     INT r, m, vl, mstart1, mcount2;
     INT s, vs, ios;
     stride bufstride;
     const R *tdW;
     twid *td;
     const S *slv;
} P;

/*
   Copy A -> B, where A and B are n0 x n1 complex matrices
   such that the (i0, i1) element has index (i0 * s0 + i1 * s1).
   The imaginary strides are of opposite signs to the real strides.
*/
static void cpy(INT n0, INT n1,
                const R *rA, const R *iA, INT sa0, INT sa1,
                R *rB, R *iB, INT sb0, INT sb1)
{
     INT i0, i1;

     for (i0 = 0; i0 < n0; ++i0) {
          const R *pra, *pia;
          R *prb, *pib;
          pra = rA; rA += sa0;
          pia = iA; iA -= sa0;
          prb = rB; rB += sb0;
          pib = iB; iB -= sb0;

          for (i1 = 0; i1 < n1; ++i1) {
               R xr, xi;
               xr = *pra; pra += sa1;
               xi = *pia; pia -= sa1;
               *prb = xr; prb += sb1;
               *pib = xi; pib -= sb1;
          }
     }
}

static const R *doit(khc2hc k, R *rA, R *iA, const R *W, INT ios, INT dist,
                     INT r, INT batchsz, R *buf, stride bufstride)
{
     cpy(r, batchsz, rA, iA, ios, dist, buf, buf + 2*batchsz*r-1, 1, r);
     W = k(buf, buf + 2*batchsz*r-1, W, bufstride, 2*batchsz + 1, r);
     cpy(r, batchsz, buf, buf + 2*batchsz*r-1, 1, r, rA, iA, ios, dist);
     return W;
}

#define BATCHSZ(radix)  (((radix) + 3) & (-4))

static void apply(const plan *ego_, R *IO)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld0 = (plan_rdft *) ego->cld0;
     plan_rdft *cldm = (plan_rdft *) ego->cldm;
     INT i, j, m = ego->m, vl = ego->vl, r = ego->r;
     INT mstart1 = ego->mstart1, mcount2 = ego->mcount2;
     INT s = ego->s, vs = ego->vs, ios = ego->ios;
     INT batchsz = BATCHSZ(r);
     R *buf;

     STACK_MALLOC(R *, buf, r * batchsz * 2 * sizeof(R));

     for (i = 0; i < vl; ++i, IO += vs) {
	  R *rA, *iA;
	  const R *W;

	  cld0->apply((plan *) cld0, IO, IO);
	       
	  rA = IO + s*mstart1; iA = IO + (r * m - mstart1) * s;
	  W = ego->tdW;
	  for (j = (mcount2-1)/2; j >= batchsz; j -= batchsz) {
	       W = doit(ego->k, rA, iA, W, ios, s, r, batchsz,
			buf, ego->bufstride);
	       rA += s * batchsz;
	       iA -= s * batchsz;
	  }
	  /* do remaining j calls, if any */
	  if (j > 0)
	       doit(ego->k, rA, iA, W, ios, s, r, j, buf, ego->bufstride);

	  cldm->apply((plan *) cldm, IO + s*(m/2), IO + s*(m/2));
     }

     STACK_FREE(buf);
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;

     X(plan_awake)(ego->cld0, wakefulness);
     X(plan_awake)(ego->cldm, wakefulness);
     X(twiddle_awake)(wakefulness, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, (ego->m + 1) / 2);
     ego->tdW = X(twiddle_shift)(ego->td, ego->mstart1);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld0);
     X(plan_destroy_internal)(ego->cldm);
     X(stride_destroy)(ego->bufstride);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const hc2hc_desc *e = slv->desc;
     INT batchsz = BATCHSZ(ego->r);

     p->print(p, "(hc2hc-directbuf/%D-%D/%D%v \"%s\"%(%p%)%(%p%))",
	      batchsz, ego->r, X(twiddle_length)(ego->r, e->tw), 
	      ego->vl, e->nam, ego->cld0, ego->cldm);
}

static int applicable0(const S *ego, 
		       rdft_kind kind, INT r, INT m, INT s, INT vl, INT vs, 
		       INT mstart1, INT mcount2,
		       R *IO)
{
     const hc2hc_desc *e = ego->desc;
     INT batchsz;
     INT mc = (mcount2 - 1) / 2;
     UNUSED(vl); UNUSED(s); UNUSED(vs); UNUSED(IO); UNUSED(mstart1);

     return (
	  1

	  && r == e->radix
	  && kind == e->genus->kind

	  /* check for alignment/vector length restrictions */
	  && (batchsz = BATCHSZ(r), 1)
	  && (m < batchsz ||
	      (e->genus->okp(e, 0, ((const R *)0) + 2*batchsz*r - 1, 
			     1, 0, 2*batchsz + 1, r)))
	  && (m < batchsz ||
	      (e->genus->okp(e, 0, ((const R *)0) + 2*(mc%batchsz)*r - 1,
			     1, 0, 2*(mc%batchsz) + 1, r)))
				 
	  );
}

static int applicable(const S *ego, 
		      rdft_kind kind, INT r, INT m, INT s, INT vl, INT vs, 
		      INT mstart1, INT mcount2,
		      R *IO, const planner *plnr)
{
     if (!applicable0(ego, kind, r, m, s, vl, vs, mstart1, mcount2, IO))
          return 0;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(512, m * r, r)) 
	  return 0;

     return 1;
}

static plan *mkcldw(const hc2hc_solver *ego_, 
		    rdft_kind kind, INT r, INT m, INT s, INT vl, INT vs, 
		    INT mstart, INT mcount,
		    R *IO, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const hc2hc_desc *e = ego->desc;
     plan *cld0, *cldm;
     INT mstart1, mcount2;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     mstart1 = mstart + (mstart == 0);
     mcount2 = 1 + 2 * (mcount - (mstart==0)
			- (m%2 == 0 && mstart+mcount == (m+2)/2));

     if (!applicable(ego, kind, r, m, s, vl, vs, mstart1, mcount2, IO, plnr))
          return (plan *)0;

     if (!X(hc2hc_mkcldrn)(kind, r, m, s, mstart, mcount,
			     IO, plnr, &cld0, &cldm))
          return (plan *)0;
	  
     pln = MKPLAN_HC2HC(P, &padt, apply);

     pln->k = ego->k;
     pln->ios = m * s;
     pln->td = 0;
     pln->tdW = 0;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->vl = vl;
     pln->vs = vs;
     pln->slv = ego;
     pln->cld0 = cld0;
     pln->cldm = cldm;
     pln->mstart1 = mstart1;
     pln->mcount2 = mcount2;
     pln->bufstride = X(mkstride)(r, 1);

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(vl * (((mcount2 - 1) / 2) / e->genus->vl),
		  &e->ops, &pln->super.super.ops);
     X(ops_madd2)(vl, &cld0->ops, &pln->super.super.ops);
     X(ops_madd2)(vl, &cldm->ops, &pln->super.super.ops);
     pln->super.super.ops.other += 4 * r * ((mcount2 - 1)/2) * vl;

     return &(pln->super.super);
}

void X(regsolver_hc2hc_directbuf)(planner *plnr, khc2hc codelet,
					 const hc2hc_desc *desc)
{
     S *slv = (S *)X(mksolver_hc2hc)(sizeof(S), desc->radix, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     REGISTER_SOLVER(plnr, &(slv->super.super));
     if (X(mksolver_hc2hc_hook)) {
	  slv = (S *)X(mksolver_hc2hc_hook)(sizeof(S), desc->radix, mkcldw);
	  slv->k = codelet;
	  slv->desc = desc;
	  REGISTER_SOLVER(plnr, &(slv->super.super));
     }
}
