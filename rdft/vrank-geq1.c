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

/* $Id: vrank-geq1.c,v 1.2 2002-08-04 21:03:45 stevenj Exp $ */


/* Plans for handling vector transform loops.  These are *just* the
   loops, and rely on child plans for the actual RDFTs.
 
   They form a wrapper around solvers that don't have apply functions
   for non-null vectors.
 
   vrank-geq1 plans also recursively handle the case of multi-dimensional
   vectors, obviating the need for most solvers to deal with this.  We
   can also play games here, such as reordering the vector loops.
 
   Each vrank-geq1 plan reduces the vector rank by 1, picking out a
   dimension determined by the vecloop_dim field of the solver. */

#include "rdft.h"

typedef struct {
     solver super;
     int vecloop_dim;
     const int *buddies;
     uint nbuddies;
} S;

typedef struct {
     plan_rdft super;

     plan *cld;
     uint vl;
     int ivs, ovs;
     const S *solver;
} P;

static void apply(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     uint i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;
     rdftapply cldapply = ((plan_rdft *) ego->cld)->apply;

     for (i = 0; i < vl; ++i) {
          cldapply(ego->cld, I + i * ivs, O + i * ovs);
     }
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cld);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     const S *s = ego->solver;
     p->print(p, "(rdft-vrank>=1-x%u/%d%(%p%))",
	      ego->vl, s->vecloop_dim, ego->cld);
}

/* Given a solver vecloop_dim, a vector sz, and whether or not the
   transform is out-of-place, return the actual dimension index that
   it corresponds to.  The basic idea here is that we return the
   vecloop_dim'th valid dimension, starting from the end if
   vecloop_dim < 0. */
static int really_pickdim(int vecloop_dim, tensor vecsz, int oop, uint *vdim)
{
     uint i;
     int count_ok = 0;
     if (vecloop_dim > 0) {
          for (i = 0; i < vecsz.rnk; ++i) {
               if (vecsz.dims[i].is == vecsz.dims[i].os || oop)
                    if (++count_ok == vecloop_dim) {
                         *vdim = i;
                         return 1;
                    }
          }
     } else if (vecloop_dim < 0) {
          for (i = vecsz.rnk; i > 0; --i) {
               if (vecsz.dims[i - 1].is == vecsz.dims[i - 1].os || oop)
                    if (++count_ok == -vecloop_dim) {
                         *vdim = i - 1;
                         return 1;
                    }
          }
     }
     return 0;
}

static int pickdim(const S *ego, tensor vecsz, int oop, uint *dp)
{
     uint i, d1;

     if (!really_pickdim(ego->vecloop_dim, vecsz, oop, dp))
          return 0;

     /* check whether some buddy solver would produce the same dim.
        If so, consider this solver unapplicable and let the buddy
        take care of it.  The smallest-indexed buddy is applicable. */
     for (i = 0; i < ego->nbuddies; ++i) {
	  if (ego->buddies[i] == ego->vecloop_dim)
	       break;  /* found self */
          if (really_pickdim(ego->buddies[i], vecsz, oop, &d1) && *dp == d1)
               return 0; /* found equivalent buddy */
     }
     return 1;
}

static int applicable(const solver *ego_, const problem *p_, uint *dp)
{
     if (RDFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;

          return (1
                  && FINITE_RNK(p->vecsz.rnk)
                  && p->vecsz.rnk > 0
                  && pickdim(ego, p->vecsz, p->I != p->O, dp)
	       );
     }

     return 0;
}

static int score(const solver *ego_, const problem *p_, const planner *plnr)
{
     const S *ego = (const S *)ego_;
     const problem_rdft *p;
     uint vdim;

     if (!applicable(ego_, p_, &vdim))
          return BAD;

     /* fftw2 behavior */
     if ((plnr->flags & CLASSIC) && (ego->vecloop_dim != ego->buddies[0]))
	  return BAD;

     p = (const problem_rdft *) p_;

     /* fftw2-like heuristic: once we've started vector-recursing,
	don't stop (unless we have to) */
     if ((plnr->flags & FORCE_VRECURSE) && p->vecsz.rnk == 1)
	  return UGLY;

     /* Heuristic: if the transform is multi-dimensional, and the
        vector stride is less than the transform size, then we
        probably want to use a rank>=2 plan first in order to combine
        this vector with the transform-dimension vectors. */
     {
	  iodim *d = p->vecsz.dims + vdim;
	  if (1
	      && p->sz.rnk > 1 
	      && X(imin)(d->is, d->os) < X(tensor_max_index)(p->sz)
	       )
          return UGLY;
     }

     /* Heuristic: don't use a vrank-geq1 for rank-0 vrank-1
	transforms, since this case is better handled by rank-0
	solvers. */
     if (p->sz.rnk == 0 && p->vecsz.rnk == 1)
	  return UGLY;

     return GOOD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_rdft *p;
     P *pln;
     plan *cld;
     problem *cldp;
     uint vdim;
     iodim *d;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, &vdim))
          return (plan *) 0;
     p = (const problem_rdft *) p_;

     /* fftw2 vector recursion: use it or lose it */
     if (p->vecsz.rnk == 1 && (plnr->flags & CLASSIC_VRECURSE))
	  plnr->flags &= ~CLASSIC_VRECURSE & ~FORCE_VRECURSE;

     cldp = X(mkproblem_rdft_d)(X(tensor_copy)(p->sz),
				X(tensor_copy_except)(p->vecsz, vdim),
				p->I, p->O, p->kind);
     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld)
          return (plan *) 0;

     pln = MKPLAN_RDFT(P, &padt, apply);

     pln->cld = cld;
     d = p->vecsz.dims + vdim;
     pln->vl = d->n;
     pln->ivs = d->is;
     pln->ovs = d->os;

     pln->solver = ego;
     pln->super.super.ops = X(ops_mul)(pln->vl, cld->ops);
     pln->super.super.pcost = pln->vl * cld->pcost;

     return &(pln->super.super);
}

static solver *mksolver(int vecloop_dim, const int *buddies, uint nbuddies)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->vecloop_dim = vecloop_dim;
     slv->buddies = buddies;
     slv->nbuddies = nbuddies;
     return &(slv->super);
}

void X(rdft_vrank_geq1_register)(planner *p)
{
     uint i;

     /* FIXME: Should we try other vecloop_dim values? */
     static const int buddies[] = { 1, -1 };

     const uint nbuddies = sizeof(buddies) / sizeof(buddies[0]);

     for (i = 0; i < nbuddies; ++i)
          REGISTER_SOLVER(p, mksolver(buddies[i], buddies, nbuddies));
}
