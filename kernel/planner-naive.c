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

/* $Id: planner-naive.c,v 1.15 2002-08-30 16:09:48 athena Exp $ */
#include "ifftw.h"

/* naive planner with no memoization */

static void mkplan(planner *ego, problem *p, plan **bestp, slvpair **pairp)
{
     plan *best = 0;

     /* if a solver is suggested (by wisdom) use it */
     if (*pairp) {
	  slvpair *sp = *pairp;
	  solver *s = sp->slv;
	  *bestp = ego->adt->slv_mkplan(ego, p, s);
	  return;
     }

     FORALL_SOLVERS(ego, s, sp, {
	  plan *pln = ego->adt->slv_mkplan(ego, p, s);

	  if (pln) {
	       X(plan_use)(pln);
	       X(evaluate_plan)(ego, pln, p);
	       if (best) {
		    if (pln->pcost < best->pcost) {
			 X(plan_destroy)(best);
			 best = pln;
			 *pairp = sp;
		    } else {
			 X(plan_destroy)(pln);
		    }
	       } else {
		    best = pln;
		    *pairp = sp;
	       }
	  }
     });

     *bestp = best;
}

/* constructor */
planner *X(mkplanner_naive)(int flags)
{
     return X(mkplanner)(sizeof(planner), mkplan, 0, flags);
}
