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

/* $Id: rdft-dht.c,v 1.3 2002-09-16 02:30:26 stevenj Exp $ */

/* Solve an R2HC/HC2R problem via post/pre processing of a DHT.  This
   is mainly useful because we can use Rader to compute DHTs of prime
   sizes.  It also allows us to express hc2r problems in terms of r2hc
   (via dht-r2hc), and to do hc2r problems without destroying the input. */

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

static void apply_r2hc(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int os;
     uint i, n;

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, I, O);
     }

     n = ego->n;
     os = ego->os;
     for (i = 1; i < n - i; ++i) {
	  E a, b;
	  a = 0.5 * O[os * i];
	  b = 0.5 * O[os * (n - i)];
	  O[os * i] = a + b;
#if FFT_SIGN == -1
	  O[os * (n - i)] = b - a;
#else
	  O[os * (n - i)] = a - b;
#endif
     }
}

/* hc2r, destroying input as usual */
static void apply_hc2r(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is;
     uint i, n = ego->n;

     for (i = 1; i < n - i; ++i) {
	  E a, b;
	  a = I[is * i];
	  b = I[is * (n - i)];
#if FFT_SIGN == -1
	  I[is * i] = a - b;
	  I[is * (n - i)] = a + b;
#else
	  I[is * i] = a + b;
	  I[is * (n - i)] = a - b;
#endif
     }

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, I, O);
     }
}

/* hc2r, without destroying input */
static void apply_hc2r_save(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     uint i, n = ego->n;

     O[0] = I[0];
     for (i = 1; i < n - i; ++i) {
	  E a, b;
	  a = I[is * i];
	  b = I[is * (n - i)];
#if FFT_SIGN == -1
	  O[os * i] = a - b;
	  O[os * (n - i)] = a + b;
#else
	  O[os * i] = a + b;
	  O[os * (n - i)] = a - b;
#endif
     }
     if (i == n - i)
	  O[os * i] = I[is * i];

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, O, O);
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
     p->print(p, "(%s-dht-%u%(%p%))", 
	      ego->super.apply == apply_r2hc ? "r2hc" : "hc2r",
	      ego->n, ego->cld);
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz.rnk == 1
		  && p->vecsz.rnk == 0
		  && (p->kind[0] == R2HC || p->kind[0] == HC2R)
	       );
     }
     return 0;
}

static int score(const solver *ego, const problem *p_, const planner *plnr)
{
     UNUSED(plnr);
     if (applicable(ego, p_)) {
	  const problem_rdft *p = (const problem_rdft *) p_;
	  uint n = p->sz.dims[0].n;
	  if (n >= RADER_MIN_GOOD && X(is_prime)(n))
	       return GOOD;  /* need to make rader-dht GOOD to exclude
				generic solver for large primes */
	  return UGLY;
     }
     return BAD;
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

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     if (p->kind[0] == R2HC || DESTROY_INPUTP(plnr))
	  cldp = X(mkproblem_rdft_1)(p->sz, p->vecsz, p->I, p->O, DHT);
     else {
	  tensor sz = X(tensor_copy_inplace)(p->sz, INPLACE_OS);
	  cldp = X(mkproblem_rdft_1)(sz, p->vecsz, p->O, p->O, DHT);
	  X(tensor_destroy)(sz);
     }
     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld)
          return (plan *)0;

     pln = MKPLAN_RDFT(P, &padt, p->kind[0] == R2HC ? 
		       apply_r2hc : (DESTROY_INPUTP(plnr) ?
				     apply_hc2r : apply_hc2r_save));
     pln->n = p->sz.dims[0].n;
     pln->is = p->sz.dims[0].is;
     pln->os = p->sz.dims[0].os;
     pln->cld = cld;
     
     pln->super.super.ops = cld->ops;
     pln->super.super.ops.other += 4 * ((pln->n - 1)/2);
     pln->super.super.ops.add += 2 * ((pln->n - 1)/2);
     if (p->kind[0] == R2HC)
	  pln->super.super.ops.mul += 2 * ((pln->n - 1)/2);
     if (pln->super.apply == apply_hc2r_save)
	  pln->super.super.ops.other += 2 + (pln->n % 2 ? 0 : 2);

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(rdft_dht_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
