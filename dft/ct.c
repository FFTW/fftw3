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

/* $Id: ct.c,v 1.30 2003-01-15 11:51:34 athena Exp $ */

/* generic Cooley-Tukey routines */
#include "dft.h"
#include "ct.h"

static void destroy(plan *ego_)
{
     plan_ct *ego = (plan_ct *) ego_;

     X(plan_destroy_internal)(ego->cld);
     X(stride_destroy)(ego->ios);
     X(stride_destroy)(ego->vs);
}

static void awake(plan *ego_, int flg)
{
     plan_ct *ego = (plan_ct *) ego_;
     plan *cld = ego->cld;

     AWAKE(cld, flg);
     X(twiddle_awake)(flg, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, ego->m);
}

static void print(plan *ego_, printer *p)
{
     plan_ct *ego = (plan_ct *) ego_;
     const solver_ct *slv = ego->slv;
     const ct_desc *e = slv->desc;

     p->print(p, "(%s-%d/%d%v \"%s\"%(%p%))",
              slv->nam, ego->r, X(twiddle_length)(ego->r, e->tw),
	      ego->vl, e->nam, ego->cld);
}

#define divides(a, b) (((int)(b) % (int)(a)) == 0)

int X(dft_ct_applicable)(const solver_ct *ego, const problem *p_)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          const ct_desc *d = ego->desc;
          return (1
                  && p->sz->rnk == 1
                  && p->vecsz->rnk <= 1
                  && divides(d->radix, p->sz->dims[0].n)
	       );
     }
     return 0;
}


static const plan_adt padt =
{
     X(dft_solve),
     awake,
     print,
     destroy
};


plan *X(mkplan_dft_ct)(const solver_ct *ego,
                       const problem *p_,
                       planner *plnr,
                       const ctadt *adt)
{
     plan_ct *pln;
     plan *cld;
     int n, r, m;
     iodim *d;
     const problem_dft *p;
     const ct_desc *e = ego->desc;

     if (!adt->applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_dft *) p_;
     d = p->sz->dims;
     n = d[0].n;
     r = e->radix;
     m = n / r;

     cld = X(mkplan_d)(plnr, adt->mkcld(ego, p));

     if (!cld)
          return (plan *) 0;

     A(adt->pln_size >= sizeof(plan_ct));
     pln = (plan_ct *) X(mkplan_dft)(adt->pln_size, &padt, adt->apply);

     pln->slv = ego;
     pln->cld = cld;
     pln->k = ego->k;
     pln->r = r;
     pln->m = m;

     pln->is = d[0].is;
     pln->os = d[0].os;

     pln->ios = pln->vs = 0;
     X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);

     pln->td = 0;
     adt->finish(pln);

     return &(pln->super.super);
}

solver *X(mksolver_dft_ct)(union kct k, const ct_desc *desc,
                           const char *nam, const solver_adt *adt)
{
     solver_ct *slv;

     slv = MKSOLVER(solver_ct, adt);

     slv->desc = desc;
     slv->k = k;
     slv->nam = nam;
     return &(slv->super);
}

/* routines to create children are shared by many solvers */
problem *X(dft_mkcld_dit)(const solver_ct *ego, const problem_dft *p)
{
     iodim *d = p->sz->dims;
     const ct_desc *e = ego->desc;
     int m = d[0].n / e->radix;

     tensor *radix = X(mktensor_1d)(e->radix, d[0].is, m * d[0].os);
     tensor *cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);

     return X(mkproblem_dft_d)(X(mktensor_1d)(m, e->radix * d[0].is, d[0].os),
			       cld_vec, p->ri, p->ii, p->ro, p->io);
}

problem *X(dft_mkcld_dif)(const solver_ct *ego, const problem_dft *p)
{
     iodim *d = p->sz->dims;
     const ct_desc *e = ego->desc;
     int m = d[0].n / e->radix;

     tensor *radix = X(mktensor_1d)(e->radix, m * d[0].is, d[0].os);
     tensor *cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);

     return X(mkproblem_dft_d)(X(mktensor_1d)(m, d[0].is, e->radix * d[0].os),
			       cld_vec, p->ri, p->ii, p->ro, p->io);
}
