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

/* $Id: nop2.c,v 1.3 2002-08-12 17:31:37 stevenj Exp $ */

/* plans for vrank -infty RDFT2s (nothing to do) */

#include "rdft.h"

static void apply(plan *ego_, R *r, R *rio, R *iio)
{
     UNUSED(ego_);
     UNUSED(r);
     UNUSED(rio);
     UNUSED(iio);
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFT2P(p_)) {
          const problem_rdft2 *p = (const problem_rdft2 *) p_;
          return(0
		 /* case 1 : -infty vector rank */
		 || (p->vecsz.rnk == RNK_MINFTY)
		 
		 /* case 2 : rank-0 in-place rdft */
		 || (1
		     && p->sz.rnk == 0
		     && FINITE_RNK(p->vecsz.rnk)
		     && (p->r == p->rio || p->r == p->iio)
		     && X(rdft2_inplace_strides)(p, RNK_MINFTY)
		      ));
     }
     return 0;
}

static void destroy(plan *ego)
{
     X(free)(ego);
}

static void print(plan *ego, printer *p)
{
     UNUSED(ego);
     p->print(p, "(rdft2-nop)");
}

static int score(const solver *ego, const problem *p, const planner *plnr)
{
     UNUSED(plnr);
     return applicable(ego, p) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     static const plan_adt padt = {
	  X(rdft2_solve), X(null_awake), print, destroy
     };
     plan_rdft2 *pln;

     UNUSED(plnr);

     if (!applicable(ego, p))
          return (plan *) 0;
     pln = MKPLAN_RDFT2(plan_rdft2, &padt, apply);
     pln->super.ops = X(ops_zero);

     return &(pln->super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     return MKSOLVER(solver, &sadt);
}

void X(rdft2_nop_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
