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

/* $Id: dft-r2hc.c,v 1.24 2004-03-23 16:49:01 athena Exp $ */

/* Compute the complex DFT by combining R2HC RDFTs on the real
   and imaginary parts.   This could be useful for people just wanting
   to link to the real codelets and not the complex ones.  It could
   also even be faster than the complex algorithms for split (as opposed
   to interleaved) real/imag complex data. */

#include "rdft.h"
#include "dft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_dft super;
     plan *cld;
     int os;
     int n;
} P;

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int os;
     int i, n;

     UNUSED(ii);

     { /* transform vector of real & imag parts: */
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, ri, ro);
     }

     os = ego->os;
     n = ego->n;
     for (i = 1; i < (n + 1)/2; ++i) {
	  E rop, iop, iom, rom;
	  rop = ro[os * i];
	  iop = io[os * i];
	  rom = ro[os * (n - i)];
	  iom = io[os * (n - i)];
	  ro[os * i] = rop - iom;
	  io[os * i] = iop + rom;
	  ro[os * (n - i)] = rop + iom;
	  io[os * (n - i)] = iop - rom;
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
     p->print(p, "(dft-r2hc-%d%(%p%))", ego->n, ego->cld);
}

#define ALLOW_RANK0 0 /* disable for now, subject to testing */

static int applicable0(const problem *p_)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return ((p->sz->rnk == 1 && p->vecsz->rnk == 0)
#if ALLOW_RANK0
		  || p->sz->rnk == 0
#endif
	       );
     }

     return 0;
}

static int split(R *r, R *i, int n, int s)
{
     return ((r > i ? r - i : i - r) >= ((int)n) * (s > 0 ? s : -s));
}

static int applicable(const problem *p_, const planner *plnr)
{
     if (!applicable0(p_)) return 0;

     {
	  const problem_dft *p = (const problem_dft *) p_;
	  if (NO_UGLYP(plnr) && DFT_R2HC_ICKYP(plnr)) return 0;

	  if (p->sz->rnk == 1 &&
	      split(p->ri, p->ii, p->sz->dims[0].n, p->sz->dims[0].is) &&
	      split(p->ro, p->io, p->sz->dims[0].n, p->sz->dims[0].os))
	       return 1;

	  return !(NO_UGLYP(plnr));
     }
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_dft *p;
     plan *cld;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     UNUSED(ego_);
     if (!applicable(p_, plnr))
          return (plan *)0;

     p = (const problem_dft *) p_;

     {
	  tensor *ri_vec = X(mktensor_1d)(2, p->ii - p->ri, p->io - p->ro);
	  tensor *cld_vec = X(tensor_append)(ri_vec, p->vecsz);
	  cld = X(mkplan_d)(plnr, 
			    X(mkproblem_rdft_1)(p->sz, cld_vec, 
						p->ri, p->ro, R2HC));
	  X(tensor_destroy2)(ri_vec, cld_vec);
     }
     if (!cld) return (plan *)0;

     pln = MKPLAN_DFT(P, &padt, apply);

#if ALLOW_RANK0
     if (p->sz->rnk == 0) {
	  pln->n = 1;
	  pln->os = 0;
     }
     else
#endif
     {
	  pln->n = p->sz->dims[0].n;
	  pln->os = p->sz->dims[0].os;
     }

     pln->cld = cld;
     
     pln->super.super.ops = cld->ops;
     pln->super.super.ops.other += 8 * ((pln->n - 1)/2);
     pln->super.super.ops.add += 4 * ((pln->n - 1)/2);

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(dft_r2hc_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
