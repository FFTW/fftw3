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

/* $Id: planner-score.c,v 1.1 2002-06-08 19:11:09 athena Exp $ */
#include "ifftw.h"

static plan *mkplan(planner *ego, problem *p)
{
     plan *best = 0;
     int best_score;

     best_score = BAD;
     FORALL_SOLVERS(ego, s, {
	  int sc = s->adt->score(s, p);
	  if (sc > best_score)
	       best_score = sc;
     });

     for (; best_score > BAD; --best_score) {
	  FORALL_SOLVERS(ego, s, {
	       int sc = s->adt->score(s, p);
	       if (sc == best_score) {
		    plan *pln = s->adt->mkplan(s, p, ego);

		    if (pln) {
			 fftw_plan_use(pln);
			 ego->ntry++;
			 ego->hook(pln, p);
			 pln->cost = fftw_measure_execution_time(pln, p);
			 if (best) {
			      if (pln->cost < best->cost) {
				   fftw_plan_destroy(best);
				   best = pln;
			      } else {
				   fftw_plan_destroy(pln);
			      }
			 } else {
			      best = pln;
			 }
		    }
	       }
	  });
	  if (best) break;
     };

     return best;
}

/* constructor */
planner *fftw_mkplanner_score(void)
{
     return fftw_mkplanner(sizeof(planner), mkplan, 0);
}
