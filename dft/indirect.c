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

/* $Id: indirect.c,v 1.2 2002-06-08 19:11:09 athena Exp $ */


/* solvers/plans for vectors of small DFT's that cannot be done
   in-place directly.  Use a rank-0 plan to rearrange the data
   before or after the transform. */

#include "dft.h"

typedef problem *(*mkcld_t) (const problem_dft *p);

typedef struct {
     dftapply apply;
     problem *(*mkcld)(const problem_dft *p);
     const char *nam;
} ndrct_adt;

typedef struct {
     solver super;
     const ndrct_adt *adt;
} S;

typedef struct {
     plan_dft super;
     plan *cld_copy, *cld;
     const S *slv;
} P;

/*-----------------------------------------------------------------------*/
/* first rearrange, then transform */
static void apply_before(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;

     UNUSED(ro); UNUSED(io); /* input == output */

     {
	  plan_dft *cld_copy = (plan_dft *) ego->cld_copy;
	  cld_copy->apply(ego->cld_copy, ri, ii, ri, ii);
     }
     {
	  plan_dft *cld = (plan_dft *) ego->cld;
	  cld->apply(ego->cld, ri, ii, ri, ii);
     }
}

static problem *mkcld_before(const problem_dft *p)
{
     uint i;
     tensor v, s;
     v = fftw_tensor_copy(p->vecsz);
     for (i = 0; i < v.rnk; ++i) v.dims[i].is = v.dims[i].os;
     s = fftw_tensor_copy(p->sz);
     for (i = 0; i < s.rnk; ++i) s.dims[i].is = s.dims[i].os;
     return fftw_mkproblem_dft_d(s, v, p->ro, p->io, p->ro, p->io);
}

static const ndrct_adt adt_before = { 
     apply_before, mkcld_before, "DFT-INDIRECT-BEFORE"
};

/*-----------------------------------------------------------------------*/
/* first transform, then rearrange */

static void apply_after(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;

     UNUSED(ro);
     UNUSED(io);		/* input == output */
     {
	  plan_dft *cld = (plan_dft *) ego->cld;
	  cld->apply(ego->cld, ri, ii, ri, ii);
     }
     {
	  plan_dft *cld_copy = (plan_dft *) ego->cld_copy;
	  cld_copy->apply(ego->cld_copy, ri, ii, ri, ii);
     }
}

static problem *mkcld_after(const problem_dft *p)
{
     uint i;
     tensor v, s;
     v = fftw_tensor_copy(p->vecsz);
     for (i = 0; i < v.rnk; ++i) v.dims[i].os = v.dims[i].is;
     s = fftw_tensor_copy(p->sz);
     for (i = 0; i < s.rnk; ++i) s.dims[i].os = s.dims[i].is;
     return fftw_mkproblem_dft_d(s, v, p->ri, p->ii, p->ri, p->ii);
}

static const ndrct_adt adt_after = { 
     apply_after, mkcld_after, "DFT-INDIRECT-AFTER"
};

/*-----------------------------------------------------------------------*/
static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     fftw_plan_destroy(ego->cld);
     fftw_plan_destroy(ego->cld_copy);
     fftw_free(ego);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     ego->cld_copy->adt->awake(ego->cld_copy, flg);
     ego->cld->adt->awake(ego->cld, flg);
}

static void print(plan *ego_, plan_printf prntf)
{
     P *ego = (P *) ego_;
     const S *s = ego->slv;
     prntf("(%s ", s->adt->nam);
     ego->cld->adt->print(ego->cld, prntf);
     prntf(" ");
     ego->cld_copy->adt->print(ego->cld_copy, prntf);
     prntf(")");
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (DFTP(p_)) {
	  const problem_dft *p = (const problem_dft *) p_;
	  return (1

		  /* problem must be in-place */
		  && p->ri == p->ro

		  /* problem must be a nontrivial transform, not just a copy */
		  && p->sz.rnk > 0

		  /* problem must require some rearrangement of data */
		  && !fftw_tensor_inplace_strides(p->sz)
		  && !fftw_tensor_inplace_strides(p->vecsz)
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p)
{
     return (applicable(ego, p)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     const S *ego = (const S *) ego_;
     P *pln;
     problem *cldp;
     plan *cld = 0, *cld_copy = 0;

     static const plan_adt padt = { 
	  fftw_dft_solve, awake, print, destroy 
     };

     if (!applicable(ego_, p_))
	  return (plan *) 0;

     cldp = fftw_mkproblem_dft_d(fftw_mktensor(0),
				 fftw_tensor_append(p->vecsz, p->sz),
				 p->ri, p->ii, p->ri, p->ii);
     cld_copy = plnr->adt->mkplan(plnr, cldp);
     fftw_problem_destroy(cldp);
     if (!cld_copy) goto nada;

     cldp = ego->adt->mkcld(p);
     cld = plnr->adt->mkplan(plnr, cldp);
     fftw_problem_destroy(cldp);
     if (!cld) goto nada;

     pln = MKPLAN_DFT(P, &padt, ego->adt->apply);
     pln->cld = cld;
     pln->cld_copy = cld_copy;
     pln->slv = ego;
     pln->super.super.cost = cld->cost + cld_copy->cost;
     pln->super.super.flops = cld->flops;

     return &(pln->super.super);

 nada:
     if (cld) fftw_plan_destroy(cld);
     if (cld_copy) fftw_plan_destroy(cld_copy);
     return (plan *)0;
}

static solver *mksolver(const ndrct_adt *adt)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void fftw_dft_indirect_register(planner *p)
{
     uint i;
     static const ndrct_adt *adts[] = { &adt_before, &adt_after };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
	  p->adt->register_solver(p, mksolver(adts[i]));
}
