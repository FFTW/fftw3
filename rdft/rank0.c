/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* $Id: rank0.c,v 1.26 2005-02-27 03:21:03 athena Exp $ */

/* plans for rank-0 RDFTs (copy operations) */

#include "rdft.h"

#ifdef HAVE_STRING_H
#include <string.h>		/* for memcpy() */
#endif

#define MAXRNK 3

typedef struct {
     plan_rdft super;
     int vl;
     int rnk;
     iodim d[MAXRNK];
     const char *nam;
} P;

typedef struct {
     solver super;
     rdftapply apply;
     int (*applicable)(const P *pln, const problem_rdft *p);
     const char *nam;
} S;

/* copy up to MAXRNK dimensions from problem into plan.  If a
   contiguous dimension exists, save its length in pln->vl */
static int fill_iodim(P *pln, const problem_rdft *p)
{
     int i;
     const tensor *vecsz = p->vecsz;

     pln->vl = 1;
     pln->rnk = 0;
     for (i = 0; i < vecsz->rnk; ++i) {
	  /* extract contiguous dimensions */
	  if (pln->vl == 1 &&
	      vecsz->dims[i].is == 1 && vecsz->dims[i].os == 1) 
	       pln->vl = vecsz->dims[i].n;
	  else if (pln->rnk == MAXRNK) 
	       return 0;
	  else 
	       pln->d[pln->rnk++] = vecsz->dims[i];
     }

     return 1;
}

/* generic higher-rank copy routine, calls cpy2d() to do the real work */
		 
/* note that this routine is not subsumed by vrank-geq1, because
   vrank-geq1 first optimizes the subproblem, which is definitely
   the wrong thing to do for copies. */
static void copy(const P *ego, R *I, R *O,
		 void (*cpy2d)(R *I, R *O,
			       int n0, int is0, int os0,
			       int n1, int is1, int os1, int vl))
{
     switch (ego->rnk) {
	 case 2:
	      cpy2d(I, O, 
		    ego->d[0].n, ego->d[0].is, ego->d[0].os,
		    ego->d[1].n, ego->d[1].is, ego->d[1].os,
		    ego->vl);
	      break;
	 case 3:
	 {
	      int n1;
	      /* loop over n1 is the right thing for transpositions.
		 Otherwise, let the planner pick the right loop order.
	      */
	      for (n1 = 0; n1 < ego->d[1].n; 
		   ++n1, I += ego->d[1].is, O += ego->d[1].os) {
		   cpy2d(I, O, 
			 ego->d[0].n, ego->d[0].is, ego->d[0].os,
			 ego->d[2].n, ego->d[2].is, ego->d[2].os,
			 ego->vl);
	      }
	      break;
	 }
	 default:
	      A(0 /* can't happen */); 
     }
}

/**************************************************************/
/* rank 0,1,2, out of place, iterative */
static void apply_iter(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;

     switch (ego->rnk) {
	 case 0: 
	      X(cpy1d)(I, O, ego->vl, 1, 1, 1);
	      break;
	 case 1:
	      X(cpy1d)(I, O, 
		       ego->d[0].n, ego->d[0].is, ego->d[0].os, 
		       ego->vl);
	      break;
	 default:
	      copy(ego, I, O, X(cpy2d));
	      break;
     }
}

static int applicable_iter(const P *pln, const problem_rdft *p)
{
     UNUSED(pln);
     return (p->I != p->O);
}

/**************************************************************/
/* out of place, tiled, no buffering */
static void apply_tiled(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     copy(ego, I, O, X(cpy2d_tiled));
}

static int applicable_tiled(const P *pln, const problem_rdft *p)
{
     return (1
	     && p->I != p->O
	     && pln->rnk >= 2

	     /* somewhat arbitrary */
	     && pln->vl < (int)(CACHESIZE / (16 * sizeof(R)))
	  );
}

/**************************************************************/
/* out of place, tiled, with buffer */
static void apply_tiledbuf(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     copy(ego, I, O, X(cpy2d_tiledbuf));
}

#define applicable_tiledbuf applicable_tiled

/**************************************************************/
/* rank 0, out of place, using memcpy */
static void apply_memcpy(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;

     A(ego->rnk == 0);
     memcpy(O, I, ego->vl * sizeof(R));
}

static int applicable_memcpy(const P *pln, const problem_rdft *p)
{
     return (1
	     && p->I != p->O 
	     && pln->rnk == 0
	     && pln->vl > 2 /* do not bother memcpy-ing complex numbers */
	     );
}

/**************************************************************/
/* rank 2, in place, square transpose, iterative */
static void apply_ip_sq(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     UNUSED(O);
     X(transpose)(I, ego->d[0].n, ego->d[0].is, ego->d[0].os, ego->vl);
}

static int transposep(const P *pln)
{
     int i;
     for (i = 0; i < pln->rnk; ++i) {
	  const iodim *a = pln->d + i;
	  const iodim *b = pln->d + pln->rnk - i - 1;
	  if (a->n != b->n || a->is != b->os)
	       return 0;
     }
     return 1;
}

static int applicable_ip_sq(const P *pln, const problem_rdft *p)
{
     return (1
	     && p->I == p->O
	     && pln->rnk == 2
	     && transposep(pln));
}

/**************************************************************/
/* rank 2, in place, square transpose, tiled */
static void apply_ip_sq_tiled(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     UNUSED(O);
     X(transpose_tiled)(I, ego->d[0].n, ego->d[0].is, ego->d[0].os, ego->vl);
}

static int applicable_ip_sq_tiled(const P *pln, const problem_rdft *p)
{
     return (1
	     && applicable_ip_sq(pln, p)

	     /* somewhat arbitrary */
	     && pln->vl * 2 < (int)(CACHESIZE / (16 * sizeof(R)))
	  );
}

/**************************************************************/
/* rank 2, in place, square transpose, tiled, buffered */
static void apply_ip_sq_tiledbuf(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     UNUSED(O);
     X(transpose_tiledbuf)(I, ego->d[0].n, ego->d[0].is, ego->d[0].os,
			   ego->vl);
}

#define applicable_ip_sq_tiledbuf applicable_ip_sq_tiled

/**************************************************************/
static int applicable(const S *ego, const problem *p_)
{
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
	  P pln;
          return (1
                  && p->sz->rnk == 0
		  && FINITE_RNK(p->vecsz->rnk)
		  && fill_iodim(&pln, p)
		  && ego->applicable(&pln, p)
	       );
     }
     return 0;
}


static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     int i;
     p->print(p, "(%s/%d", ego->nam, ego->vl);
     for (i = 0; i < ego->rnk; ++i)
	  p->print(p, "%v", ego->d[i].n);
     p->print(p, ")");
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const problem_rdft *p;
     const S *ego = (const S *) ego_;
     P *pln;
     int retval;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, X(plan_null_destroy)
     };

     UNUSED(plnr);

     if (!applicable(ego, p_))
          return (plan *) 0;

     p = (const problem_rdft *) p_;
     pln = MKPLAN_RDFT(P, &padt, ego->apply);

     retval = fill_iodim(pln, p);
     A(retval);
     A(pln->vl > 0); /* because FINITE_RNK(p->vecsz->rnk) holds */
     pln->nam = ego->nam;

     /* X(tensor_sz)(p->vecsz) loads, X(tensor_sz)(p->vecsz) stores */
     X(ops_other)(2 * X(tensor_sz)(p->vecsz), &pln->super.super.ops);
     return &(pln->super.super);
}


void X(rdft_rank0_register)(planner *p)
{
     unsigned i;
     static struct {
	  rdftapply apply;
	  int (*applicable)(const P *, const problem_rdft *);
	  const char *nam;
     } tab[] = {
	  { apply_memcpy,   applicable_memcpy,   "rdft-rank0-memcpy" },
	  { apply_iter,     applicable_iter,     "rdft-rank0-iter" },
	  { apply_tiled,    applicable_tiled,    "rdft-rank0-tiled" },
	  { apply_tiledbuf, applicable_tiledbuf, "rdft-rank0-tiledbuf" },
	  { apply_ip_sq,    applicable_ip_sq,    "rdft-rank0-ip-sq" },
	  { 
	       apply_ip_sq_tiled,
	       applicable_ip_sq_tiled,
	       "rdft-rank0-ip-sq-tiled" 
	  },
	  { 
	       apply_ip_sq_tiledbuf,
	       applicable_ip_sq_tiledbuf,
	       "rdft-rank0-ip-sq-tiledbuf" 
	  },
     };

     for (i = 0; i < sizeof(tab) / sizeof(tab[0]); ++i) {
	  static const solver_adt sadt = { mkplan };
	  S *slv = MKSOLVER(S, &sadt);
	  slv->apply = tab[i].apply;
	  slv->applicable = tab[i].applicable;
	  slv->nam = tab[i].nam;
	  REGISTER_SOLVER(p, &(slv->super));
     }
}
