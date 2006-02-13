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

/* $Id: hc2hc-direct.c,v 1.10 2006-02-13 12:59:06 athena Exp $ */

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
     INT r, m, vl, mstart1, mcount2;
     INT s, vs;
     stride ios;
     const R *tdW;
     twid *td;
     const S *slv;
} P;

static void apply(const plan *ego_, R *IO)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld0 = (plan_rdft *) ego->cld0;
     plan_rdft *cldm = (plan_rdft *) ego->cldm;
     INT i, r = ego->r, m = ego->m, vl = ego->vl;
     INT mstart1 = ego->mstart1, mcount2 = ego->mcount2;
     INT s = ego->s, vs = ego->vs;

     for (i = 0; i < vl; ++i, IO += vs) {
	  cld0->apply((plan *) cld0, IO, IO);
	  ego->k(IO + s * mstart1, IO + (r * m - mstart1) * s, 
		 ego->tdW, ego->ios, mcount2, s);
	  cldm->apply((plan *) cldm, IO + s*(m/2), IO + s*(m/2));
     }
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
     X(stride_destroy)(ego->ios);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const hc2hc_desc *e = slv->desc;

     p->print(p, "(hc2hc-direct-%D/%D%v \"%s\"%(%p%)%(%p%))",
              ego->r, X(twiddle_length)(ego->r, e->tw), ego->vl, e->nam,
	      ego->cld0, ego->cldm);
}

static int applicable0(const S *ego, 
		       rdft_kind kind, INT r, INT m, INT s, INT vl, INT vs, 
		       INT mstart1, INT mcount2,
		       R *IO)
{
     const hc2hc_desc *e = ego->desc;
     UNUSED(vl);

     return (
	  1

	  && r == e->radix
	  && kind == e->genus->kind

	  /* check for alignment/vector length restrictions */
	  && (e->genus->okp(e, IO + s * mstart1, IO + s * (r * m - mstart1),
			    m * s, 0, mcount2, s))
	  && (e->genus->okp(e, IO + s * mstart1 + vs, 
			    IO + s * (r * m - mstart1) + vs,
			    m * s, 0, mcount2, s))
				 
	  );
}

static int applicable(const S *ego, 
		      rdft_kind kind, INT r, INT m, INT s, INT vl, INT vs, 
		      INT mstart1, INT mcount2,
		      R *IO, const planner *plnr)
{
     if (!applicable0(ego, kind, r, m, s, vl, vs, mstart1, mcount2, IO))
          return 0;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(16, m * r, r))
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
     pln->ios = X(mkstride)(r, m * s);
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

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(vl * (((mcount2 - 1) / 2) / e->genus->vl),
		  &e->ops, &pln->super.super.ops);
     X(ops_madd2)(vl, &cld0->ops, &pln->super.super.ops);
     X(ops_madd2)(vl, &cldm->ops, &pln->super.super.ops);

     pln->super.super.could_prune_now_p = (r >= 5 && r < 64 && m >= r);

     return &(pln->super.super);
}

void X(regsolver_hc2hc_direct)(planner *plnr, khc2hc codelet,
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
