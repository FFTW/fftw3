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

/* $Id: ct.c,v 1.35 2003-05-16 17:34:16 athena Exp $ */

#include "dft.h"

typedef struct {
     solver super;
     int r;
     int dec;
     const int *buddies;
} S;

typedef struct {
     plan_dft super;
     plan *cld;
     plan *cldw;
     const S *slv;
     int r;
} P;

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
     p->print(p, "(dft-ct-%s/%d%(%p%)%(%p%))",
	      ego->slv->dec == DECDIT ? "dit" : "dif",
	      ego->r, ego->cldw, ego->cld);
}

#define divides(a, b) (((int)(b) % (int)(a)) == 0)
static int choose_radix(const S *ego, int n, int *rp)
{
     int r = ego->r;
     const int *p;
     if (r == 0) {
	  r = X(first_divisor)(n);
	  for (p = ego->buddies; *p; ++p)
	       if (*p == r) return 0; /* ignore if buddy exists with
					 the same radix */
     }
     *rp = r;
     return 1;
}

static int applicable0(const S *ego, const problem *p_, planner *plnr)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
	  int r;

          return (1
                  && p->sz->rnk == 1
                  && p->vecsz->rnk <= 1 

                  /* DIF destroys the input and we don't like it */
                  && (ego->dec == DECDIT || 
		      p->ri == p->ro || 
		      DESTROY_INPUTP(plnr))
		  
		  && choose_radix(ego, p->sz->dims[0].n, &r)
		  && p->sz->dims[0].n > r
                  && divides(r, p->sz->dims[0].n));
     }
     return 0;
}


static int applicable(const S *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p;

     if (!applicable0(ego, p_, plnr))
          return 0;

     p = (const problem_dft *) p_;

     /* emulate fftw2 behavior */
     if (NO_VRECURSEP(plnr) && (p->vecsz->rnk > 0))  return 0;

     return 1;
}


static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p;
     P *pln = 0;
     plan *cld = 0, *cldw = 0;
     int n, r, m, vl, ivs, ovs;
     iodim *d;
     tensor *t1, *t2;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_dft *) p_;
     d = p->sz->dims;
     n = d[0].n;
     choose_radix(ego, n, &r);
     m = n / r;

     X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs);

     switch (ego->dec) {
	 case DECDIT:
	 {
	      cldw = X(mkplan_d)(plnr, 
				 X(mkproblem_dftw)(DECDIT, r, m, 
						   d[0].os, d[0].os, 
						   vl, ovs, ovs,
						   p->ro, p->io) );
	      if (!cldw) goto nada;

	      t1 = X(mktensor_1d)(r, d[0].is, m * d[0].os);
	      t2 = X(tensor_append)(t1, p->vecsz);
	      X(tensor_destroy)(t1);

	      cld = X(mkplan_d)(plnr, 
				X(mkproblem_dft_d)(
				     X(mktensor_1d)(m, r * d[0].is, d[0].os),
				     t2, p->ri, p->ii, p->ro, p->io)
		   );
	      if (!cld) goto nada;

	      pln = MKPLAN_DFT(P, &padt, apply_dit);
	      break;
	 }
	 case DECDIF:
	 {
	      cldw = X(mkplan_d)(plnr, 
				 X(mkproblem_dftw)(DECDIF, r, m, 
						   d[0].is, d[0].is, 
						   vl, ivs, ivs, 
						   p->ri, p->ii) );
	      if (!cldw) goto nada;

	      t1 = X(mktensor_1d)(r, m * d[0].is, d[0].os);
	      t2 = X(tensor_append)(t1, p->vecsz);
	      X(tensor_destroy)(t1);

	      cld = X(mkplan_d)(plnr, 
				X(mkproblem_dft_d)(
				     X(mktensor_1d)(m, d[0].is, r * d[0].os),
				     t2, p->ri, p->ii, p->ro, p->io)
		   );
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

static solver *mksolver(int r, const int *buddies, int dec)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->r = r;
     slv->dec = dec;
     slv->buddies = buddies;
     return &(slv->super);
}

void X(dft_ct_register)(planner *p)
{
     int *q;
     static const int r[] = {
	  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 
	  0
     };

     /* r = 0 : choose smallest prime factor */
     REGISTER_SOLVER(p, mksolver(0, r, DECDIT));
     REGISTER_SOLVER(p, mksolver(0, r, DECDIF));

     for (q = r; *q; ++q) {
          REGISTER_SOLVER(p, mksolver(*q, r, DECDIT));
          REGISTER_SOLVER(p, mksolver(*q, r, DECDIF));
     }
}

