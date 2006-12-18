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

/* Recursive "radix 2" distributed transpose, which breaks a transpose
   over p processes into two transposes over p/2 processes plus
   a combination step with O(p) messages.  If performed recursively,
   this produces a transpose with O(p log p) messages vs. O(p^2)
   messages for a direct approach.

   However, this is not necessarily an improvement.  The total size
   of all the messages is actually increased from O(N) to O(N log p)
   where N is the total data size.  Also, the amount of local data
   rearrangements is increased.  So, it's not clear, a priori, what
   the best algorithm will be, and we'll leave it to the planner.

   Note also that, for in-place transposes, this algorithm requires
   a substantial amount of scratch space (= 50% of local data size).
   So, we'll probably want to disable it in CONSERVE_MEMORY mode.
   
   We only implement this for the case where the p divides the
   transpose dimensions (i.e. equal block sizes = n / p), currently.

   (Similar O(p log p) algorithms can be implemented for larger
    radices r > 2, but they appear to be more complicated to code
    because two distributed transpose operations seem to be required
    among the p/r process groups, rather than one as for r=2.)
*/

#include "mpi-transpose.h"
#include <string.h>

typedef struct {
     plan_mpi_transpose super;

     plan *cld1, *cld2, *cldt, *cld3, *cld4;

     MPI_Comm comm;

     INT exchange_size, exchange_offset;
     int exchange_pe;

     int preserve_input;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld1, *cld2, *cldt, *cld3, *cld4;
     INT exchange_size, exchange_offset;
     MPI_Status status;
     R *buf;

     /* transpose locally to get contiguous chunks */
     cld1 = (plan_rdft *) ego->cld1;
     cld1->apply(ego->cld1, I, O);

     if (ego->preserve_input) I = O;

     /* transpose locally again to collect pairs of blocks
	offset by p/2 */
     cld2 = (plan_rdft *) ego->cld2;
     cld2->apply(ego->cld2, O, I);

     /* global transpose over half the processes */
     cldt = (plan_rdft *) ego->cldt;
     cldt->apply(ego->cldt, I, O);

     /* transpose locally to separate pairs of blocks offset by p/2 */
     cld3 = (plan_rdft *) ego->cld3;
     cld3->apply(ego->cld3, O, I);

     /* exchange data with exchange_pe */
     exchange_size = ego->exchange_size;
     exchange_offset = ego->exchange_offset;
     buf = I == O ? (R *) MALLOC(sizeof(R) * exchange_size, BUFFERS) : O;
     memcpy(buf, I + exchange_offset, sizeof(R) * exchange_size);
     MPI_Sendrecv(buf, (int) exchange_size, FFTW_MPI_TYPE,
		  ego->exchange_pe, 0,
		  I + exchange_offset, (int) exchange_size, FFTW_MPI_TYPE,
		  ego->exchange_pe, 0,
		  ego->comm, &status);
     if (I == O) X(ifree)(buf);

     /* do final local transpose */
     cld4 = (plan_rdft *) ego->cld4;
     cld4->apply(ego->cld4, I, O);     
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const problem_mpi_transpose *p = (const problem_mpi_transpose *) p_;
     int n_pes;
     UNUSED(ego_);
     UNUSED(plnr);
     MPI_Comm_size(p->comm, &n_pes);
     /* TODO: ugly if too few processors? */
     return (1
	     && n_pes % 2 == 0
	     && n_pes > 2 /* no point for 2 processes */
	     && p->nx % p->block == 0 && p->nx / p->block == n_pes
	     && p->ny % p->tblock == 0 && p->ny / p->tblock == n_pes
	  );
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cld1, wakefulness);
     X(plan_awake)(ego->cld2, wakefulness);
     X(plan_awake)(ego->cldt, wakefulness);
     X(plan_awake)(ego->cld3, wakefulness);
     X(plan_awake)(ego->cld4, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     MPI_Comm_free(&ego->comm);
     X(plan_destroy_internal)(ego->cld4);
     X(plan_destroy_internal)(ego->cld3);
     X(plan_destroy_internal)(ego->cldt);
     X(plan_destroy_internal)(ego->cld2);
     X(plan_destroy_internal)(ego->cld1);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(mpi-transpose-radix2");
     if (ego->cld1) p->print(p, "%(%p%)", ego->cld1);
     if (ego->cld2) p->print(p, "%(%p%)", ego->cld2);
     if (ego->cldt) p->print(p, "%(%p%)", ego->cldt);
     if (ego->cld3) p->print(p, "%(%p%)", ego->cld3);
     if (ego->cld4) p->print(p, "%(%p%)", ego->cld4);
     p->print(p, ")");
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_mpi_transpose *p;
     P *pln;
     plan *cld1 = 0, *cld2 = 0, *cldt = 0, *cld3 = 0, *cld4 = 0;
     INT b, bt, vn;
     int me, np;
     MPI_Comm comm2;
     R *I, *O;
     static const plan_adt padt = {
          XM(transpose_solve), awake, print, destroy
     };

     UNUSED(ego);

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_mpi_transpose *) p_;
     vn = p->vn;
     I = p->I;
     O = p->O;

     MPI_Comm_size(p->comm, &np);
     MPI_Comm_rank(p->comm, &me);
     b = XM(block)(p->nx, p->block, me);
     bt = XM(block)(p->ny, p->tblock, me);
     A(b == p->nx / np && bt == p->ny / np);

     if (p->flags & SCRAMBLED_IN) /* I is already transposed */
	  cld1 = X(mkplan_d)(plnr, 
			     X(mkproblem_rdft_0_d)(X(mktensor_1d)
						   (b * p->ny * vn, 1, 1),
						   I, O));
     else /* transpose b x ny x vn -> ny x b x vn */
	  cld1 = X(mkplan_d)(plnr, 
			     X(mkproblem_rdft_0_d)(X(mktensor_3d)
						   (b, p->ny * vn, vn,
						    p->ny, vn, b * vn,
						    vn, 1, 1),
						   I, O));
     if (XM(any_true)(!cld1, p->comm)) goto nada;

     if (NO_DESTROY_INPUTP(plnr)) I = O;

     /* transpose ny x b x vn = 2 x np/2 x bt x b x vn
            -> np/2 x 2 x bt x b x vn */
     cld2 = X(mkplan_d)(plnr,X(mkproblem_rdft_0_d)
			(X(mktensor_3d)
			 (2, np/2 * bt * b * vn, bt * b * vn,
			  np/2, bt * b * vn, 2 * bt * b * vn,
			  bt * b * vn, 1, 1),
			 O, I));
     if (XM(any_true)(!cld2, p->comm)) goto nada;

     MPI_Comm_split(p->comm, me < np/2, me, &comm2);
     cldt = X(mkplan_d)(plnr, XM(mkproblem_transpose)
			(np/2, np/2, 2 * bt * b * vn, 
			 I, O, 1, 1, comm2,
			 p->flags | SCRAMBLED_IN | SCRAMBLED_OUT));
     MPI_Comm_free(&comm2);
     if (XM(any_true)(!cldt, p->comm)) goto nada;

     /* transpose np/2 x 2 x bt x b x vn -> 2 x np/2 x bt x b x vn */
     cld3 = X(mkplan_d)(plnr,X(mkproblem_rdft_0_d)
			(X(mktensor_3d)
			 (np/2, 2 * bt * b * vn, bt * b * vn,
			  2, bt * b * vn, np/2 * bt * b * vn,
			  bt * b * vn, 1, 1),
			 O, I));
     if (XM(any_true)(!cld3, p->comm)) goto nada;

     /* Finally, transpose 2 x np/2 x bt x b x vn = np x bt x b x vn 
            -> np x b x bt x vn (SCRAMBLED_OUT)
        or  -> bt x np x b x vn (!SCRAMBLED_OUT) */
     if (p->flags & SCRAMBLED_OUT)
	  cld4 = X(mkplan_d)(plnr,
			     X(mkproblem_rdft_0_d)(
				  X(mktensor_4d)
				  (np, bt * b * vn, bt * b * vn,
				   bt, b * vn, vn,
				   b, vn, bt * vn,
				   vn, 1, 1),
				  I, O));
     else
	  cld4 = X(mkplan_d)(plnr,
			     X(mkproblem_rdft_0_d)(X(mktensor_3d)
						   (np, bt * b * vn, b * vn,
						    bt, b * vn, np * b * vn,
						    b * vn, 1, 1),
						   I, O));
     if (XM(any_true)(!cld4, p->comm)) goto nada;

     pln = MKPLAN_MPI_TRANSPOSE(P, &padt, apply);

     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->cldt = cldt;
     pln->cld3 = cld3;
     pln->cld4 = cld4;
     pln->preserve_input = NO_DESTROY_INPUTP(plnr);

     pln->exchange_pe = (me + np/2) % np;
     pln->exchange_size = np/2 * bt * b * vn;
     pln->exchange_offset = me < np/2 ? pln->exchange_size : 0;

     MPI_Comm_dup(p->comm, &pln->comm);

     X(ops_zero)(&pln->super.super.ops);
     if (cld1)
	  X(ops_add)(&pln->super.super.ops, &cld1->ops, &pln->super.super.ops);
     if (cld2)
	  X(ops_add)(&pln->super.super.ops, &cld2->ops, &pln->super.super.ops);
     if (cldt)
	  X(ops_add)(&pln->super.super.ops, &cldt->ops, &pln->super.super.ops);
     if (cld3)
	  X(ops_add)(&pln->super.super.ops, &cld3->ops, &pln->super.super.ops);
     if (cld4)
	  X(ops_add)(&pln->super.super.ops, &cld4->ops, &pln->super.super.ops);
     /* FIXME: should MPI operations be counted in "other" somehow? */

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld4);
     X(plan_destroy_internal)(cld3);
     X(plan_destroy_internal)(cldt);
     X(plan_destroy_internal)(cld2);
     X(plan_destroy_internal)(cld1);
     return (plan *) 0;
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_MPI_TRANSPOSE, mkplan };
     return MKSOLVER(solver, &sadt);
}

void XM(transpose_radix2_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
