/*
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
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

/* $Id: planner.c,v 1.2 2002-06-06 12:07:33 athena Exp $ */
#include "ifftw.h"

struct pair_s {
     solver *car;
     pair *cdr;
};

static pair *const nil = 0;

static inline pair *cons(solver *car, pair *cdr)
{
     pair *n = (pair *) fftw_malloc(sizeof(pair), PAIRS);
     n->car = car;
     n->cdr = cdr;
     return n;
}

static solver *car(pair *n)
{
     return n->car;
}

static pair *cdr(pair *n)
{
     return n->cdr;
}

static pair *solvers(planner *ego)
{
     return ego->solvers;
}

static void register_solver(planner *ego, solver *s)
{
     fftw_solver_use(s);
     ego->solvers = cons(s, ego->solvers);
}

static void hooknil(plan *plan, problem *p)
{
     UNUSED(plan);
     UNUSED(p);
     /* do nothing */
}

static const planner_adt padt = {
     car,
     cdr,
     solvers,
     register_solver
};

/*
 * create a planner
 */
planner *fftw_mkplanner(size_t sz, 
			plan *(*mkplan)(planner *, problem *),
			void (*destroy) (planner *))
{
     planner *p = (planner *) fftw_malloc(sz, PLANNERS);

     p->adt = &padt;
     p->ntry = 0;
     p->hook = hooknil;
     p->solvers = nil;
     p->destroy = destroy;
     p->mkplan = mkplan;
     return p;
}

void fftw_planner_destroy(planner *ego)
{
     pair *l, *l0;

     if (ego->destroy)
	  ego->destroy(ego);

     /* destroy all solvers */
     for (l = ego->solvers; l; l = l0) {
	  l0 = cdr(l);
	  fftw_solver_destroy(car(l));
	  fftw_free(l);
     }

     fftw_free(ego);
}

/* set planner hook */
void fftw_planner_set_hook(planner *p, void (*hook)(plan *, problem *))
{
     p->hook = hook;
}
