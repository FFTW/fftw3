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

/* $Id: dht-r2hc.c,v 1.7 2002-09-19 01:47:17 athena Exp $ */

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
     uint n;
} P;

static void apply(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int os = ego->os;
     uint i, n = ego->n;

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
     X(plan_destroy)(ego->cld);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(dht-r2hc-%u%(%p%))", ego->n, ego->cld);
}

static int applicable0(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz.rnk == 1
		  && p->vecsz.rnk == 0
		  && p->kind[0] == DHT
	       );
     }
     return 0;
}

static int applicable(const solver *ego, const problem *p, const planner *plnr)
{
     if (plnr->problem_flags & NO_DHT_R2HC) return 0;
     if (NO_UGLYP(plnr)) return 0;
     return (applicable0(ego, p));
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

     /* stop infinite loops with rdft-dht.c */
     plnr->problem_flags |= NO_DHT_R2HC; 

     cldp = X(mkproblem_rdft_1)(p->sz, p->vecsz, p->I, p->O, R2HC);
     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld)
          return (plan *)0;

     pln = MKPLAN_RDFT(P, &padt, apply);

     pln->n = p->sz.dims[0].n;
     pln->os = p->sz.dims[0].os;
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
