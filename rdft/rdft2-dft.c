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

/* $Id: rdft2-dft.c,v 1.3 2002-08-24 15:05:08 athena Exp $ */

/* Compute RDFT of even size via a DFT of size n/2 */

#include "rdft.h"
#include "dft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_dft super;
     plan *cld;
     twid *td;
     int is, os, ivs, ovs;
     uint n, vl;
} P;

static void radix2_dit(R *rio, R *iio, const R *W, uint n, int dist)
{
     uint i;
     R *riop, *iiop;
     R *riom, *iiom;

     /* i = n/2 when n is even */
     if (!(n & 1)) {
	  uint n2 = n / 2;
	  iio[n2 * dist] = -iio[n2 * dist];
     }

     riop = rio;
     iiop = iio;
     riom = rio + n * dist;
     iiom = iio + n * dist;

     /* i = 0 and i = n */
     {
	  E rop = *riop;
	  E iop = *iiop;
	  *riop = rop + iop;   riop += dist;
	  *iiop = 0.0;         iiop += dist;
	  *riom = rop - iop;   riom -= dist;
	  *iiom = 0.0;         iiom -= dist;
     }

     /* middle elements */
     for (i = 1; i < (n + 1) / 2; ++i) {
	  E rop = *riop;
	  E iop = *iiop;
	  E rom = *riom;
	  E iom = *iiom;
	  E wr = W[2 * i];
	  E wi = W[2 * i + 1];
	  E re = rop + rom;
	  E ie = iop - iom;
	  E rd = rom - rop;
	  E id = iop + iom;
	  E tr = rd * wr - id * wi;
	  E ti = id * wr + rd * wi;
	  *riop = 0.5 * (re + ti);    riop += dist;
	  *iiop = 0.5 * (ie + tr);    iiop += dist;
	  *riom = 0.5 * (re - ti);    riom -= dist;
	  *iiom = 0.5 * (tr - ie);    iiom -= dist;
     }
}

static void apply(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;

     {
	  /* transform input as a vector of complex numbers */
	  plan_dft *cld = (plan_dft *) ego->cld;
	  cld->apply((plan *) cld, r, r + ego->is, rio, iio);
     }

     {
          uint i, vl = ego->vl, n2 = ego->n / 2;
          int ovs = ego->ovs, os = ego->os;
	  const R *W = ego->td->W;
          for (i = 0; i < vl; ++i, rio += ovs, iio += ovs)
	       radix2_dit(rio, iio, W, n2, os);
     }
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld, flg);

     if (flg) {
	  static const tw_instr twinstr[] = {
	       {TW_FULL, 0, 2},
	       {TW_NEXT, 1, 0}
	  };
	  X(mktwiddle)(&ego->td, twinstr, ego->n, 2, 
		       (ego->n / 2 + 1) / 2);
     } else {
	  X(twiddle_destroy)(&ego->td);
     }
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy) (ego->cld);
     X(free)(ego);
}

static void print(plan *ego_, printer * p)
{
     P *ego = (P *) ego_;
     p->print(p, "(r2hc-dft-%u%v%(%p%))", ego->n, ego->vl, ego->cld);
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFT2P(p_)) {
	  const problem_rdft2 *p = (const problem_rdft2 *) p_;
	  return (1 
		  && R2HC_KINDP(p->kind)
		  && p->vecsz.rnk <= 1
		  && p->sz.rnk == 1
		  && (p->sz.dims[0].n % 2) == 0
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p_, const planner *plnr)
{
     UNUSED(plnr);
     if (applicable(ego, p_)) 
	  return GOOD;
     return BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_rdft2 *p;
     problem *cldp;
     plan *cld;
     const iodim *d;

     static const plan_adt padt = {
	  X(rdft2_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_))
	  return (plan *) 0;

     p = (const problem_rdft2 *) p_;
     d = p->sz.dims;

     {
	  cldp = X(mkproblem_dft_d) (
	       X(mktensor_1d)(d[0].n / 2, d[0].is * 2, d[0].os),
	       X(tensor_copy)(p->vecsz),
	       p->r, p->r + d[0].is, p->rio, p->iio);
     }

     cld = MKPLAN(plnr, cldp);
     X(problem_destroy) (cldp);
     if (!cld)
	  return (plan *) 0;

     pln = MKPLAN_RDFT2(P, &padt, apply);

     pln->n = d[0].n;
     pln->os = d[0].os;
     pln->is = d[0].is;
     X(tensor_tornk1)(&p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     pln->cld = cld;
     pln->td = 0;

     pln->super.super.ops = cld->ops; /* FIXME */

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(rdft2_dft_register) (planner *p) 
{
     REGISTER_SOLVER(p, mksolver());
}
