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

/* $Id: planner.c,v 1.152 2003-05-07 18:24:46 athena Exp $ */
#include "ifftw.h"
#include <string.h>

/* GNU Coding Standards, Sec. 5.2: "Please write the comments in a GNU
   program in English, because English is the one language that nearly
   all programmers in all countries can read."

                    ingemisco tanquam reus
		    culpa rubet vultus meus
		    supplicanti parce [rms]
*/

#define BLESSEDP(solution) ((solution)->flags & BLESSING)
#define VALIDP(solution) ((solution)->flags & H_VALID)
#define UNBLESS(flags)  flags &= ~BLESSING

#define MAXNAM 64  /* maximum length of registrar's name.
		      Used for reading wisdom.  There is no point
		      in doing this right */

/* Flags f1 subsumes flags f2 iff f1 is less/equally impatient than
   f2, defining a partial ordering. */
#define IMPATIENCE(flags) ((flags) & IMPATIENCE_FLAGS)
#define NONIMPATIENCE(flags) ((flags) & NONIMPATIENCE_FLAGS)
#define ORDERED(f1, f2) (SUBSUMES(f1, f2) || SUBSUMES(f2, f1))
#define SUBSUMES(f1, f2) ((IMPATIENCE(f1) & (f2)) == IMPATIENCE(f1))

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

static int slookup(planner *ego, char *nam, int id)
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
static unsigned h1(planner *ego, const md5sig s)
{
     return s[0] % ego->hashsiz;
}

/* second hash function (for double hashing) */
static unsigned h2(planner *ego, const md5sig s)
{
     return 1U + s[1] % (ego->hashsiz - 1);
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

static solution *hlookup(planner *ego, const md5sig s, unsigned short flags)
{
     unsigned g, h = h1(ego, s), d = h2(ego, s);

     ++ego->lookup;

     for (g = h; ; g = addmod(g, d, ego->hashsiz)) {
	  solution *l = ego->solutions + g;
	  ++ego->lookup_iter;
	  if (VALIDP(l)) {
	       if (md5eq(s, l->s) && ORDERED(l->flags, flags)) { 
		    ++ego->succ_lookup;
		    return l; 
	       }
	  } else {
	       return 0;
	  }
	  A((g + d) % ego->hashsiz != h);
     }
}


static void hinsert0(planner *ego, const md5sig s, unsigned short flags,
		     int slvndx, solution *l)
{
     ++ego->insert;
     if (!l) { 	 
	  /* search for nonfull slot */
	  unsigned g, h = h1(ego, s), d = h2(ego, s); 
	  ++ego->insert_unknown;
	  for (g = h; ; g = addmod(g, d, ego->hashsiz)) {
	       ++ego->insert_iter;
	       l = ego->solutions + g;
	       if (!VALIDP(l)) break;
	       A((g + d) % ego->hashsiz != h);
	  }
     }

     /* fill slot */
     l->flags = flags | H_VALID;
     l->slvndx = (short)slvndx;
     sigcpy(s, l->s);
}

static void rehash(planner *ego, unsigned nsiz)
{
     unsigned osiz = ego->hashsiz, h;
     solution *osol = ego->solutions, *nsol;

     nsiz = (unsigned)X(next_prime)((int)nsiz);
     nsol = (solution *)MALLOC(nsiz * sizeof(solution), HASHT);
     ++ego->nrehash;

     /* init new table */
     for (h = 0; h < nsiz; ++h) 
	  nsol[h].flags = 0;

     /* install new table */
     ego->hashsiz = nsiz;
     ego->solutions = nsol;

     /* copy table */
     for (h = 0; h < osiz; ++h) {
	  solution *l = osol + h;
	  if (VALIDP(l))
	       hinsert0(ego, l->s, l->flags, l->slvndx, 0);
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

static void hgrow(planner *ego)
{
     unsigned nelem = ego->nelem;
     if (minsz(nelem) >= ego->hashsiz)
	  rehash(ego, nextsz(nelem));
}

static void hshrink(planner *ego)
{
     unsigned nelem = ego->nelem;
     /* always rehash after deletions */
     rehash(ego, nextsz(nelem));
}

/* inherit blessing, but only if the solver is the same */
static unsigned short merge_flags(unsigned short dstflags, int dstndx,
				  unsigned short srcflags, int srcndx)
{
     if (srcndx == dstndx)
	  dstflags |= (srcflags & BLESSING); /* ne me perdas illa die */
     return dstflags;
}

static void hinsert(planner *ego, const md5sig s, 
		    unsigned short flags, int slvndx)
{
     solution *l;

     if ((l = hlookup(ego, s, flags))) {
	  if (SUBSUMES(flags, l->flags)) {
	       /* overwrite old solution */
	       flags = merge_flags(flags, slvndx, l->flags, l->slvndx);
	  } else {
	       A(SUBSUMES(l->flags, flags));
	       l->flags = merge_flags(l->flags, l->slvndx, flags, slvndx);
	       return;
	  }
     } else {
	  ++ego->nelem;
	  hgrow(ego);
     }
     hinsert0(ego, s, flags, slvndx, l);
}

static void hcurse_subsumed(planner *ego)
{
     unsigned h;

     /* unbless any entries that are unreachable because they
        are subsumed by less-impatient ones.  */
     for (h = 0; h < ego->hashsiz; ++h) {
	  solution *l = ego->solutions + h;
	  if (VALIDP(l)) {
	       unsigned d = h2(ego, l->s), g = addmod(h, d, ego->hashsiz);
	       for (; ; g = addmod(g, d, ego->hashsiz)) {
		    solution *m = ego->solutions + g;
		    if (VALIDP(m)) {
			 if (md5eq(l->s, m->s) &&
			     SUBSUMES(l->flags, m->flags)) {
			      /* ne cadant in obscurum */
			      l->flags = merge_flags(l->flags, l->slvndx,
						     m->flags, m->slvndx);

			      /* cum vix justus sit securus */
			      UNBLESS(m->flags);
			 }
		    }
		    else break;
		    A((g + d) % ego->hashsiz != h);
	       }
	  }
     }
}


static void invoke_hook(planner *ego, plan *pln, const problem *p, 
			int optimalp)
{
     if (ego->hook)
	  ego->hook(pln, p, optimalp);
}

static void evaluate_plan(planner *ego, plan *pln, const problem *p)
{
     if (!BELIEVE_PCOSTP(ego) || pln->pcost == 0.0) {
	  ego->nplan++;

	  if (ESTIMATEP(ego)) {
	  estimate:
	       /* heuristic */
	       pln->pcost = 0.0
		    + pln->ops.add
		    + pln->ops.mul
		    + 2 * pln->ops.fma
		    + pln->ops.other;
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

static plan *search(planner *ego, problem *p, slvdesc **descp)
{
     plan *best = 0;
     int best_not_yet_timed = 1;
     int pass;

     if (NO_SEARCHP(ego)) {
	  /* D("invalid search for %P %x\n", p, ego->planner_flags); */
	  return 0;
     }

     for (pass = 0; pass < 2; ++pass) {
	  unsigned short nflags = ego->planner_flags;
	  
	  if (best) break;

	  switch (pass) {
	      case 0: 
		   /* skip pass 0 during exhaustive search */
		   if (!NO_EXHAUSTIVEP(ego)) continue;
		   nflags |= NO_UGLY;
		   break;
	      case 1:
		   /* skip pass 1 if NO_UGLY */
		   if (NO_UGLYP(ego)) continue;
		   break;
	  }

          FORALL_SOLVERS(ego, s, sp, {
	       plan *pln = invoke_solver(ego, p, s, nflags);

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
			      *descp = sp;
			 } else {
			      X(plan_destroy_internal)(pln);
			 }
		    } else {
			 best = pln;
			 *descp = sp;
		    }
	       }
	  });
     }

     return best;
}

static plan *mkplan(planner *ego, problem *p)
{
     plan *pln;
     md5 m;
     slvdesc *sp;
     unsigned short flags;
     ASSERT_ALIGNED_DOUBLE;

     /* Canonical form. */
     if (!NO_EXHAUSTIVEP(ego)) ego->planner_flags &= ~NO_UGLY;
	  
     ++ego->nprob;
     md5hash(&m, p, ego);

     pln = 0;

     {
	  solution *sol; /* new scope for sol */

	  if ((sol = hlookup(ego, m.s, ego->planner_flags))) {
	       if (SUBSUMES(sol->flags, ego->planner_flags)) {
		    /* wisdom is acceptable */
		    if (sol->slvndx < 0) 
			 return 0;   /* known to be infeasible */

		    /* use solver to obtain a plan */
		    sp = ego->slvdescs + sol->slvndx;
		    pln = 
			 invoke_solver(ego, p, sp->slv, 
				       (0
					| NO_SEARCH 
					| IMPATIENCE(sol->flags)
					| NONIMPATIENCE(ego->planner_flags) ));

		    /* if (!pln) then the entry is bogus, but
		       we currently do nothing about it. */
		    /* CAVEAS: Do not use ``sol'' here, because the
		       pointer is possibly dangling after the call to
		       invoke_solver(). */
	       } else {
		    A(SUBSUMES(ego->planner_flags, sol->flags));
	       }
	  }
     }


     if (!pln)
	  pln = search(ego, p, &sp);

     flags = ego->planner_flags;

     if (pln) {
	  /* Postulate de iure that NO_UGLY subsumes ~NO_UGLY if the
	     problem is feasible. Also postulate that NO_SEARCH
	     subsumes ~NO_SEARCH. */
	  flags &= ~(NO_UGLY | NO_SEARCH);
     }

     hinsert(ego, m.s, flags, pln ? sp - ego->slvdescs : -1);

     if (pln)
	  invoke_hook(ego, pln, p, 1);
     return pln;
}

/* destroy hash table entries.  If FORGET_EVERYTHING, destroy the whole
   table.  If FORGET_ACCURSED, then destroy entries that are not blessed. */
static void forget(planner *ego, amnesia a)
{
     unsigned h;

     /* garbage-collect while we are at it */ 
     if (a != FORGET_EVERYTHING)
	  hcurse_subsumed(ego);

     for (h = 0; h < ego->hashsiz; ++h) {
	  solution *l = ego->solutions + h;
	  if (VALIDP(l)) {
	       if (a == FORGET_EVERYTHING ||
		   (a == FORGET_ACCURSED && !BLESSEDP(l))) {
		    /* confutatis maledictis
		       flammis acribus addictis */
		    l->flags &= ~H_VALID;
		    --ego->nelem;
	       }
	  }
     }
     /* nil inultum remanebit */

     hshrink(ego);
}

static void htab_destroy(planner *ego)
{
     forget(ego, FORGET_EVERYTHING);
     X(ifree)(ego->solutions);
     ego->nelem = 0U;
}

/* FIXME: what sort of version information should we write? */
#define WISDOM_PREAMBLE PACKAGE "-" VERSION " " STRINGIZE(X(wisdom))

/* tantus labor non sit cassus */
static void exprt(planner *ego, printer *p)
{
     unsigned h;

     hcurse_subsumed(ego);

     p->print(p, "(" WISDOM_PREAMBLE "%(");
     for (h = 0; h < ego->hashsiz; ++h) {
	  solution *l = ego->solutions + h;
	  if (VALIDP(l) && BLESSEDP(l) && l->slvndx >= 0) {
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

     if (!sc->scan(sc, "(" WISDOM_PREAMBLE))
	  return 0; /* don't need to restore hashtable */

     /* make a backup copy of the hash table (cache the hash) */
     {
	  unsigned h, hsiz = ego->hashsiz;
	  sol = (solution *)MALLOC(hsiz * sizeof(solution), HASHT);
	  for (h = 0; h < hsiz; ++h)
	       sol[h] = ego->solutions[h];
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
     X(ifree0)(ego->solutions);
     ego->solutions = sol;
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
     p->nplan = p->nprob = p->nrehash = 0;
     p->pcost = p->epcost = 0.0;
     p->succ_lookup = p->lookup = p->lookup_iter = 0;
     p->insert = p->insert_iter = p->insert_unknown = 0;
     p->hook = 0;
     p->cur_reg_nam = 0;

     p->slvdescs = 0;
     p->nslvdesc = p->slvdescsiz = 0;

     p->solutions = 0;
     p->hashsiz = p->nelem = 0U;

     p->problem_flags = 0;
     p->planner_flags = 0;
     p->nthr = 1;

     hgrow(p);			/* so that hashsiz > 0 */

     return p;
}

void X(planner_destroy)(planner *ego)
{
     /* destroy hash table */
     htab_destroy(ego);

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

void X(planner_dump)(planner *ego, int verbose)
{
     unsigned valid = 0, empty = 0, infeasible = 0;
     unsigned h;
     UNUSED(verbose); /* historical */

     for (h = 0; h < ego->hashsiz; ++h) {
	  solution *l = ego->solutions + h; 
	  if (VALIDP(l)) {
	       ++valid; 
	       if (l->slvndx < 0) ++infeasible;
	  } else
	       ++empty;
	  
     }

     D("nplan = %d\n", ego->nplan);
     D("nprob = %d\n", ego->nprob);
     D("pcost = %g\n", ego->pcost);
     D("epcost = %g\n", ego->epcost);
     D("lookup = %d\n", ego->lookup);
     D("succ_lookup = %d\n", ego->succ_lookup);
     D("lookup_iter = %d\n", ego->lookup_iter);
     D("insert = %d\n", ego->insert);
     D("insert_iter = %d\n", ego->insert_iter);
     D("insert_unknown = %d\n", ego->insert_unknown);
     D("nrehash = %d\n", ego->nrehash);
     D("hashsiz = %u\n", ego->hashsiz);
     D("empty = %d\n", empty);
     D("valid = %d\n", valid);
     D("infeasible = %d\n", infeasible);
     A(ego->nelem == valid);
}

#endif
