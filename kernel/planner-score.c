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

/* $Id: planner-score.c,v 1.21 2002-09-12 20:10:05 athena Exp $ */
#include "ifftw.h"

/* scoring planner */

/* 
   The usage of the planner->score field is nonobvious.
  
   We want to compute the score of a plan as the minimum of the
   ``natural'' score of the plan (as determined by solver->score) and
   the score of all children.  We do not want to have each solver
   compute this quantity.

   Instead, planner->score initially holds the natural score of the
   parent.  The child planner updates planner->score to the minimum of
   planner->score and the score of the child.  The parent planner
   reads this value when the child planner returns.

   In effect, planner->score is a hidden additional function argument
   that we are too lazy to push around.
*/

static void mkplan(planner *ego, problem *p, plan **bestp, slvdesc **descp)
{
     plan *best = 0;
     int best_score;
     int best_not_yet_timed = 1;
     int parent_score = ego->score; /* save for later */
     slvdesc *sp;

     *bestp = 0;

     /* if a solver is suggested (by wisdom) use it */
     if ((sp = *descp)) {
	  solver *s = sp->slv;
	  ego->score = s->adt->score(s, p, ego); /* natural score of solver */
	  best = ego->adt->slv_mkplan(ego, p, s);
	  if (best) {
	       X(plan_use)(best);
	       best->score = ego->score;
	       goto done;
	  }
	  /* BEST may be 0 in case of md5 collision */
     }

     /* find highest score */
     best_score = BAD;
     FORALL_SOLVERS(ego, s, sp, {
	  int sc = s->adt->score(s, p, ego);
	  if (sc > best_score) 
	       best_score = sc;
     });

     for (; best_score > BAD; --best_score) {
          FORALL_SOLVERS(ego, s, sp, {
	       if (s->adt->score(s, p, ego) == best_score) {
		    plan *pln;

		    ego->score = best_score; /* natural score of this solver */
		    pln = ego->adt->slv_mkplan(ego, p, s);

		    if (pln) {
			 pln->score = ego->score; /* min of natural score
						     and score of children */

			 X(plan_use)(pln);

			 if (best) {
			      if (best_not_yet_timed) {
				   X(evaluate_plan)(ego, best, p);
				   best_not_yet_timed = 0;
			      }
			      X(evaluate_plan)(ego, pln, p);
			      if (pln->pcost < best->pcost) {
				   X(plan_destroy)(best);
				   best = pln;
				   *descp = sp;
			      } else {
				   X(plan_destroy)(pln);
			      }
			 } else {
			      best = pln;
			      *descp = sp;
			 }
		    }
	       }
	  });


	  /* condition may be false if children have lower score */
	  if (best && best->score >= best_score)
	       break;
     };

 done:
     ego->score = X(imin)(parent_score, best ? best->score : GOOD);
     *bestp = best;
}

/* constructor */
planner *X(mkplanner_score)(void)
{
     return X(mkplanner)(sizeof(planner), mkplan);
}
