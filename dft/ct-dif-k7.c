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

/* $Id: ct-dif-k7.c,v 1.2 2002-06-18 21:48:41 athena Exp $ */

/* decimation in time Cooley-Tukey */
#include "dft.h"
#include "ct.h"

static void applyr(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     plan_ct *ego = (plan_ct *) ego_;
     plan *cld0 = ego->cld;
     plan_dft *cld = (plan_dft *) cld0;

     {
          uint i, m = ego->m, vl = ego->vl;
          int is = ego->is, ivs = ego->ivs;

          for (i = 0; i < vl; ++i)
               ego->k.dif_k7(ri + i * ivs, ego->W, ego->iios, m, is);
     }

     /* two-dimensional r x vl sub-transform: */
     cld->apply(cld0, ri, ii, ro, io);
}

static void applyi(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     plan_ct *ego = (plan_ct *) ego_;
     plan *cld0 = ego->cld;
     plan_dft *cld = (plan_dft *) cld0;

     {
          uint i, m = ego->m, vl = ego->vl;
          int is = ego->is, ivs = ego->ivs;

          for (i = 0; i < vl; ++i)
               ego->k.dif_k7(ii + i * ivs, ego->W, ego->iios, m, is);
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

		  /* check pointers */
		  && (p->ri - p->ii == e->sign)
		  && (p->ro - p->io == e->sign)

                  /* if hardwired strides, test whether they match */
                  && (!e->is || e->is == (int)(d[0].n / e->radix) * d[0].is)
	       );
     }
     return 0;
}

static void finish(plan_ct *ego)
{
     ego->iios = ego->m * ego->is;
     ego->super.super.ops =
          X(ops_add)(ego->cld->ops,
		     X(ops_mul)(ego->vl * ego->m, ego->slv->desc->ops));
}

static int score(const solver *ego_, const problem *p_, int flags)
{
     const solver_ct *ego = (const solver_ct *) ego_;
     const problem_dft *p = (const problem_dft *) p_;
     uint n;
     UNUSED(flags);
     
     if (!applicable(ego, p_))
          return BAD;

     n = p->sz.dims[0].n;
     if (n <= 16 || n / ego->desc->radix <= 4)
          return UGLY;

     return GOOD;
}


static plan *mkplan(const solver *ego_, const problem *p, planner *plnr)
{
     const solver_ct *ego = (const solver_ct *)ego_;
     static const ctadt adtr = { X(dft_mkcld_dif), finish, applicable, applyr };
     static const ctadt adti = { X(dft_mkcld_dif), finish, applicable, applyi };

     return X(mkplan_dft_ct)(ego, p, plnr, 
			     ego->desc->sign == 1 ? &adti : &adtr);
}

solver *X(mksolver_dft_ct_dif_k7)(kdft_dif_k7 codelet, const ct_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     static const char name[] = "dft-dif-k7";
     union kct k;
     k.dif_k7 = codelet;

     return X(mksolver_dft_ct)(k, desc, name, &sadt);
}
