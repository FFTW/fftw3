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

/* $Id: planner.c,v 1.85 2002-09-14 20:35:28 stevenj Exp $ */
#include "ifftw.h"
#include <string.h>

#define IMPATIENCE(flags) ((flags) & IMPATIENCE_MASK)
#define BLESSEDP(solution) ((solution)->flags & BLESSING)
#define VALIDP(solution) ((solution)->flags & H_VALID)

#define MAXNAM 64  /* maximum length of registrar's name.
		      Used for reading wisdom.  There is no point
		      in doing this right */
		      
/*
  slvdesc management:
*/
static uint hash_regnam(const char *s)
{
     uint h = 0xDEADBEEFul;
     do {
	  h = h * 17 + *s;
     } while (*s++);
     return h;
}

static void sgrow(planner *ego)
{
     uint osiz = ego->slvdescsiz, nsiz = 1 + osiz + osiz / 4;
     slvdesc *ntab = (slvdesc *)fftw_malloc(nsiz * sizeof(slvdesc), SLVDESCS);
     slvdesc *otab = ego->slvdescs;
     uint i;

     ego->slvdescs = ntab;
     ego->slvdescsiz = nsiz;
     for (i = 0; i < osiz; ++i)
	  ntab[i] = otab[i];
     if (otab) 
	  X(free)(otab);
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
	  n->nam_hash = hash_regnam(n->reg_nam);
     }
}

static short slookup(planner *ego, char *nam, int id)
{
     uint h = hash_regnam(nam); /* used to avoid strcmp in the common case */
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
static uint h1(planner *ego, md5uint *s)
{
     return s[0] % ego->hashsiz;
}

/* second hash function (for double hashing) */
static uint h2(planner *ego, md5uint *s)
{
     return 1 + s[1] % (ego->hashsiz - 1);
}

static void md5hash(md5 *m, const problem *p, const planner *plnr)
{
     X(md5begin)(m);
     X(md5uint)(m, sizeof(R)); /* so we don't mix different precisions */
     X(md5uint)(m, plnr->problem_flags);
     X(md5uint)(m, plnr->nthr);
     p->adt->hash(p, m);
     X(md5end)(m);
}

static int md5eq(md5uint *a, md5uint *b)
{
     return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static void sigcpy(md5uint *a, md5uint *b)
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
     md5uint s[4];
     unsigned short flags;
     short slvndx;
};

static solution *hlookup(planner *ego, md5uint *s)
{
     uint g, h = h1(ego, s), d = h2(ego, s);

     ++ego->lookup;

     for (g = h; ; g = (g + d) % ego->hashsiz) {
	  solution *l = ego->solutions + g;
	  ++ego->lookup_iter;
	  if (VALIDP(l)) {
	       if (md5eq(s, l->s)) { ++ego->succ_lookup; return l; }
	  } else {
	       return 0;
	  }
	  A((g + d) % ego->hashsiz != h);
     }
}


static void hinsert0(planner *ego, md5uint *s, unsigned short flags,
		     short slvndx, solution *l)
{
     ++ego->insert;
     if (!l) { 	 
	  /* search for nonfull slot */
	  uint g, h = h1(ego, s), d = h2(ego, s); 
	  ++ego->insert_unknown;
	  for (g = h; ; g = (g + d) % ego->hashsiz) {
	       ++ego->insert_iter;
	       l = ego->solutions + g;
	       if (!VALIDP(l)) break;
	       A((g + d) % ego->hashsiz != h);
	  }
     }

     /* fill slot */
     l->flags = flags | H_VALID;
     l->slvndx = slvndx;
     sigcpy(s, l->s);
}

static void rehash(planner *ego, uint nsiz)
{
     uint osiz = ego->hashsiz, h;
     solution *osol = ego->solutions, *nsol;

     nsiz = X(next_prime)(nsiz);
     nsol = (solution *)fftw_malloc(nsiz * sizeof(solution), HASHT);
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

     if (osol)
	  X(free)(osol);
}

static uint minsz(uint nelem)
{
     return 1 + nelem + nelem / 8;
}

static uint nextsz(uint nelem)
{
     return minsz(minsz(nelem));
}

static void hgrow(planner *ego)
{
     uint nelem = ego->nelem;
     if (minsz(nelem) >= ego->hashsiz)
	  rehash(ego, nextsz(nelem));
}

static void hshrink(planner *ego)
{
     uint nelem = ego->nelem;
     /* always rehash after deletions */
     rehash(ego, nextsz(nelem));
}

static void hinsert(planner *ego, md5uint *s, 
		    unsigned short flags, short slvndx)
{
     solution *l;

     if ((l = hlookup(ego, s))) {
	  /* overwrite old solution */
	  if (IMPATIENCE(flags) > IMPATIENCE(l->flags))
	       return; /* don't overwrite less impatient solution */

	  flags |= l->flags & BLESSING; /* ne me perdas illa die */
     } else {
	  ++ego->nelem;
	  hgrow(ego);
     }
     hinsert0(ego, s, flags, slvndx, l);
}

/* maintain dynamic scoping of flags, nthr: */
static plan *slv_mkplan(planner *ego, problem *p, solver *s)
{
     uint problem_flags = ego->problem_flags;
     unsigned short planner_flags = ego->planner_flags;
     uint nthr = ego->nthr;
     plan *pln;
     pln = s->adt->mkplan(s, p, ego);
     ego->problem_flags = problem_flags;
     ego->planner_flags = planner_flags;
     ego->nthr = nthr;
     return pln;
}

static plan *mkplan(planner *ego, problem *p)
{
     solution *sol;
     plan *pln;
     md5 m;
     slvdesc *sp;

     ++ego->nprob;

     md5hash(&m, p, ego);

     if ((sol = hlookup(ego, m.s))) {
	  if (sol->slvndx < 0) return 0;   /* known to be infeasible */
	  sp = ego->slvdescs + sol->slvndx;

	  /* reject wisdom if too impatient */
	  if (IMPATIENCE(sol->flags) > IMPATIENCE(ego->planner_flags))
	       sp = 0;
     } else {
	  sp = 0; /* nothing known about this problem */
     }

     ego->inferior_mkplan(ego, p, &pln, &sp);
     hinsert(ego, m.s, ego->planner_flags, 
	     sp ? sp - ego->slvdescs : -1);

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
     X(free)(ego->solutions);
     ego->nelem = 0;
}

/* FIXME: what sort of version information should we write? */
#define WISDOM_PREAMBLE PACKAGE "-" VERSION "-wisdom "

/* tantus labor non sit cassus */
static void exprt(planner *ego, printer *p)
{
     uint h;

     p->print(p, "(" WISDOM_PREAMBLE "%(");
     for (h = 0; h < ego->hashsiz; ++h) {
	  solution *l = ego->solutions + h;
	  if (VALIDP(l) && BLESSEDP(l) && l->slvndx >= 0) {
	       slvdesc *sp = ego->slvdescs + l->slvndx;
	       /* qui salvandos salvas gratis
		  salva me fons pietatis */
	       p->print(p, "(%s %d #x%x #x%M #x%M #x%M #x%M)\n",
			sp->reg_nam, sp->reg_id, (uint)l->flags,
			l->s[0], l->s[1], l->s[2], l->s[3]);
	  }
     }
     p->print(p, "%))\n");
}

static int imprt(planner *ego, scanner *sc)
{
     char buf[MAXNAM + 1];
     md5uint sig[4];
     uint flags;
     int reg_id;
     short slvndx;

     if (!sc->scan(sc, "(" WISDOM_PREAMBLE))
	  goto bad;


     while (1) {
	  if (sc->scan(sc, ")"))
	       break;

	  if (!sc->scan(sc, "(%*s %d #x%x #x%M #x%M #x%M #x%M)",
			MAXNAM, buf, &reg_id, &flags, 
			sig + 0, sig + 1, sig + 2, sig + 3))
	       goto bad;

	  if ((slvndx = slookup(ego, buf, reg_id)) < 0)
	       goto bad; /* TODO: panic? */

	  /* inter oves locum praesta */
	  hinsert(ego, sig, (unsigned short)flags, slvndx);
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
					plan **, slvdesc **))
{
     static const planner_adt padt = {
	  register_solver,
	  mkplan, forget, exprt, imprt, slv_mkplan
     };

     planner *p = (planner *) fftw_malloc(sz, PLANNERS);

     p->adt = &padt;
     p->inferior_mkplan = infmkplan;
     p->nplan = p->nprob = p->nrehash = 0;
     p->succ_lookup = p->lookup = p->lookup_iter = 0;
     p->insert = p->insert_iter = p->insert_unknown = 0;
     p->hook = hooknil;
     p->cur_reg_nam = 0;

     p->slvdescs = 0;
     p->nslvdesc = p->slvdescsiz = 0;

     p->solutions = 0;
     p->hashsiz = p->nelem = 0;

     p->score = BAD;            /* unused, but we initialize it anyway */
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

     if (ego->slvdescs)
	  X(free)(ego->slvdescs);

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
     if (!(ego->planner_flags & IMPATIENT) || pln->pcost == 0.0) {
	  ego->nplan++;
	  if (ego->planner_flags & ESTIMATE) {
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
     uint valid = 0, empty = 0;
     uint h;
     UNUSED(verbose); /* historical */

     for (h = 0; h < ego->hashsiz; ++h) 
	  if (VALIDP(ego->solutions + h)) ++valid; else ++empty;

     D("nplan = %u\n", ego->nplan);
     D("nprob = %u\n", ego->nprob);
     D("lookup = %u\n", ego->lookup);
     D("succ_lookup = %u\n", ego->succ_lookup);
     D("lookup_iter = %u\n", ego->lookup_iter);
     D("insert = %u\n", ego->insert);
     D("insert_iter = %u\n", ego->insert_iter);
     D("insert_unknown = %u\n", ego->insert_unknown);
     D("nrehash = %d\n", ego->nrehash);
     D("hashsiz = %d\n", ego->hashsiz);
     D("empty = %d\n", empty);
     D("valid = %d\n", valid);
     A(ego->nelem == valid);
}
#endif
