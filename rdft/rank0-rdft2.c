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

/* $Id: rank0-rdft2.c,v 1.5 2003-02-28 23:28:58 stevenj Exp $ */

/* plans for rank-0 RDFT2 (copy operations, plus setting 0 imag. parts) */

#include "rdft.h"

#ifdef HAVE_STRING_H
#include <string.h>		/* for memcpy() */
#endif

typedef struct {
     solver super;
} S;

typedef struct {
     plan_rdft super;
     int vl;
     int ivs, ovs;
     plan *cldcpy;
} P;

static int applicable(const problem *p_)
{
     if (RDFT2P(p_)) {
          const problem_rdft2 *p = (const problem_rdft2 *) p_;
          return (1
                  && p->sz->rnk == 0
		  && (p->kind == HC2R
		      || (((p->r != p->rio && p->r != p->iio)
			   || X(rdft2_inplace_strides)(p, RNK_MINFTY))
			  && p->vecsz->rnk <= 1))
	       );
     }
     return 0;
}

static void apply_r2hc(const plan *ego_, R *r, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     int i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;

     for (i = 4; i <= vl; i += 4) {
          R r0, r1, r2, r3;
          r0 = *r; r += ivs;
          r1 = *r; r += ivs;
          r2 = *r; r += ivs;
          r3 = *r; r += ivs;
          *rio = r0; rio += ovs;
	  *iio = 0.0; iio += ovs;
          *rio = r1; rio += ovs;
	  *iio = 0.0; iio += ovs;
          *rio = r2; rio += ovs;
	  *iio = 0.0; iio += ovs;
	  *rio = r3; rio += ovs;
	  *iio = 0.0; iio += ovs;
     }
     for (; i < vl + 4; ++i) {
          R r0;
          r0 = *r; r += ivs;
          *rio = r0; rio += ovs;
	  *iio = 0.0; iio += ovs;
     }
}

/* in-place r2hc rank-0: set imaginary parts of output to 0 */
static void apply_r2hc_inplace(const plan *ego_, R *r, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     int i, vl = ego->vl;
     int ovs = ego->ovs;

     UNUSED(r);
     UNUSED(rio);
     for (i = 4; i <= vl; i += 4) {
	  *iio = 0.0; iio += ovs;
	  *iio = 0.0; iio += ovs;
	  *iio = 0.0; iio += ovs;
	  *iio = 0.0; iio += ovs;
     }
     for (; i < vl + 4; ++i) {
	  *iio = 0.0; iio += ovs;
     }
}

/* a rank-0 HC2R rdft2 problem is just a copy from rio to r,
   so we can use a rank-0 rdft plan */
static void apply_hc2r(const plan *ego_, R *r, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cldcpy = (plan_rdft *) ego->cldcpy;
     UNUSED(iio);
     cldcpy->apply((plan *) cldcpy, rio, r);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     if (ego->cldcpy)
	  AWAKE(ego->cldcpy, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     if (ego->cldcpy)
	  X(plan_destroy_internal)(ego->cldcpy);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     if (ego->cldcpy)
	  p->print(p, "(rdft2-hc2r-rank0%(%p%))", ego->cldcpy);
     else
	  p->print(p, "(rdft2-r2hc-rank0%v)", ego->vl);
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const problem_rdft2 *p;
     plan *cldcpy = (plan *) 0;
     P *pln;

     static const plan_adt padt = {
	  X(rdft2_solve), awake, print, destroy
     };

     UNUSED(ego_);

     if (!applicable(p_))
          return (plan *) 0;

     p = (const problem_rdft2 *) p_;

     if (p->kind == HC2R) {
	  cldcpy = X(mkplan_d)(plnr,
			       X(mkproblem_rdft_d)(
				    X(mktensor_0d)(),
				    X(tensor_copy)(p->vecsz),
				    p->rio, p->r, (rdft_kind *) 0));
	  if (!cldcpy) return (plan *) 0;
     }

     pln = MKPLAN_RDFT2(P, &padt, 
			p->kind == R2HC ? 
			(p->r == p->rio ? apply_r2hc_inplace : apply_r2hc) 
			: apply_hc2r);
     
     if (p->kind == R2HC)
	  X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     pln->cldcpy = cldcpy;

     if (p->kind == R2HC) {
	  /* vl loads, 2*vl stores */
	  X(ops_other)(3 * pln->vl, &pln->super.super.ops);
     }
     else {
	  pln->super.super.ops = cldcpy->ops;
     }

     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(rdft2_rank0_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
