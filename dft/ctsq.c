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

/* $Id: ctsq.c,v 1.12 2006-01-27 02:10:50 athena Exp $ */

/* special ``square transpose'' cooley-tukey solver for in-place problems */
#include "ct.h"

typedef struct {
     solver super;
     const ct_desc *desc;
     kdftwsq k;
     int dec;
} S;

typedef struct {
     plan_dft super;
     kdftwsq k;
     twid *td;
     plan *cld;
     stride ios, vs;
     INT m, r, dist;
     const S *slv;
} P;

static void apply_dif(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;

     UNUSED(ro);  /* == ri */
     UNUSED(io);  /* == ii */
     ego->k(ri, ii, ego->td->W, ego->ios, ego->vs, ego->m, ego->dist);

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, ri, ii, ri, ii);
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cld, wakefulness);
     X(twiddle_awake)(wakefulness, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, ego->m);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
     X(stride_destroy)(ego->ios);
     X(stride_destroy)(ego->vs);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const ct_desc *e = slv->desc;

     p->print(p, "(dft-ctsq-%s/%D \"%s\"%(%p%))",
	      slv->dec == DECDIT ? "dit" : "dif",
	      ego->r, e->nam, ego->cld);
}

#define divides(a, b) (((b) % (a)) == 0)
static int applicable(const S *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     const ct_desc *e = ego->desc;
     iodim *d = p->sz->dims, *vd = p->vecsz->dims;

     return (1
	     && p->ri == p->ro  /* inplace only */
	     && p->sz->rnk == 1
	     && p->vecsz->rnk == 1

	     && d[0].n > e->radix
	     && divides(e->radix, d[0].n)

	     /* conditions for transposition */
	     && vd[0].n == e->radix
	     && d[0].os == vd[0].is
	     && d[0].is == e->radix * vd[0].is
	     && vd[0].os == d[0].n * vd[0].is

	     /* SIMD strides etc. */
	     && (e->genus->okp(e, p->ri, p->ii, vd[0].os, vd[0].is, 
			       d[0].n / e->radix, 0, d[0].n / e->radix,
			       d[0].is, plnr))
	  );
}


static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p;
     P *pln = 0;
     plan *cld = 0;
     iodim *d, *vd;
     const ct_desc *e = ego->desc;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_dft *) p_;
     d = p->sz->dims;
     vd = p->vecsz->dims;

     switch (ego->dec) {
	 case DECDIT:
	 {
	      A(0); /* not implemented */
	 }
	 case DECDIF:
	 {
	      cld = X(mkplan_d)(plnr, 
				X(mkproblem_dft_d)(
				     X(mktensor_1d)(d[0].n / e->radix, 
						    d[0].is, d[0].is),
				     X(mktensor_2d)(vd[0].n, 
						    vd[0].os, vd[0].os,
						    e->radix, 
						    vd[0].is,vd[0].is),
				     p->ro, p->io, p->ro, p->io));
	      if (!cld) goto nada;
	      
	      pln = MKPLAN_DFT(P, &padt, apply_dif);
	      pln->ios = X(mkstride)(e->radix, vd[0].os);
	      pln->vs = X(mkstride)(e->radix, vd[0].is);
	      pln->dist = d[0].is;
	      break;
	 }

	 default: A(0);
     }

     pln->cld = cld;
     pln->slv = ego;
     pln->r = e->radix;
     pln->m = d[0].n / pln->r;
     pln->td = 0;
     pln->k = ego->k;

     X(ops_madd)(pln->m / e->genus->vl, &e->ops, &cld->ops,
		 &pln->super.super.ops);
     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld);
     return (plan *) 0;
}

solver *X(mksolver_ctsq)(kdftwsq codelet, const ct_desc *desc, int dec)
{
     static const solver_adt sadt = { PROBLEM_DFT, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->k = codelet;
     slv->desc = desc;
     slv->dec = dec;
     return &(slv->super);
}

