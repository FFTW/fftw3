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

/* $Id: indirect.c,v 1.3 2002-08-05 18:17:58 stevenj Exp $ */


/* solvers/plans for vectors of small RDFT's that cannot be done
   in-place directly.  Use a rank-0 plan to rearrange the data
   before or after the transform. */

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

     UNUSED(O); /* input == output */

     {
          plan_rdft *cldcpy = (plan_rdft *) ego->cldcpy;
          cldcpy->apply(ego->cldcpy, I, I);
     }
     {
          plan_rdft *cld = (plan_rdft *) ego->cld;
          cld->apply(ego->cld, I, I);
     }
}

static problem *mkcld_before(const problem_rdft *p)
{
     uint i;
     tensor v, s;
     v = X(tensor_copy)(p->vecsz);
     for (i = 0; i < v.rnk; ++i)
          v.dims[i].is = v.dims[i].os;
     s = X(tensor_copy)(p->sz);
     for (i = 0; i < s.rnk; ++i)
          s.dims[i].is = s.dims[i].os;
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

     UNUSED(O);	/* input == output */
     {
          plan_rdft *cld = (plan_rdft *) ego->cld;
          cld->apply(ego->cld, I, I);
     }
     {
          plan_rdft *cldcpy = (plan_rdft *) ego->cldcpy;
          cldcpy->apply(ego->cldcpy, I, I);
     }
}

static problem *mkcld_after(const problem_rdft *p)
{
     uint i;
     tensor v, s;
     v = X(tensor_copy)(p->vecsz);
     for (i = 0; i < v.rnk; ++i)
          v.dims[i].os = v.dims[i].is;
     s = X(tensor_copy)(p->sz);
     for (i = 0; i < s.rnk; ++i)
          s.dims[i].os = s.dims[i].is;
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

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
                  && FINITE_RNK(p->vecsz.rnk)

                  /* problem must be in-place */
                  && p->I == p->O

                  /* problem must be a nontrivial transform, not just a copy */
                  && p->sz.rnk > 0

                  /* problem must require some rearrangement of data */
                  && !(X(tensor_inplace_strides)(p->sz)
		       && X(tensor_inplace_strides)(p->vecsz))
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p, const planner *plnr)
{
     UNUSED(plnr);
     return (applicable(ego, p)) ? GOOD : BAD;
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

     if (!applicable(ego_, p_))
          return (plan *) 0;

     {
	  tensor sz_real = X(rdft_real_sz)(p->kind, p->sz);
	  cldp = X(mkproblem_rdft_d)(X(mktensor)(0),
				     X(tensor_append)(p->vecsz, sz_real),
				     p->I, p->I, R2HC);
	  X(tensor_destroy)(sz_real);
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
     static const solver_adt sadt = { mkplan, score };
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
