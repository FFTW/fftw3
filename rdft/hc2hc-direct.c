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

/* $Id: hc2hc-direct.c,v 1.1 2004-03-21 17:38:45 athena Exp $ */

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
     R *W;
     twid *td;
     int r, m, vl;
     int s, vs;
     stride ios;
     const S *slv;
} P;

static void apply(const plan *ego_, R *IO)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld0 = (plan_rdft *) ego->cld0;
     plan_rdft *cldm = (plan_rdft *) ego->cldm;
     int i, r = ego->r, m = ego->m, vl = ego->vl;
     int s = ego->s, vs = ego->vs;

     for (i = 0; i < vl; ++i, IO += vs) {
	  cld0->apply((plan *) cld0, IO, IO);
	  ego->k(IO + s, IO + (r * m - 1) * s, ego->W, ego->ios, m, s);
	  cldm->apply((plan *) cldm, IO + s*(m/2), IO + s*(m/2));
     }
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld0, flg);
     AWAKE(ego->cldm, flg);

     if (flg) {
	  const tw_instr *tw = ego->slv->desc->tw;
	  X(mktwiddle)(&ego->td, tw, ego->r * ego->m, ego->r, 
		       (ego->m + 1) / 2);
	  /* 0th twiddle is handled by cld0: */
	  ego->W = ego->td->W + X(twiddle_length)(ego->r, tw);
     } else {
	  X(twiddle_destroy)(&ego->td);
          ego->W = 0;
     }
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

     p->print(p, "(hc2hc-direct-%d/%d%v \"%s\"%(%p%)%(%p%))",
              ego->r, X(twiddle_length)(ego->r, e->tw), ego->vl, e->nam,
	      ego->cld0, ego->cldm);
}

static int applicable0(const S *ego, 
		       rdft_kind kind, int r, int m, int s, int vl, int vs, 
		       R *IO)
{
     const hc2hc_desc *e = ego->desc;
     UNUSED(vl);

     return (
	  1

	  && r == e->radix
	  && kind == e->genus->kind

	  /* check for alignment/vector length restrictions */
	  && (e->genus->okp(e, IO + s, IO + s * (r * m - 1),
			    m * s, 0, m, s))
	  && (e->genus->okp(e, IO + s + vs, IO + s * (r * m - 1) + vs,
			    m * s, 0, m, s))
				 
	  );
}

static int applicable(const S *ego, 
		      rdft_kind kind, int r, int m, int s, int vl, int vs, 
		      R *IO, const planner *plnr)
{
     if (!applicable0(ego, kind, r, m, s, vl, vs, IO))
          return 0;

     if (NO_UGLYP(plnr)) {
	  if (X(ct_uglyp)(16, m * r, r)) return 0;
	  if (NONTHREADED_ICKYP(plnr))
	       return 0; /* prefer threaded version */
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

     if (!X(rdft_ct_mkcldrn)(kind, r, m, s, IO, plnr,
			     &cld0, &cldm))
          return (plan *)0;
	  
     pln = MKPLAN_HC2HC(P, &padt, apply);

     pln->k = ego->k;
     pln->ios = X(mkstride)(r, m * s);
     pln->td = 0;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->vl = vl;
     pln->vs = vs;
     pln->slv = ego;
     pln->cld0 = cld0;
     pln->cldm = cldm;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(vl * (m / e->genus->vl), &e->ops, &pln->super.super.ops);

     return &(pln->super.super);
}

solver *X(mksolver_rdft_hc2hc_direct)(khc2hc codelet,
				      const hc2hc_desc *desc)
{
     S *slv = (S *)X(mksolver_rdft_ct)(sizeof(S), desc->radix, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     return &(slv->super.super);
}
