/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

/* $Id: hc2hc.c,v 1.18 2002-09-22 19:00:59 athena Exp $ */

/* generic Cooley-Tukey routines */
#include "rdft.h"
#include "hc2hc.h"

static void destroy(plan *ego_)
{
     plan_hc2hc *ego = (plan_hc2hc *) ego_;

     X(plan_destroy)(ego->cld);
     X(plan_destroy)(ego->cld0);
     X(plan_destroy)(ego->cldm);
     X(stride_destroy)(ego->ios);
     X(stride_destroy)(ego->vs);
}

static void awake(plan *ego_, int flg)
{
     plan_hc2hc *ego = (plan_hc2hc *) ego_;

     AWAKE(ego->cld, flg);
     AWAKE(ego->cld0, flg);
     AWAKE(ego->cldm, flg);

     if (flg) {
	  const tw_instr *tw = ego->slv->desc->tw;
	  X(mktwiddle)(&ego->td, tw, ego->n, ego->r, (ego->m + 1) / 2);
	  /* 0th twiddle is handled by cld0: */
	  ego->W = ego->td->W + X(twiddle_length)(ego->r, tw);
     } else {
	  X(twiddle_destroy)(&ego->td);
          ego->W = 0;
     }
}

static void print(plan *ego_, printer *p)
{
     plan_hc2hc *ego = (plan_hc2hc *) ego_;
     const solver_hc2hc *slv = ego->slv;
     const hc2hc_desc *e = slv->desc;

     p->print(p, "(%s-%u/%u%v \"%s\"%(%p%)%(%p%)%(%p%))",
              slv->nam, ego->r, X(twiddle_length)(ego->r, e->tw),
	      ego->vl, e->nam, ego->cld0, ego->cldm, ego->cld);
}

#define divides(a, b) (((uint)(b) % (uint)(a)) == 0)

int X(rdft_hc2hc_applicable)(const solver_hc2hc *ego, const problem *p_)
{
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          const hc2hc_desc *d = ego->desc;
          return (1
                  && p->sz->rnk == 1
                  && p->vecsz->rnk <= 1
		  && p->kind[0] == d->genus->kind
                  && divides(d->radix, p->sz->dims[0].n)
		  && d->radix < p->sz->dims[0].n /* avoid inf. loops in cld0 */
	       );
     }
     return 0;
}


static const plan_adt padt =
{
     X(rdft_solve),
     awake,
     print,
     destroy
};

plan *X(mkplan_rdft_hc2hc)(const solver_hc2hc *ego,
			   const problem *p_,
			   planner *plnr,
			   const hc2hcadt *adt)
{
     plan_hc2hc *pln;
     plan *cld = 0, *cld0 = 0, *cldm = 0;
     uint n, r, m;
     problem *cldp = 0, *cld0p = 0, *cldmp = 0;
     iodim *d;
     const problem_rdft *p;
     const hc2hc_desc *e = ego->desc;

     if (!adt->applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_rdft *) p_;
     d = p->sz->dims;
     n = d[0].n;
     r = e->radix;
     m = n / r;

     adt->mkcldrn(ego, p, &cldp, &cld0p, &cldmp);

     cld = X(mkplan_d)(plnr, cldp); cldp = 0;
     if (!cld) goto nada;

     cld0 = X(mkplan_d)(plnr, cld0p); cld0p = 0;
     if (!cld0) goto nada;

     cldm = X(mkplan_d)(plnr, cldmp); cldmp = 0;
     if (!cldm) goto nada;

     A(adt->pln_size >= sizeof(plan_hc2hc));
     pln = (plan_hc2hc *) X(mkplan_rdft)(adt->pln_size, &padt, adt->apply);

     pln->slv = ego;
     pln->cld = cld;
     pln->cld0 = cld0;
     pln->cldm = cldm;
     pln->k = ego->k;
     pln->n = n;
     pln->r = r;
     pln->m = m;

     pln->is = d[0].is;
     pln->os = d[0].os;

     pln->ios = pln->vs = 0;
     X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     pln->td = 0;
     adt->finish(pln);

     return &(pln->super.super);

 nada:
     X(problem_destroy)(cldmp);
     X(problem_destroy)(cld0p);
     X(problem_destroy)(cldp);
     X(plan_destroy)(cldm);
     X(plan_destroy)(cld0);
     X(plan_destroy)(cld);
     return (plan *) 0;
}

solver *X(mksolver_rdft_hc2hc)(khc2hc k, const hc2hc_desc *desc,
			       const char *nam, const solver_adt *adt)
{
     solver_hc2hc *slv;

     slv = MKSOLVER(solver_hc2hc, adt);

     slv->desc = desc;
     slv->k = k;
     slv->nam = nam;
     return &(slv->super);
}

/* routines to create children are shared by many solvers */

void X(rdft_mkcldrn_dit)(const solver_hc2hc *ego, const problem_rdft *p,
                         problem **cldp, problem **cld0p, problem **cldmp)
{
     iodim *d = p->sz->dims;
     const hc2hc_desc *e = ego->desc;
     uint m = d[0].n / e->radix;
     int omid = d[0].os * (m/2);

     tensor *null, *radix = X(mktensor_1d)(e->radix, d[0].is, m * d[0].os);
     tensor *cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);
     A(p->kind[0] == R2HC);

     *cldp = X(mkproblem_rdft_d)(X(mktensor_1d)(m, e->radix*d[0].is, d[0].os),
				 cld_vec, p->I, p->O, p->kind);

     radix = X(mktensor_1d)(e->radix, m * d[0].os, m * d[0].os);
     null = X(mktensor_0d)();
     *cld0p = X(mkproblem_rdft_1)(radix, null, p->O, p->O, R2HC);
     *cldmp = X(mkproblem_rdft_1)(m%2 ? null : radix, null,
				  p->O + omid, p->O + omid, R2HCII);
     X(tensor_destroy2)(null, radix);
}


void X(rdft_mkcldrn_dif)(const solver_hc2hc *ego, const problem_rdft *p,
                         problem **cldp, problem **cld0p, problem **cldmp)
{
     iodim *d = p->sz->dims;
     const hc2hc_desc *e = ego->desc;
     uint m = d[0].n / e->radix;
     int imid = d[0].is * (m/2);

     tensor *null, *radix = X(mktensor_1d)(e->radix, m * d[0].is, d[0].os);
     tensor *cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);
     A(p->kind[0] == HC2R);

     *cldp = X(mkproblem_rdft_d)(X(mktensor_1d)(m, d[0].is, e->radix*d[0].os),
				 cld_vec, p->I, p->O, p->kind);

     radix = X(mktensor_1d)(e->radix, m * d[0].is, m * d[0].is);
     null = X(mktensor_0d)();
     *cld0p = X(mkproblem_rdft_1)(radix, null, p->I, p->I, HC2R);
     *cldmp = X(mkproblem_rdft_1)(m%2 ? null : radix, null, 
				  p->I + imid, p->I + imid, HC2RIII);
     X(tensor_destroy2)(null, radix);
}
