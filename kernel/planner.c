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

/* $Id: planner.c,v 1.52 2002-09-01 23:51:50 athena Exp $ */
#include "ifftw.h"

#define IMPATIENCE(flags) ((flags) & IMPATIENCE_MASK)
#define MODULO_EQV(flags) ((flags) & EQV_MASK)
#define BLESS(s) (s)->flags |= BLESSING
#define BLESSEDP(s) ((s)->flags & BLESSING)

/* Entry in the solutions hash table */

struct solutions_s {
     md5uint s[4];
     uint flags;
     slvdesc *sp;
     solutions *cdr;
};

/* slvdesc management */
static slvdesc *mkslvdesc(solver *slv, const char *reg_nam, int id)
{
     slvdesc *n = (slvdesc *) fftw_malloc(sizeof(slvdesc), SLVDESCS);
     n->slv = slv;
     n->reg_nam = reg_nam;
     n->id = id;
     n->cdr = 0;
     return n;
}

static void register_solver(planner *ego, solver *s)
{
     if (s) { /* add s to end of solver list */
	  slvdesc *n;
	  A(s);
	  X(solver_use)(s);
	  n = mkslvdesc(s, ego->cur_reg_nam, ego->idcnt++);
	  *ego->solvers_tail = n;
	  ego->solvers_tail = &n->cdr;
     }
}

/* memoization routines */
static uint sig_to_hash_index(planner *ego, md5uint *s)
{
     return s[0] % ego->hashsiz;
}

static void hash(md5 *m, const problem *p, uint flags, uint nthr)
{
     X(md5begin)(m);
     X(md5uint)(m, MODULO_EQV(flags));
     X(md5uint)(m, nthr);
     p->adt->hash(p, m);
     X(md5end)(m);
}

static int hasheq(md5uint *a, md5uint *b)
{
     return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static void hashcpy(md5uint *a, md5uint *b)
{
     b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; b[3] = a[3];
}

static void merge_flags(solutions *l, uint flags)
{
     l->flags |= flags & (0
			  | BLESSING
	  );
}

static int solvedby(md5uint *s, uint flags, solutions *l)
{
     return (IMPATIENCE(flags) >= IMPATIENCE(l->flags)
	     /* other flags are included in md5 */
	     && hasheq(s, l->s));
}

static solutions *lookup(planner *ego, problem *p)
{
     solutions *l;
     uint h;
     md5 m;

     hash(&m, p, ego->flags, ego->nthr);
     h = sig_to_hash_index(ego, m.s);

     ++ego->access;
     for (l = ego->sols[h]; l; l = l->cdr) 
          if (solvedby(m.s, ego->flags, l)) {
	       ++ego->hit;
	       return l;
	  }

     return 0;
}

static void solution_destroy(solutions *s)
{
     X(free)(s);
}

/* remove solutions from *ps that are equivalent to newsol */
static void remove_equivalent(solutions *newsol, solutions **ps)
{
     solutions *s;

     while ((s = *ps)) {
	  if (solvedby(s->s, s->flags, newsol)) {
	       /* destroy solution but keep relevant planner flags */
	       merge_flags(newsol, s->flags);
	       *ps = s->cdr;
	       solution_destroy(s);
	  } else {
	       /* ensure that we are not replacing a solution with a
		  worse solution */
	       A(!solvedby(newsol->s, newsol->flags, s));
	       ps = &s->cdr;
	  }
     }
}

static void insert0(planner *ego, solutions *l)
{
     uint h = l->s[0] % ego->hashsiz;
     solutions **ps = ego->sols + h;

     /* remove old, less impatient solutions */
     remove_equivalent(l, ps);

     /* insert new solution */
     l->cdr = *ps; *ps = l;
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
               insert0(ego, s);
          }
     }

     if (osol)
          X(free)(osol);
}

static solutions *insert(planner *ego, uint flags, uint nthr,
			 problem *p, slvdesc *sp)
{
     solutions *l = (solutions *)fftw_malloc(sizeof(solutions), HASHT);
     md5 m;

     ++ego->cnt;
     if (ego->cnt > ego->hashsiz)
          rehash(ego);

     hash(&m, p, flags, nthr);
     l->flags = flags;
     l->sp = sp; 
     hashcpy(m.s, l->s);
     insert0(ego, l);
     return l;
}

static plan *slv_mkplan(planner *ego, problem *p, solver *s)
{
     uint flags = ego->flags;
     uint nthr = ego->nthr;
     plan *pln;
     pln = s->adt->mkplan(s, p, ego);
     ego->flags = flags;
     ego->nthr = nthr;
     return pln;
}

static plan *mkplan(planner *ego, problem *p)
{
     solutions *sol;
     plan *pln;
     slvdesc *sp = 0;

     if ((sol = lookup(ego, p))) {
	  if ((sp = sol->sp)) {
	       /* call solver to create plan */
	       ego->inferior_mkplan(ego, p, &pln, &sp);

	       /* PLN = 0 in the unlikely case of MD5 collision */ 
	       if (!pln) goto search;

	       X(plan_use)(pln);

	       /* inherit blessings etc. from planner */
	       merge_flags(sol, ego->flags);
	  } else {
	       /* impossible problem */
	       pln = 0;
	  }
     } else {
     search:
	  /* not in table.  Run inferior planner */
	  ++ego->nprob;
	  ego->inferior_mkplan(ego, p, &pln, &sp);
	  insert(ego, ego->flags, ego->nthr, p, sp);
     }


     if (pln)
	  ego->hook(pln, p, 1);
     return pln;
}

/* destroy hash table entries.  If FORGET_EVERYTHING, destroy the whole
   table.  If FORGET_ACCURSED, then destroy entries that are not blessed. */
static void forget(planner *ego, amnesia a)
{
     uint h;

     for (h = 0; h < ego->hashsiz; ++h) {
	  solutions **ps = ego->sols + h, *s;
	  while ((s = *ps)) {
	       if (a == FORGET_EVERYTHING ||
		   (a == FORGET_ACCURSED && BLESSEDP(s))) {
		    /* confutatis maledictis flammis acribus addictis */
		    *ps = s->cdr;
		    solution_destroy(s);
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
#define WISDOM_PREAMBLE PACKAGE "-" VERSION "-wisdom "

/* tantus labor non sit cassus */
static void exprt(planner *ego, printer *p)
{
#if 0
     solutions *s;
     uint h;

     p->print(p, "(" WISDOM_PREAMBLE);
     for (h = 0; h < ego->hashsiz; ++h) 
	  for (s = ego->sols[h]; s; s = s->cdr)
	       /* qui salvandos salvas gratis
		  salva me fons pietatis */
	       if (BLESSEDP(s))
		    p->print(p, "(s %d %d)", 
			     s->sp ? s->sp->id : 0, s->flags);
     p->print(p, ")");
#endif
}

static int imprt(planner *ego, scanner *sc)
{
#if 0
     slvdesc **slvrs;
     problem *p = 0;
     int i, ret = 0;

     /* need to cache an array of solvers for fast lookup by id */
     slvrs = (slvdesc **) fftw_malloc(sizeof(slvdesc *) * (ego->idcnt - 1),
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
	  uint nthr;

	  if (sc->scan(sc, ")"))
	       break;
	  if (!sc->scan(sc, "(s %d %d %u %P)", &id, &flags, &nthr, &p))
	       goto done;
	  if (id < 1 || id >= ego->idcnt)
	       goto done;
	  s = insert(ego, flags, nthr, p, slvrs[id - 1]);
	  BLESS(s);
	  X(problem_destroy)(p); p = 0;
     }
     ret = 1;
 done:
     X(free)(slvrs);
     if (p)
	  X(problem_destroy)(p);
     return ret;
#endif
}

#if 0
static void clear_problem_marks(planner *ego)
{
     prbdesc *pp;
     for (pp = ego->problems; pp; pp = pp->cdr)
	  pp->mark = 0;
}

static void mark_problem(planner *ego, problem *p)
{
     prbdesc *pp;
     for (pp = ego->problems; pp; pp = pp->cdr)
	  if (pp->adt == p->adt) {
	       pp->mark = 1;
	       return;
	  }
}
#endif

static void exprt_conf(planner *ego, printer *p)
{
#if 0
     solutions *s;
     prbdesc *pp;
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
               if (BLESSEDP(s) && s->sp && s->sp->reg_nam && s->sp->id > 0) {
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
#endif
}

static void hooknil(plan *pln, const problem *p, int optimalp)
{
     UNUSED(pln);
     UNUSED(p);
     UNUSED(optimalp);
     /* do nothing */
}

/*
 * create a planner
 */
planner *X(mkplanner)(size_t sz,
		      void (*infmkplan)(planner *ego, problem *p, 
					plan **, slvdesc **),
                      void (*destroy) (planner *),
		      uint flags)
{
     static const planner_adt padt = {
	  register_solver, 
	  mkplan, forget, exprt, imprt, exprt_conf, slv_mkplan
     };

     planner *p = (planner *) fftw_malloc(sz, PLANNERS);

     p->adt = &padt;
     p->inferior_mkplan = infmkplan;
     p->destroy = destroy;
     p->nplan = p->nprob = p->hit = p->access = 0;
     p->hook = hooknil;
     p->cur_reg_nam = 0;
     p->solvers = 0;
     p->solvers_tail = &p->solvers;
     p->sols = 0;
     p->hashsiz = 0;
     p->cnt = 0;
     p->flags = flags;
     p->nthr = 1;
     p->idcnt = 1;              /* ID == 0 means no solution */
     rehash(p);			/* so that hashsiz > 0 */

     return p;
}

void X(planner_destroy)(planner *ego)
{
     slvdesc *l, *l0;

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
void X(planner_set_hook)(planner *p, 
			 void (*hook)(plan *, const problem *, int))
{
     p->hook = hook;
}

void X(evaluate_plan)(planner *ego, plan *pln, const problem *p)
{
     if (!(ego->flags & IMPATIENT) || pln->pcost == 0.0) {
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
     ego->hook(pln, p, 0);
}

/*
 * Debugging code:
 */
#ifdef FFTW_DEBUG

void X(planner_dump)(planner *ego, int verbose)
{
     int cnt = 0, cnt_null = 0, max_len = 0, empty = 0;
     solutions *s;
     uint h;

     if (verbose > 2) {
          for (h = 0; h < ego->hashsiz; ++h) {
               D("\nbucket %d:\n", h);

               for (s = ego->sols[h]; s; s = s->cdr) 
		    D("%u %s\n", s->s[0], s->sp ? s->sp->reg_nam : 0);
          }
     }

     for (h = 0; h < ego->hashsiz; ++h) {
          int l = 0;

          for (s = ego->sols[h]; s; s = s->cdr) {
               ++l;
               ++cnt;
               if (!s->sp)
                    ++cnt_null;
          }
          if (l > max_len)
               max_len = l;
          if (!l)
               ++empty;
     }

     D("nplan = %u\n", ego->nplan);
     D("nprob = %u\n", ego->nprob);
     D("access = %u\n", ego->access);
     D("hit = %u\n", ego->hit);
     D("hashsiz = %d\n", ego->hashsiz);
     D("cnt = %d\n", cnt);
     D("cnt_null = %d\n", cnt_null);
     D("max_len = %d\n", max_len);
     D("empty = %d\n", empty);
}
#endif
