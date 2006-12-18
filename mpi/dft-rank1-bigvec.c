/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* Complex DFTs of rank == 1 when the vector length vn is >= # processes.
   In this case, we don't need to use a six-step type algorithm, and can
   instead transpose the DFT dimension with the vector dimension to 
   make the DFT local.

   In this case, TRANSPOSED_IN/OUT mean that the DFT dimension is pre/post
   transposed with the vector dimension, respectively. */

#include "mpi-dft.h"
#include "mpi-transpose.h"
#include "dft.h"

/* from api/extract-reim.c */
extern void X(extract_reim)(int sign, R *c, R **r, R **i);

typedef struct {
     plan_mpi_dft super;

     plan *cldt_before, *cld, *cldt_after;
     INT roff, ioff;
     int preserve_input;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;
     plan_rdft *cldt_before, *cldt_after;
     INT roff = ego->roff, ioff = ego->ioff;
     
     /* global transpose, if not TRANSPOSED_IN */
     cldt_before = (plan_rdft *) ego->cldt_before;
     if (cldt_before) {
	  cldt_before->apply(ego->cldt_before, I, O);
	  
	  if (ego->preserve_input) I = O;
	  
	  /* 1d DFT(s) */
	  cld = (plan_dft *) ego->cld;
	  cld->apply(ego->cld, O+roff, O+ioff, I+roff, I+ioff);
	  
	  /* global transpose, if not TRANSPOSED_OUT */
	  cldt_after = (plan_rdft *) ego->cldt_after;
	  if (cldt_after)
	       cldt_after->apply(ego->cldt_after, I, O);
     }
     else {
	  /* 1d DFT(s) */
	  cld = (plan_dft *) ego->cld;
	  cld->apply(ego->cld, I+roff, I+ioff, O+roff, O+ioff);
	  
	  /* global transpose, if not TRANSPOSED_OUT */
	  cldt_after = (plan_rdft *) ego->cldt_after;
	  if (cldt_after)
	       cldt_after->apply(ego->cldt_after, O, O);
     }
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const problem_mpi_dft *p = (const problem_mpi_dft *) p_;
     int n_pes;
     UNUSED(ego_);
     UNUSED(plnr);
     MPI_Comm_size(p->comm, &n_pes);
     return (1
	     && p->sz->rnk == 1
	     && p->vn >= n_pes /* TODO: relax this, using more memory? */
	     && (!NO_UGLYP(plnr) /* ugly if dft-serial is applicable */
                 || !XM(dft_serial_applicable)(p))
	     && !SCRAMBLEDP(p->flags)
	  );
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cldt_before, wakefulness);
     X(plan_awake)(ego->cld, wakefulness);
     X(plan_awake)(ego->cldt_after, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cldt_after);
     X(plan_destroy_internal)(ego->cld);
     X(plan_destroy_internal)(ego->cldt_before);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(mpi-dft-rank1-bigvec");
     if (ego->cldt_before) p->print(p, "%(%p%)", ego->cldt_before);
     if (ego->cld) p->print(p, "%(%p%)", ego->cld);
     if (ego->cldt_after) p->print(p, "%(%p%)", ego->cldt_after);
     p->print(p, ")");
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_mpi_dft *p;
     P *pln;
     plan *cld = 0, *cldt_before = 0, *cldt_after = 0;
     R *ri, *ii, *ro, *io, *I, *O;
     INT vblock, vb;
     int my_pe, n_pes;
     static const plan_adt padt = {
          XM(dft_solve), awake, print, destroy
     };

     UNUSED(ego);

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_mpi_dft *) p_;

     MPI_Comm_rank(p->comm, &my_pe);
     MPI_Comm_size(p->comm, &n_pes);
     
     vblock = XM(default_block)(p->vn, n_pes);
     vb = XM(block)(p->vn, vblock, my_pe);

     if (!(p->flags & TRANSPOSED_IN)) { /* transpose dims[0].n with vn */
	  cldt_before = X(mkplan_d)(plnr,
				    XM(mkproblem_transpose)(
					 p->sz->dims[0].n, p->vn, 2,
					 I = p->I, O = p->O,
					 p->sz->dims[0].b[IB], vblock,
					 p->comm,
					 TRANSPOSED_OUT));
	  if (XM(any_true)(!cldt_before, p->comm)) goto nada;	  
	  if (NO_DESTROY_INPUTP(plnr)) { I = O; }
     }
     else {
	  I = p->O; O = p->I; /* swap for 1d DFT to avoid a copy from I->O */
     }

     X(extract_reim)(p->sign, I, &ri, &ii);
     X(extract_reim)(p->sign, O, &ro, &io);

     cld = X(mkplan_d)(plnr,
		       X(mkproblem_dft_d)(X(mktensor_1d)(p->sz->dims[0].n,
							 vb*2, vb*2),
					  X(mktensor_1d)(vb, 2, 2),
					  ro, io, ri, ii));
     if (XM(any_true)(!cld, p->comm)) goto nada;	  

     if (!(p->flags & TRANSPOSED_OUT)) {
	  cldt_after = X(mkplan_d)(plnr,
				    XM(mkproblem_transpose)(
					 p->vn, p->sz->dims[0].n, 2,
					 I, cldt_before ? O : I,
					 vblock, p->sz->dims[0].b[OB], 
					 p->comm,
					 TRANSPOSED_IN));
	  if (XM(any_true)(!cldt_after, p->comm)) goto nada;	  
     }

     pln = MKPLAN_MPI_DFT(P, &padt, apply);

     pln->cldt_before = cldt_before;
     pln->cld = cld;
     pln->cldt_after = cldt_after;
     pln->preserve_input = NO_DESTROY_INPUTP(plnr);
     pln->roff = ro - p->O;
     pln->ioff = io - p->O;

     pln->super.super.ops = cld->ops;
     if (cldt_before) X(ops_add2)(&cldt_before->ops, &pln->super.super.ops);
     if (cldt_after) X(ops_add2)(&cldt_after->ops, &pln->super.super.ops);

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cldt_after);
     X(plan_destroy_internal)(cld);
     X(plan_destroy_internal)(cldt_before);
     return (plan *) 0;
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_MPI_DFT, mkplan };
     return MKSOLVER(solver, &sadt);
}

void XM(dft_rank1_bigvec_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
