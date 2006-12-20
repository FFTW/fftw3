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
     INT r, rs;
     INT m, ms;
     INT v, vs;
     INT mstart1, mcount2;
     stride bufstride;
     const R *tdW;
     twid *td;
     const S *slv;
} P;

static const R *doit(khc2hc k, R *Ap, R *Am, const R *W, INT rs, INT ms,
                     INT r, INT batchsz, R *bufp, stride bufstride)
{
     INT b = WS(bufstride, 1);
     R *bufm = bufp + b - 1;

     X(cpy2d_ci)(Ap, bufp, r, rs, b, batchsz,  ms,  1, 1);
     X(cpy2d_ci)(Am, bufm, r, rs, b, batchsz, -ms, -1, 1);

     W = k(bufp, bufm, W, bufstride, batchsz, 1);

     X(cpy2d_co)(bufp, Ap, r, b, rs, batchsz,  1,  ms, 1);
     X(cpy2d_co)(bufm, Am, r, b, rs, batchsz, -1, -ms, 1);

     return W;
}

#define BATCHSZ(radix)  (((radix) + 3) & (-4))

static void apply(const plan *ego_, R *IO)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld0 = (plan_rdft *) ego->cld0;
     plan_rdft *cldm = (plan_rdft *) ego->cldm;
     INT i, j, m = ego->m, v = ego->v, r = ego->r;
     INT mstart1 = ego->mstart1, mcount2 = ego->mcount2;
     INT ms = ego->ms, vs = ego->vs, rs = ego->rs;
     INT batchsz = BATCHSZ(r);
     R *buf;

     STACK_MALLOC(R *, buf, r * batchsz * 2 * sizeof(R));

     for (i = 0; i < v; ++i, IO += vs) {
	  R *rA, *iA;
	  const R *W;

	  cld0->apply((plan *) cld0, IO, IO);
	       
	  rA = IO + ms*mstart1; iA = IO + (m - mstart1) * ms;
	  W = ego->tdW;
	  for (j = mcount2; j >= batchsz; j -= batchsz) {
	       W = doit(ego->k, rA, iA, W, rs, ms, r, batchsz, 
			buf, ego->bufstride);
	       rA += ms * batchsz;
	       iA -= ms * batchsz;
	  }
	  /* do remaining j calls, if any */
	  if (j > 0)
	       doit(ego->k, rA, iA, W, rs, ms, r, j, buf, ego->bufstride);

	  cldm->apply((plan *) cldm, IO + ms*(m/2), IO + ms*(m/2));
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
	      ego->v, e->nam, ego->cld0, ego->cldm);
}

static int applicable0(const S *ego, rdft_kind kind, INT r)
{
     const hc2hc_desc *e = ego->desc;

     return (
	  1

	  && r == e->radix
	  && kind == e->genus->kind
				 
	  );
}

static int applicable(const S *ego, rdft_kind kind, INT r, INT m,
		      const planner *plnr)
{
     if (!applicable0(ego, kind, r))
          return 0;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(512, m * r, r)) 
	  return 0;

     return 1;
}

static plan *mkcldw(const hc2hc_solver *ego_, 
		    rdft_kind kind, INT r, INT m, INT ms, INT v, INT vs, 
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
     mcount2 = mcount - (mstart==0) - (m%2 == 0 && mstart+mcount == (m+2)/2);

     if (!applicable(ego, kind, r, m, plnr))
          return (plan *)0;

     if (!X(hc2hc_mkcldrn)(kind, r, m, ms, mstart, mcount, 
			   IO, plnr, &cld0, &cldm))
          return (plan *)0;
	  
     pln = MKPLAN_HC2HC(P, &padt, apply);

     pln->k = ego->k;
     pln->rs = m * ms;
     pln->td = 0;
     pln->tdW = 0;
     pln->r = r;
     pln->m = m;
     pln->ms = ms;
     pln->v = v;
     pln->vs = vs;
     pln->slv = ego;
     pln->cld0 = cld0;
     pln->cldm = cldm;
     pln->mstart1 = mstart1;
     pln->mcount2 = mcount2;
     pln->bufstride = X(mkstride)(r, 2 * BATCHSZ(r));

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(v * (mcount2 / e->genus->vl),
		  &e->ops, &pln->super.super.ops);
     X(ops_madd2)(v, &cld0->ops, &pln->super.super.ops);
     X(ops_madd2)(v, &cldm->ops, &pln->super.super.ops);
     pln->super.super.ops.other += 4 * r * mcount2 * v;

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
