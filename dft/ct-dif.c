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

/* $Id: ct-dif.c,v 1.7 2002-06-11 14:35:52 athena Exp $ */

/* decimation in time Cooley-Tukey */
#include "dft.h"
#include "ct.h"

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     plan_ct *ego = (plan_ct *) ego_;
     plan *cld0 = ego->cld;
     plan_dft *cld = (plan_dft *) cld0;

     {
          uint i, m = ego->m, vl = ego->vl;
          int is = ego->is, ivs = ego->ivs;

          for (i = 0; i < vl; ++i)
               ego->k.dif(ri + i * ivs, ii + i * ivs, ego->W, ego->ios, m, is);
     }

     /* two-dimensional r x vl sub-transform: */
     cld->apply(cld0, ri, ii, ro, io);
}

static int applicable(const solver_ct *ego, const problem *p_)
{
     if (X(dft_ct_applicable)(ego, p_)) {
          const ct_desc *e = ego->desc;
          const problem_dft *p = (const problem_dft *) p_;
          iodim *d = p->sz.dims;

          return (1
                  /* DIF destroys the input and we don't like it */
                  && p->ri == p->ro

                  /* if hardwired strides, test whether they match */
                  && (!e->is || e->is == (int)(d[0].n / e->radix) * d[0].is)
	       );
     }
     return 0;
}

static void finish(plan_ct *ego)
{
     ego->ios = X(mkstride)(ego->r, ego->m * ego->is);
     ego->super.super.ops =
          X(ops_add)(ego->cld->ops,
		     X(ops_mul)(ego->vl * ego->m, ego->slv->desc->ops));
}

static problem *mkcld(const solver_ct *ego, const problem_dft *p)
{
     iodim *d = p->sz.dims;
     const ct_desc *e = ego->desc;
     uint m = d[0].n / e->radix;

     tensor radix = X(mktensor_1d)(e->radix, m * d[0].is, d[0].os);
     tensor cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);

     return X(mkproblem_dft_d)(X(mktensor_1d)(m, d[0].is, e->radix * d[0].os),
			       cld_vec, p->ri, p->ii, p->ro, p->io);
}

static int score(const solver *ego_, const problem *p_)
{
     const solver_ct *ego = (const solver_ct *) ego_;
     const problem_dft *p = (const problem_dft *) p_;
     uint n;

     if (!applicable(ego, p_))
          return BAD;

     n = p->sz.dims[0].n;
     if (n <= 16 || n / ego->desc->radix <= 4)
          return UGLY;

     return GOOD;
}


static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     static const ctadt adt = {
	  mkcld, finish, applicable, apply
     };
     return X(mkplan_dft_ct)((const solver_ct *) ego, p, plnr, &adt);
}


solver *X(mksolver_dft_ct_dif)(kdft_dif codelet, const ct_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     static const char name[] = "dft-dif";
     union kct k;
     k.dif = codelet;

     return X(mksolver_dft_ct)(k, desc, name, &sadt);
}
