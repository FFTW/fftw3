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

/* $Id: hc2hc-directbuf.c,v 1.2 2004-03-22 14:04:37 athena Exp $ */

#include "ct.h"

typedef struct {
     ct_solver super;
     const hc2hc_desc *desc;
     khc2hc k;
} S;

typedef struct {
     plan_hc2hc super;
     khc2hc k;
     plan *cld0, *cldm; /* children for 0th and middle butterflies */
     twid *td;
     int twlen;
     int r, m, vl;
     int s, vs, ios;
     stride bufstride;
     const S *slv;
} P;

/*
   Copy A -> B, where A and B are n0 x n1 complex matrices
   such that the (i0, i1) element has index (i0 * s0 + i1 * s1).
   The imaginary strides are of opposite signs to the real strides.
*/
static void cpy(int n0, int n1,
                const R *rA, const R *iA, int sa0, int sa1,
                R *rB, R *iB, int sb0, int sb1)
{
     int i0, i1;

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

static const R *doit(khc2hc k, R *rA, R *iA, const R *W, int ios, int dist,
                     int r, int batchsz, R *buf, stride bufstride)
{
     cpy(r, batchsz, rA, iA, ios, dist, buf, buf + 2*batchsz*r-1, 1, r);
     W = k(buf, buf + 2*batchsz*r-1, W, bufstride, 2*batchsz + 1, r);
     cpy(r, batchsz, buf, buf + 2*batchsz*r-1, 1, r, rA, iA, ios, dist);
     return W;
}

#define BATCHSZ 4 /* FIXME: parametrize? */

static void apply(const plan *ego_, R *IO)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld0 = (plan_rdft *) ego->cld0;
     plan_rdft *cldm = (plan_rdft *) ego->cldm;
     int i, j, m = ego->m, vl = ego->vl, r = ego->r;
     int s = ego->s, vs = ego->vs, ios = ego->ios;
     R *buf;

     STACK_MALLOC(R *, buf, r * BATCHSZ * 2 * sizeof(R));

     for (i = 0; i < vl; ++i, IO += vs) {
	  R *rA, *iA;
	  const R *W;

	  cld0->apply((plan *) cld0, IO, IO);
	       
	  rA = IO + s; iA = IO + (r * m - 1) * s;
	  W = ego->td->W + ego->twlen;
	  for (j = (m-1)/2; j >= BATCHSZ; j -= BATCHSZ) {
	       W = doit(ego->k, rA, iA, W, ios, s, r, BATCHSZ,
			buf, ego->bufstride);
	       rA += s * (int)BATCHSZ;
	       iA -= s * (int)BATCHSZ;
	  }
	  /* do remaining j calls, if any */
	  if (j > 0)
	       doit(ego->k, rA, iA, W, ios, s, r, j, buf, ego->bufstride);

	  cldm->apply((plan *) cldm, IO + s*(m/2), IO + s*(m/2));
     }

     STACK_FREE(buf);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld0, flg);
     AWAKE(ego->cldm, flg);
     X(twiddle_awake)(flg, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, (ego->m + 1) / 2);
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

     p->print(p, "(hc2hc-directbuf-%d/%d%v \"%s\"%(%p%)%(%p%))",
              ego->r, X(twiddle_length)(ego->r, e->tw), ego->vl, e->nam,
	      ego->cld0, ego->cldm);
}

static int applicable0(const S *ego, 
		       rdft_kind kind, int r, int m, int s, int vl, int vs, 
		       R *IO)
{
     const hc2hc_desc *e = ego->desc;
     UNUSED(vl); UNUSED(s); UNUSED(vs); UNUSED(IO);

     return (
	  1

	  && r == e->radix
	  && kind == e->genus->kind

	  /* check for alignment/vector length restrictions */
	  && (m < BATCHSZ ||
	      (e->genus->okp(e, 0, ((const R *)0)+2*BATCHSZ*r-1, 
			     1, 0, 2*BATCHSZ + 1, r)))
	  && (m < BATCHSZ ||
	      (e->genus->okp(e, 0, ((const R *)0) + 2*(((m-1)/2)%BATCHSZ)*r-1,
			     1, 0, 2*(((m-1)/2) % BATCHSZ) + 1, r)))
				 
	  );
}

static int applicable(const S *ego, 
		      rdft_kind kind, int r, int m, int s, int vl, int vs, 
		      R *IO, const planner *plnr)
{
     if (!applicable0(ego, kind, r, m, s, vl, vs, IO))
          return 0;

     if (NO_UGLYP(plnr)) {
	  if (X(ct_uglyp)(512, m * r, r)) return 0;
     }

     return 1;
}

static plan *mkcldw(const ct_solver *ego_, 
		    rdft_kind kind, int r, int m, int s, int vl, int vs, 
		    R *IO, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const hc2hc_desc *e = ego->desc;
     plan *cld0, *cldm;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };


     if (!applicable(ego, kind, r, m, s, vl, vs, IO, plnr))
          return (plan *)0;

     if (!X(rdft_ct_mkcldrn)(kind, r, m, s, IO, plnr, &cld0, &cldm))
          return (plan *)0;
	  
     pln = MKPLAN_HC2HC(P, &padt, apply);

     pln->k = ego->k;
     pln->ios = m * s;
     pln->td = 0;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->vl = vl;
     pln->vs = vs;
     pln->slv = ego;
     pln->cld0 = cld0;
     pln->cldm = cldm;
     pln->twlen = X(twiddle_length)(r, e->tw);
     pln->bufstride = X(mkstride)(r, 1);

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(vl * (((m - 1) / 2) / e->genus->vl),
		  &e->ops, &pln->super.super.ops);
     X(ops_madd2)(vl, &cld0->ops, &pln->super.super.ops);
     X(ops_madd2)(vl, &cldm->ops, &pln->super.super.ops);
     pln->super.super.ops.other += 4 * r * ((m - 1)/2) * vl;

     return &(pln->super.super);
}

solver *X(mksolver_rdft_hc2hc_directbuf)(khc2hc codelet,
					 const hc2hc_desc *desc)
{
     S *slv = (S *)X(mksolver_rdft_ct)(sizeof(S), desc->radix, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     return &(slv->super.super);
}
