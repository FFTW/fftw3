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

/* $Id: planner-naive.c,v 1.1 2002-06-03 22:10:12 athena Exp $ */
#include "ifftw.h"

/* naive planner with no memoization */

static plan *mkplan(planner *ego, problem *p)
{
     plan *best = (plan *) 0;
     double best_time = 1.0e30;
     double this_time;

     FORALL_SOLVERS(ego, s, {
	  plan *pln = s->adt->mkplan(s, p, ego);

	  if (pln) {
	       fftw_plan_use(pln);
	       ego->ntry++;
	       ego->hook(pln, p);
	       this_time = fftw_measure_execution_time(pln, p, 0);
	       if (this_time < best_time) {
		    if (best)
			 fftw_plan_destroy(best);
		    best = pln;
		    best_time = this_time;
	       } else {
		    fftw_plan_destroy(pln);
	       }
	  }
     });

     return best;
}

/* constructor */
planner *fftw_mkplanner_naive(void)
{
     return fftw_mkplanner(sizeof(planner), mkplan, 0);
}
