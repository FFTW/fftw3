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

/* Plans for distributed in-place transposes (or out-of-place, while
   preserving the input).  These require a slightly tricky sequence of
   exchanges between processors.  The amount of buffer space required
   is proportional to the local size divided by the number of
   processors (i.e. to total array size divided by the number of
   processors squared). */

#include "mpi_transpose.h"
#include <string.h>

typedef struct {
     plan_mpi_transpose super;

     plan *cld1, *cld2, *cld3, *cld4;
     
     int n_pes, my_pe, *sched;
     INT *send_block_sizes, *send_block_offsets;
     INT *recv_block_sizes, *recv_block_offsets;
     MPI_Comm comm;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld1, *cld2, *cld3, *cld4;
     int *sched;

     /* transpose locally to get contiguous chunks */
     cld1 = (plan_rdft *) ego->cld1;
     if (cld1)
	  cld1->apply(ego->cld1, I, O);
     /* else !cld1, then SCRAMBLED_IN is true and transpose is already in O */

     /* transpose chunks globally */
     sched = ego->sched;
     if (sched) {
	  int n_pes = ego->n_pes, my_pe = ego->my_pe, i;
	  INT *sbs = ego->send_block_sizes;
	  INT *sbo = ego->send_block_offsets;
	  INT *rbs = ego->recv_block_sizes;
	  INT *rbo = ego->recv_block_offsets;
	  MPI_Comm comm = ego->comm;
	  MPI_status status;
	  R *buf = (R*) MALLOC(sizeof(R) * sbs[0], BUFFERS);

	  for (i = 0; i < n_pes; ++i) {
	       int pe = sched[i];
	       if (my_pe == pe) {
		    if (rbo[pe] != sbo[pe])
			 memmove(O + rbo[pe], O + sbo[pe],
				 sbs[pe] * sizeof(R));
	       }
	       else {
		    memcpy(buf, O + sbo[pe], sbs[pe] * sizeof(R));
		    MPI_Sendrecv(buf, (int) (sbs[pe]), FFTW_MPI_TYPE,
				 pe, my_pe * n_pes + pe,
				 O + rbo[pe], (int) (rbs[pe]), FFTW_MPI_TYPE,
				 pe, pe * n_pes + my_pe,
				 comm, &status);
		    /* TODO: explore non-synchronous send/recv? */
	       }
	  }
	  
	  X(ifree)(buf);
     }

     /* transpose locally, again, to get ordinary row-major;
        this may take two transposes if the block sizes are unequal
        (3 subplans, two of which operate on disjoint data) */
     cld2 = (plan_rdft *) ego->cld2;
     cld2->apply(ego->cld2, O, O);
     cld3 = (plan_rdft *) ego->cld3;
     if (cld3) {
          cld3->apply(ego->cld3, O, O); /* leftover from cld2 */
	  cld4 = (plan_rdft *) ego->cld4;
	  if (cld4)
	       cld4->apply(ego->cld4, O, O);
	  /* else SCRAMBLED_OUT is true and user wants O transposed */
     }
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const problem_mpi_transpose *p = (const problem_mpi_transpose *) p_;
     UNUSED(ego_);
     UNUSED(p_);
     UNUSED(plnr);
     return (1); /* FIXME: ugly if out-of-place and destroy_input? */
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cld1, wakefulness);
     X(plan_awake)(ego->cld2, wakefulness);
     X(plan_awake)(ego->cld3, wakefulness);
     X(plan_awake)(ego->cld4, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(ifree0)(ego->sched);
     X(ifree0)(ego->send_block_sizes);
     MPI_Comm_free(&ego->comm);
     X(plan_destroy_internal)(ego->cld4);
     X(plan_destroy_internal)(ego->cld3);
     X(plan_destroy_internal)(ego->cld2);
     X(plan_destroy_internal)(ego->cld1);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(mpi_transpose-inplace");
     if (ego->cld1) p->print(p, "%(%p%)", ego->cld1);
     if (ego->cld2) p->print(p, "%(%p%)", ego->cld2);
     if (ego->cld3) p->print(p, "%(%p%)", ego->cld3);
     if (ego->cld4) p->print(p, "%(%p%)", ego->cld4);
     p->print(p, ")");
}

/* Given a process which_pe and a number of processes npes, fills
   the array sched[npes] with a sequence of processes to communicate
   with for a deadlock-free, optimum-overlap all-to-all communication.
   (All processes must call this routine to get their own schedules.)
   The schedule can be re-ordered arbitrarily as long as all processes
   apply the same permutation to their schedules.

   The algorithm here is based upon the one described in:
       J. A. M. Schreuder, "Constructing timetables for sport
       competitions," Mathematical Programming Study 13, pp. 58-67 (1980). 
   In a sport competition, you have N teams and want every team to
   play every other team in as short a time as possible (maximum overlap
   between games).  This timetabling problem is therefore identical
   to that of an all-to-all communications problem.  In our case, there
   is one wrinkle: as part of the schedule, the process must do
   some data transfer with itself (local data movement), analogous
   to a requirement that each team "play itself" in addition to other
   teams.  With this wrinkle, it turns out that an optimal timetable
   (N parallel games) can be constructed for any N, not just for even
   N as in the original problem described by Schreuder.
*/
static void fill1_comm_sched(int *sched, int which_pe, int npes)
{
     int pe, i, n, s = 0;
     A(which_pe >= 0 && which_pe < npes);
     if (npes % 2 == 0) {
	  n = npes;
	  sched[s++] = which_pe;
     }
     else
	  n = npes + 1;
     for (pe = 0; pe < n - 1; ++pe) {
	  if (npes % 2 == 0) {
	       if (pe == which_pe) sched[s++] = npes - 1;
	       else if (npes - 1 == which_pe) sched[s++] = pe;
	  }
	  else if (pe == which_pe) sched[s++] = pe;

	  if (pe != which_pe && which_pe < n - 1) {
	       i = (pe - which_pe + (n - 1)) % (n - 1);
	       if (i < n/2)
		    sched[s++] = (pe + i) % (n - 1);
	       
	       i = (which_pe - pe + (n - 1)) % (n - 1);
	       if (i < n/2)
		    sched[s++] = (pe - i + (n - 1)) % (n - 1);
	  }
     }
     A(s == npes);
}

/* sort the communication schedule sched for npes so that the schedule
   on process sortpe is ascending or descending (!ascending). */
static void sort1_comm_sched(int *sched, int npes, int sortpe, int ascending)
{
     int *sortsched, i;
     sortsched = (int *) MALLOC(npes * sizeof(int) * 2, OTHER);
     fill1_comm_sched(sortsched, sortpe, npes);
     if (ascending)
	  for (i = 0; i < npes; ++i)
	       sortsched[npes + sortsched[i]] = sched[i];
     else
	  for (i = 0; i < npes; ++i)
	       sortsched[2*npes - 1 - sortsched[i]] = sched[i];
     for (i = 0; i < npes; ++i)
	  sched[i] = sortsched[npes + i];
     X(ifree)(sortsched);
}

static tensor *mktensor_4d(INT n0, INT is0, INT os0,
			   INT n1, INT is1, INT os1,
			   INT n2, INT is2, INT os2,
			   INT n3, INT is3, INT os3)
{
     tensor *x = X(mktensor)(4);
     x->dims[0].n = n0;
     x->dims[0].is = is0;
     x->dims[0].os = os0;
     x->dims[1].n = n1;
     x->dims[1].is = is1;
     x->dims[1].os = os1;
     x->dims[2].n = n2;
     x->dims[2].is = is2;
     x->dims[2].os = os2;
     x->dims[3].n = n3;
     x->dims[3].is = is3;
     x->dims[3].os = os3;
     return x;
}

static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     const problem_mpi_transpose *p;
     P *pln;
     plan *cld1 = 0, *cld2 = 0, *cld3 = 0, *cld4 = 0;
     INT b, bt, nxb;
     INT *sbs, *sbo, *rbs, *rbo;
     int pe, my_pe, n_pes, sort_pe = -1, ascending = 1;
     static const plan_adt padt = {
          X(mpi_transpose_solve), awake, print, destroy
     };

     UNUSED(ego);

     if (!applicable(ego, p, plnr))
          return (plan *) 0;

     p = (const problem_mpi_transpose *) p_;
     b = X(current_block)(p->nx, p->block, p->comm);

     if (!(p->flags & SCRAMBLED_IN)) {
	  cld1 = X(mkplan_d)(plnr, 
			     X(mkproblem_rdft_0_d)(X(mktensor_3d)
						   (b, p->ny * p->vn, p->vn,
						    p->ny, p->vn, b * p->vn,
						    p->vn, 1, 1),
						   p->I, p->O));
	  if (!cld1) goto nada;
     }

     b = p->block;
     bt = X(current_block)(p->ny, p->tblock, p->comm);
     nxb = p->nx / p->block;
     if (p->nx == nxb * p->block) { /* divisible => ordinary transpose */
	  if (!(p->flags & SCRAMBLED_OUT)) {
	       b *= p->vn;
	       cld2 = X(mkplan_d)(plnr, 
				  X(mkproblem_rdft_0_d)(X(mktensor_3d)
							(nxb, bt * b, b,
							 bt, b, nxb * b,
							 b, 1, 1),
							p->O, p->O));
	  }
	  else {
	       cld2 = X(mkplan_d)(plnr, 
				  X(mkproblem_rdft_0_d)(
				       mktensor_4d
				       (nxb, bt * b * vn, bt * b * vn,
					bt, b * vn, vn,
					b, vn, bt * vn,
					vn, 1, 1),
				       p->O, p->O));
	  }
	  if (!cld2) goto nada;
     }
     else {
	  cld2 = X(mkplan_d)(plnr,
			     X(mkproblem_rdft_0_d)(
				  mktensor_4d
				  (nxb, bt * b * vn, bt * b * vn,
				   bt, b * vn, vn,
				   b, vn, bt * vn,
				   vn, 1, 1),
				  p->O, p->O));
	  if (!cld2) goto nada;
	  nxb = p->nx - nxb * p->block
	  cld3 = X(mkplan_d)(plnr,
			     X(mkproblem_rdft_0_d)(
				  X(mktensor_3d)
				  (bt, nxb * vn, vn,
				   nxb, vn, bt * vn,
				   vn, 1, 1),
				  p->O, p->O));
	  if (!cld3) goto nada;
	  if (!(p->flags & SCRAMBLED_OUT)) {
	       cld4 = X(mkplan_d)(plnr,
				  X(mkproblem_rdft_0_d)(
				       X(mktensor_3d)
				       (p->nx, bt * vn, vn,
					bt, vn, p->nx * vn,
					vn, 1, 1),
				       p->O, p->O));
	       if (!cld4) goto nada;
	  }
     }

     pln = MKPLAN_MPI_TRANSPOSE(P, &padt, apply);

     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->cld3 = cld3;
     pln->cld4 = cld4;

     MPI_Comm_dup(p->comm, &pln->comm);

     MPI_Comm_rank(p->comm, &my_pe);
     MPI_Comm_size(p->comm, &n_pes);

     /* Compute sizes/offsets of blocks to exchange between processors */
     sbs = (INT *) MALLOC(4 * n_pes * sizeof(INT));
     sbo = sbs + n_pes;
     rbs = sbo + n_pes;
     rbo = rbs + n_pes;
     b = X(some_block)(p->nx, p->block, my_pe, n_pes);
     bt = X(some_block)(p->ny, p->tblock, my_pe, n_pes);
     for (pe = 0; pe < n_pes; ++pe) {
	  INT db, dbt; /* destination block sizes */
	  db = X(some_block)(p->nx, p->block, pe, n_pes);
	  dbt = X(some_block)(p->ny, p->tblock, pe, n_pes);

	  sbs[pe] = b * dbt * vn;
	  sbo[pe] = pe * (b * p->tblock) * vn;
	  rbs[pe] = db * bt * vn;
	  rbo[pe] = pe * (p->block * bt) * vn;

	  if (sbs[pe] == 0 && rbs[pe] == 0)
	       n_pes = pe; /* no need to deal with this or later pe's */
	  else if (db * dbt > 0 && db * p->tblock != p->block * dbt) {
	       A(sort_pe == -1); /* only one process should need sorting */
	       sort_pe = pe;
	       ascending = db * p->tblock > p->block * dbt;
	  }
     }
     pln->n_pes = n_pes;
     pln->my_pe = my_pe;
     pln->send_block_sizes = sbs;
     pln->send_block_offsets = sbo;
     pln->recv_block_sizes = rbs;
     pln->recv_block_offsets = rbo;

     if (my_pe >= n_pes) {
	  pln->sched = 0; /* this process is not doing anything */
     }
     else {
	  pln->sched = (INT *) MALLOC(n_pes * sizeof(INT));
	  fill1_comm_sched(pln->sched, my_pe, n_pes);
	  if (sort_pe >= 0)
	       sort1_comm_sched(pln->sched, n_pes, sort_pe, ascending);
     }

     /* FIXME: OPS */

     return &(pln->super);

 nada:
     X(plan_destroy_internal)(cld4);
     X(plan_destroy_internal)(cld3);
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
