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

/* $Id: rank-geq2-rdft2.c,v 1.1 2002-08-14 07:26:06 stevenj Exp $ */

/* plans for RDFT2 of rank >= 2 (multidimensional) */

#include "rdft.h"
#include "dft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_dft super;
     plan *cldr, *cldc;
} P;

static void apply_r2hc(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;

     {
	  plan_rdft2 *cldr = (plan_rdft2 *) ego->cldr;
	  cldr->apply((plan *) cldr, r, rio, iio);
     }
     
     {
	  plan_dft *cldc = (plan_dft *) ego->cldc;
	  cldc->apply((plan *) cldc, rio, iio, rio, iio);
     }
}

static void apply_hc2r(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;

     {
	  plan_dft *cldc = (plan_dft *) ego->cldc;
	  cldc->apply((plan *) cldc, rio, iio, rio, iio);
     }

     {
	  plan_rdft2 *cldr = (plan_rdft2 *) ego->cldr;
	  cldr->apply((plan *) cldr, r, rio, iio);
     }
     
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cldr, flg);
     AWAKE(ego->cldc, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cldr);
     X(plan_destroy)(ego->cldc);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(rdft2-rank>=2%(%p%)%(%p%))", ego->cldr, ego->cldc);
}
 
static int applicable(const solver *ego_, const problem *p_, 
		      const planner *plnr)
{
     if (RDFT2P(p_)) {
          const problem_rdft2 *p = (const problem_rdft2 *) p_;
          const S *ego = (const S *)ego_;
          return (1
                  && p->sz.rnk >= 2
                  && (0

		      /* can work out-of-place, but HC2R destroys input */
                      || (p->r != p->rio && p->r != p->iio && 
			  (p->kind == R2HC || (plnr->flags & DESTROY_INPUT)))

		      /* FIXME: what are sufficient conditions for inplace? */
                      || (!(p->r != p->rio && p->r != p->iio) &&
			  X(rdft2_inplace_strides)(p, RNK_MINFTY))
		       )
	       );
     }

     return 0;
}

/* TODO: revise this. */
static int score(const solver *ego_, const problem *p_, const planner *plnr)
{
     const S *ego = (const S *)ego_;
     const problem_rdft2 *p = (const problem_rdft2 *) p_;

     if (!applicable(ego_, p_, plnr))
          return BAD;

     /* Heuristic: if the vector stride is greater than the transform
        sz, don't use (prefer to do the vector loop first with a
        vrank-geq1 plan). */
     if (p->vecsz.rnk > 0 &&
	 X(tensor_min_stride)(p->vecsz) > X(tensor_max_index)(p->sz))
          return UGLY;

     return GOOD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_rdft2 *p;
     P *pln;
     plan *cldr = 0, *cldc = 0;
     tensor sz1, sz2, vecszi, sz2i;
     inplace_kind k;
     problem *cldp;
     uint i;

     static const plan_adt padt = {
	  X(rdft2_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *) 0;

     p = (const problem_rdft2 *) p_;
     X(tensor_split)(p->sz, &sz1, p->sz.rnk - 1, &sz2);

     k = p->kind == R2HC ? INPLACE_OS : INPLACE_IS;
     vecszi = X(tensor_copy_inplace)(p->vecsz, k);
     sz2i = X(tensor_copy_inplace)(sz2, k);
     sz2i.dims[0].n = sz2i.dims[0].n/2 + 1; /* complex data is ~half of real */

     cldp = X(mkproblem_rdft2_d)(X(tensor_copy)(sz2),
				 X(tensor_append)(p->vecsz, sz1),
				 p->r, p->rio, p->iio, p->kind);
     cldr = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldr)
          goto nada;

     cldp = X(mkproblem_dft_d)(X(tensor_copy_inplace)(sz1, k),
                               X(tensor_append)(vecszi, sz2i),
                               p->rio, p->iio, p->rio, p->iio);
     cldc = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldc)
          goto nada;

     pln = MKPLAN_RDFT2(P, &padt, p->kind == R2HC ? apply_r2hc : apply_hc2r);

     pln->cldr = cldr;
     pln->cldc = cldc;

     pln->super.super.ops = X(ops_add)(cldr->ops, cldc->ops);

     X(tensor_destroy)(sz2i);
     X(tensor_destroy)(vecszi);
     X(tensor_destroy)(sz2);
     X(tensor_destroy)(sz1);

     return &(pln->super.super);

 nada:
     if (cldr)
          X(plan_destroy)(cldr);
     if (cldc)
          X(plan_destroy)(cldc);
     X(tensor_destroy)(sz2i);
     X(tensor_destroy)(vecszi);
     X(tensor_destroy)(sz2);
     X(tensor_destroy)(sz1);
     return (plan *) 0;
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(rdft2_rank_geq2_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
