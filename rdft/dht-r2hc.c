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

/* $Id: dht-r2hc.c,v 1.19 2003-03-15 20:29:43 stevenj Exp $ */

/* Solve a DHT problem (Discrete Hartley Transform) via post-processing
   of an R2HC problem. */

#include "rdft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_rdft super;
     plan *cld;
     int os;
     int n;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int os = ego->os;
     int i, n = ego->n;

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, I, O);
     }

     for (i = 1; i < n - i; ++i) {
	  E a, b;
	  a = O[os * i];
	  b = O[os * (n - i)];
#if FFT_SIGN == -1
	  O[os * i] = a - b;
	  O[os * (n - i)] = a + b;
#else
	  O[os * i] = a + b;
	  O[os * (n - i)] = a - b;
#endif
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
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dht-r2hc-%d%(%p%))", ego->n, ego->cld);
}

static int applicable0(const problem *p_, const planner *plnr)
{
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && !NO_DHT_R2HCP(plnr)
		  && p->sz->rnk == 1
		  && p->vecsz->rnk == 0
		  && p->kind[0] == DHT
	       );
     }
     return 0;
}

static int applicable(const solver *ego, const problem *p, const planner *plnr)
{
     UNUSED(ego);
     return (!NO_UGLYP(plnr) && applicable0(p, plnr));
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_rdft *p;
     plan *cld;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     /* stop infinite loops with rdft-dht.c */
     plnr->problem_flags |= NO_DHT_R2HC; 

     cld = X(mkplan_d)(plnr, 
		       X(mkproblem_rdft_1)(p->sz, p->vecsz, p->I, p->O, R2HC));
     if (!cld) return (plan *)0;

     pln = MKPLAN_RDFT(P, &padt, apply);

     pln->n = p->sz->dims[0].n;
     pln->os = p->sz->dims[0].os;
     pln->cld = cld;
     
     pln->super.super.ops = cld->ops;
     pln->super.super.ops.other += 4 * ((pln->n - 1)/2);
     pln->super.super.ops.add += 2 * ((pln->n - 1)/2);

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(dht_r2hc_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
