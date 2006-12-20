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


#include "ct-hc2c.h"

typedef struct {
     hc2c_solver super;
     const hc2c_desc *desc;
     int bufferedp;
     khc2c k;
} S;

typedef struct {
     plan_hc2c super;
     khc2c k;
     plan *cld0, *cldm; /* children for 0th and middle butterflies */
     INT r, m, v;
     INT ms, vs;
     stride rs, bufstride;
     const R *tdW;
     twid *td;
     const S *slv;
} P;

/*************************************************************
  Nonbuffered code
 *************************************************************/
static void apply(const plan *ego_, R *cr, R *ci)
{
     const P *ego = (const P *) ego_;
     plan_rdft2 *cld0 = (plan_rdft2 *) ego->cld0;
     plan_rdft2 *cldm = (plan_rdft2 *) ego->cldm;
     INT i, m = ego->m, v = ego->v;
     INT ms = ego->ms, vs = ego->vs;

     for (i = 0; i < v; ++i, cr += vs, ci += vs) {
	  cld0->apply((plan *) cld0, cr, ci, cr, ci);
	  ego->k(cr + ms, ci + ms, cr + (m-1)*ms, ci + (m-1)*ms,
		 ego->tdW, ego->rs, (m-1)/2, ms);
	  cldm->apply((plan *) cldm, cr + (m/2)*ms, ci + (m/2)*ms, 
		      cr + (m/2)*ms, ci + (m/2)*ms);
     }
}

/*************************************************************
  Buffered code
 *************************************************************/

/* should not be 2^k to avoid associativity conflicts */
static INT compute_batchsize(INT radix)
{
     radix /= 2;

     /* round up to multiple of 4 */
     radix += 3;
     radix &= -4;

     return (radix + 2);
}

static const R *dobatch(khc2c k, R *Rp, R *Ip, R *Rm, R *Im, const R *W, 
			INT r, INT rs,
			INT batchsz, INT ms, 
			R *bufp, stride bufstride)
{
     INT b = WS(bufstride, 1);
     R *bufm = bufp + b - 2;

     X(cpy2d_pair_ci)(Rp, Ip, bufp, bufp + 1,
		      r, rs, b,
		      batchsz, ms, 2);
     X(cpy2d_pair_ci)(Rm, Im, bufm, bufm + 1,
		      r, rs, b,
		      batchsz, -ms, -2);
     W = k(bufp, bufp + 1, bufm, bufm + 1, W, bufstride, batchsz, 1);
     X(cpy2d_pair_co)(bufp, bufp + 1, Rp, Ip, 
		      r, b, rs,
		      batchsz, 2, ms);
     X(cpy2d_pair_co)(bufm, bufm + 1, Rm, Im,
		      r, b, rs,
		      batchsz, -2, -ms);
     return W;
}

static void apply_buf(const plan *ego_, R *cr, R *ci)
{
     const P *ego = (const P *) ego_;
     plan_rdft2 *cld0 = (plan_rdft2 *) ego->cld0;
     plan_rdft2 *cldm = (plan_rdft2 *) ego->cldm;
     INT i, j, m = ego->m, v = ego->v;
     INT batchsz = compute_batchsize(ego->r);
     INT b;
     R *buf;
     INT m1 = (m-1)/2;

     STACK_MALLOC(R *, buf, ego->r * batchsz * 4 * sizeof(R));

     for (i = 0; i < v; ++i) {
	  R *Rp = cr + i * ego->vs;
	  R *Ip = ci + i * ego->vs;
	  R *Rm = Rp + m * ego->ms;
	  R *Im = Ip + m * ego->ms;
	  const R *W = ego->tdW;

	  cld0->apply((plan *) cld0, Rp, Ip, Rp, Ip);
	  Rp += ego->ms; Ip += ego->ms; Rm -= ego->ms; Im -= ego->ms; 

	  for (j = 0; j < m1 - batchsz; j += batchsz) {
	       W = dobatch(ego->k, Rp, Ip, Rm, Im, W, 
			   ego->r, WS(ego->rs, 1), batchsz, ego->ms, 
			   buf, ego->bufstride);
	       Rp += ego->ms * batchsz; Ip += ego->ms * batchsz;
	       Rm -= ego->ms * batchsz; Im -= ego->ms * batchsz;
	  }

	  b = m1 - j;
	  dobatch(ego->k, Rp, Ip, Rm, Im, W, 
		  ego->r, WS(ego->rs, 1), b, ego->ms, buf, ego->bufstride);
	  Rp += ego->ms * b; Ip += ego->ms * b;
	  Rm -= ego->ms * b; Im -= ego->ms * b;

	  cldm->apply((plan *) cldm, Rp, Ip, Rp, Ip);
     }

     STACK_FREE(buf);
}

/*************************************************************
  common code
 *************************************************************/
static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;

     X(plan_awake)(ego->cld0, wakefulness);
     X(plan_awake)(ego->cldm, wakefulness);
     X(twiddle_awake)(wakefulness, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, (ego->m + 1) / 2);
     ego->tdW = X(twiddle_shift)(ego->td, 1);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld0);
     X(plan_destroy_internal)(ego->cldm);
     X(stride_destroy)(ego->rs);
     X(stride_destroy)(ego->bufstride);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const hc2c_desc *e = slv->desc;

     if (slv->bufferedp)
	  p->print(p, "(hc2c-directbuf/%D-%D/%D%v \"%s\"%(%p%)%(%p%))",
		   compute_batchsize(ego->r), ego->r,
		   X(twiddle_length)(ego->r, e->tw), ego->v, e->nam,
		   ego->cld0, ego->cldm);
     else
	  p->print(p, "(hc2c-direct-%D/%D%v \"%s\"%(%p%)%(%p%))",
		   ego->r, X(twiddle_length)(ego->r, e->tw), ego->v, e->nam,
		   ego->cld0, ego->cldm);
}

static int applicable0(const S *ego, rdft_kind kind, INT r)
{
     const hc2c_desc *e = ego->desc;

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

     if (NO_UGLYP(plnr) && X(ct_uglyp)((ego->bufferedp? (INT)512 : (INT)16),
				       m * r, r))
	  return 0;

     return 1;
}

static plan *mkcldw(const hc2c_solver *ego_, rdft_kind kind,
		    INT r, INT rs,
		    INT m, INT ms, 
		    INT v, INT vs,
		    R *cr, R *ci,
		    planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const hc2c_desc *e = ego->desc;
     plan *cld0 = 0, *cldm = 0;
     INT imid = (m / 2) * ms;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     if (!applicable(ego, kind, r, m, plnr))
          return (plan *)0;

     cld0 = X(mkplan_d)(
	  plnr, 
	  X(mkproblem_rdft2_d)(X(mktensor_1d)(r, rs, rs),
			       X(mktensor_0d)(),
			       cr, ci, cr, ci, kind));
     if (!cld0) goto nada;

     cldm = X(mkplan_d)(
	  plnr, 
	  X(mkproblem_rdft2_d)(((m % 2) ?
				X(mktensor_0d)() : X(mktensor_1d)(r, rs, rs) ),
			       X(mktensor_0d)(),
			       cr + imid, ci + imid, cr + imid, ci + imid,
			       kind == R2HC ? R2HCII : HC2RIII));
     if (!cldm) goto nada;

     pln = MKPLAN_HC2C(P, &padt, ego->bufferedp ? apply_buf : apply);

     pln->k = ego->k;
     pln->td = 0;
     pln->tdW = 0;
     pln->r = r; pln->rs = X(mkstride)(r, rs);
     pln->m = m; pln->ms = ms;
     pln->v = v; pln->vs = vs;
     pln->slv = ego;
     pln->bufstride = X(mkstride)(r, 4 * compute_batchsize(r));
     pln->cld0 = cld0;
     pln->cldm = cldm;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(v * (((m - 1) / 2) / e->genus->vl),
		  &e->ops, &pln->super.super.ops);
     X(ops_madd2)(v, &cld0->ops, &pln->super.super.ops);
     X(ops_madd2)(v, &cldm->ops, &pln->super.super.ops);

     if (ego->bufferedp) {
	  /* 8 load/stores * N * V */
	  pln->super.super.ops.other += 8 * r * m * v;
     }

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld0);
     X(plan_destroy_internal)(cldm);
     return 0;
}

static void regone(planner *plnr, khc2c codelet,
		   const hc2c_desc *desc, 
		   hc2c_kind hc2ckind, 
		   int bufferedp)
{
     S *slv = (S *)X(mksolver_hc2c)(sizeof(S), desc->radix, hc2ckind, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     slv->bufferedp = bufferedp;
     REGISTER_SOLVER(plnr, &(slv->super.super));
}

void X(regsolver_hc2c_direct)(planner *plnr, khc2c codelet,
			      const hc2c_desc *desc,
			      hc2c_kind hc2ckind)
{
     regone(plnr, codelet, desc, hc2ckind, /* bufferedp */0);
     regone(plnr, codelet, desc, hc2ckind, /* bufferedp */1);
}
