/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* $Id: ctsq.c,v 1.2 2003-05-17 11:01:42 athena Exp $ */

/* special ``square transpose'' in-place cooley-tukey solver */
/* FIXME: do out-of-place too? */
#include "dft.h"

typedef struct {
     solver super;
     int dec;
} S;

typedef struct {
     plan_dft super;
     plan *cld;
     plan *cldw;
     const S *slv;
     int r;
} P;

#if 0
static void apply_dit(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;
     plan_dftw *cldw;

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, ri, ii, ro, io);

     cldw = (plan_dftw *) ego->cldw;
     cldw->apply(ego->cldw, ro, io);
}
#endif

static void apply_dif(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;
     plan_dftw *cldw;

     cldw = (plan_dftw *) ego->cldw;
     cldw->apply(ego->cldw, ri, ii);

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, ri, ii, ro, io);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld, flg);
     AWAKE(ego->cldw, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cldw);
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dft-ctsq-%s/%d%(%p%)%(%p%))",
	      ego->slv->dec == DECDIT ? "dit" : "dif",
	      ego->r, ego->cldw, ego->cld);
}

#define divides(a, b) (((int)(b) % (int)(a)) == 0)
static int applicable(const S *ego, const problem *p_, planner *plnr)
{
     UNUSED(ego); UNUSED(plnr);
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          iodim *d = p->sz->dims, *vd = p->vecsz->dims;

	  /* 
	     FIXME: these conditions are too strong.  In principle,
	     the dftw problem can always be transposed.  In practice,
	     we currently have no way to deal with general transpositions. 
	  */
	     
          return (1

                  && p->ri == p->ro  /* inplace only */
                  && p->sz->rnk == 1
                  && p->vecsz->rnk == 1

		  /* radix == vd[0].n */
		  && d[0].n > vd[0].n
                  && divides(vd[0].n, d[0].n)

		  /* strides must be transposed */
                  && d[0].os == vd[0].is
                  && d[0].is == vd[0].n * vd[0].is
                  && vd[0].os == d[0].n * vd[0].is
	       );
     }
     return 0;
}


static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p;
     P *pln = 0;
     plan *cld = 0, *cldw = 0;
     iodim *d, *vd;
     int n, r, m;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_dft *) p_;
     d = p->sz->dims;
     vd = p->vecsz->dims;
     n = d[0].n;
     r = vd[0].n;
     m = n / r;


     switch (ego->dec) {
	 case DECDIT:
	 {
	      A(0); /* not implemented */
	 }

	 case DECDIF:
	 {
	      cldw = X(mkplan_d)(
		   plnr, 
		   X(mkproblem_dftw)(DECDIF, r, m, d[0].is, d[0].os, 
				     r, vd[0].is, vd[0].os,
				     p->ri, p->ii) );
	      if (!cldw) goto nada;

	      cld = X(mkplan_d)(
		   plnr, 
		   X(mkproblem_dft_d)(
			X(mktensor_1d)(m, d[0].is, d[0].is),
			X(mktensor_2d)(r, vd[0].is, vd[0].is,
				       r, vd[0].os, vd[0].os),
			p->ro, p->io, p->ro, p->io));
	      if (!cld) goto nada;
	      
	      pln = MKPLAN_DFT(P, &padt, apply_dif);
	      break;
	 }

	 default: A(0);
	      
     }

     pln->cld = cld;
     pln->cldw = cldw;
     pln->slv = ego;
     pln->r = r;
     X(ops_add)(&cld->ops, &cldw->ops, &pln->super.super.ops);
     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cldw);
     X(plan_destroy_internal)(cld);
     return (plan *) 0;
}

static solver *mksolver(int dec)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->dec = dec;
     return &(slv->super);
}

void X(dft_ctsq_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver(DECDIF));
}
