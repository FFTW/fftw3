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

/* $Id: redft00e-r2hc.c,v 1.4 2002-08-24 15:19:30 athena Exp $ */

/* Do a REDFT00 problem via an R2HC problem, with some pre/post-processing. */

#include "reodft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_rdft super;
     plan *cld;
     twid *td;
     int is, os;
     uint n;
} P;

/* Use the trick from FFTPACK, also documented in a similar form
   by Numerical Recipes. */

static void apply(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     uint i, n = ego->n;
     R *W = ego->td->W;
     R *buf;
     E csum;

     buf = (R *) fftw_malloc(sizeof(R) * n, BUFFERS);

     buf[0] = I[0] + I[is * n];
     csum = I[0] - I[is * n];
     for (i = 1; i < n - i; ++i) {
	  E a, b, apb, amb;
	  a = I[is * i];
	  b = I[is * (n - i)];
	  csum += W[2*i] * (2.0*(a - b));
	  amb = W[2*i+1] * (2.0*(a - b));
	  apb = (a + b);
	  buf[i] = apb - amb;
	  buf[n - i] = apb + amb;
     }
     if (i == n - i) {
	  buf[i] = 2.0 * I[is * i];
     }

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, buf, buf);
     }

     O[0] = buf[0];
     O[os] = csum;
     for (i = 1; i + i < n; ++i) {
	  uint k = i + i;
	  O[os * k] = buf[i];
	  O[os * (k + 1)] = O[os * (k - 1)] - buf[n - i];
     }
     if (i + i == n) {
	  O[os * n] = buf[i];
     }

     X(free)(buf);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     static const tw_instr redft00e_tw[] = {
          { TW_COS, 0, 1 },
          { TW_SIN, 0, 1 },
          { TW_NEXT, 1, 0 }
     };

     AWAKE(ego->cld, flg);
     X(twiddle_awake)(flg, &ego->td, redft00e_tw, 2*ego->n, 1, (ego->n+1)/2);
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
     p->print(p, "(redft00e-r2hc-%u%(%p%))", ego->n * 2, ego->cld);
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz.rnk == 1
		  && p->vecsz.rnk == 0
		  && p->kind == REDFT00
		  && p->sz.dims[0].n % 2 == 0
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p, const planner *plnr)
{
     UNUSED(plnr);
     return (applicable(ego, p)) ? UGLY : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_rdft *p;
     problem *cldp;
     plan *cld;
     R *buf;
     uint n;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     n = p->sz.dims[0].n / 2;
     buf = (R *) fftw_malloc(sizeof(R) * n, BUFFERS);

     {
	  tensor sz = X(mktensor_1d)(n, 1, 1);
	  cldp = X(mkproblem_rdft)(sz, p->vecsz, buf, buf, R2HC);
	  X(tensor_destroy)(sz);
     }

     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     X(free)(buf);
     if (!cld)
          return (plan *)0;

     pln = MKPLAN_RDFT(P, &padt, apply);

     pln->n = n;
     pln->is = p->sz.dims[0].is;
     pln->os = p->sz.dims[0].os;
     pln->cld = cld;
     pln->td = 0;
     
     pln->super.super.ops = cld->ops;
     /* FIXME */

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(redft00e_r2hc_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
