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

#include "rdft.h"

typedef struct {
     solver super;
     rdft_kind kind;
} S;

typedef struct {
     plan_rdft super;
     plan *cld;
     twid *td;
     int os;
     int r, m;
     rdft_kind kind;
} P;

/***************************************************************************/

static void apply_dit(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int n, m, r;
     int i, j, k;
     int os, osm;
     E *buf;
     const R *W;
     R *X, *YO, *YI;
     E rsum, isum;
     int wp, wincr;

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, I, O);
     }

     r = ego->r;

     STACK_MALLOC(E *, buf, r * 2 * sizeof(E));
     
     osm = (m = ego->m) * (os = ego->os);
     n = m * r;
     W = ego->td->W;

     X = O;
     YO = O + r * osm;
     YI = O + osm;

     /* compute the transform of the r 0th elements (which are real) */
     for (i = 0; i + i < r; ++i) {
	  rsum = K(0.0);
	  isum = K(0.0);
	  wincr = m * i;
	  for (j = 0, wp = 0; j < r; ++j) {
	       E tw_r = W[2*wp];
	       E tw_i = W[2*wp+1] ;
	       E re = X[j * osm];
	       rsum += re * tw_r;
	       isum += re * tw_i;
	       wp += wincr;
	       if (wp >= n)
		    wp -= n;
	  }
	  buf[2*i] = rsum;
	  buf[2*i+1] = isum;
     }

     /* store the transform back onto the A array */
     X[0] = buf[0];
     for (i = 1; i + i < r; ++i) {
	  X[i * osm] = buf[2*i];
	  YO[-i * osm] = buf[2*i+1];
     }

     X += os;
     YI -= os;
     YO -= os;

     /* compute the transform of the middle elements (which are complex) */
     for (k = 1; k + k < m; ++k, X += os, YI -= os, YO -= os) {
	  for (i = 0; i < r; ++i) {
	       rsum = K(0.0);
	       isum = K(0.0);
	       wincr = k + m * i;
	       for (j = 0, wp = 0; j < r; ++j) {
		    E tw_r = W[2*wp];
		    E tw_i = W[2*wp+1] ;
		    E re = X[j * osm];
		    E im = YI[j * osm];
		    rsum += re * tw_r - im * tw_i;
		    isum += re * tw_i + im * tw_r;
		    wp += wincr;
		    if (wp >= n)
			 wp -= n;
	       }
	       buf[2*i] = rsum;
	       buf[2*i+1] = isum;
	  }

	  /* store the transform back onto the A array */
	  for (i = 0; i + i < r; ++i) {
	       X[i * osm] = buf[2*i];
	       YO[-i * osm] = buf[2*i+1];
	  }
	  for (; i < r; ++i) {
	       X[i * osm] = -buf[2*i+1];
	       YO[-i * osm] = buf[2*i];
	  }
     }

     /* no final element, since m is odd */

     STACK_FREE(buf);
}

static void apply_dif(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int n, m, r;
     int i, j, k;
     int is, ism;
     E *buf;
     const R *W;
     R *X, *YO, *YI;
     E rsum, isum;
     int wp, wincr;

     r = ego->r;

     STACK_MALLOC(E *, buf, r * 2 * sizeof(E));
     
     ism = (m = ego->m) * (is = ego->os);
     n = m * r;
     W = ego->td->W;

     X = I;
     YI = I + r * ism;
     YO = I + ism;

     /* 
      * compute the transform of the r 0th elements (which are halfcomplex)
      * yielding real numbers
      */
     /* copy the input into the temporary array */
     buf[0] = X[0];
     for (i = 1; i + i < r; ++i) {
	  buf[2*i] = X[i * ism];
	  buf[2*i+1] = YI[-i * ism];
     }

     for (i = 0; i < r; ++i) {
	  rsum = K(0.0);
	  wincr = m * i;
	  for (j = 1, wp = wincr; j + j < r; ++j) {
	       E tw_r = W[2*wp];
	       E tw_i = W[2*wp+1];
	       E re = buf[2*j];
	       E im = buf[2*j+1];
	       rsum += re * tw_r + im * tw_i;
	       wp += wincr;
	       if (wp >= n)
		    wp -= n;
	  }
	  X[i * ism] = K(2.0) * rsum + buf[0];
     }

     X += is;
     YI -= is;
     YO -= is;

     /* compute the transform of the middle elements (which are complex) */
     for (k = 1; k + k < m; ++k, X += is, YI -= is, YO -= is) {
	  /* copy the input into the temporary array */
	  for (i = 0; i + i < r; ++i) {
	       buf[2*i] = X[i * ism];
	       buf[2*i+1] = YI[-i * ism];
	  }
	  for (; i < r; ++i) {
	       buf[2*i+1] = -X[i * ism];
	       buf[2*i] = YI[-i * ism];
	  }

	  for (i = 0; i < r; ++i) {
	       rsum = K(0.0);
	       isum = K(0.0);
	       wincr = m * i;
	       for (j = 0, wp = k * i; j < r; ++j) {
		    E tw_r = W[2*wp];
		    E tw_i = W[2*wp+1];
		    E re = buf[2*j];
		    E im = buf[2*j+1];
		    rsum += re * tw_r + im * tw_i;
		    isum += im * tw_r - re * tw_i;
		    wp += wincr;
		    if (wp >= n)
			 wp -= n;
	       }
	       X[i * ism] = rsum;
	       YO[i * ism] = isum;
	  }
     }

     /* no final element, since m is odd */

     STACK_FREE(buf);

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, I, O);
     }

}

/***************************************************************************/

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     static const tw_instr generic_tw[] = {
	  { TW_GENERIC, 0, 0 },
	  { TW_NEXT, 1, 0 }
     };

     AWAKE(ego->cld, flg);
     /* FIXME: can we get away with fewer twiddles? */
     X(twiddle_awake)(flg, &ego->td, generic_tw,
		      ego->r * ego->m, ego->r, ego->m);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;

     p->print(p, "(rdft-generic-%s-%d%(%p%))", 
	      ego->kind == R2HC ? "r2hc-dit" : "hc2r-dif",
	      ego->r, ego->cld);
}

static int applicable0(const solver *ego_, const problem *p_)
{
     if (RDFTP(p_)) {
	  const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz->rnk == 1
		  && p->vecsz->rnk == 0
		  && p->sz->dims[0].n > 1
                  && p->sz->dims[0].n % 2 /* ensure r and n/r odd */
                  && p->kind[0] == ego->kind
	       );
     }

     return 0;
}

static int applicable(const solver *ego_, const problem *p_, 
		      const planner *plnr)
{
     if (NO_UGLYP(plnr)) return 0; /* always ugly */
     if (!applicable0(ego_, p_)) return 0;

     if (NO_LARGE_GENERICP(plnr)) {
          const problem_rdft *p = (const problem_rdft *) p_;
	  if (X(first_divisor)(p->sz->dims[0].n) >= GENERIC_MIN_BAD) return 0; 
     }
     return 1;
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_rdft *p = (const problem_rdft *) p_;
     P *pln = 0;
     int n, r, m;
     int is, os;
     plan *cld = (plan *) 0;
     problem *cldp;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_, plnr))
          goto nada;

     n = p->sz->dims[0].n;
     is = p->sz->dims[0].is;
     os = p->sz->dims[0].os;

     r = X(first_divisor)(n);
     m = n / r;

     if (R2HC_KINDP(p->kind[0])) {
	  cldp = X(mkproblem_rdft_d)(X(mktensor_1d)(m, r * is, os),
				     X(mktensor_1d)(r, is, m * os),
				     p->I, p->O, p->kind);
     }
     else {
	  cldp = X(mkproblem_rdft_d)(X(mktensor_1d)(m, is, r * os),
				     X(mktensor_1d)(r, m * is, os),
				     p->I, p->O, p->kind);
     }
     if (!(cld = X(mkplan_d)(plnr, cldp))) goto nada;

     pln = MKPLAN_RDFT(P, &padt, R2HC_KINDP(p->kind[0]) ? apply_dit:apply_dif);

     pln->os = R2HC_KINDP(p->kind[0]) ? os : is;
     pln->r = r;
     pln->m = m;
     pln->cld = cld;
     pln->td = 0;
     pln->kind = p->kind[0];

     X(ops_zero)(&pln->super.super.ops);
     pln->super.super.ops.add = 4 * r * r;
     pln->super.super.ops.mul = 4 * r * r;
     /* loads + stores, minus loads + stores for all DIT codelets */
     pln->super.super.ops.other = 4 * r + 4 * r * r - (6*r - 2);
     X(ops_madd)((m - 1)/2, &pln->super.super.ops, &cld->ops,
		 &pln->super.super.ops);
     pln->super.super.ops.add += 2 * r * r;
     pln->super.super.ops.mul += 2 * r * r;
     pln->super.super.ops.other += 3 * r + 3 * r * r - 2*r;

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld);
     X(ifree0)(pln);
     return (plan *) 0;
}

/* constructors */

static solver *mksolver(rdft_kind kind)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->kind = kind;
     return &(slv->super);
}

void X(rdft_generic_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver(R2HC));
     REGISTER_SOLVER(p, mksolver(HC2R));
}
