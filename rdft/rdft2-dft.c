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

/* $Id: rdft2-dft.c,v 1.1 2002-08-23 17:22:17 athena Exp $ */

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
     R *W;
     int is, os;
     uint n;
} P;

static void apply(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;

     {
	  /* transform input as a vector of complex numbers */
	  plan_dft *cld = (plan_dft *) ego->cld;
	  cld->apply((plan *) cld, r, r + is, rio, iio);
     }

     {
	  R *W = ego->W;
	  uint n = ego->n / 2;
	  uint i;
	  R *riom, *iiom;

	  /* i = n/2 when n is even */
	  if (!(n & 1)) {
	       uint n2 = n / 2;
	       iio[n2 * os] = -iio[n2 * os];
	  }

	  riom = rio + n * os;
	  iiom = iio + n * os;

	  /* i = 0 and i = n */
	  {
	       E rop = *rio;
	       E iop = *iio;
	       *rio = rop + iop;   rio += os;
	       *iio = 0.0;         iio += os;
	       *riom = rop - iop;  riom -= os;
	       *iiom = 0.0;        iiom -= os;
	  }

	  /* middle elements */
	  for (i = 1; i < (n + 1) / 2; ++i) {
	       E rop = *rio;
	       E iop = *iio;
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
	       *rio = 0.5 * (re + ti);    rio += os;
	       *iio = 0.5 * (ie + tr);    iio += os;
	       *riom = 0.5 * (re - ti);   riom -= os;
	       *iiom = 0.5 * (tr - ie);   iiom -= os;
	  }
     }
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld, flg);

     if (flg) {
          if (!ego->td) {
	       static const tw_instr twinstr[] = {
		    {TW_FULL, 0, 2},
		    {TW_NEXT, 1, 0}
	       };
               ego->td = X(mktwiddle)(twinstr, ego->n, 2, 
				      (ego->n / 2 + 1) / 2);
               ego->W = ego->td->W;
          }
     } else {
          if (ego->td)
               X(twiddle_destroy)(ego->td);
          ego->td = 0;
          ego->W = 0;
     }
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     if (ego->td)
	  X(twiddle_destroy)(ego->td);
     X(plan_destroy) (ego->cld);
     X(free)(ego);
}

static void print(plan *ego_, printer * p)
{
     P *ego = (P *) ego_;
     p->print(p, "(r2hc-dft-%u%(%p%))", ego->n, ego->cld);
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFT2P(p_)) {
	  const problem_rdft2 *p = (const problem_rdft2 *) p_;
	  return (1 
		  && p->sz.rnk == 1
		  && (p->sz.dims[0].n % 2) == 0
		  && p->vecsz.rnk == 0
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

     cldp = X(mkproblem_dft_d) (
	  X(mktensor_1d)(d[0].n / 2, d[0].is * 2, d[0].os),
	  X(mktensor(0)),
	  p->r, p->r + d[0].is, p->rio, p->iio);

     cld = MKPLAN(plnr, cldp);
     X(problem_destroy) (cldp);
     if (!cld)
	  return (plan *) 0;

     pln = MKPLAN_RDFT2(P, &padt, apply);

     pln->n = d[0].n;
     pln->os = d[0].os;
     pln->is = d[0].is;
     pln->cld = cld;
     pln->td = 0;
     pln->W = 0;

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
