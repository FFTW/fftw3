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
     khc2c k;
} S;

typedef struct {
     plan_hc2c super;
     khc2c k;
     plan *cld0, *cldm; /* children for 0th and middle butterflies */
     INT r, m, v;
     INT ms, vs;
     stride rs;
     const R *tdW;
     twid *td;
     const S *slv;
} P;

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
		 ego->tdW, ego->rs, m, ms);
	  cldm->apply((plan *) cldm, cr + (m/2)*ms, ci + (m/2)*ms, 
		      cr + (m/2)*ms, ci + (m/2)*ms);
     }
}

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
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const hc2c_desc *e = slv->desc;

     p->print(p, "(hc2c-direct-%D/%D%v \"%s\"%(%p%)%(%p%))",
              ego->r, X(twiddle_length)(ego->r, e->tw), ego->v, e->nam,
	      ego->cld0, ego->cldm);
}

static int applicable0(const S *ego, rdft_kind kind,
		       INT r, INT rs,
		       INT m, INT ms, 
		       INT v, INT vs,
		       R *cr, R *ci)
{
     const hc2c_desc *e = ego->desc;
     UNUSED(v);
     return (
	  1

	  && r == e->radix
	  && kind == e->genus->kind
	  
	  /* check for alignment/vector length restrictions */
	  && (e->genus->okp(e, 
			    cr + ms, ci + ms, 
			    cr + (m - 1) * ms, ci + (m - 1) * ms,
			    rs, 0, m, ms))

	  /* v-loop iteration */
	  && ((cr += vs), (ci += vs), 1)

	  && (e->genus->okp(e, 
			    cr + ms, ci + ms, 
			    cr + (m - 1) * ms, ci + (m - 1) * ms,
			    rs, 0, m, ms))
	  );
}

static int applicable(const S *ego, rdft_kind kind,
		      INT r, INT rs,
		      INT m, INT ms, 
		      INT v, INT vs,
		      R *cr, R *ci, const planner *plnr)
{
     if (!applicable0(ego, kind, r, rs, m, ms, v, vs, cr, ci))
          return 0;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(16, m * r, r))
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

     if (!applicable(ego, kind, r, rs, m, ms, v, vs, cr, ci, plnr))
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

     pln = MKPLAN_HC2C(P, &padt, apply);

     pln->k = ego->k;
     pln->td = 0;
     pln->tdW = 0;
     pln->r = r; pln->rs = X(mkstride)(r, rs);
     pln->m = m; pln->ms = ms;
     pln->v = v; pln->vs = vs;
     pln->slv = ego;
     pln->cld0 = cld0;
     pln->cldm = cldm;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(v * (((m - 1) / 2) / e->genus->vl),
		  &e->ops, &pln->super.super.ops);
     X(ops_madd2)(v, &cld0->ops, &pln->super.super.ops);
     X(ops_madd2)(v, &cldm->ops, &pln->super.super.ops);

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld0);
     X(plan_destroy_internal)(cldm);
     return 0;
}

void X(regsolver_hc2c_direct)(planner *plnr, khc2c codelet,
			      const hc2c_desc *desc)
{
     S *slv = (S *)X(mksolver_hc2c)(sizeof(S), desc->radix, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     REGISTER_SOLVER(plnr, &(slv->super.super));
}
