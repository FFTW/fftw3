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

#include "dft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_dft super;
     twid *td;
     int n, is, os;
} P;


static void cdot(int n,
		 const R *xr, const R *xi, int xs,
		 const R *w, int i,
		 R *or, R *oi)
{
     int i0 = 0, n0 = n;

     E rr = *xr, ri = 0, ir = *xi, ii = 0;
     while (--n0) {
	  xr += xs; xi += xs;
	  i0 += i; if (i0 >= n) i0 -= n;
	  rr += *xr * w[2*i0];
	  ri += *xr * w[2*i0+1];
	  ii += *xi * w[2*i0+1];
	  ir += *xi * w[2*i0];
     }
     *or = rr - ii;
     *oi = ri + ir;
}

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int i;
     int n = ego->n, is = ego->is, os = ego->os;
     const R *W = ego->td->W;

     for (i = 0; i < n; ++i) 
	  cdot(n,
	       ri, ii, is,
	       W, i,
	       ro + i * os, io + i * os);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     static const tw_instr generic_tw[] = {
	  { TW_GENERIC, 0, 0 },
	  { TW_NEXT, 1, 0 }
     };

     X(twiddle_awake)(flg, &ego->td, generic_tw, ego->n, 1, ego->n);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;

     p->print(p, "(dft-generic-%d)", ego->n);
}

static int applicable0(const problem *p_)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
		  && p->sz->rnk == 1
		  && p->vecsz->rnk == 0
		  && p->ri != p->ro  /* out of place only */
		  && X(is_prime)(p->sz->dims[0].n)
	       );
     }

     return 0;
}

static int applicable(const solver *ego, const problem *p_, 
		      const planner *plnr)
{
     UNUSED(ego);
     if (NO_UGLYP(plnr)) return 0; /* always ugly */
     if (!applicable0(p_)) return 0;

     if (NO_LARGE_GENERICP(plnr)) {
          const problem_dft *p = (const problem_dft *) p_;
	  if (p->sz->dims[0].n >= GENERIC_MIN_BAD) return 0; 
     }
     return 1;
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     P *pln;
     int n;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, X(plan_null_destroy)
     };

     if (!applicable(ego, p_, plnr))
          return (plan *)0;

     pln = MKPLAN_DFT(P, &padt, apply);

     pln->n = n = p->sz->dims[0].n;
     pln->is = p->sz->dims[0].is;
     pln->os = p->sz->dims[0].os;
     pln->td = 0;

     X(ops_zero)(&pln->super.super.ops);
     pln->super.super.ops.add = 4 * n * (n - 1);
     pln->super.super.ops.mul = 4 * n * (n - 1);
     pln->super.super.ops.other = 4 * n * n;

     return &(pln->super.super);
}

/* constructors */

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(dft_generic_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
