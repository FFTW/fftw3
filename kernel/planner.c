/*
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Massachusetts Institute of Technology
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

/* $Id: planner.c,v 1.159 2005-01-15 21:35:42 stevenj Exp $ */
#include "ifftw.h"
#include <string.h>

/* GNU Coding Standards, Sec. 5.2: "Please write the comments in a GNU
   program in English, because English is the one language that nearly
   all programmers in all countries can read."

                    ingemisco tanquam reus
		    culpa rubet vultus meus
		    supplicanti parce [rms]
*/

#define VALIDP(solution) ((solution)->flags & H_VALID)
#define LIVEP(solution) ((solution)->flags & H_LIVE)

#define MAXNAM 64  /* maximum length of registrar's name.
		      Used for reading wisdom.  There is no point
		      in doing this right */

/* Flags f1 subsumes flags f2 iff f1 is less/equally impatient than
   f2, defining a partial ordering. */
#define IMPATIENCE(flags) ((flags) & IMPATIENCE_FLAGS)
#define SUBSUMES(f1, f2) ((IMPATIENCE(f1) & (f2)) == IMPATIENCE(f1))
#define BLISS(flags) ((flags) & BLESSING)

#ifdef FFTW_DEBUG
static void check(hashtab *ht);
#endif

static unsigned addmod(unsigned a, unsigned b, unsigned p)
{
     /* gcc-2.95/sparc produces incorrect code for the fast version below. */
#if defined(__sparc__) && defined(__GNUC__)
     /* slow version  */
     return (a + b) % p;
#else
     /* faster version */
     unsigned c = a + b;
     return c >= p ? c - p : c;
#endif
}

/*
  slvdesc management:
*/
static void sgrow(planner *ego)
{
     unsigned osiz = ego->slvdescsiz, nsiz = 1 + osiz + osiz / 4;
     slvdesc *ntab = (slvdesc *)MALLOC(nsiz * sizeof(slvdesc), SLVDESCS);
     slvdesc *otab = ego->slvdescs;
     unsigned i;

     ego->slvdescs = ntab;
     ego->slvdescsiz = nsiz;
     for (i = 0; i < osiz; ++i)
	  ntab[i] = otab[i];
     X(ifree0)(otab);
}

static void register_solver(planner *ego, solver *s)
{
     slvdesc *n;
     if (s) { /* add s to solver list */
	  X(solver_use)(s);

	  if (ego->nslvdesc >= ego->slvdescsiz)
	       sgrow(ego);

	  n = ego->slvdescs + ego->nslvdesc++;

	  n->slv = s;
	  n->reg_nam = ego->cur_reg_nam;
	  n->reg_id = ego->cur_reg_id++;
	  
	  A(strlen(n->reg_nam) < MAXNAM);
	  n->nam_hash = X(hash)(n->reg_nam);
     }
}

static ptrdiff_t slookup(planner *ego, char *nam, int id)
{
     unsigned h = X(hash)(nam); /* used to avoid strcmp in the common case */
     FORALL_SOLVERS(ego, s, sp, {
	  UNUSED(s);
	  if (sp->reg_id == id && sp->nam_hash == h
	      && !strcmp(sp->reg_nam, nam))
	       return sp - ego->slvdescs;
     });
     return -1;
}

/*
  md5-related stuff:
*/

/* first hash function */
static unsigned h1(const hashtab *ht, const md5sig s)
{
     return s[0] % ht->hashsiz;
}

/* second hash function (for double hashing) */
static unsigned h2(const hashtab *ht, const md5sig s)
{
     return 1U + s[1] % (ht->hashsiz - 1);
}

static void md5hash(md5 *m, const problem *p, const planner *plnr)
{
     X(md5begin)(m);
     X(md5unsigned)(m, sizeof(R)); /* so we don't mix different precisions */
     X(md5unsigned)(m, plnr->problem_flags);
     X(md5int)(m, plnr->nthr);
     p->adt->hash(p, m);
     X(md5end)(m);
}

static int md5eq(const md5sig a, const md5sig b)
{
     return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static void sigcpy(const md5sig a, md5sig b)
{
     b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; b[3] = a[3];
}

/*
  memoization routines :
*/

/*
   liber scriptus proferetur
   in quo totum continetur
   unde mundus iudicetur
*/
struct solution_s {
     md5sig s;
     unsigned short flags;
     short slvndx;
};

static solution *htab_lookup(hashtab *ht, const md5sig s, unsigned short flags)
{
     unsigned g, h = h1(ht, s), d = h2(ht, s);

     ++ht->lookup;

     for (g = h; ; g = addmod(g, d, ht->hashsiz)) {
	  solution *l = ht->solutions + g;
	  ++ht->lookup_iter;
	  if (VALIDP(l)) {
	       if (LIVEP(l) && md5eq(s, l->s) && SUBSUMES(l->flags, flags)) { 
		    ++ht->succ_lookup;
		    return l; 
	       }
	  } else {
	       return 0;
	  }
	  A((g + d) % ht->hashsiz != h);
     }
}

static solution *hlookup(planner *ego, const md5sig s, unsigned short flags)
{
     solution *sol = htab_lookup(&ego->htab_blessed, s, flags | BLESSING);
     if (!sol) sol = htab_lookup(&ego->htab_unblessed, s, flags);
     return sol;
}

static void fill_slot(hashtab *ht, const md5sig s, unsigned short flags,
		      int slvndx, solution *slot)
{
     ++ht->insert;
     ++ht->nelem;
     A(!LIVEP(slot));
     slot->flags = flags | (H_VALID | H_LIVE);
     slot->slvndx = (short)slvndx;
     sigcpy(s, slot->s);
}

static void kill_slot(hashtab *ht, solution *slot)
{
     A(LIVEP(slot)); /* ==> */ A(VALIDP(slot));

     --ht->nelem;
     slot->flags = H_VALID;
}

static void hinsert0(hashtab *ht, const md5sig s, unsigned short flags,
		     int slvndx)
{
     solution *l;
     unsigned g, h = h1(ht, s), d = h2(ht, s); 

     ++ht->insert_unknown;

     /* search for nonfull slot */
     for (g = h; ; g = addmod(g, d, ht->hashsiz)) {
	  ++ht->insert_iter;
	  l = ht->solutions + g;
	  if (!LIVEP(l)) break;
	  A((g + d) % ht->hashsiz != h);
     }

     fill_slot(ht, s, flags, slvndx, l);
}

static void rehash(hashtab *ht, unsigned nsiz)
{
     unsigned osiz = ht->hashsiz, h;
     solution *osol = ht->solutions, *nsol;

     nsiz = (unsigned)X(next_prime)((int)nsiz);
     nsol = (solution *)MALLOC(nsiz * sizeof(solution), HASHT);
     ++ht->nrehash;

     /* init new table */
     for (h = 0; h < nsiz; ++h) 
	  nsol[h].flags = 0;

     /* install new table */
     ht->hashsiz = nsiz;
     ht->solutions = nsol;
     ht->nelem = 0;

     /* copy table */
     for (h = 0; h < osiz; ++h) {
	  solution *l = osol + h;
	  if (LIVEP(l))
	       hinsert0(ht, l->s, l->flags, l->slvndx);
     }

     X(ifree0)(osol);
}

static unsigned minsz(unsigned nelem)
{
     return 1U + nelem + nelem / 8U;
}

static unsigned nextsz(unsigned nelem)
{
     return minsz(minsz(nelem));
}

static void hgrow(hashtab *ht)
{
     unsigned nelem = ht->nelem;
     if (minsz(nelem) >= ht->hashsiz)
	  rehash(ht, nextsz(nelem));
}

#if 0
/* shrink the hash table, never used */
static void hshrink(hashtab *ht)
{
     unsigned nelem = ht->nelem;
     /* always rehash after deletions */
     rehash(ht, nextsz(nelem));
}
#endif

static void htab_insert(hashtab *ht, const md5sig s, 
			unsigned short flags, int slvndx)
{
     unsigned g, h = h1(ht, s), d = h2(ht, s);
     solution *first = 0;

     /* Remove all entries that are subsumed by the new one.
	Overwrite the first found, if any exists. */
     for (g = h; ; g = addmod(g, d, ht->hashsiz)) {
	  solution *l = ht->solutions + g;
	  ++ht->insert_iter;
	  if (VALIDP(l)) {
	       if (LIVEP(l) && md5eq(s, l->s)) {
		    if (SUBSUMES(flags, l->flags)) {
			 if (!first) first = l;
			 kill_slot(ht, l);
		    } else {
			 /* It is an error to insert an element that
			    is subsumed by an existing entry. */
			 /* CHECK THIS */
			 A(!SUBSUMES(l->flags, flags));
		    }
	       }
	  } else 
	       break;
     }

     if (first) {
	  /* overwrite FIRST */
	  fill_slot(ht, s, flags, slvndx, first);
     } else {
	  /* create a new entry */
 	  hgrow(ht);
	  hinsert0(ht, s, flags, slvndx);
     }
}

static void hinsert(planner *ego, const md5sig s, 
		    unsigned short flags, int slvndx)
{
     htab_insert(BLISS(flags) ? &ego->htab_blessed : &ego->htab_unblessed,
		 s, flags, slvndx );
}


static void invoke_hook(planner *ego, plan *pln, const problem *p, 
			int optimalp)
{
     if (ego->hook)
	  ego->hook(ego, pln, p, optimalp);
}

double X(iestimate_cost)(const plan *pln)
{
     return 0.0
	  + pln->ops.add
	  + pln->ops.mul
	  + 2 * pln->ops.fma
	  + pln->ops.other;
}

static void evaluate_plan(planner *ego, plan *pln, const problem *p)
{
     if (!BELIEVE_PCOSTP(ego) || pln->pcost == 0.0) {
	  ego->nplan++;

	  if (ESTIMATEP(ego)) {
	  estimate:
	       /* heuristic */
	       pln->pcost = X(iestimate_cost)(pln);
	       ego->epcost += pln->pcost;
	  } else {
	       double t = X(measure_execution_time)(pln, p);

	       if (t < 0) {  /* unavailable cycle counter */
		    /* Real programmers can write FORTRAN in any language */
		    goto estimate;
	       }

	       pln->pcost = t;
	       ego->pcost += t;
	  }
     }
     
     invoke_hook(ego, pln, p, 0);
}

/* maintain dynamic scoping of flags, nthr: */
static plan *invoke_solver(planner *ego, problem *p, solver *s, 
			   unsigned short nflags)
{
     unsigned short planner_flags = ego->planner_flags;
     unsigned problem_flags = ego->problem_flags;
     int nthr = ego->nthr;
     plan *pln;
     ego->planner_flags = nflags;
     pln = s->adt->mkplan(s, p, ego);
     ego->problem_flags = problem_flags;
     ego->nthr = nthr;
     ego->planner_flags = planner_flags;
     return pln;
}

static plan *search0(planner *ego, problem *p, int *slvndx,
		     unsigned short planner_flags)
{
     plan *best = 0;
     int best_not_yet_timed = 1;

     FORALL_SOLVERS(ego, s, sp, {
	  plan *pln = invoke_solver(ego, p, s, planner_flags);

	  if (pln) {
	       if (best) {
		    if (best_not_yet_timed) {
			 evaluate_plan(ego, best, p);
			 best_not_yet_timed = 0;
		    }
		    evaluate_plan(ego, pln, p);
		    if (pln->pcost < best->pcost) {
			 X(plan_destroy_internal)(best);
			 best = pln;
			 *slvndx = sp - ego->slvdescs;
		    } else {
			 X(plan_destroy_internal)(pln);
		    }
	       } else {
		    best = pln;
		    *slvndx = sp - ego->slvdescs;
	       }
	  }
     });

     return best;
}

static plan *search(planner *ego, problem *p, int *slvndx,
		    unsigned short planner_flags)
{
     plan *pln = 0;

     /* try a first search in NO_UGLY mode, unless we are
	searching exhaustively */
     if (planner_flags & NO_EXHAUSTIVE) 
	  pln = search0(ego, p, slvndx, planner_flags | NO_UGLY);
     else {
	  /* assertion holds because of canonicalization in mkplan()
	     below */
	  A(!(planner_flags & NO_UGLY));
     }

     if (!pln) {
	  /* if we still don't have a plan, try a search in UGLY
	     mode, unless the mode was NO_UGLY to begin with */
	  if (!(planner_flags & NO_UGLY)) {
	       pln = search0(ego, p, slvndx, planner_flags);
	  }
     }

     return pln;
}

static plan *mkplan(planner *ego, problem *p)
{
     plan *pln;
     md5 m;
     int slvndx;
     unsigned short flags_of_solution;
     solution *sol;

     ASSERT_ALIGNED_DOUBLE;

#ifdef FFTW_DEBUG
     check(&ego->htab_blessed);
     check(&ego->htab_unblessed);
#endif

     /* Canonical form. */
     if (!NO_EXHAUSTIVEP(ego)) ego->planner_flags &= ~NO_UGLY;

     ++ego->nprob;
     md5hash(&m, p, ego);

     pln = 0;

     flags_of_solution = ego->planner_flags;
     if ((sol = hlookup(ego, m.s, flags_of_solution))) { 
	  /* wisdom is acceptable */

	  slvndx = sol->slvndx;
	  if (slvndx < 0) 
	       return 0;   /* known to be infeasible */

	  flags_of_solution = (0
			       | IMPATIENCE(sol->flags)

			       /* inherit blessing either from wisdom
				  or from the planner */
			       | BLISS(sol->flags)
			       | BLISS(flags_of_solution));

	  /* use solver to obtain a plan */
	  pln = invoke_solver(ego, p, ego->slvdescs[slvndx].slv,
			      flags_of_solution);	  

	  sol = 0; /* Paranoid: SOL may be dangling after
		      invoke_solver(); make sure we don't accidentally
		      reuse it. */

	  /* if (!pln) then the entry is bogus, but
	     we currently do nothing about it. */

	  /* CHECK THIS */
	  A(pln);
     }


     if (!pln) {
	  flags_of_solution = ego->planner_flags;
	  pln = search(ego, p, &slvndx, flags_of_solution);
     }

     if (pln) {
	  /* Postulate de iure that NO_UGLY subsumes ~NO_UGLY when the
	     problem is feasible. */
	  flags_of_solution &= ~NO_UGLY;

	  hinsert(ego, m.s, flags_of_solution, slvndx);

	  invoke_hook(ego, pln, p, 1);
     } else {
	  hinsert(ego, m.s, flags_of_solution, -1);
     }

     return pln;
}

static void htab_destroy(hashtab *ht)
{
     X(ifree)(ht->solutions);
     ht->solutions = 0;
     ht->nelem = 0U;
}

static void mkhashtab(hashtab *ht)
{
     ht->nrehash = 0;
     ht->succ_lookup = ht->lookup = ht->lookup_iter = 0;
     ht->insert = ht->insert_iter = ht->insert_unknown = 0;

     ht->solutions = 0;
     ht->hashsiz = ht->nelem = 0U;
     hgrow(ht);			/* so that hashsiz > 0 */
}

/* destroy hash table entries.  If FORGET_EVERYTHING, destroy the whole
   table.  If FORGET_ACCURSED, then destroy entries that are not blessed. */
static void forget(planner *ego, amnesia a)
{
     switch (a) {
	 case FORGET_EVERYTHING:
	      htab_destroy(&ego->htab_blessed);
	      mkhashtab(&ego->htab_blessed);
	      /* fall through */
	 case FORGET_ACCURSED:
	      htab_destroy(&ego->htab_unblessed);
	      mkhashtab(&ego->htab_unblessed);
	      break;
	 default:
	      break;
     }
}

/* FIXME: what sort of version information should we write? */
#define WISDOM_PREAMBLE PACKAGE "-" VERSION " " STRINGIZE(X(wisdom))

/* tantus labor non sit cassus */
static void exprt(planner *ego, printer *p)
{
     unsigned h;
     hashtab *ht = &ego->htab_blessed;

     p->print(p, "(" WISDOM_PREAMBLE "%(");
     for (h = 0; h < ht->hashsiz; ++h) {
	  solution *l = ht->solutions + h;
	  if (LIVEP(l) && l->slvndx >= 0) {
	       slvdesc *sp = ego->slvdescs + l->slvndx;
	       /* qui salvandos salvas gratis
		  salva me fons pietatis */
	       p->print(p, "(%s %d #x%x #x%M #x%M #x%M #x%M)\n",
			sp->reg_nam, sp->reg_id, (int)l->flags,
			l->s[0], l->s[1], l->s[2], l->s[3]);
	  }
     }
     p->print(p, "%))\n");
}

/* mors stupebit et natura
   cum resurget creatura */
static int imprt(planner *ego, scanner *sc)
{
     char buf[MAXNAM + 1];
     md5uint sig[4];
     int flags;
     int reg_id;
     int slvndx;
     solution *sol;
     hashtab *ht = &ego->htab_blessed;

     if (!sc->scan(sc, "(" WISDOM_PREAMBLE))
	  return 0; /* don't need to restore hashtable */

     /* make a backup copy of the hash table (cache the hash) */
     {
	  unsigned h, hsiz = ht->hashsiz;
	  sol = (solution *)MALLOC(hsiz * sizeof(solution), HASHT);
	  for (h = 0; h < hsiz; ++h)
	       sol[h] = ht->solutions[h];
     }

     while (1) {
	  if (sc->scan(sc, ")"))
	       break;

	  /* qua resurget ex favilla */
	  if (!sc->scan(sc, "(%*s %d #x%x #x%M #x%M #x%M #x%M)",
			MAXNAM, buf, &reg_id, &flags, 
			sig + 0, sig + 1, sig + 2, sig + 3))
	       goto bad;

	  if ((slvndx = slookup(ego, buf, reg_id)) < 0)
	       goto bad;

	  /* inter oves locum praesta */
	  hinsert(ego, sig, (unsigned short)flags, slvndx);
     }

     X(ifree0)(sol);
     return 1;

 bad:
     /* ``The wisdom of FFTW must be above suspicion.'' */
     X(ifree0)(ht->solutions);
     ht->solutions = sol;
     return 0;
}

/*
 * create a planner
 */
planner *X(mkplanner)(void)
{
     static const planner_adt padt = {
	  register_solver, mkplan, forget, exprt, imprt
     };

     planner *p = (planner *) MALLOC(sizeof(planner), PLANNERS);

     p->adt = &padt;
     p->nplan = p->nprob = 0;
     p->pcost = p->epcost = 0.0;
     p->hook = 0;
     p->cur_reg_nam = 0;

     p->slvdescs = 0;
     p->nslvdesc = p->slvdescsiz = 0;

     p->problem_flags = 0;
     p->planner_flags = 0;
     p->nthr = 1;

     mkhashtab(&p->htab_blessed);
     mkhashtab(&p->htab_unblessed);

     return p;
}

void X(planner_destroy)(planner *ego)
{
     /* destroy hash table */
     htab_destroy(&ego->htab_blessed);
     htab_destroy(&ego->htab_unblessed);

     /* destroy solvdesc table */
     FORALL_SOLVERS(ego, s, sp, {
	  UNUSED(sp);
	  X(solver_destroy)(s);
     });

     X(ifree0)(ego->slvdescs);
     X(ifree)(ego); /* dona eis requiem */
}

plan *X(mkplan_d)(planner *ego, problem *p)
{
     plan *pln = ego->adt->mkplan(ego, p);
     X(problem_destroy)(p);
     return pln;
}

/*
 * Debugging code:
 */
#ifdef FFTW_DEBUG
static void check(hashtab *ht)
{
     unsigned live = 0;
     unsigned i;

     for (i = 0; i < ht->hashsiz; ++i) {
	  solution *l = ht->solutions + i; 
	  if (LIVEP(l)) 
	       ++live; 
     }

     A(ht->nelem == live);

     for (i = 0; i < ht->hashsiz; ++i) {
	  solution *l1 = ht->solutions + i; 
	  int foundit = 0;
	  if (LIVEP(l1)) {
	       unsigned g, h = h1(ht, l1->s), d = h2(ht, l1->s);

	       for (g = h; ; g = addmod(g, d, ht->hashsiz)) {
		    solution *l = ht->solutions + g;
		    ++ht->lookup_iter;
		    if (VALIDP(l)) {
			 if (l1 == l)
			      foundit = 1;
			 else if (LIVEP(l) && md5eq(l1->s, l->s)) {
			      A(!SUBSUMES(l->flags, l1->flags));
			      A(!SUBSUMES(l1->flags, l->flags));
			 }
		    } else 
			 break;
	       }

	       A(foundit);
	  }
     }
}
#endif
