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

/* $Id: planner.c,v 1.66 2002-09-09 21:03:32 athena Exp $ */
#include "ifftw.h"
#include <string.h> /* strlen */

#define IMPATIENCE(flags) ((flags) & IMPATIENCE_MASK)
#define MODULO_EQV(flags) ((flags) & EQV_MASK)
#define BLESSEDP(s) ((s)->flags & BLESSING)

#define MAXNAM 64  /* maximum length of registrar's name.
		      Used for reading wisdom.  There is no point
		      in doing this right */
		      
/*
   liber scriptus proferetur
   in quo totum continetur
   unde mundus iudicetur
*/
struct solutions_s {
     md5uint s[4];
     uint flags;
     slvdesc *sp;
     solutions *cdr;
};

/* slvdesc management */
static uint hash_regnam(const char *s)
{
     uint h = 0xDEADBEEFul;
     do {
	  h = h * 17 + *s;
     } while (*s++);
     return h;
}

static void register_solver(planner *ego, solver *s)
{
     slvdesc *n;
     if (s) { /* add s to solver list */
	  X(solver_use)(s);
	  n = (slvdesc *) fftw_malloc(sizeof(slvdesc), SLVDESCS);
	  n->slv = s;
	  n->reg_nam = ego->cur_reg_nam;
	  n->reg_id = ego->cur_reg_id++;

	  A(strlen(n->reg_nam) < MAXNAM);
	  n->nam_hash = hash_regnam(n->reg_nam);

	  /* cons! onto solvers list */
	  n->cdr = ego->solvers;
	  ego->solvers = n;
     }
}

/* TODO: use hash table ? */
static slvdesc *slvdesc_lookup(planner *ego, char *nam, int id)
{
     slvdesc *sp;
     uint h = hash_regnam(nam); /* used to avoid strcmp in the common case */
     for (sp = ego->solvers; sp; sp = sp->cdr)
	  if (sp->reg_id == id && sp->nam_hash == h 
	      && !strcmp(sp->reg_nam, nam))
	       break;
     return sp;
}

/* memoization routines */
static uint sig_to_hash_index(planner *ego, md5uint *s)
{
     return s[0] % ego->hashsiz;
}

static void hash(md5 *m, const problem *p, uint flags, uint nthr)
{
     X(md5begin)(m);
     X(md5uint)(m, sizeof(R)); /* so we don't mix different precisions */
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

static solutions *insert1(planner *ego, md5uint *hsh, uint flags, slvdesc *sp)
{
     solutions *l = (solutions *)fftw_malloc(sizeof(solutions), HASHT);

     ++ego->cnt;
     if (ego->cnt > ego->hashsiz)
          rehash(ego);

     l->flags = flags;
     l->sp = sp;
     hashcpy(hsh, l->s);
     insert0(ego, l);
     return l;
}

static solutions *insert(planner *ego, uint flags, uint nthr,
			 problem *p, slvdesc *sp)
{
     md5 m;
     hash(&m, p, flags, nthr);
     return insert1(ego, m.s, flags, sp);
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
     plan *pln = 0;
     slvdesc *sp = 0;

     if ((sol = lookup(ego, p))) {
	  if ((sp = sol->sp)) {
	       /* call solver to create plan */
	       ego->inferior_mkplan(ego, p, &pln, &sp);
	  }
     }

     if (!pln) {
	  /* Lookup failed. Run inferior planner */
	  ++ego->nprob;
	  ego->inferior_mkplan(ego, p, &pln, &sp);
     }

     insert(ego, ego->flags, ego->nthr, p, sp);

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
		   (a == FORGET_ACCURSED && !BLESSEDP(s))) {
		    /* confutatis maledictis flammis acribus addictis */
		    *ps = s->cdr;
		    solution_destroy(s);
	       } else {
		    /* voca me cum benedictis */
		    ps = &s->cdr;
	       }
	  }
     }
     /* nil inultum remanebit */
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
     solutions *s;
     uint h;

     p->print(p, "(" WISDOM_PREAMBLE "%(");
     for (h = 0; h < ego->hashsiz; ++h)
	  for (s = ego->sols[h]; s; s = s->cdr)
	       if (BLESSEDP(s) && s->sp) {
		    /* qui salvandos salvas gratis
		       salva me fons pietatis */
		    p->print(p, "(%s %d #x%x #x%M #x%M #x%M #x%M)\n",
			     s->sp->reg_nam, s->sp->reg_id, s->flags,
			     s->s[0], s->s[1], s->s[2], s->s[3]);
	       }
     p->print(p, "%))\n");
}

static int imprt(planner *ego, scanner *sc)
{
     char buf[MAXNAM + 1];
     md5uint sig[4];
     uint flags;
     int reg_id;
     slvdesc *sp;

     if (!sc->scan(sc, "(" WISDOM_PREAMBLE))
	  goto bad;


     while (1) {
	  if (sc->scan(sc, ")"))
	       break;

	  if (!sc->scan(sc, "(%*s %d #x%x #x%M #x%M #x%M #x%M)",
			MAXNAM, buf, &reg_id, &flags, 
			sig + 0, sig + 1, sig + 2, sig + 3))
	       goto bad;

	  sp = slvdesc_lookup(ego, buf, reg_id);
	  if (!sp)
	       goto bad; /* TODO: panic? */

	  /* inter oves locum praesta */
	  insert1(ego, sig, flags, sp);
     }
     return 1;

 bad:
     return 0;
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
	  mkplan, forget, exprt, imprt, slv_mkplan
     };

     planner *p = (planner *) fftw_malloc(sz, PLANNERS);

     p->adt = &padt;
     p->inferior_mkplan = infmkplan;
     p->destroy = destroy;
     p->nplan = p->nprob = p->hit = p->access = 0;
     p->hook = hooknil;
     p->cur_reg_nam = 0;
     p->solvers = 0;
     p->sols = 0;
     p->hashsiz = 0;
     p->cnt = 0;
     p->score = BAD;            /* unused, but we initialize it anyway */
     p->flags = flags;
     p->nthr = 1;
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
