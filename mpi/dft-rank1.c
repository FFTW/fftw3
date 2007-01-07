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

/* Complex DFTs of rank == 1 via six-step algorithm. */

#include "mpi-dft.h"
#include "mpi-transpose.h"
#include "dft.h"

typedef struct {
     plan_mpi_dft super;

     triggen *t;
     plan *cldt_before, *cldm, *cldt_middle, *cldr, *cldt_after;
     INT roff, ioff;
     int preserve_input;
     INT vn, m, rmin, rmax, r;
} P;

static void do_twiddle(triggen *t, INT rmin, INT rmax, INT m,
		       INT vn, R *xr, R *xi)
{
     void (*rotate)(triggen *, INT, R, R, R *) = t->rotate;
     INT ir, im, iv;
     for (im = 0; im < m; ++im)
	  for (ir = rmin; ir <= rmax; ++ir)
	       for (iv = 0; iv < vn; ++iv) {
		    /* FIXME: modify/inline rotate function
		       so that it can do whole vn vector at once? */
		    R c[2];
		    rotate(t, ir * im, *xr, *xi, c);
		    *xr = c[0]; *xi = c[1];
		    xr += 2; xi += 2;
	       }
}

/* radix-r DFT of size r*m.  This is equivalent to an m x r 2d DFT,
   plus twiddle factors between the size-m and size-r 1d DFTs, where
   the m dimension is initially distributed.  The output is transposed
   to r x m where the r dimension is distributed. */
static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_dft *cldm, *cldr;
     plan_rdft *cldt_before, *cldt_middle, *cldt_after;
     INT roff = ego->roff, ioff = ego->ioff;
     
     /* global transpose (m x r -> r x m), if not SCRAMBLED_IN */
     cldt_before = (plan_rdft *) ego->cldt_before;
     if (cldt_before) {
	  cldt_before->apply(ego->cldt_before, I, O);

	  if (ego->preserve_input) I = O;

	  /* TODO: perform size-m DFTs and twiddle multiplications
	     in batches, similar to dftw-genericbuf.c? */

	  /* 1d DFTs of size m */
	  cldm = (plan_dft *) ego->cldm;
	  cldm->apply(ego->cldm, O+roff, O+ioff, I+roff, I+ioff);

	  /* multiply I by twiddle factors */
	  do_twiddle(ego->t, ego->rmin, ego->rmax, ego->m, ego->vn,
		     I+roff, I+ioff);
	  
	  /* global transpose (r x m -> m x r) */
	  cldt_middle = (plan_rdft *) ego->cldt_middle;
	  cldt_middle->apply(ego->cldt_middle, I, O);

	  cldt_after = (plan_rdft *) ego->cldt_after;
	  cldr = (plan_dft *) ego->cldr;
	  if (cldt_after) {
	       /* 1d DFTs of size r */
	       cldr->apply(ego->cldr, O+roff, O+ioff, I+roff, I+ioff);

	       /* final global transpose (m x r -> r x m) */
	       cldt_after->apply(ego->cldt_after, I, O);
	  }
	  else { /* SCRAMBLED_OUT */
	       /* 1d DFTs of size r */
	       cldr->apply(ego->cldr, O+roff, O+ioff, O+roff, O+ioff);
	  }
     }
     else { /* SCRAMBLED_IN */
	  /* TODO: perform size-m DFTs and twiddle multiplications
	     in batches, similar to dftw-genericbuf.c? */

	  /* 1d DFTs of size m */
	  cldm = (plan_dft *) ego->cldm;
	  cldm->apply(ego->cldm, I+roff, I+ioff, O+roff, O+ioff);

	  if (ego->preserve_input) I = O;	  

	  /* multiply O by twiddle factors */
	  do_twiddle(ego->t, ego->rmin, ego->rmax, ego->m, ego->vn,
		     O+roff, O+ioff);
	  
	  /* global transpose (r x m -> m x r) */
	  cldt_middle = (plan_rdft *) ego->cldt_middle;
	  cldt_middle->apply(ego->cldt_middle, O, I);

	  cldt_after = (plan_rdft *) ego->cldt_after;
	  cldr = (plan_dft *) ego->cldr;
	  if (cldt_after) {
	       /* 1d DFTs of size r */
	       cldr->apply(ego->cldr, I+roff, I+ioff, I+roff, I+ioff);

	       /* final global transpose (m x r -> r x m) */
	       cldt_after->apply(ego->cldt_after, I, O);
	  }
	  else { /* SCRAMBLED_OUT */
	       /* 1d DFTs of size r */
	       cldr->apply(ego->cldr, I+roff, I+ioff, O+roff, O+ioff);
	  }
     }
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr,
		      INT *r, INT rblock[2], INT mblock[2])
{
     const problem_mpi_dft *p = (const problem_mpi_dft *) p_;
     int n_pes;
     UNUSED(ego_);
     UNUSED(plnr);
     MPI_Comm_size(p->comm, &n_pes);
     return (1
	     && p->sz->rnk == 1

	     && ONLY_SCRAMBLEDP(p->flags)

	     && (!NO_UGLYP(plnr) /* ugly if dft-serial is applicable */
                 || !XM(dft_serial_applicable)(p))

	     /* disallow if dft-rank1-bigvec is applicable since the
		data distribution may be slightly different (ugh!) */
	     && (p->vn < n_pes || p->flags)

	     && (*r = XM(choose_radix)(p->sz->dims[0], n_pes,
				       p->flags, p->sign,
				       rblock, mblock))
	  );
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cldt_before, wakefulness);
     X(plan_awake)(ego->cldm, wakefulness);
     X(plan_awake)(ego->cldt_middle, wakefulness);
     X(plan_awake)(ego->cldr, wakefulness);
     X(plan_awake)(ego->cldt_after, wakefulness);

     switch (wakefulness) {
         case SLEEPY:
              X(triggen_destroy)(ego->t); ego->t = 0;
              break;
         default:
              ego->t = X(mktriggen)(AWAKE_SQRTN_TABLE, ego->r * ego->m);
              break;
     }
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cldt_after);
     X(plan_destroy_internal)(ego->cldr);
     X(plan_destroy_internal)(ego->cldt_middle);
     X(plan_destroy_internal)(ego->cldm);
     X(plan_destroy_internal)(ego->cldt_before);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(mpi-dft-rank1/%D", ego->r);
     if (ego->cldt_before) p->print(p, "%(%p%)", ego->cldt_before);
     if (ego->cldm) p->print(p, "%(%p%)", ego->cldm);
     if (ego->cldt_middle) p->print(p, "%(%p%)", ego->cldt_middle);
     if (ego->cldr) p->print(p, "%(%p%)", ego->cldr);
     if (ego->cldt_after) p->print(p, "%(%p%)", ego->cldt_after);
     p->print(p, ")");
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_mpi_dft *p;
     P *pln;
     plan *cldm = 0, *cldr = 0, 
	  *cldt_before = 0, *cldt_middle = 0, *cldt_after = 0;
     R *ri, *ii, *ro, *io, *I, *O;
     INT r, rblock[2], rb, m, mblock[2], mb;
     int my_pe, n_pes;
     static const plan_adt padt = {
          XM(dft_solve), awake, print, destroy
     };

     UNUSED(ego);

     if (!applicable(ego, p_, plnr, &r, rblock, mblock))
          return (plan *) 0;

     p = (const problem_mpi_dft *) p_;

     MPI_Comm_rank(p->comm, &my_pe);
     MPI_Comm_size(p->comm, &n_pes);

     m = p->sz->dims[0].n / r;
     mb = XM(block)(m, mblock[IB], my_pe);
     rb = XM(block)(r, rblock[IB], my_pe);

     if (!(p->flags & SCRAMBLED_IN)) { /* transpose m x r -> r x m */
	  cldt_before = X(mkplan_d)(plnr,
				    XM(mkproblem_transpose)(
					 m, r, p->vn * 2,
					 I = p->I, O = p->O,
					 mblock[IB], rblock[IB],
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

     cldm = X(mkplan_d)(plnr,
			X(mkproblem_dft_d)(X(mktensor_1d)(m, rb*p->vn*2,
							  rb*p->vn*2),
					   X(mktensor_1d)(rb * p->vn, 2, 2),
					   ro, io, ri, ii));
     if (XM(any_true)(!cldm, p->comm)) goto nada;	  

     if (!cldt_before && NO_DESTROY_INPUTP(plnr)) {
	  O = I; ro = ri; io = ii;
     }

     /* transpose r x m -> m x r */
     cldt_middle = X(mkplan_d)(plnr,
			       XM(mkproblem_transpose)(
				    r, m, p->vn * 2,
				    I, O,
				    rblock[IB], mblock[OB],
				    p->comm,
				    TRANSPOSED_IN | TRANSPOSED_OUT));
     if (XM(any_true)(!cldt_middle, p->comm)) goto nada;	  

     mb = XM(block)(m, mblock[OB], my_pe);

     if (!(p->flags & SCRAMBLED_OUT)) { /* transpose m x r -> r x m */
	  cldt_after = X(mkplan_d)(plnr,
				    XM(mkproblem_transpose)(
					 m, r, p->vn * 2,
					 NO_DESTROY_INPUTP(plnr) ? p->O : p->I,
					 p->O,
					 mblock[OB], rblock[OB],
					 p->comm,
					 TRANSPOSED_IN));
	  if (XM(any_true)(!cldt_after, p->comm)) goto nada;	  
	  if (!cldt_before) { ri = ro; ii = io; }
     }
     else if (cldt_before) { ri = ro; ii = io; }

     cldr = X(mkplan_d)(plnr,
			X(mkproblem_dft_d)(X(mktensor_1d)(r, mb*p->vn*2,
							  mb*p->vn*2),
					   X(mktensor_1d)(mb * p->vn, 2, 2),
					   ro, io, ri, ii));
     if (XM(any_true)(!cldr, p->comm)) goto nada;	  

     pln = MKPLAN_MPI_DFT(P, &padt, apply);

     pln->cldt_before = cldt_before;
     pln->cldt_middle = cldt_middle;
     pln->cldt_after = cldt_after;
     pln->cldm = cldm;
     pln->cldr = cldr;
     pln->preserve_input = NO_DESTROY_INPUTP(plnr);
     pln->roff = ro - p->O;
     pln->ioff = io - p->O;
     pln->vn = p->vn;
     pln->m = m;
     pln->r = r;
     pln->rmin = rblock[IB] * my_pe;
     pln->rmax = pln->rmin + rb - 1;
     pln->t = 0;

     pln->super.super.ops = cldm->ops;
     {
          double n0 = (1 + pln->rmax - pln->rmin) * (pln->m - 1) * pln->vn;
          pln->super.super.ops.mul += 8 * n0;
          pln->super.super.ops.add += 4 * n0;
          pln->super.super.ops.other += 8 * n0;
     }
     X(ops_add2)(&cldr->ops, &pln->super.super.ops);
     X(ops_add2)(&cldt_middle->ops, &pln->super.super.ops);
     if (cldt_before)
	  X(ops_add2)(&cldt_before->ops, &pln->super.super.ops);
     if (cldt_after)
	  X(ops_add2)(&cldt_after->ops, &pln->super.super.ops);

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cldt_after);
     X(plan_destroy_internal)(cldr);
     X(plan_destroy_internal)(cldt_middle);
     X(plan_destroy_internal)(cldm);
     X(plan_destroy_internal)(cldt_before);
     return (plan *) 0;
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_MPI_DFT, mkplan };
     return MKSOLVER(solver, &sadt);
}

void XM(dft_rank1_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
