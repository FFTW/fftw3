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

/* $Id: indirect.c,v 1.17 2002-08-31 13:55:57 athena Exp $ */


/* solvers/plans for vectors of small DFT's that cannot be done
   in-place directly.  Use a rank-0 plan to rearrange the data
   before or after the transform.  Can also change an out-of-place
   plan into a copy + in-place (where the in-place transform
   is e.g. unit stride). */

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
     plan *cldcpy, *cld;
     const S *slv;
} P;

/*-----------------------------------------------------------------------*/
/* first rearrange, then transform */
static void apply_before(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;

     {
          plan_dft *cldcpy = (plan_dft *) ego->cldcpy;
          cldcpy->apply(ego->cldcpy, ri, ii, ro, io);
     }
     {
          plan_dft *cld = (plan_dft *) ego->cld;
          cld->apply(ego->cld, ro, io, ro, io);
     }
}

static problem *mkcld_before(const problem_dft *p)
{
     tensor v, s;
     v = X(tensor_copy_inplace)(p->vecsz, INPLACE_OS);
     s = X(tensor_copy_inplace)(p->sz, INPLACE_OS);
     return X(mkproblem_dft_d)(s, v, p->ro, p->io, p->ro, p->io);
}

static const ndrct_adt adt_before =
{
     apply_before, mkcld_before, "dft-indirect-before"
};

/*-----------------------------------------------------------------------*/
/* first transform, then rearrange */

static void apply_after(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;

     {
          plan_dft *cld = (plan_dft *) ego->cld;
          cld->apply(ego->cld, ri, ii, ri, ii);
     }
     {
          plan_dft *cldcpy = (plan_dft *) ego->cldcpy;
          cldcpy->apply(ego->cldcpy, ri, ii, ro, io);
     }
}

static problem *mkcld_after(const problem_dft *p)
{
     tensor v, s;
     v = X(tensor_copy_inplace)(p->vecsz, INPLACE_IS);
     s = X(tensor_copy_inplace)(p->sz, INPLACE_IS);
     return X(mkproblem_dft_d)(s, v, p->ri, p->ii, p->ri, p->ii);
}

static const ndrct_adt adt_after =
{
     apply_after, mkcld_after, "dft-indirect-after"
};

/*-----------------------------------------------------------------------*/
static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cld);
     X(plan_destroy)(ego->cldcpy);
     X(free)(ego);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cldcpy, flg);
     AWAKE(ego->cld, flg);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     const S *s = ego->slv;
     p->print(p, "(%s%(%p%)%(%p%))", s->adt->nam, ego->cld, ego->cldcpy);
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     if (DFTP(p_)) {
	  const S *ego = (const S *) ego_;
          const problem_dft *p = (const problem_dft *) p_;
          return (1
                  && FINITE_RNK(p->vecsz.rnk)

                  /* problem must be a nontrivial transform, not just a copy */
                  && p->sz.rnk > 0

                  && (0

		      /* problem must be in-place & require some
		         rearrangement of the data */
		      || (p->ri == p->ro
			  && !(X(tensor_inplace_strides)(p->sz)
			       && X(tensor_inplace_strides)(p->vecsz)))

		      /* or problem must be out of place, transforming
			 from stride 1/2 to bigger stride, for apply_after */
		      || (p->ri != p->ro && ego->adt->apply == apply_after
			  && (plnr->flags & DESTROY_INPUT)
			  && X(tensor_min_istride)(p->sz) <= 2
			  && X(tensor_min_ostride)(p->sz) > 2)
			  
		      /* or problem must be out of place, transforming
			 to stride 1/2 from bigger stride, for apply_before */
		      || (p->ri != p->ro && ego->adt->apply == apply_before
			  && X(tensor_min_ostride)(p->sz) <= 2
			  && X(tensor_min_istride)(p->sz) > 2)
		       )
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p, const planner *plnr)
{
     return (applicable(ego, p, plnr)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     const S *ego = (const S *) ego_;
     P *pln;
     problem *cldp;
     plan *cld = 0, *cldcpy = 0;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *) 0;

     cldp = X(mkproblem_dft_d)(X(mktensor)(0),
                               X(tensor_append)(p->vecsz, p->sz),
                               p->ri, p->ii, p->ro, p->io);
     cldcpy = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldcpy)
          goto nada;

     cldp = ego->adt->mkcld(p);
     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld)
          goto nada;

     pln = MKPLAN_DFT(P, &padt, ego->adt->apply);
     pln->cld = cld;
     pln->cldcpy = cldcpy;
     pln->slv = ego;
     pln->super.super.ops = X(ops_add)(cld->ops, cldcpy->ops);

     return &(pln->super.super);

 nada:
     if (cld)
          X(plan_destroy)(cld);
     if (cldcpy)
          X(plan_destroy)(cldcpy);
     return (plan *)0;
}

static solver *mksolver(const ndrct_adt *adt)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(dft_indirect_register)(planner *p)
{
     uint i;
     static const ndrct_adt *const adts[] = {
	  &adt_before, &adt_after
     };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
          REGISTER_SOLVER(p, mksolver(adts[i]));
}
