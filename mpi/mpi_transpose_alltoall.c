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

/* plans for distributed out-of-place transpose using MPI_Alltoall,
   and which destroy the input array */

#include "mpi_transpose.h"

typedef struct {
     plan_mpi_transpose super;

     plan *cld1, *cld2;

     MPI_Comm comm;
     int *send_block_sizes, *send_block_offsets;
     int *recv_block_sizes, *recv_block_offsets;

     INT nx, nxb1, b, bt;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld1, *cld2;

     /* transpose locally to get contiguous chunks */
     cld1 = (plan_rdft *) ego->cld1;
     if (cld1)
	  cld1->apply(ego->cld1, I, O);
     /* else !cld1, then SCRAMBLED_IN is true and transpose is already in O */

     /* transpose chunks globally */
     MPI_Alltoallv(O, ego->send_block_sizes, ego->send_block_offsets,
		   FFTW_MPI_TYPE,
		   I, ego->recv_block_sizes, ego->recv_block_offsets,
		   FFTW_MPI_TYPE,
		   ego->comm);

     /* transpose locally, again, to get ordinary row-major */
     cld2 = (plan_rdft *) ego->cld2;
     if (cld2) /* block size divides nx: ordinary transpose */
	  cld2->apply(ego->cld2, I, O);
     else { /* unequal block sizes, do manual "transpose" loops */
	  /* TODO: cache-oblivious algorithm?  Use memcpy? */
	  INT i, j, k;
	  INT nxb1 = ego->nxb1, bt = ego->bt, b = ego->b;
	  INT nx = ego->nx;
	  INT btb = bt * b;
	  for (i = 0; i < nxb1; ++i) {
	       for (j = 0; j < bt; ++j) 
		    for (k = 0; k < b; ++k)
			 O[j * nx + k] = I[j * b + k];
	       I += btb;
	       O += b;
	  }
	  b = nx - nxb1 * b; /* leftover block size */
	  for (j = 0; j < bt; ++j)
	       for (k = 0; k < b; ++k)
		    O[j * nx + k] = I[j * b + k];
     }
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const problem_mpi_transpose *p = (const problem_mpi_transpose *) p_;
     UNUSED(ego_);
     return (1
	     && p->I != p->O
	     && !(p->flags & SCRAMBLED_OUT) /* not useful to support here */
	     && !NO_DESTROY_INPUTP(plnr)
	  );
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cld1, wakefulness);
     X(plan_awake)(ego->cld2, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(ifree0)(ego->send_block_sizes);
     MPI_Comm_free(&ego->comm);
     X(plan_destroy_internal)(ego->cld2);
     X(plan_destroy_internal)(ego->cld1);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(mpi_transpose-alltoall");
     if (ego->cld1) p->print(p, "%(%p%)", ego->cld1);
     if (ego->cld2) p->print(p, "%(%p%)", ego->cld2);
     p->print(p, ")");
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_mpi_transpose *p;
     P *pln;
     plan *cld1 = 0, *cld2 = 0;
     INT b, bt, nxb, vn;
     int *sbs, *sbo, *rbs, *rbo;
     int pe, my_pe, n_pes;
     static const plan_adt padt = {
          X(mpi_transpose_solve), awake, print, destroy
     };

     UNUSED(ego);

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_mpi_transpose *) p_;
     vn = p->vn;

     b = X(current_block)(p->nx, p->block, p->comm);

     if (!(p->flags & SCRAMBLED_IN)) {
	  cld1 = X(mkplan_d)(plnr, 
			     X(mkproblem_rdft_0_d)(X(mktensor_3d)
						   (b, p->ny * vn, vn,
						    p->ny, vn, b * vn,
						    vn, 1, 1),
						   p->I, p->O));
	  if (!cld1) goto nada;
     }

     b = p->block * vn;
     bt = X(current_block)(p->ny, p->tblock, p->comm);
     nxb = (p->nx + p->block - 1) / p->block;
     if (p->nx == nxb * p->block) { /* divisible => ordinary transpose */
	  cld2 = X(mkplan_d)(plnr, 
			     X(mkproblem_rdft_0_d)(X(mktensor_3d)
						   (nxb, bt * b, b,
						    bt, b, nxb * b,
						    b, 1, 1),
						   p->I, p->O));
	  if (!cld2) goto nada;
     }

     pln = MKPLAN_MPI_TRANSPOSE(P, &padt, apply);

     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->nx = p->nx * vn;
     pln->nxb1 = nxb - 1;
     pln->b = b;
     pln->bt = bt;

     MPI_Comm_dup(p->comm, &pln->comm);

     /* Compute sizes/offsets of blocks to send for all-to-all
        command.  TODO: In the special case where all block sizes are
        equal, we could use the MPI_Alltoall command.  It's not clear
        whether/why this would be any faster, though. */
     MPI_Comm_rank(p->comm, &my_pe);
     MPI_Comm_size(p->comm, &n_pes);
     sbs = (int *) MALLOC(4 * n_pes * sizeof(int), PLANS);
     sbo = sbs + n_pes;
     rbs = sbo + n_pes;
     rbo = rbs + n_pes;
     b = X(some_block)(p->nx, p->block, my_pe, n_pes);
     bt = X(some_block)(p->ny, p->tblock, my_pe, n_pes);
     for (pe = 0; pe < n_pes; ++pe) {
	  INT db, dbt; /* destination block sizes */
	  db = X(some_block)(p->nx, p->block, pe, n_pes);
	  dbt = X(some_block)(p->ny, p->tblock, pe, n_pes);

	  /* MPI requires type "int" here; apparently it
	     has no 64-bit API?  Grrr. */
	  sbs[pe] = (int) (b * dbt * vn);
	  sbo[pe] = (int) (pe * (b * p->tblock) * vn);
	  rbs[pe] = (int) (db * bt * vn);
	  rbo[pe] = (int) (pe * (p->block * bt) * vn);
     }
     pln->send_block_sizes = sbs;
     pln->send_block_offsets = sbo;
     pln->recv_block_sizes = rbs;
     pln->recv_block_offsets = rbo;

     if (cld1 && cld2)
	  X(ops_add)(&cld1->ops, &cld2->ops, &pln->super.super.ops);
     else if (cld2)
	  pln->super.super.ops = cld2->ops;
     else {
	  if (cld1)
	       pln->super.super.ops = cld1->ops;
	  else
	       X(ops_zero)(&pln->super.super.ops);
	  pln->super.super.ops.other += bt * p->nx * vn * 2; /* transpose */
     }
     /* FIXME: should MPI operations be counted in "other" somehow? */

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld2);
     X(plan_destroy_internal)(cld1);
     return (plan *) 0;
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_MPI_TRANSPOSE, mkplan };
     return MKSOLVER(solver, &sadt);
}

void X(mpi_transpose_alltoall_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
