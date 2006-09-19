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

/* "MPI" DFTs where all of the data is on one processor...just
   call through to serial API. */

#include "mpi-dft.h"
#include "dft.h"

/* from api/extract-reim.c */
extern void X(extract_reim)(int sign, R *c, R **r, R **i);

typedef struct {
     plan_mpi_dft super;
     plan *cld;
     INT roff, ioff;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;
     INT roff = ego->roff, ioff = ego->ioff;
     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, I+roff, I+ioff, O+roff, O+ioff);
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cld, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(mpi-dft-serial %(%p%))", ego->cld);
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_mpi_dft *p = (problem_mpi_dft *) p_;
     P *pln;
     plan *cld;
     int my_pe, n_pes;
     R *ri, *ii, *ro, *io;
     static const plan_adt padt = {
          X(mpi_dft_solve), awake, print, destroy
     };

     UNUSED(ego);

     /* check whether applicable: */
     MPI_Comm_size(p->comm, &n_pes);
     if (X(some_block)(p->n[0], p->block, 0, n_pes) < p->n[0]
	 || (p->rnk > 1
	     && X(some_block)(p->n[1], p->tblock, 0, n_pes) < p->n[1])
	 || (p->rnk == 1 && (p->flags & (SCRAMBLED_IN | SCRAMBLED_OUT))))
          return (plan *) 0;

     X(extract_reim)(p->sign, p->I, &ri, &ii);
     X(extract_reim)(p->sign, p->O, &ro, &io);

     MPI_Comm_rank(p->comm, &my_pe);
     if (my_pe == 0) {
	  int i, rnk = p->rnk;
	  tensor *sz = X(mktensor)(p->rnk);
	  sz->dims[rnk - 1].is = sz->dims[rnk - 1].os = 2 * p->vn;
	  sz->dims[rnk - 1].n = p->n[rnk - 1];
	  for (i = rnk - 1; i > 0; --i) {
	       sz->dims[i - 1].is = sz->dims[i - 1].os = 
		    sz->dims[i].is * p->n[i];
	       sz->dims[i - 1].n = p->n[i - 1];
	  }
	  
	  if (p->flags & SCRAMBLED_IN) { /* transposed input, rnk>1 */
	       sz->dims[0].is = sz->dims[1].is;
	       sz->dims[1].is = sz->dims[0].is * sz->dims[0].n;
	  }
	  if (((p->flags & SCRAMBLED_OUT) != 0) ^
	      ((p->flags & TRANSPOSED) != 0)) { /* transposed output, rnk>1 */
	       sz->dims[0].os = sz->dims[1].os;
	       sz->dims[1].os = sz->dims[0].os * sz->dims[0].n;
	  }
	  
	  cld = X(mkplan_d)(plnr,
			    X(mkproblem_dft_d)(sz,
					       X(mktensor_1d)(p->vn, 2, 2),
					       ri, ii, ro, io));
     }
     else { /* idle process: make nop plan */
	  cld = X(mkplan_d)(plnr,
			    X(mkproblem_dft_d)(X(mktensor_0d)(),
					       X(mktensor_1d)(0,0,0),
					       ri, ii, ro, io));
     }
     if (!cld) return (plan *) 0;

     pln = MKPLAN_MPI_DFT(P, &padt, apply);
     pln->cld = cld;
     pln->roff = ro - p->O;
     pln->ioff = io - p->O;
     X(ops_cpy)(&cld->ops, &pln->super.super.ops);
     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_MPI_DFT, mkplan };
     return MKSOLVER(solver, &sadt);
}

void X(mpi_dft_serial_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
