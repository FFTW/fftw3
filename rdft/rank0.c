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

/* $Id: rank0.c,v 1.17 2005-02-22 15:06:13 athena Exp $ */

/* plans for rank-0 RDFTs (copy operations) */

#include "rdft.h"

#ifdef HAVE_STRING_H
#include <string.h>		/* for memcpy() */
#endif

#define MAXRNK 2

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

     /* canonicalize vrank-1 in-place problems so that rnk == 2,
	because this is the only case considered in this file. */
     if (p->I == p->O && pln->rnk == 1 && pln->vl > 1) {
	  pln->d[0].n = pln->vl;
	  pln->d[0].is = pln->d[0].os = 1;
	  pln->rnk++;
	  pln->vl = 1;
     }
     
     return 1;
}

/**************************************************************/
/* rank 0,1,2, out of place, iterative */
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
	      X(cpy1d)(I, O, 
		       ego->d[0].n, ego->d[0].is, ego->d[0].os, 
		       vl);
	      break;
	 case 2:
	      X(cpy2d)(I, O, 
		       ego->d[0].n, ego->d[0].is, ego->d[0].os,
		       ego->d[1].n, ego->d[1].is, ego->d[1].os,
		       vl);
	      break;
	 default:
	      A(0 /* can't happen */); 
     }
}

static int applicable_iter(const P *pln, const problem_rdft *p)
{
     return (p->I != p->O) && (pln->rnk <= 2);
}

/**************************************************************/
/* rank 2, out of place, recursive */
static void apply_rec(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int vl = ego->vl;

     A(ego->rnk == 2);
     X(cpy2d_rec)(I, O, 
		  ego->d[0].n, ego->d[0].is, ego->d[0].os,
		  ego->d[1].n, ego->d[1].is, ego->d[1].os,
		  vl);
}

static int applicable_rec(const P *pln, const problem_rdft *p)
{
     return (p->I != p->O) && (pln->rnk == 2);
}

/**************************************************************/
/* rank 2, out of place, recursive with buffer, vl <= 2 */
static void apply_recbuf(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int vl = ego->vl;

     A(ego->rnk == 2);
     A(vl <= 2);
     X(cpy2d_recbuf)(I, O, 
		     ego->d[0].n, ego->d[0].is, ego->d[0].os,
		     ego->d[1].n, ego->d[1].is, ego->d[1].os,
		     vl);
}

static int applicable_recbuf(const P *pln, const problem_rdft *p)
{
     return (p->I != p->O) && (pln->rnk == 2) && (pln->vl <= 2);
}

/**************************************************************/
/* rank 0, out of place, using memcpy */
static void apply_memcpy(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int vl = ego->vl;

     A(ego->rnk == 0);
     memcpy(O, I, vl * sizeof(R));
}

static int applicable_memcpy(const P *pln, const problem_rdft *p)
{
     return (p->I != p->O) && (pln->rnk == 0);
}

/**************************************************************/
/* rank 2, in place, square transpose, iterative */
static void apply_ip_sq(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     UNUSED(O);
     X(transpose)(I, ego->d[0].n, ego->d[0].is, ego->d[0].os, ego->vl);
}

static int applicable_ip_sq(const P *pln, const problem_rdft *p)
{
     return (1
	     && p->I == p->O
	     && pln->rnk == 2
	     && pln->d[0].n == pln->d[1].n
	     && pln->d[0].is == pln->d[1].os
	     && pln->d[0].os == pln->d[1].is);
}

/**************************************************************/
/* rank 2, in place, square transpose, recursive */
static void apply_ip_sq_rec(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     UNUSED(O);
     X(transpose_rec)(I, ego->d[0].n, ego->d[0].is, ego->d[0].os, ego->vl);
}

#define applicable_ip_sq_rec applicable_ip_sq

/**************************************************************/
/* rank 2, in place, square transpose, recursive with buffer */
static void apply_ip_sq_recbuf(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     UNUSED(O);
     X(transpose_recbuf)(I, ego->d[0].n, ego->d[0].is, ego->d[0].os, ego->vl);
}

static int applicable_ip_sq_recbuf(const P *pln, const problem_rdft *p)
{
     return pln->vl <= 2 && applicable_ip_sq_rec(pln, p);
}

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
	  int (*applicable)(const P *pln, const problem_rdft *p);
	  const char *nam;
     } tab[] = {
	  { apply_iter,      applicable_iter,      "rdft-rank0" },
	  { apply_rec,       applicable_rec,       "rdft-rank0-rec" },
	  { apply_recbuf,    applicable_recbuf,    "rdft-rank0-recbuf" },
	  { apply_memcpy,    applicable_memcpy,    "rdft-rank0-memcpy" },
	  { apply_ip_sq,     applicable_ip_sq,     "rdft-rank0-ip-sq" },
	  { apply_ip_sq_rec, applicable_ip_sq_rec, "rdft-rank0-ip-sq-rec" },
	  { 
	       apply_ip_sq_recbuf,
	       applicable_ip_sq_recbuf,
	       "rdft-rank0-ip-sq-recbuf" 
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
