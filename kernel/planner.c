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

/* $Id: planner.c,v 1.8 2002-06-10 14:55:40 athena Exp $ */
#include "ifftw.h"

struct pair_s {
     solver *car;
     pair *cdr;
};

struct solutions_s {
     problem *p;
     plan *pln;
     solutions *cdr;
};

/* pair management */
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
     X(solver_use)(s);
     ego->solvers = cons(s, ego->solvers);
}

static void hooknil(plan *plan, problem *p)
{
     UNUSED(plan);
     UNUSED(p);
     /* do nothing */
}

/* memoization routines */
static uint hash(planner *ego, const problem *p)
{
     return (p->adt->hash(p) % ego->hashsiz);
}

static int lookup(planner *ego, problem *p, plan **pln)
{
     solutions *l;
     int h;

     h = hash(ego, p);

     for (l = ego->sols[h]; l; l = l->cdr) {
          if (p->adt->equal(p, l->p)) {
               *pln = l->pln;
               return 1;
          }
     }
     return 0;
}

static void really_insert(planner *ego, solutions *l)
{
     unsigned int h = hash(ego, l->p);
     l->cdr = ego->sols[h];
     ego->sols[h] = l;
}

static void rehash(planner *ego)
{
     unsigned int osiz = ego->hashsiz;
     unsigned int nsiz = 2 * osiz + 1;
     solutions **osol = ego->sols;
     solutions **nsol =
          (solutions **)fftw_malloc(nsiz * sizeof(solutions *), HASHT);
     solutions *s, *s_cdr;
     unsigned int h;

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

static void insert(planner *ego, problem *p, plan *plan)
{
     solutions *l = (solutions *)
	  fftw_malloc(sizeof(solutions), HASHT);

     ++ego->cnt;
     if (ego->cnt > ego->hashsiz)
          rehash(ego);

     if (plan)
          X(plan_use)(plan);
     l->pln = plan;
     l->p = X(problem_dup)(p);

     really_insert(ego, l);
}

static plan *mkplan(planner *ego, problem *p)
{
     plan *best;

     if (lookup(ego, p, &best)) {
          if (best)
               X(plan_use)(best);
          return best;
     }

     best = ego->inferior_mkplan(ego, p);

     if (ego->memoize && (best || ego->memoize_failures))
          insert(ego, p, best);

     return best;
}

static void htab_destroy(planner *ego)
{
     solutions *s;

     if (ego) {
          unsigned int h;

          for (h = 0; h < ego->hashsiz; ++h) {
               s = ego->sols[h];
               while (s) {
                    solutions *s_cdr = s->cdr;
                    if (s->pln)
                         X(plan_destroy)(s->pln);
                    X(problem_destroy)(s->p);
                    X(free)(s);
                    s = s_cdr;
               }
          }

          X(free)(ego->sols);
     }
}

/*
 * create a planner
 */
planner *X(mkplanner)(size_t sz,
                      plan *(*inferior_mkplan)(planner *, problem *),
                      void (*destroy) (planner *))
{
     static const planner_adt padt = {
	  car, cdr, solvers, register_solver, mkplan
     };

     planner *p = (planner *) fftw_malloc(sz, PLANNERS);

     p->adt = &padt;
     p->inferior_mkplan = inferior_mkplan;
     p->destroy = destroy;
     p->ntry = 0;
     p->hook = hooknil;
     p->solvers = 0;
     p->sols = 0;
     p->hashsiz = 0;
     p->cnt = 0;
     p->memoize = 1;
     p->memoize_failures = 0;
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
          l0 = cdr(l);
          X(solver_destroy)(car(l));
          X(free)(l);
     }

     X(free)(ego);
}

/* set planner hook */
void X(planner_set_hook)(planner *p, void (*hook)(plan *, problem *))
{
     p->hook = hook;
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
     unsigned int h;
     if (verbose) {
          printer *pr = X(mkprinter)(sizeof(printer), putchr);
          for (h = 0; h < ego->hashsiz; ++h) {
               printf("bucket %d\n", h);

               for (s = ego->sols[h]; s; s = s->cdr) {
                    if (s->pln) 
                         s->pln->adt->print(s->pln, pr);
		    else 
                         printf("(null)");
                    printf("\n");
               }
               printf("\n");
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

     printf("ntry = %d\n", ego->ntry);
     printf("hashsiz = %d\n", ego->hashsiz);
     printf("cnt = %d\n", cnt);
     printf("cnt_null = %d\n", cnt_null);
     printf("max_len = %d\n", max_len);
     printf("empty = %d\n", empty);
}
#endif
