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

/* $Id: planner.c,v 1.38 2002-08-04 21:34:04 stevenj Exp $ */
#include "ifftw.h"

/* Entry in the solutions hash table */
/*
   The meaning of PLN and SP is as follows:
   
    if (PLN)          then PLN solves PROBLEM
    else if (SP)      then SP->SOLVER can be called to yield a plan 
    else                   PROBLEM has been seen before and cannot be solved.
*/

struct solutions_s {
     problem *p;
     int flags;
     plan *pln;
     slvpair *sp;
     int blessed;
     solutions *cdr;
};

/* slvpair management */
static slvpair *mkpair(solver *slv, const char *reg_nam, int id)
{
     slvpair *n = (slvpair *) fftw_malloc(sizeof(slvpair), SLVPAIRS);
     n->slv = slv;
     n->reg_nam = reg_nam;
     n->id = id;
     n->cdr = 0;
     return n;
}

static void register_solver(planner *ego, solver *s)
{
     if (s) { /* add s to end of solver list */
	  slvpair *n;
	  X(solver_use)(s);
	  n = mkpair(s, ego->cur_reg_nam, ego->idcnt++);
	  *ego->last_solver_cdr = n;
	  ego->last_solver_cdr = &n->cdr;
     }
}

static void register_problem(planner *ego, const problem_adt *adt)
{
     prbpair *p;
     for (p = ego->problems; p; p = p->cdr)
	  if (p->adt == adt)
	       return;
     p = (prbpair *) fftw_malloc(sizeof(prbpair), OTHER);
     p->adt = adt;
     p->reg_nam = ego->cur_reg_nam;
     p->cdr = ego->problems;
     ego->problems = p;
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
     return ((p->adt->hash(p) ^ (ego->flags * 317227)) % ego->hashsiz);
}

static solutions *lookup(planner *ego, problem *p)
{
     solutions *l;
     int h;

     h = hash(ego, p);

     for (l = ego->sols[h]; l; l = l->cdr) 
          if ((ego->flags & IMPATIENCE_FLAGS) > (l->flags & IMPATIENCE_FLAGS)
	      && p->adt->equal(p, l->p)) 
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

static solutions *insert(planner *ego, 
			 int flags, problem *p, plan *pln, slvpair *sp)
{
     solutions *l = (solutions *)fftw_malloc(sizeof(solutions), HASHT);

     ++ego->cnt;
     if (ego->cnt > ego->hashsiz)
          rehash(ego);

     if (pln) {
          X(plan_use)(pln);
	  l->blessed = pln->blessed;
     }
     else
	  l->blessed = 0;
     l->flags = flags;
     l->pln = pln;
     l->sp = sp; 
     l->p = X(problem_dup)(p);

     really_insert(ego, l);
     return l;
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
     slvpair *sp;

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
	  insert(ego, ego->flags, p, pln, sp);
	  return pln;  /* plan already USEd by inferior */
     }
}

static int is_blessed(solutions *s)
{
     return(!(s->flags & ESTIMATE)
	    && (s->blessed || (s->pln && s->pln->blessed)));
}

/* destroy hash table entries.  If FORGET_EVERYTHING, destroy the whole
   table.  If FORGET_ACCURSED, then destroy entries that are not blessed.
   If FORGET_PLANS, then destroy all the plans but remember the solvers. */
static void forget(planner *ego, amnesia a)
{
     uint h;

     for (h = 0; h < ego->hashsiz; ++h) {
	  solutions **ps = ego->sols + h, *s;
	  while ((s = *ps)) {
	       s->blessed = is_blessed(s); /* remember pln blessing */
	       if (s->pln && a == FORGET_PLANS) {
		    X(plan_destroy)(s->pln);
		    s->pln = 0;
	       }

	       if (a == FORGET_EVERYTHING ||
		   (a == FORGET_ACCURSED && !s->blessed)) {
		    /* confutatis maledictis flammis acribus addictis */
		    *ps = s->cdr;
		    if (s->pln)
			 X(plan_destroy)(s->pln);
		    X(problem_destroy)(s->p);
		    X(free)(s);
	       } else {
		    /* voca me cum benedictis */
		    ps = &s->cdr;
	       }
	  }
     }
}

static void htab_destroy(planner *ego)
{
     forget(ego, FORGET_EVERYTHING);
     X(free)(ego->sols);
}

/* FIXME: what sort of version information should we write? */
#define WISDOM_PREAMBLE "fftw-wisdom " PACKAGE "-" VERSION " "

/* tantus labor non sit cassus */
static void exprt(planner *ego, printer *p)
{
     solutions *s;
     uint h;

     p->print(p, "(" WISDOM_PREAMBLE);
     for (h = 0; h < ego->hashsiz; ++h) 
	  for (s = ego->sols[h]; s; s = s->cdr)
	       /* qui salvandos salvas gratis
		  salva me fons pietatis */
	       if (is_blessed(s))
		    p->print(p, "(s %d %d %P)", 
			     s->sp ? s->sp->id : 0, s->flags, s->p);
     p->print(p, ")");
}

static int imprt(planner *ego, scanner *sc)
{
     slvpair **slvrs;
     problem *p = 0;
     int i, ret = 0;

     /* need to cache an array of solvers for fast lookup by id */
     slvrs = (slvpair **) fftw_malloc(sizeof(slvpair *) * (ego->idcnt - 1),
				      OTHER);
     for (i = 0; i + 1 < ego->idcnt; ++i) slvrs[i] = 0;
     FORALL_SOLVERS(ego, s, p, {
	  UNUSED(s);
	  A(p->id > 0 && p->id < ego->idcnt);
	  slvrs[p->id - 1] = p;
     });
     for (i = 0; i + 1 < ego->idcnt; ++i) { A(slvrs[i]); }

     if (!sc->scan(sc, "(" WISDOM_PREAMBLE))
	  goto done;

     while (1) {
	  solutions *s;
	  int id, flags;

	  if (sc->scan(sc, ")"))
	       break;
	  if (!sc->scan(sc, "(s %d %d %P)", &id, &flags, &p))
	       goto done;
	  if (id < 1 || id >= ego->idcnt)
	       goto done;
	  s = insert(ego, flags, p, (plan *) 0, slvrs[id - 1]);
	  s->blessed = 1;
	  X(problem_destroy)(p); p = 0;
     }
     ret = 1;
 done:
     X(free)(slvrs);
     if (p)
	  X(problem_destroy)(p);
     return ret;
}

static void clear_problem_marks(planner *ego)
{
     prbpair *pp;
     for (pp = ego->problems; pp; pp = pp->cdr)
	  pp->mark = 0;
}

static void mark_problem(planner *ego, problem *p)
{
     prbpair *pp;
     for (pp = ego->problems; pp; pp = pp->cdr)
	  if (pp->adt == p->adt) {
	       pp->mark = 1;
	       return;
	  }
}

static void exprt_conf(planner *ego, printer *p)
{
     solutions *s;
     prbpair *pp;
     uint h;
     const char *prev_reg_nam = (const char *) 0;
     int idcnt = 0, reg_nam_cnt = 1, blessed_reg_nam = 0;

     /* FIXME: need to use external API */

     p->print(p, "void wisdom_conf(planner *p)\n{\n");

#ifndef __cplusplus
     p->print(p, "#ifdef __cplusplus\n     extern \"C\" {\n#endif\n");
#endif

     clear_problem_marks(ego);
     for (h = 0; h < ego->hashsiz; ++h)
          for (s = ego->sols[h]; s; s = s->cdr) {
               if (is_blessed(s) && s->sp && s->sp->reg_nam && s->sp->id > 0) {
		    s->sp->id = 0; /* mark to prevent duplicates */
		    mark_problem(ego, s->p);
		    p->print(p, "          extern void %s(planner*);\n",
			     s->sp->reg_nam);
	       }
	  }
     for (pp = ego->problems; pp; pp = pp->cdr)
	  if (pp->mark)
	       p->print(p, "          extern void %s(planner*),\n", 
			pp->reg_nam);

#ifndef __cplusplus
     p->print(p, "#ifdef __cplusplus\n     }\n#endif\n");
#endif

     p->print(p, "     static const solvtab s = {\n");

     for (pp = ego->problems; pp; pp = pp->cdr)
	  if (pp->mark)
	       p->print(p, "          SOLVTAB(%s),\n", pp->reg_nam);
     clear_problem_marks(ego);

     /* output solvtab entries, and compute correct id's of blessed
	entries as they will appear in the generated table (ugh). */
     FORALL_SOLVERS(ego, s, sp, {
	  UNUSED(s);
	  if (prev_reg_nam && sp->reg_nam == prev_reg_nam)
	       ++reg_nam_cnt;
	  else {
	       if (blessed_reg_nam)
		    idcnt += reg_nam_cnt;
	       prev_reg_nam = sp->reg_nam;
	       reg_nam_cnt = 1;
	       blessed_reg_nam = 0;
	  }
	  if (sp->id == 0) {
	       if (!blessed_reg_nam) /* i.e., not already printed */
		    p->print(p, "          SOLVTAB(%s),\n", sp->reg_nam);
	       blessed_reg_nam = 1;
	       sp->id = idcnt + reg_nam_cnt;
	  }
     });
     if (blessed_reg_nam)
	  idcnt += reg_nam_cnt;
     
     p->print(p, "          SOLVTAB_END\n"
	      "     }; /* yields %d solvers */\n", idcnt);

     /* export wisdom as hard-coded string, w/ids corresponding to solvtab */
     p->print(p, "     static const char * const wisdom = \"");
     ego->adt->exprt(ego, p);
     p->print(p, "\";\n");

     /* reset ids: */
     idcnt = 1;
     FORALL_SOLVERS(ego, s, sp, {
	  UNUSED(s);
          sp->id = idcnt++;
     });

     p->print(p, "\n"
	      "     %s(s, p);\n"
	      "     /* FIXME: import wisdom */\n"
	      "}\n", STRINGIZE(X(solvtab_exec)));
}

/*
 * create a planner
 */
planner *X(mkplanner)(size_t sz,
		      void (*infmkplan)(planner *ego, problem *p, 
					plan **, slvpair **),
                      void (*destroy) (planner *),
		      int flags)
{
     static const planner_adt padt = {
	  register_solver, register_problem,
	  mkplan, forget, exprt, imprt, exprt_conf, slv_mkplan
     };

     planner *p = (planner *) fftw_malloc(sz, PLANNERS);

     p->adt = &padt;
     p->inferior_mkplan = infmkplan;
     p->destroy = destroy;
     p->nplan = p->nprob = 0;
     p->hook = hooknil;
     p->cur_reg_nam = 0;
     p->solvers = 0;
     p->last_solver_cdr = &p->solvers;
     p->problems = 0;
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
     slvpair *l, *l0;
     prbpair *p, *p0;

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
     
     /* destroy all problems (a feature sure to be popular) */
     for (p = ego->problems; p; p = p0) {
          p0 = p->cdr;
          X(free)(p);
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
