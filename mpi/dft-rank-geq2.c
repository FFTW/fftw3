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

#include "mpi-dft.h"
#include "mpi-transpose.h"
#include "dft.h"

typedef struct {
     plan_mpi_dft super;

     plan *cld1, *cldt1, *cld2, *cldt2;
     INT roff, ioff;
     int preserve_input;
} P;

static void apply(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld1, *cld2;
     plan_rdft *cldt1, *cldt2;
     INT roff = ego->roff, ioff = ego->ioff;
     
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
     else /* TRANSPOSED_OUT format */
	  cld2->apply(ego->cld2, I+roff, I+ioff, O+roff, O+ioff);
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     const problem_mpi_dft *p = (const problem_mpi_dft *) p_;
     UNUSED(ego_);
     UNUSED(plnr);
     return (1
	     && p->sz->rnk > 1
	     && p->flags == 0 /* TRANSPOSED/SCRAMBLED_IN/OUT not supported */
	     && XM(is_block1d)(p->sz, IB) && XM(is_block1d)(p->sz, OB)
	     && (!NO_UGLYP(plnr) /* ugly if dft-serial is applicable */
		 || !XM(dft_serial_applicable)(p))
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
     p->print(p, "(mpi-dft-rank-geq2");
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
     R *ri, *ii, *ro, *io, *I, *O;
     tensor *sz;
     int i, my_pe, n_pes, tflags;
     INT b, block, tblock, fblock, nx, ny, nl;
     int transposed_in, transposed_out;
     static const plan_adt padt = {
          XM(dft_solve), awake, print, destroy
     };

     UNUSED(ego);

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_mpi_dft *) p_;

     X(extract_reim)(p->sign, I = p->I, &ri, &ii);
     X(extract_reim)(p->sign, O = p->O, &ro, &io);
     MPI_Comm_rank(p->comm, &my_pe);
     MPI_Comm_size(p->comm, &n_pes);

     sz = X(mktensor)(p->sz->rnk - 1); /* tensor of last rnk-1 dimensions */
     i = p->sz->rnk - 2; A(i >= 0);
     sz->dims[i].n = p->sz->dims[i+1].n;
     sz->dims[i].is = sz->dims[i].os = 2 * p->vn;
     for (--i; i >= 0; --i) {
	  sz->dims[i].n = p->sz->dims[i+1].n;
	  sz->dims[i].is = sz->dims[i].os = sz->dims[i+1].n * sz->dims[i+1].is;
     }
     nl = sz->dims[0].is / 2; /* product of dimensions after 2nd */
     if ((transposed_in =
	  (XM(num_blocks)(p->sz->dims[0].n, p->sz->dims[0].b[IB]) == 1))) {
	  /* TRANSPOSED_IN: 0th dim is local, 1st is distributed */
	  INT s = sz->dims[0].is;
	  b = XM(block)(nx = p->sz->dims[1].n, 
			block = p->sz->dims[1].b[IB], my_pe);
	  sz->dims[0].n = ny = p->sz->dims[0].n;
	  sz->dims[0].is = sz->dims[0].os = s * b;
	  cld1 = X(mkplan_d)(plnr,
			     X(mkproblem_dft_d)(sz,
						X(mktensor_2d)(b, s, s,
							       p->vn, 2, 2),
						ri, ii, ro, io));
	  tblock = p->sz->dims[0].b[OB];
	  fblock = p->sz->dims[1].b[OB];
     }
     else {
          INT os = sz->dims[0].is;
          INT is = sz->dims[0].n * os;
	  b = XM(block)(nx = p->sz->dims[0].n, 
			block = p->sz->dims[0].b[IB], my_pe);
	  ny = p->sz->dims[1].n;
          sz->dims[0].os = os * b; /* transpose first dimension with b */
          cld1 = X(mkplan_d)(plnr,
                             X(mkproblem_dft_d)(sz,
                                                X(mktensor_2d)(b, is, os,
                                                               p->vn, 2, 2),
                                                ri, ii, ro, io));
	  tblock = p->sz->dims[1].b[OB];
	  fblock = p->sz->dims[0].b[OB];
     }
     if (XM(any_true)(!cld1, p->comm)) goto nada;

     if (NO_DESTROY_INPUTP(plnr)) { ri = ro; ii = io; I = O; }

     tflags = TRANSPOSED_IN;
     if (!transposed_in || tblock == ny)
	  tflags = tflags | TRANSPOSED_OUT;
     if (!(transposed_out = tblock < ny))
	  tblock = XM(default_block)(ny, n_pes);

     cldt1 = X(mkplan_d)(plnr,
			 XM(mkproblem_transpose)(nx, ny, nl*2,
						 O, I,
						 block, tblock,
						 p->comm, tflags));
     if (XM(any_true)(!cldt1, p->comm)) goto nada;

     b = XM(block)(ny, tblock, my_pe);
     if (tflags & TRANSPOSED_OUT) { /* last dimension transposed after cldt1 */
          INT s = b * nl * 2;  /* we have nx x b x nl complex array */
          R *sro, *sio;
          if (transposed_out) { sro = ro; sio = io; }
          else { sro = ri; sio = ii; }
          cld2 = X(mkplan_d)(plnr,
                             X(mkproblem_dft_d)(X(mktensor_1d)(nx, s, s),
                                                X(mktensor_1d)(b * nl, 2, 2),
                                                ri, ii, sro, sio));
     }
     else { /* !TRANSPOSED_OUT (=> TRANSPOSED_IN && TRANSPOSED_OUT) */
	  INT s = nx * nl * 2; /* we have b x nx x nl complex array */
	  cld2 = X(mkplan_d)(plnr,
			     X(mkproblem_dft_d)(X(mktensor_1d)(nx, nl*2, nl*2),
						X(mktensor_2d)(b, s, s,
							       nl, 2, 2),
						ri, ii, ro, io));
     }
     if (XM(any_true)(!cld2, p->comm)) goto nada;

     if (!transposed_out) {
	  tflags = TRANSPOSED_IN;
	  if (transposed_in)
	       tflags = tflags | TRANSPOSED_OUT;
	  cldt2 = X(mkplan_d)(plnr,
			      XM(mkproblem_transpose)(ny, nx, nl*2,
						      I, O,
						      tblock, fblock,
						      p->comm, tflags));
	  if (XM(any_true)(!cldt2, p->comm)) goto nada;
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
     if (cld1) X(ops_add2)(&cld1->ops, &pln->super.super.ops);
     if (cld2) X(ops_add2)(&cld2->ops, &pln->super.super.ops);
     if (cldt1) X(ops_add2)(&cldt1->ops, &pln->super.super.ops);
     if (cldt2) X(ops_add2)(&cldt2->ops, &pln->super.super.ops);

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

void XM(dft_rank_geq2_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
