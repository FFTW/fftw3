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

/* $Id: direct-k7.c,v 1.2 2002-06-14 21:42:35 athena Exp $ */

/* direct DFT solver, if we have a codelet.  
   This code handles K7-specific codelets */

#include "dft.h"

typedef struct {
     solver super;
     const kdft_k7_desc *desc;
     kdft_k7 k;
} S;

typedef struct {
     plan_dft super;

     int is, os;
     uint vl;
     int ivs, ovs;
     kdft_k7 k;
     const S *slv;
} P;

static void applyr(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     UNUSED(ii); UNUSED(io);
     ego->k(ri, ro, ego->is, ego->os, ego->vl, ego->ivs, ego->ovs);
}

static void applyi(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     UNUSED(ri); UNUSED(ro);
     ego->k(ii, io, ego->is, ego->os, ego->vl, ego->ivs, ego->ovs);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     const S *s = ego->slv;
     const kdft_k7_desc *d = s->desc;

     p->print(p, "(dft-direct-k7-%u%v)", d->sz, ego->vl);
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (SINGLE_PRECISION && DFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_dft *p = (const problem_dft *) p_;
          const kdft_k7_desc *d = ego->desc;

          return (
	       1
	       && p->sz.rnk == 1
	       && p->vecsz.rnk <= 1
	       && p->sz.dims[0].n == d->sz

	       /* check pointers */
	       && (p->ri - p->ii == d->sign)
	       && (p->ro - p->io == d->sign)

	       && (0
		   /* can operate out-of-place */
		   || p->ri != p->ro

		   /*
		    * can compute one transform in-place, no matter
		    * what the strides are.
		    */
		   || p->vecsz.rnk == 0


		   /* can operate in-place as long as strides are the same */
		   || (X(tensor_inplace_strides)(p->sz) &&
		       X(tensor_inplace_strides)(p->vecsz))
		    )
	       );
     }

     return 0;
}

static int score(const solver *ego, const problem *p)
{
     return (applicable(ego, p)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_dft *p;
     iodim *d, *vd;
     const kdft_k7_desc *e = ego->desc;

     static const plan_adt padt = {
	  X(dft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *)0;

     p = (const problem_dft *) p_;

     pln = MKPLAN_DFT(P, &padt, e->sign == -1 ? applyr : applyi);

     d = p->sz.dims;
     vd = p->vecsz.dims;

     pln->k = ego->k;
     pln->is = d[0].is;
     pln->os = d[0].os;

     if (p->vecsz.rnk == 0) {
          pln->vl = 1;
          pln->ivs = pln->ovs = 0;
     } else {
          pln->vl = vd[0].n;
          pln->ivs = vd[0].is;
          pln->ovs = vd[0].os;
     }

     pln->slv = ego;
     pln->super.super.ops = X(ops_mul)(pln->vl, e->ops);

     return &(pln->super.super);
}

/* constructor */
solver *X(mksolver_dft_direct_k7)(kdft_k7 k, const kdft_k7_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = k;
     slv->desc = desc;
     return &(slv->super);
}
