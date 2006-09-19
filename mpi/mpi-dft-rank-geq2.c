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

/* Complex DFTs of rank >= 2 */

#include "mpi_dft.h"
#include "mpi_transpose.h"
#include "dft.h"

/* from api/extract-reim.c */
extern void X(extract_reim)(int sign, R *c, R **r, R **i);

typedef struct {
     plan_mpi_dft super;

     plan *cld1, *cldt1, *cld2, *cldt2;
     ptrdiff_t roff, ioff;
     int preserve_input;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld1, *cld2;
     plan_rdft *cldt1, *cldt2;
     ptrdiff_t roff = ego->roff, ioff = ego->ioff;
     
     /* DFT local dimensions */
     cld1 = (plan_dft *) ego->cld1;
     cld1->apply(ego->cld1, I+roff, I+ioff, O+roff, O+ioff);

     if (ego->preserve_input) I = O;

     /* global transpose */
     cldt1 = (plan_rdft *) ego->cldt1;
     cldt1->apply(ego->cldt1, O, I);

     /* DFT new local dimension of transposed data (cld2)
	and do global transpose back, if desired */
     cld2 = (plan_dft *) ego->cld2;
     cldt2 = (plan_rdft *) ego->cldt2;
     if (cldt2) {
	  cld2->apply(ego->cld2, I+roff, I+ioff, I+roff, I+ioff);
	  cldt2->apply(ego->cldt2, I, O);
     }
     else /* TRANSPOSED format */
	  cld2->apply(ego->cld2, I+roff, I+ioff, O+roff, O+ioff);
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const problem_mpi_dft *p = (const problem_mpi_dft *) p_;
     UNUSED(ego_);
     UNUSED(plnr);
     return (1
	     && p->rnk > 1
	  );
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cld1, wakefulness);
     X(plan_awake)(ego->cld2, wakefulness);
     X(plan_awake)(ego->cldt1, wakefulness);
     X(plan_awake)(ego->cldt2, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cldt2);
     X(plan_destroy_internal)(ego->cldt1);
     X(plan_destroy_internal)(ego->cld2);
     X(plan_destroy_internal)(ego->cld1);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(mpi_dft-rank-geq2");
     if (ego->cld1) p->print(p, "%(%p%)", ego->cld1);
     if (ego->cldt1) p->print(p, "%(%p%)", ego->cldt1);
     if (ego->cld2) p->print(p, "%(%p%)", ego->cld2);
     if (ego->cldt2) p->print(p, "%(%p%)", ego->cldt2);
     p->print(p, ")");
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_mpi_dft *p;
     P *pln;
     plan *cld1 = 0, *cld2 = 0, *cldt1 = 0, *cldt2 = 0;
     R *ri, *ii, *ro, *io;
     tensor *sz;
     int i;
     INT b, nl;
     int tflags;
     static const plan_adt padt = {
          X(mpi_dft_solve), awake, print, destroy
     };

     UNUSED(ego);

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_mpi_dft *) p_;

     X(extract_reim)(p->sign, p->I, &ri, &ii);
     X(extract_reim)(p->sign, p->O, &ro, &io);

     sz = X(mktensor)(p->rnk - 1); /* tensor of last rnk-1 dimensions */
     i = p->rnk - 2; A(i >= 0);
     sz->dims[i].n = p->n[i + 1];
     sz->dims[i].is = sz->dims[i].os = 2 * p->vn;
     for (--i; i >= 0; --i) {
	  sz->dims[i].n = p->n[i + 1];
	  sz->dims[i].is = sz->dims[i].os = 
	       sz->dims[i + 1].n * sz->dims[i + 1].is;
     }
     nl = sz->dims[0].is / 2; /* product of dimensions after 2nd */
     b = X(current_block)(p->n[0], p->block, p->comm);     
     if (p->flags & SCRAMBLED_IN) { /* first dimension already transposed */
	  INT s = sz->dims[0].is;
	  sz->dims[0].is = sz->dims[0].os = s * b;
	  cld1 = X(mkplan_d)(plnr,
			     X(mkproblem_dft_d)(sz,
						X(mktensor_2d)(b, s, s,
							       p->vn, 2, 2),
						ri, ii, ro, io));
     }
     else {
	  INT os = sz->dims[0].is;
	  INT is = sz->dims[0].n * os;
	  sz->dims[0].os = os * b; /* transpose first dimension with b */
	  cld1 = X(mkplan_d)(plnr,
			     X(mkproblem_dft_d)(sz,
						X(mktensor_2d)(b, is, os,
							       p->vn, 2, 2),
						ri, ii, ro, io));
     }
     if (!cld1) goto nada;

     if (NO_DESTROY_INPUTP(plnr)) { ri = ro; ii = io; }

     tflags = (p->flags & ~TRANSPOSED) | SCRAMBLED_IN;
     if (!(p->flags & TRANSPOSED))
	  tflags = tflags | SCRAMBLED_OUT;
     cldt1 = X(mkplan_d)(plnr,
			 X(mkproblem_mpi_transpose)(nl*2, p->n[0],p->n[1],
						    ro, ri,
						    p->block, p->tblock,
						    p->comm,
						    tflags));
     if (!cldt1) goto nada;

     b = X(current_block)(p->n[1], p->tblock, p->comm);     
     if (tflags & SCRAMBLED_OUT) { /* last dimension transposed after cldt1 */
	  INT s = b * nl * 2;  /* we have n[0] x b x nl complex array */
	  R *sro, *sio;
	  if (p->flags & TRANSPOSED) { sro = ro; sio = io; }
	  else { sro = ri; sio = ii; }
	  cld2 = X(mkplan_d)(plnr,
			     X(mkproblem_dft_d)(X(mktensor_1d)(p->n[0], s, s),
						X(mktensor_1d)(b * nl, 2, 2),
						ri, ii, sro, sio));
     }
     else { /* !TRANSPOSED and !SCRAMBLED_OUT */
	  INT s = p->n[0] * nl * 2; /* we have b x n[0] x nl complex array */
	  cld2 = X(mkplan_d)(plnr,
			     X(mkproblem_dft_d)(X(mktensor_1d)(p->n[0], 
							       nl*2, nl*2),
						X(mktensor_2d)(b, s, s,
							       nl, 2, 2),
						ri, ii, ro, io));
     }
     if (!cld2) goto nada;

     if (!(p->flags & TRANSPOSED)) {
	  tflags = p->flags | SCRAMBLED_IN;
	  cldt2 = X(mkplan_d)(plnr,
			      X(mkproblem_mpi_transpose)(nl*2, p->n[1],p->n[0],
							 ri, ro,
							 p->tblock, p->block,
							 p->comm,
							 tflags));
	  if (!cldt2) goto nada;
     }

     pln = MKPLAN_MPI_DFT(P, &padt, apply);

     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->cldt1 = cldt1;
     pln->cldt2 = cldt2;
     pln->preserve_input = NO_DESTROY_INPUTP(plnr);
     pln->roff = ro - p->O;
     pln->ioff = io - p->O;

     X(ops_zero)(&pln->super.super.ops);
     if (cld1)
	  X(ops_add)(&pln->super.super.ops, &cld1->ops, &pln->super.super.ops);
     if (cld2)
	  X(ops_add)(&pln->super.super.ops, &cld2->ops, &pln->super.super.ops);
     if (cldt1)
	  X(ops_add)(&pln->super.super.ops, &cldt1->ops, &pln->super.super.ops);
     if (cldt2)
	  X(ops_add)(&pln->super.super.ops, &cldt2->ops, &pln->super.super.ops);

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cldt2);
     X(plan_destroy_internal)(cldt1);
     X(plan_destroy_internal)(cld2);
     X(plan_destroy_internal)(cld1);
     return (plan *) 0;
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_MPI_DFT, mkplan };
     return MKSOLVER(solver, &sadt);
}

void X(mpi_dft_rank_geq2_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
