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

/* $Id: rank0.c,v 1.13 2005-02-20 23:22:15 athena Exp $ */

/* plans for rank-0 RDFTs (copy operations) */

#include "rdft.h"

#ifdef HAVE_STRING_H
#include <string.h>		/* for memcpy() */
#endif

typedef struct {
     solver super;
     rdftapply apply;
     const char *nam;
     int minrnk;
     int maxrnk;
} S;

#define MAXRNK 2

typedef struct {
     plan_rdft super;
     int vl;
     int rnk;
     iodim d[MAXRNK];
     const char *nam;
} P;

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

/* execution routines */
#define CUTOFF 4

static void cpy1d(R *I, R *O, int n0, int is0, int os0, int vl)
{
     int i0, v;
     switch (vl) {
	 case 1:
	      for (; n0 > 0; --n0, I += is0, O += os0) 
		   *O = *I;
	      break;
	 case 2:
	      for (; n0 > 0; --n0, I += is0, O += os0) {
		   R x0 = I[0];
		   R x1 = I[1];
		   O[0] = x0;
		   O[1] = x1;
	      }
	      break;
	 default:
	      for (i0 = 0; i0 < n0; ++i0) 
		   for (v = 0; v < vl; ++v) {
			R x0 = I[i0 * is0 + v];
			O[i0 * os0 + v] = x0;
		   }
	      break;
     }
}

static void cpy2d(R *I, R *O,
		  int n0, int is0, int os0,
		  int n1, int is1, int os1,
		  int vl)
{
     int i0, i1, v;
     switch (vl) {
	 case 1:
	      for (i1 = 0; i1 < n1; ++i1)
		   for (i0 = 0; i0 < n0; ++i0) {
			R x0 = I[i0 * is0 + i1 * is1];
			O[i0 * os0 + i1 * os1] = x0;
		   }
	      break;
	 case 2:
	      for (i1 = 0; i1 < n1; ++i1)
		   for (i0 = 0; i0 < n0; ++i0) {
			R x0 = I[i0 * is0 + i1 * is1];
			R x1 = I[i0 * is0 + i1 * is1 + 1];
			O[i0 * os0 + i1 * os1] = x0;
			O[i0 * os0 + i1 * os1 + 1] = x1;
		   }
	      break;
	 default:
	      for (i1 = 0; i1 < n1; ++i1)
		   for (i0 = 0; i0 < n0; ++i0) 
			for (v = 0; v < vl; ++v) {
			     R x0 = I[i0 * is0 + i1 * is1 + v];
			     O[i0 * os0 + i1 * os1 + v] = x0;
			}
	      break;
     }
}

static void cpy2d_rec(R *I, R *O,
		      int n0, int is0, int os0,
		      int n1, int is1, int os1,
		      int vl)
{
 tail:
     if (n0 >= n1 && n0 > CUTOFF) {
	  int nm = n0 / 2;
	  cpy2d_rec(I, O, nm, is0, os0, n1, is1, os1, vl);
	  I += nm * is0; O += nm * os0; n0 -= nm; goto tail;
     } else if (/* n1 >= n0 && */ n1 > CUTOFF) {
	  int nm = n1 / 2;
	  cpy2d_rec(I, O, n0, is0, os0, nm, is1, os1, vl);
	  I += nm * is1; O += nm * os1; n1 -= nm; goto tail;
     } else 
	  cpy2d(I, O, n0, is0, os0, n1, is1, os1, vl);
}

static void apply_iter(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int vl = ego->vl;

     switch (ego->rnk) {
	 case 0: 
	      for (; vl > 0; --vl, ++I, ++O) 
		   *O = *I;
	      break;
	 case 1:
	      cpy1d(I, O, 
		    ego->d[0].n, ego->d[0].is, ego->d[0].os, 
		    vl);
	      break;
	 case 2:
	      cpy2d(I, O, 
		    ego->d[0].n, ego->d[0].is, ego->d[0].os,
		    ego->d[1].n, ego->d[1].is, ego->d[1].os,
		    vl);
	      break;
	 default:
	      A(0 /* can't happen */); 
     }
}

static void apply_rec(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int vl = ego->vl;

     A(ego->rnk == 2);
     cpy2d_rec(I, O, 
	       ego->d[0].n, ego->d[0].is, ego->d[0].os,
	       ego->d[1].n, ego->d[1].is, ego->d[1].os,
	       vl);
}

static void apply_memcpy(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int vl = ego->vl;

     A(ego->rnk == 0);
     memcpy(O, I, vl * sizeof(R));
}

static int applicable(const S *ego, const problem *p_)
{
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
	  P pln;
          return (1
		  && p->I != p->O
                  && p->sz->rnk == 0
		  && FINITE_RNK(p->vecsz->rnk)
		  && fill_iodim(&pln, p)
		  && pln.rnk >= ego->minrnk
		  && pln.rnk <= ego->maxrnk
	       );
     }
     return 0;
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(%s/%d%v)", ego->nam, ego->rnk, ego->vl);
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
     pln->nam = ego->nam;

     /* vl loads, vl stores */
     X(ops_other)(2 * X(tensor_sz)(p->vecsz), &pln->super.super.ops);
     return &(pln->super.super);
}


void X(rdft_rank0_register)(planner *p)
{
     unsigned i;
     static struct {
	  rdftapply apply;
	  int minrnk;
	  int maxrnk;
	  const char *nam;
     } tab[] = {
	  { apply_iter,     0, 2, "rdft-rank0" },
	  { apply_rec ,     2, 2, "rdft-rank0-rec" },
	  { apply_memcpy ,  0, 0, "rdft-rank0-memcpy" },
     };

     for (i = 0; i < sizeof(tab) / sizeof(tab[0]); ++i) {
	  static const solver_adt sadt = { mkplan };
	  S *slv = MKSOLVER(S, &sadt);
	  slv->apply = tab[i].apply;
	  slv->minrnk = tab[i].minrnk;
	  slv->maxrnk = tab[i].maxrnk;
	  slv->nam = tab[i].nam;
	  REGISTER_SOLVER(p, &(slv->super));
     }
}
