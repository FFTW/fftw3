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

/* $Id: planner.c,v 1.21 2002-07-14 19:03:51 stevenj Exp $ */
#include "ifftw.h"

struct pair_s {
     solver *slv;
     int id;
     pair *cdr;
};

/* Entry in the solutions hash table */
/*
   The meaning of PLN and SP is as follows:
   
    if (PLN)          then PLN solves PROBLEM
    else if (SP)      then SP->SOLVER can be called to yield a plan 
    else                   PROBLEM has been seen before and cannot be solved.
*/

struct solutions_s {
     problem *p;
     plan *pln;
     pair *sp;
     solutions *cdr;
};

/* pair management */
static inline pair *cons(solver *slv, int id, pair *cdr)
{
     pair *n = (pair *) fftw_malloc(sizeof(pair), PAIRS);
     n->slv = slv;
     n->id = id;
     n->cdr = cdr;
     return n;
}

static solver *slv(pair *n) { return n->slv; }
static pair *cdr(pair *n) { return n->cdr; }
static pair *solvers(planner *ego) { return ego->solvers; }

static void register_solver(planner *ego, solver *s)
{
     if (s) {
	  X(solver_use)(s);
	  ego->solvers = cons(s, ego->idcnt++, ego->solvers);
     }
}

static void hooknil(plan *pln, const problem *p)
{
     UNUSED(pln);
     UNUSED(p);
     /* do nothing */
}

/* memoization routines */
static uint hash(planner *ego, const problem *p)
{
     return (p->adt->hash(p) % ego->hashsiz);
}

static solutions *lookup(planner *ego, problem *p)
{
     solutions *l;
     int h;

     h = hash(ego, p);

     for (l = ego->sols[h]; l; l = l->cdr) 
          if (p->adt->equal(p, l->p)) 
	       return l;
     return 0;
}

static void really_insert(planner *ego, solutions *l)
{
     uint h = hash(ego, l->p);
     l->cdr = ego->sols[h];
     ego->sols[h] = l;
}

static void rehash(planner *ego)
{
     uint osiz = ego->hashsiz;
     uint nsiz = 2 * osiz + 1;
     solutions **osol = ego->sols;
     solutions **nsol =
          (solutions **)fftw_malloc(nsiz * sizeof(solutions *), HASHT);
     solutions *s, *s_cdr;
     uint h;

     for (h = 0; h < nsiz; ++h) 
          nsol[h] = 0;

     ego->hashsiz = nsiz;
     ego->sols = nsol;

     for (h = 0; h < osiz; ++h) {
          for (s = osol[h]; s; s = s_cdr) {
               s_cdr = s->cdr;
               really_insert(ego, s);
          }
     }

     if (osol)
          X(free)(osol);
}

static void insert(planner *ego, problem *p, plan *pln, pair *sp)
{
     solutions *l = (solutions *)fftw_malloc(sizeof(solutions), HASHT);

     ++ego->cnt;
     if (ego->cnt > ego->hashsiz)
          rehash(ego);

     if (pln)
          X(plan_use)(pln);
     l->pln = pln;
     l->sp = sp; 
     l->p = X(problem_dup)(p);

     really_insert(ego, l);
}

static plan *slv_mkplan(planner *ego, problem *p, solver *s)
{
     int flags = ego->flags;
     plan *pln;
     pln = s->adt->mkplan(s, p, ego);
     ego->flags = flags;
     return pln;
}

static plan *mkplan(planner *ego, problem *p)
{
     solutions *sol;
     plan *pln;
     pair *sp;

     if ((sol = lookup(ego, p))) {
          if ((pln = sol->pln))
	       goto use_plan_and_return; /* we have a plan */

	  if ((sp = sol->sp)) {
	       /* call solver to create plan */
	       solver *slv = sp->slv;
	       pln = sol->pln = ego->adt->slv_mkplan(ego, p, slv);
	       X(plan_use)(pln);

	       A(pln);  /* solver must be applicable or something
			   is screwed up */
	       goto use_plan_and_return;
	  }

	  /* no solution */
	  return 0;

     use_plan_and_return:
	  X(plan_use)(pln);
	  return pln;
     } else {
	  /* not in table.  Run inferior planner */
	  ++ego->nprob;
	  ego->inferior_mkplan(ego, p, &pln, &sp);
	  insert(ego, p, pln, sp);
	  return pln;  /* plan already USEd by inferior */
     }
}

/* destroy hash table entries.  If EVERYTHINGP, destroy the whole
   table.  Otherwise, just destroy plans */
static void forget(planner *ego, int everythingp)
{
     solutions *s;
     uint h;

     for (h = 0; h < ego->hashsiz; ++h) {
	  s = ego->sols[h];
	  while (s) {
	       solutions *s_cdr = s->cdr;
	       if (s->pln) {
		    X(plan_destroy)(s->pln);
		    s->pln = 0;
	       }
	       if (everythingp) {
		    X(problem_destroy)(s->p);
		    X(free)(s);
	       }
	       s = s_cdr;
	  }

	  if (everythingp)
	       ego->sols[h] = 0;
     }
}

static void htab_destroy(planner *ego)
{
     forget(ego, 1);
     X(free)(ego->sols);
}

static void print_solution(solutions *s, printer *p)
{
     p->print(p, "(s %d %P)\n", s->sp ? s->sp->id : 0, s->p);
}

static void exprt(planner *ego, printer *p)
{
     solutions *s;
     uint h;

     for (h = 0; h < ego->hashsiz; ++h) 
	  for (s = ego->sols[h]; s; s = s->cdr) 
	       print_solution(s, p);
} 

/*
 * create a planner
 */
planner *X(mkplanner)(size_t sz,
		      void (*infmkplan)(planner *ego, problem *p, 
					plan **, pair **),
                      void (*destroy) (planner *),
		      int flags)
{
     static const planner_adt padt = {
	  slv, cdr, solvers, register_solver, mkplan, forget, exprt, slv_mkplan
     };

     planner *p = (planner *) fftw_malloc(sz, PLANNERS);

     p->adt = &padt;
     p->inferior_mkplan = infmkplan;
     p->destroy = destroy;
     p->nplan = p->nprob = 0;
     p->hook = hooknil;
     p->solvers = 0;
     p->sols = 0;
     p->hashsiz = 0;
     p->cnt = 0;
     p->flags = flags;
     p->idcnt = 1;              /* ID == 0 means no solution */
     rehash(p);			/* so that hashsiz > 0 */

     return p;
}

void X(planner_destroy)(planner *ego)
{
     pair *l, *l0;

     /* destroy local state, if any */
     if (ego->destroy)
          ego->destroy(ego);

     /* destroy hash table */
     htab_destroy(ego);

     /* destroy all solvers */
     for (l = ego->solvers; l; l = l0) {
          l0 = l->cdr;
          X(solver_destroy)(l->slv);
          X(free)(l);
     }

     X(free)(ego);
}

/* set planner hook */
void X(planner_set_hook)(planner *p, void (*hook)(plan *, const problem *))
{
     p->hook = hook;
}

void X(evaluate_plan)(planner *ego, plan *pln, const problem *p)
{
     if (!(ego->flags & CLASSIC) || pln->pcost == 0.0) {
	  ego->nplan++;
	  if (ego->flags & ESTIMATE) {
	       /* heuristic */
	       pln->pcost = 0
		    + pln->ops.add
		    + pln->ops.mul
		    + 2 * pln->ops.fma
		    + pln->ops.other;
	  } else {
	       pln->pcost = X(measure_execution_time)(pln, p);
	  }
     }
     ego->hook(pln, p);
}

/*
 * Debugging code:
 */
#ifdef FFTW_DEBUG
#include <stdio.h>

static void putchr(printer *p, char c)
{
     UNUSED(p);
     putchar(c);
}

void X(planner_dump)(planner *ego, int verbose)
{
     int cnt = 0, cnt_null = 0, max_len = 0, empty = 0;
     solutions *s;
     uint h;
     if (verbose) {
          printer *pr = X(mkprinter)(sizeof(printer), putchr);
          for (h = 0; h < ego->hashsiz; ++h) {
               pr->print(pr, "\nbucket %d:\n", h);

               for (s = ego->sols[h]; s; s = s->cdr) 
		    pr->print(pr, "%P:%(%p%)\n", s->p, s->pln);
          }
          X(printer_destroy)(pr);
     }

     for (h = 0; h < ego->hashsiz; ++h) {
          int l = 0;

          for (s = ego->sols[h]; s; s = s->cdr) {
               ++l;
               ++cnt;
               if (!s->pln)
                    ++cnt_null;
          }
          if (l > max_len)
               max_len = l;
          if (!l)
               ++empty;
     }

     printf("nplan = %u\n", ego->nplan);
     printf("nprob = %u\n", ego->nprob);
     printf("hashsiz = %d\n", ego->hashsiz);
     printf("cnt = %d\n", cnt);
     printf("cnt_null = %d\n", cnt_null);
     printf("max_len = %d\n", max_len);
     printf("empty = %d\n", empty);
}
#endif
