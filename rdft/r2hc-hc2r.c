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

/* $Id: r2hc-hc2r.c,v 1.8 2002-08-26 04:05:53 stevenj Exp $ */

/* Solve an HC2R problem by using an R2HC problem of the same size.
   The two problems can be expressed in terms of one another by
   some pre- and post-processing, inspired by the DHT. */

#include "rdft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_rdft super;
     plan *cld;
     int is, os;
     uint n;
} P;

/* 1 to destroy input array (like our ordinary hb/hc2r algorithm): */
#define MUNGE_INPUT 0

static void apply(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     uint i, n = ego->n;

#if MUNGE_INPUT
#  define O1 I
#  define os1 is
#else
     O[0] = I[0];
#  define O1 O
#  define os1 os
#endif
     for (i = 1; i < (n + 1)/2; ++i) {
	  E a, b;
	  a = I[is * i];
	  b = I[is * (n - i)];
	  O1[os1 * i] = a + b;
	  O1[os1 * (n - i)] = a - b;
     }
#if !MUNGE_INPUT
     if (2*i == n)
	  O[os * i] = I[is * i];
#endif

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, O1, O);
     }
#undef O1
#undef os1

     for (i = 1; i < (n + 1)/2; ++i) {
	  E a, b;
	  a = O[os * i];
	  b = O[os * (n - i)];
	  O[os * i] = a + b;
	  O[os * (n - i)] = a - b;
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
     p->print(p, "(rdft-r2hc-hc2r-%u%(%p%))", ego->n, ego->cld);
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     UNUSED(ego_);
#if !MUNGE_INPUT
     UNUSED(plnr);
#endif
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
#if MUNGE_INPUT
		  && (p->I == p->O || (plnr->flags & DESTROY_INPUT))
#endif
		  && p->sz.rnk == 1
		  && p->vecsz.rnk == 0
		  && p->kind[0] == HC2R
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p, const planner *plnr)
{
     UNUSED(plnr);
     return (applicable(ego, p, plnr)) ? UGLY : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_rdft *p;
     problem *cldp;
     plan *cld;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *)0;

     p = (const problem_rdft *) p_;

#if MUNGE_INPUT
     cldp = X(mkproblem_rdft_1)(p->sz, p->vecsz, p->I, p->O, R2HC);
#else
     {
	  tensor sz = X(tensor_copy)(p->sz);
	  sz.dims[0].is = sz.dims[0].os;
	  cldp = X(mkproblem_rdft_1)(sz, p->vecsz, p->O, p->O, R2HC);
	  X(tensor_destroy)(sz);
     }
#endif

     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld)
          return (plan *)0;

     pln = MKPLAN_RDFT(P, &padt, apply);

     pln->n = p->sz.dims[0].n;
     pln->is = p->sz.dims[0].is;
     pln->os = p->sz.dims[0].os;
     pln->cld = cld;
     
     pln->super.super.ops = cld->ops;
     pln->super.super.ops.other += 8 * ((pln->n - 1)/2);
     pln->super.super.ops.add += 4 * ((pln->n - 1)/2);
#if !MUNGE_INPUT
     pln->super.super.ops.other += 2 + (pln->n % 2 ? 0 : 2);
#endif

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(rdft_r2hc_hc2r_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
