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

/* $Id: dft-r2hc.c,v 1.1 2002-07-23 18:09:19 stevenj Exp $ */

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
     plan_rdft super;
     plan *cldr;
     plan *cldi;
     int os;
     uint n;
} P;

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     int os;
     uint i, n;

     {
	  plan_rdft *cld = (plan_rdft *) ego->cldr;
	  cld->apply((plan *) cld, ri, ro);
     }

     {
	  plan_rdft *cld = (plan_rdft *) ego->cldi;
	  cld->apply((plan *) cld, ii, io);
     }

     os = ego->os;
     n = ego->n;
     for (i = 1; i < (n + 1)/2; ++i) {
	  R rop, iop, iom, rom;
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
     AWAKE(ego->cldr, flg);
     AWAKE(ego->cldi, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cldi);
     X(plan_destroy)(ego->cldr);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(dft-r2hc-%u%(%p%)", ego->n, ego->cldr);
     if (ego->cldi != ego->cldr)
	  p->print(p, "%(%p%)", ego->cldi);
     p->putchr(p, ')');
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
	       && p->sz.rnk == 1
	       && p->vecsz.rnk == 0
	       );
     }

     return 0;
}

static int split(R *r, R *i, uint n, int s)
{
     return ((r > i ? r - i : i - r) >= ((int)n) * (s > 0 ? s : -s));
}

static int score(const solver *ego, const problem *p_, int flags)
{
     if (applicable(ego, p_)) {
	  const problem_dft *p = (const problem_dft *) p_;
	  if (flags & CLASSIC)
	       return UGLY;
	  if (split(p->ri, p->ii, p->sz.dims[0].n, p->sz.dims[0].is) &&
	      split(p->ro, p->io, p->sz.dims[0].n, p->sz.dims[0].os))
	       return GOOD;
	  return UGLY;
     }
     return BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_dft *p;
     problem *cldp;
     plan *cldr, *cldi;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_dft *) p_;

     cldp = X(mkproblem_rdft)(p->sz, p->vecsz, p->ri, p->ro, R2HC);
     cldr = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldr)
          return (plan *)0;

     cldp = X(mkproblem_rdft)(p->sz, p->vecsz, p->ii, p->io, R2HC);
     cldi = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldi) {
	  X(plan_destroy)(cldr);
          return (plan *)0;
     }

     pln = MKPLAN_DFT(P, &padt, apply);

     pln->n = p->sz.dims[0].n;
     pln->os = p->sz.dims[0].os;
     pln->cldr = cldr;
     pln->cldi = cldi;
     
     pln->super.super.ops = X(ops_add)(cldr->ops, cldi->ops);
     pln->super.super.ops.other += 4 * (pln->n - 1);
     pln->super.super.ops.add += 2 * (pln->n - 1);

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(dft_r2hc_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
