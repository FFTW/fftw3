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
     plan *cld;
     twid *td;
     int os;
     int r, m;
} P;

/***************************************************************************/

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int n, m, r, j;
     int os, osm;
     E *buf;
     const R *W;

     {
	  plan_dft *cld = (plan_dft *) ego->cld;
	  cld->apply((plan *) cld, ri, ii, ro, io);
     }

     r = ego->r;

     STACK_MALLOC(E *, buf, r * 2 * sizeof(E));
     
     osm = (m = ego->m) * (os = ego->os);
     n = m * r;
     W = ego->td->W;
     for (j = 0; j < m; ++j, ro += os, io += os) {
	  int k;
	  for (k = 0; k < r; ++k) {
	       E rb = ro[0], ib = io[0];
	       int i, iw, iw_inc = j + m * k;
	       for (i = 1, iw = iw_inc; i < r; ++i) {
		    E xr = ro[i*osm], xi = io[i*osm];
		    E wr = W[2*iw], wi = W[2*iw+1];
		    /* note that W[iw] is the product of the DIT twiddle
		       factor and the size-r DFT twiddle factor */
		    rb += xr * wr - xi * wi;
		    ib += xr * wi + xi * wr;
		    iw += iw_inc;
		    if (iw >= n)
			 iw -= n;
	       }
	       buf[2*k] = rb;
	       buf[2*k+1] = ib;
	  }
	  for (k = 0; k < r; ++k) {
	       ro[k*osm] = buf[2*k];
	       io[k*osm] = buf[2*k+1];
	  }
     }

     STACK_FREE(buf);
}

/***************************************************************************/

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     static const tw_instr generic_tw[] = {
	  { TW_GENERIC, 0, 0 },
	  { TW_NEXT, 1, 0 }
     };

     AWAKE(ego->cld, flg);
     X(twiddle_awake)(flg, &ego->td, generic_tw,
		      ego->r * ego->m, ego->r, ego->m);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;

     p->print(p, "(dft-generic-dit-%d%(%p%))", ego->r, ego->cld);
}

static int applicable0(const problem *p_)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
	       && p->sz->rnk == 1
	       && p->vecsz->rnk == 0
	       && p->sz->dims[0].n > 1
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
	  if (X(first_divisor)(p->sz->dims[0].n) >= GENERIC_MIN_BAD) return 0; 
     }
     return 1;
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     P *pln = 0;
     int n, r, m;
     int is, os;
     plan *cld = (plan *) 0;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_, plnr))
          goto nada;

     n = p->sz->dims[0].n;
     is = p->sz->dims[0].is;
     os = p->sz->dims[0].os;

     r = X(first_divisor)(n);
     m = n / r;

     cld = X(mkplan_d)(plnr, 
		       X(mkproblem_dft_d)(X(mktensor_1d)(m, r * is, os),
					  X(mktensor_1d)(r, is, m * os),
					  p->ri, p->ii, p->ro, p->io));
     if (!cld) goto nada;

     pln = MKPLAN_DFT(P, &padt, apply);

     pln->os = os;
     pln->r = r;
     pln->m = m;
     pln->cld = cld;
     pln->td = 0;

     X(ops_zero)(&pln->super.super.ops);
     pln->super.super.ops.add = 4 * r * (r-1);
     pln->super.super.ops.mul = 4 * r * (r-1);
     /* loads + stores, minus loads + stores for all DIT codelets */
     pln->super.super.ops.other = 4 * r + 4 * r * r - (6*r - 2);
     X(ops_madd)(m, &pln->super.super.ops, &cld->ops, &pln->super.super.ops);

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld);
     X(ifree0)(pln);
     return (plan *) 0;
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
