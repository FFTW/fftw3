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

/* $Id: rank-geq2.c,v 1.6 2002-06-11 11:32:20 athena Exp $ */

/* plans for DFT of rank >= 2 (multidimensional) */

#include "dft.h"

typedef struct {
     solver super;
     int spltrnk;
     const int *buddies;
     uint nbuddies;
} S;

typedef struct {
     plan_dft super;

     plan *cld1, *cld2;
     const S *solver;
} P;

/* Compute multi-dimensional DFT by applying the two cld plans
   (lower-rnk DFTs). */
static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     plan_dft *cld1, *cld2;

     cld1 = (plan_dft *) ego->cld1;
     cld1->apply(ego->cld1, ri, ii, ro, io);

     cld2 = (plan_dft *) ego->cld2;
     cld2->apply(ego->cld2, ro, io, ro, io);
}


static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld1, flg);
     AWAKE(ego->cld2, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cld2);
     X(plan_destroy)(ego->cld1);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     const S *s = ego->solver;
     p->print(p, "(dft-rank>=2/%d%(%p%)%(%p%))",
	      s->spltrnk, ego->cld1, ego->cld2);
}

/* Given a solver spltrnk and a vector rnk, return the actual
   rank that it corresponds to.  (This returned RANK is the rank of
   the first sub-transform, comprising the first rank dimensions.) */
static uint really_picksplit(int spltrnk, uint rnk)
{
     A(rnk > 1);		/* must be at least 2d to split */
     if (spltrnk > 0)	/* positive: split from the front */
          return X(uimin)(rnk - 1, (uint)spltrnk);
     else if (spltrnk < 0)	/* negative: split from the back */
          return X(uimax)(1, rnk + (uint)spltrnk);
     else			/* zero: split in the middle */
          return (rnk / 2);
}

static int picksplit(const S *ego, uint rnk, uint *rp)
{
     uint i, r0;
     r0 = really_picksplit(ego->spltrnk, rnk);

     /* check whether some buddy solver would produce the same split.
        If so, consider this solver unapplicable and let the buddy
        take care of it.  The smallest buddy is applicable. */
     for (i = 0; i < ego->nbuddies; ++i)
          if (1
	      && ego->buddies[i] < ego->spltrnk
	      && really_picksplit(ego->buddies[i], rnk) == r0
	       )
               return 0;

     *rp = r0;
     return 1;
}

static int applicable(const solver *ego_, const problem *p_, uint *rp)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          const S *ego = (const S *)ego_;
          return (1
                  && p->sz.rnk >= 2
                  && picksplit(ego, p->sz.rnk, rp)
                  && (0

                      /* can always operate out-of-place */
                      || p->ri != p->ro

                      /* Can operate in-place as long as all dimension
			 strides are the same, provided that the cld
			 plans work in-place.  (This condition is
			 sufficient, but is it necessary?) */
                      || X(tensor_inplace_strides)(p->sz)
		       )
	       );
     }

     return 0;
}

/* TODO: revise this. */
static int score(const solver *ego_, const problem *p_)
{
     const problem_dft *p = (const problem_dft *) p_;
     const S *ego = (const S *) ego_;
     uint dummy;

     if (!applicable(ego_, p_, &dummy))
          return BAD;

     /* Prefer spltrnk values within the dimension range.  Also
        prefer positive spltrnk to negative (assuming that they
        come in positive/negative pairs.  We do this to discourage the
        planner from trying effectively duplicate plans. */
     if ((ego->spltrnk > 0 && ego->spltrnk >= (int)p->sz.rnk) ||
	 (ego->spltrnk < 0 && -2 * ego->spltrnk >= (int)p->sz.rnk))
          return UGLY;

     /* Heuristic: if the vector stride is greater than the transform
        sz, don't use (prefer to do the vector loop first with a
        vecloop plan). */
     if (p->vecsz.rnk > 0 &&
	 X(tensor_min_stride)(p->vecsz) > X(tensor_max_index)(p->sz))
          return UGLY;

     return GOOD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p;
     P *pln;
     plan *cld1 = 0, *cld2 = 0;
     tensor sz1, sz2;
     problem *cldp;
     uint spltrnk;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, &spltrnk))
          return (plan *) 0;

     p = (const problem_dft *) p_;
     X(tensor_split)(p->sz, &sz1, spltrnk, &sz2);

     cldp = X(mkproblem_dft_d)(X(tensor_copy)(sz2),
                               X(tensor_append)(p->vecsz, sz1),
                               p->ri, p->ii, p->ro, p->io);
     cld1 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld1)
          goto nada;

     cldp = X(mkproblem_dft_d)(X(tensor_copy)(sz1),
                               X(tensor_append)(p->vecsz, sz2),
                               p->ro, p->io, p->ro, p->io);
     cld2 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld2)
          goto nada;

     pln = MKPLAN_DFT(P, &padt, apply);

     pln->cld1 = cld1;
     pln->cld2 = cld2;

     pln->solver = ego;
     pln->super.super.cost = cld1->cost + cld2->cost;
     pln->super.super.flops = X(flops_add)(cld1->flops, cld2->flops);

     X(tensor_destroy)(sz1);
     X(tensor_destroy)(sz2);

     return &(pln->super.super);

 nada:
     if (cld2)
          X(plan_destroy)(cld2);
     if (cld1)
          X(plan_destroy)(cld1);
     X(tensor_destroy)(sz2);
     X(tensor_destroy)(sz1);
     return (plan *) 0;
}

static solver *mksolver(int spltrnk, const int *buddies, uint nbuddies)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->spltrnk = spltrnk;
     slv->buddies = buddies;
     slv->nbuddies = nbuddies;
     return &(slv->super);
}

void X(dft_rank_geq2_register)(planner *p)
{
     uint i;
     static const int buddies[] = { 1, -1, 0 };
     const uint nbuddies = sizeof(buddies) / sizeof(buddies[0]);

     for (i = 0; i < nbuddies; ++i)
          REGISTER_SOLVER(p, mksolver(buddies[i], buddies, nbuddies));

     /* FIXME:

     Should we try more buddies? 

     Another possible variant is to swap cld1 and cld2 (or
     rather, to swap their problems; they are not interchangeable
     because cld2 must be in-place).  In past versions of FFTW,
     however, I seem to recall that such rearrangements have made
     little or no difference. 
     */
}
