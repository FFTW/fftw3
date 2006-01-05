/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* $Id: nop2.c,v 1.12 2006-01-05 03:04:27 stevenj Exp $ */

/* plans for vrank -infty RDFT2s (nothing to do), as well as in-place
   rank-0 HC2R.  Note that in-place rank-0 R2HC is *not* a no-op, because
   we have to set the imaginary parts of the output to zero. */

#include "rdft.h"

static void apply(const plan *ego_, R *r, R *rio, R *iio)
{
     UNUSED(ego_);
     UNUSED(r);
     UNUSED(rio);
     UNUSED(iio);
}

static int applicable(const solver *ego_, const problem *p_)
{
     const problem_rdft2 *p = (const problem_rdft2 *) p_;
     UNUSED(ego_);

     return(0
	    /* case 1 : -infty vector rank */
	    || (p->vecsz->rnk == RNK_MINFTY)
		 
	    /* case 2 : rank-0 in-place HC2R rdft */
	    || (1
		&& p->kind == HC2R
		&& p->sz->rnk == 0
		&& FINITE_RNK(p->vecsz->rnk)
		&& (p->r == p->rio || p->r == p->iio)
		&& X(rdft2_inplace_strides)(p, RNK_MINFTY)
		 ));
}

static void print(const plan *ego, printer *p)
{
     UNUSED(ego);
     p->print(p, "(rdft2-nop)");
}

static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     static const plan_adt padt = {
	  X(rdft2_solve), X(null_awake), print, X(plan_null_destroy)
     };
     plan_rdft2 *pln;

     UNUSED(plnr);

     if (!applicable(ego, p))
          return (plan *) 0;
     pln = MKPLAN_RDFT2(plan_rdft2, &padt, apply);
     X(ops_zero)(&pln->super.ops);

     return &(pln->super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_RDFT2, mkplan };
     return MKSOLVER(solver, &sadt);
}

void X(rdft2_nop_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
