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

/* $Id: indirect.c,v 1.17 2002-09-21 21:47:35 athena Exp $ */


/* solvers/plans for vectors of small RDFT's that cannot be done
   in-place directly.  Use a rank-0 plan to rearrange the data
   before or after the transform.  Can also change an out-of-place
   plan into a copy + in-place (where the in-place transform
   is e.g. unit stride). */

#include "rdft.h"

typedef problem *(*mkcld_t) (const problem_rdft *p);

typedef struct {
     rdftapply apply;
     problem *(*mkcld)(const problem_rdft *p);
     const char *nam;
} ndrct_adt;

typedef struct {
     solver super;
     const ndrct_adt *adt;
} S;

typedef struct {
     plan_rdft super;
     plan *cldcpy, *cld;
     const S *slv;
} P;

/*-----------------------------------------------------------------------*/
/* first rearrange, then transform */
static void apply_before(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;

     {
          plan_rdft *cldcpy = (plan_rdft *) ego->cldcpy;
          cldcpy->apply(ego->cldcpy, I, O);
     }
     {
          plan_rdft *cld = (plan_rdft *) ego->cld;
          cld->apply(ego->cld, O, O);
     }
}

static problem *mkcld_before(const problem_rdft *p)
{
     tensor v, s;
     v = X(tensor_copy_inplace)(&p->vecsz, INPLACE_OS);
     s = X(tensor_copy_inplace)(&p->sz, INPLACE_OS);
     return X(mkproblem_rdft_d)(s, v, p->O, p->O, p->kind);
}

static const ndrct_adt adt_before =
{
     apply_before, mkcld_before, "rdft-indirect-before"
};

/*-----------------------------------------------------------------------*/
/* first transform, then rearrange */

static void apply_after(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;

     {
          plan_rdft *cld = (plan_rdft *) ego->cld;
          cld->apply(ego->cld, I, I);
     }
     {
          plan_rdft *cldcpy = (plan_rdft *) ego->cldcpy;
          cldcpy->apply(ego->cldcpy, I, O);
     }
}

static problem *mkcld_after(const problem_rdft *p)
{
     tensor v, s;
     v = X(tensor_copy_inplace)(&p->vecsz, INPLACE_IS);
     s = X(tensor_copy_inplace)(&p->sz, INPLACE_IS);
     return X(mkproblem_rdft_d)(s, v, p->I, p->I, p->kind);
}

static const ndrct_adt adt_after =
{
     apply_after, mkcld_after, "rdft-indirect-after"
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

static int applicable0(const solver *ego_, const problem *p_,
		       const planner *plnr)
{
     if (RDFTP(p_)) {
	  const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
                  && FINITE_RNK(p->vecsz.rnk)

                  /* problem must be a nontrivial transform, not just a copy */
                  && p->sz.rnk > 0

                  && (0

		      /* problem must be in-place & require some
		         rearrangement of the data */
		      || (p->I == p->O
			  && !(X(tensor_inplace_strides)(&p->sz)
			       && X(tensor_inplace_strides)(&p->vecsz)))

		      /* or problem must be out of place, transforming
			 from stride 1/2 to bigger stride, for apply_after */
		      || (p->I != p->O && ego->adt->apply == apply_after
			  && DESTROY_INPUTP(plnr)
			  && X(tensor_min_istride)(&p->sz) <= 2
			  && X(tensor_min_ostride)(&p->sz) > 2)
			  
		      /* or problem must be out of place, transforming
			 to stride 1/2 from bigger stride, for apply_before */
		      || (p->I != p->O && ego->adt->apply == apply_before
			  && X(tensor_min_ostride)(&p->sz) <= 2
			  && X(tensor_min_istride)(&p->sz) > 2)
			  
		       )
	       );
     }

     return 0;
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     if (!applicable0(ego_, p_, plnr)) return 0;
	  
     if (NO_INDIRECT_OP_P(plnr)) {
	  const problem_rdft *p = (const problem_rdft *)p_;
	  if (p->I != p->O) return 0;
     }

     return 1;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const problem_rdft *p = (const problem_rdft *) p_;
     const S *ego = (const S *) ego_;
     P *pln;
     problem *cldp;
     plan *cld = 0, *cldcpy = 0;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *) 0;

     plnr->planner_flags |= NO_BUFFERING;

     {
	  tensor sz_real = X(rdft_real_sz)(p->kind, &p->sz);
	  cldp = X(mkproblem_rdft_d)(X(mktensor)(0),
				     X(tensor_append)(&p->vecsz, &sz_real),
				     p->I, p->O, (rdft_kind *) 0);
	  X(tensor_destroy)(&sz_real);
     }
     cldcpy = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldcpy)
          goto nada;

     cldp = ego->adt->mkcld(p);
     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld)
          goto nada;

     pln = MKPLAN_RDFT(P, &padt, ego->adt->apply);
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
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(rdft_indirect_register)(planner *p)
{
     uint i;
     static const ndrct_adt *const adts[] = {
	  &adt_before, &adt_after
     };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
          REGISTER_SOLVER(p, mksolver(adts[i]));
}
