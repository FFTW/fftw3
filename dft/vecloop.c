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

/* $Id: vecloop.c,v 1.1 2002-06-06 12:07:33 athena Exp $ */


/* Plans for handling vector transform loops.  These are *just* the
   loops, and rely on child plans for the actual DFTs.

   They form a wrapper around solvers that don't have apply functions
   for non-null vectors.

   vecloop plans also recursively handle the case of multi-dimensional
   vectors, obviating the need for most solvers to deal with this.  We
   can also play games here, such as reordering the vector loops.

   Each vecloop plan reduces the vector rank by 1, picking out a
   dimension determined by the vecloop_dim field of the solver. */

#include "dft.h"

typedef struct {
     solver super;
     int vecloop_dim;
     const int *buddies;
     uint nbuddies;
} S;

typedef struct {
     plan_dft super; 

     plan *cld;
     uint vl;
     int ivs, ovs;
     const S *solver;
} P;

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     uint i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;
     dftapply cldapply = ((plan_dft *) ego->cld)->apply;

     for (i = 0; i < vl; ++i) {
	  cldapply(ego->cld,
		   ri + i * ivs, ii + i * ivs, ro + i * ovs, io + i * ovs);
     }
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     ego->cld->adt->awake(ego->cld, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     fftw_plan_destroy(ego->cld);
     fftw_free(ego);
}

static void print(plan *ego_, plan_printf prntf)
{
     P *ego = (P *) ego_;
     const S *s = ego->solver;
     prntf("(DFT-VECLOOP-x%u/%d ", ego->vl, s->vecloop_dim);
     ego->cld->adt->print(ego->cld, prntf);
     prntf(")");
}

/* Given a solver vecloop_dim, a vector sz, and whether or not the
   transform is out-of-place, return the actual dimension index that
   it corresponds to.  The basic idea here is that we return the
   vecloop_dim'th valid dimension, starting from the end if
   vecloop_dim < 0. */
static int really_pickdim(int vecloop_dim, tensor vecsz, int oop)
{
     uint i;
     int count_ok = 0;
     if (vecloop_dim > 0) {
	  for (i = 0; i < vecsz.rnk; ++i) {
	       if (vecsz.dims[i].is == vecsz.dims[i].os || oop)
		    if (++count_ok == vecloop_dim)
			 return (int)i;
	  }
     } else if (vecloop_dim < 0) {
	  for (i = vecsz.rnk - 1; i >= 0; --i) {
	       if (vecsz.dims[i].is == vecsz.dims[i].os || oop)
		    if (++count_ok == -vecloop_dim)
			 return (int)i;
	  }
     }
     return -1;
}

static int pickdim(const S *ego, tensor vecsz, int oop)
{
     uint i;
     int d = really_pickdim(ego->vecloop_dim, vecsz, oop);

     /* check whether some buddy solver would have produced the same
        dim.  If so, consider this solver unapplicable and let the
        buddy take care of it.  The smallest buddy is applicable. */

     for (i = 0; i < ego->nbuddies; ++i) {
	  if (ego->buddies[i] < ego->vecloop_dim &&
	      d == really_pickdim(ego->buddies[i], vecsz, oop))
	       return -1;
     }

     return d;
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (DFTP(p_)) {
	  const S *ego = (const S *) ego_;
	  const problem_dft *p = (const problem_dft *) p_;

	  if (p->vecsz.rnk > 0 &&
	      pickdim(ego, p->vecsz, p->ri != p->ro) >= 0)
	       return 1;
     }

     return 0;
}

static enum score score(const solver *ego_, const problem *p_)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p = (const problem_dft *) p_;
     int vecloop_dim;
     int op;

     if (!applicable(ego_, p_))
	  return BAD;

     op = p->ri != p->ro;	/* out-of-place? */

     /* Heuristic: if the transform is multi-dimensional, and the
        vector stride is less than the transform size, then we
        probably want to use an dft-nd plan first in order to combine
        this vector with the transform-dimension vectors. */
     vecloop_dim = pickdim(ego, p->vecsz, op);
     if (p->sz.rnk > 1 &&
	 fftw_imin(p->vecsz.dims[vecloop_dim].is, 
		   p->vecsz.dims[vecloop_dim].os) <
	 fftw_tensor_max_index(p->sz))
	  return UGLY;

     /* Another heuristic: don't use a vecloop for rnk-0 transforms
        and a vector rnk-1, since this is handled by apply-null */
     if (p->sz.rnk == 0 && p->vecsz.rnk == 1)
	  return UGLY;

     return GOOD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *planner)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p;
     P *pln;
     plan *cld;
     problem *cldp;
     int vecloop_dim;

     static const plan_adt padt = { 
	  fftw_dft_solve, awake, print, destroy 
     };


     if (!applicable(ego_, p_))
	  return (plan *) 0;
     p = (const problem_dft *) p_;

     vecloop_dim = pickdim(ego, p->vecsz, p->ri != p->ro);
     cldp = 
	  fftw_mkproblem_dft_d(fftw_tensor_copy(p->sz),
			       fftw_tensor_copy_except(p->vecsz, 
						       (uint) vecloop_dim),
			       p->ri, p->ii, p->ro, p->io);
     cld = planner->mkplan(planner, cldp);
     fftw_problem_destroy(cldp);
     if (!cld)
	  return (plan *) 0;

     pln = MKPLAN_DFT(P, &padt, apply);

     pln->cld = cld;
     pln->vl = p->vecsz.dims[vecloop_dim].n;
     pln->ivs = p->vecsz.dims[vecloop_dim].is;
     pln->ovs = p->vecsz.dims[vecloop_dim].os;

     pln->solver = ego;
     pln->super.super.cost = cld->cost * pln->vl;
     pln->super.super.flops = fftw_flops_mul(pln->vl, cld->flops);

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

void fftw_dft_vecloop_register(planner *p)
{
     uint i;

     /* FIXME: Should we try other vecloop_dim values? */
     static const int buddies[] = { 1, -1 };

     const uint nbuddies = sizeof(buddies) / sizeof(buddies[0]);

     for (i = 0; i < nbuddies; ++i)
	  p->adt->register_solver(p, mksolver(buddies[i], buddies, nbuddies));
}
