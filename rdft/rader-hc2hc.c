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
#include "dft.h"

/*
 * Compute transforms with large prime factors using Rader's trick:
 * turn the factors into convolutions of size n - 1, which you then
 * perform via a pair of FFTs.  This file contains only twiddle hc2hc
 * transforms, which are actually ordinary complex transforms in a
 * slightly funny order.
 */

typedef struct {
     solver super;
     rdft_kind kind;
} S;

typedef struct {
     plan_rdft super;

     plan *cldr, *cldr0;
     plan *cld;
     R *W;
     R *omega;
     int m, r, g, ginv;
     int os, ios;
     rdft_kind kind;
} P;

static rader_tl *twiddles = 0;

/***************************************************************************/

/* Below, we extensively use the identity that fft(x*)* = ifft(x) in
   order to share data between forward and backward transforms and to
   obviate the necessity of having separate forward and backward
   plans. */

static void apply_aux(int r, plan_dft *cldr, const R *omega,
		      R *buf, R *ro, R i0, R *io)
{
     R r0;
     int k;

     /* compute DFT of buf, operating in-place */
     cldr->apply((plan *) cldr, buf, buf+1, buf, buf+1);

     /* set output DC component: */
     ro[0] = (r0 = ro[0]) + buf[0];
     io[0] = i0 + buf[1];

     /* now, multiply by omega: */
     for (k = 0; k < r - 1; ++k) {
	  R rB, iB, rW, iW;
	  rW = omega[2*k];
	  iW = omega[2*k+1];
	  rB = buf[2*k];
	  iB = buf[2*k+1];
	  buf[2*k] = rW * rB - iW * iB;
	  buf[2*k+1] = -(rW * iB + iW * rB);
     }
     
     /* this will add input[0] to all of the outputs after the ifft */
     buf[0] += r0;
     buf[1] -= i0;

     /* inverse FFT: */
     cldr->apply((plan *) cldr, buf, buf+1, buf, buf+1);
}

static void apply_dit(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_dft *cldr;
     int os, ios;
     int j, k, gpower, g, ginv, r, m;
     R *buf, *rio, *ii, *io;
     const R *omega, *W;

     /* size-m child transforms: */
     {
	   plan_rdft *cld = (plan_rdft *) ego->cld;
	   cld->apply((plan *) cld, I, O);
     }

     /* 0th twiddle transform is just size-r (prime) R2HC: */
     {
	   plan_rdft *cldr0 = (plan_rdft *) ego->cldr0;
	   cldr0->apply((plan *) cldr0, O, O);
     }

     cldr = (plan_dft *) ego->cldr;
     r = ego->r;
     m = ego->m;
     g = ego->g; 
     ginv = ego->ginv;
     omega = ego->omega;
     W = ego->W;
     os = ego->os;
     ios = ego->ios;
     gpower = 1;
     rio = O + os;
     ii = O + (m - 1) * os;
     io = O + (r * m - 1) * os;

     buf = (R *) MALLOC(sizeof(R) * (r - 1) * 2, BUFFERS);

     for (j = 2; j < m; j += 2, rio += os, ii -= os, io -= os, W += 2*(r-1)) {
	  /* First, permute the input and multiply by W, storing in buf: */
	  A(gpower == 1);
	  for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	       R rA, iA, rW, iW;
	       rA = rio[gpower * ios];
	       iA = ii[gpower * ios];
	       rW = W[2*k];
	       iW = W[2*k+1];
	       buf[2*k] = rW * rA - iW * iA;
	       buf[2*k+1] = rW * iA + iW * rA;
	  }
	  /* gpower == g^(r-1) mod r == 1 */;
	  
	  apply_aux(r, cldr, omega, buf, rio, ii[0], io);

	  /* finally, do inverse permutation to unshuffle the output: */
	  A(gpower == 1);
	  for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, ginv, r)) {
	       rio[gpower * ios] = buf[2*k];
	       io[-gpower * ios] = -buf[2*k+1];
	  }
	  A(gpower == 1);

	  /* second half of array must be fiddled to get real/imag
             parts in correct spots: */
	  for (k = (r+1)/2; k < r; ++k) {
	       R t;
	       t = rio[k * ios];
	       rio[k * ios] = -io[-k * ios];
	       io[-k * ios] = t;
	  }
     }

     /* Avoid funny m/2-th iter by requiring m odd.  This always
	happens anyway because all the factors of 2 get divided out
	first by codelets (Rader is UGLY for small factors). */

     X(ifree)(buf);
}

static void apply_dif(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     plan_dft *cldr;
     int is, ios;
     int j, k, gpower, g, ginv, r, m;
     R *buf, *rio, *ii, *io;
     const R *omega, *W;

     /* 0th twiddle transform is just size-r (prime) HC2R: */
     {
	   plan_rdft *cldr0 = (plan_rdft *) ego->cldr0;
	   cldr0->apply((plan *) cldr0, I, I);
     }

     cldr = (plan_dft *) ego->cldr;
     r = ego->r;
     m = ego->m;
     g = ego->g; 
     ginv = ego->ginv;
     omega = ego->omega;
     W = ego->W + 2*(r-1); /* simplify reverse indexing of W */
     is = ego->os;
     ios = ego->ios;
     gpower = 1;
     rio = I + is;
     io = I + (m - 1) * is;
     ii = I + (r * m - 1) * is;

     buf = (R *) MALLOC(sizeof(R) * (r - 1) * 2, BUFFERS);

     for (j = 2; j < m; j += 2, rio += is, ii -= is, io -= is, W += 2*(r-1)) {
	  /* second half of array must be unfiddled to get real/imag
             parts from correct spots: */
	  for (k = (r+1)/2; k < r; ++k) {
	       R t;
	       t = rio[k * ios];
	       rio[k * ios] = ii[-k * ios];
	       ii[-k * ios] = -t;
	  }

	  /* First, permute the input, storing in buf: */
	  A(gpower == 1);
	  for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	       buf[2*k] = rio[gpower * ios];
	       buf[2*k+1] = -ii[-gpower * ios];
	  }
	  /* gpower == g^(r-1) mod r == 1 */;
	  A(gpower == 1);
	  
	  apply_aux(r, cldr, omega, buf, rio, -ii[0], io);
	  io[0] = -io[0];

	  /* finally, do inverse permutation to unshuffle the output,
             also multiplying by the inverse twiddle factors W*.
	     The twiddle factors are accessed in reverse order W[-k],
	     because here we exponentiating ginv and not g as in
	     mktwiddle. */
	  { /* W[-0] = W[0] case must be handled specially */
               R rA, iA, rW, iW;
               rA = buf[0]; iA = buf[1];
               rW = W[-2*(r-1)]; iW = W[-2*(r-1) + 1];
               rio[ios] = rA * rW + iA * iW;
               io[ios] = iA * rW - rA * iW;
	  }
	  gpower = ginv;
	  for (k = 1; k < r - 1; ++k, gpower = MULMOD(gpower, ginv, r)) {
	       R rA, iA, rW, iW;
	       rA = buf[2*k]; iA = buf[2*k+1];
	       rW = W[-2*k]; iW = W[-2*k+1];
	       rio[gpower * ios] = rA * rW + iA * iW;
	       io[gpower * ios] = iA * rW - rA * iW;
	  }
	  A(gpower == 1);
     }

     /* Avoid funny m/2-th iter by requiring m odd.  This always
	happens anyway because all the factors of 2 get divided out
	first by codelets (Rader is UGLY for small factors). */

     X(ifree)(buf);

     /* size-m child transforms: */
     {
	   plan_rdft *cld = (plan_rdft *) ego->cld;
	   cld->apply((plan *) cld, I, O);
     }
}

static R *mktwiddle(int m, int r, int g)
{
     int i, j, gpower;
     int n = r * m;
     R *W;

     if ((W = X(rader_tl_find)(m, r, g, twiddles)))
	  return W;

     W = (R *)MALLOC(sizeof(R) * (r - 1) * ((m-1)/2) * 2, TWIDDLES);
     for (i = 1; i < (m+1)/2; ++i) {
	  for (gpower = 1, j = 0; j < r - 1;
	       ++j, gpower = MULMOD(gpower, g, r)) {
	       int k = (i - 1) * (r - 1) + j;
	       W[2*k] = X(cos2pi)(i * gpower, n);
	       W[2*k+1] = FFT_SIGN * X(sin2pi)(i * gpower, n);
	  }
	  A(gpower == 1);
     }

     X(rader_tl_insert)(m, r, g, W, &twiddles);
     return W;
}

static void free_twiddle(R *twiddle)
{
     X(rader_tl_delete)(twiddle, &twiddles);
}

/***************************************************************************/

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cldr0, flg);
     AWAKE(ego->cldr, flg);
     AWAKE(ego->cld, flg);

     if (flg) {
	  if (!ego->omega) 
	       ego->omega = 
		    X(dft_rader_mkomega)(ego->cldr, ego->r, ego->ginv);
	  if (!ego->W)
	       ego->W = mktwiddle(ego->m, ego->r, ego->g);
     } else {
	  X(dft_rader_free_omega)(&ego->omega);
	  free_twiddle(ego->W);
	  ego->W = 0;
     }
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
     X(plan_destroy_internal)(ego->cldr);
     X(plan_destroy_internal)(ego->cldr0);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;

     p->print(p, "(rdft-rader-%s-%d%(%p%)%(%p%)%(%p%))",
	      ego->kind == R2HC ? "r2hc-dit" : "hc2r-dif",
              ego->r, ego->cldr0, ego->cldr, ego->cld);
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
		  && p->sz->dims[0].n % 4 /* make sure n / r = m is odd */
		  && p->kind[0] == ego->kind
		  && !X(is_prime)(p->sz->dims[0].n) /* avoid inf. loops planning cldr0 */
	       );
     }

     return 0;
}

static int applicable(const solver *ego_, const problem *p_, 
		      const planner *plnr)
{
     return (!NO_UGLYP(plnr) && applicable0(ego_, p_));
}

static int mkP(P *pln, int r, R *O, int ios, rdft_kind kind, planner *plnr)
{
     plan *cldr = (plan *) 0;
     plan *cldr0 = (plan *) 0;
     R *buf = (R *) 0;

     cldr0 = X(mkplan_d)(plnr, 
			 X(mkproblem_rdft_1_d)(X(mktensor_1d)(r, ios, ios),
					       X(mktensor_1d)(1, 0, 0),
					       O, O, kind));
     if (!cldr0) goto nada;

     /* initial allocation for the purpose of planning */
     buf = (R *) MALLOC(sizeof(R) * (r - 1) * 2, BUFFERS);

     cldr = X(mkplan_d)(plnr, X(mkproblem_dft_d)(X(mktensor_1d)(r - 1, 2, 2),
						 X(mktensor_1d)(1, 0, 0),
						 buf, buf + 1, buf, buf + 1));
     if (!cldr) goto nada;

     X(ifree)(buf);

     pln->cldr = cldr;
     pln->cldr0 = cldr0;
     pln->omega = 0;
     pln->r = r;
     pln->g = X(find_generator)(r);
     pln->ginv = X(power_mod)(pln->g, r - 2, r);
     pln->kind = kind;
     A(MULMOD(pln->g, pln->ginv, r) == 1);

     X(ops_add)(&cldr->ops, &cldr->ops, &pln->super.super.ops);
     pln->super.super.ops.other += (r - 1) * (4 * 2 + 6) + 6;
     pln->super.super.ops.add += 2 * (r - 1) * 2 + 4;
     pln->super.super.ops.mul += 2 * (r - 1) * 4;

     return 1;

 nada:
     X(ifree0)(buf);
     X(plan_destroy_internal)(cldr);
     X(plan_destroy_internal)(cldr0);
     return 0;
}

static plan *mkplan_dit(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_rdft *p = (const problem_rdft *) p_;
     P *pln = 0;
     int n, is, os, r, m;
     plan *cld = (plan *) 0;

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


     cld = X(mkplan_d)(plnr, 
		       X(mkproblem_rdft_d)(X(mktensor_1d)(m, r * is, os),
					   X(mktensor_1d)(r, is, m * os),
					   p->I, p->O, p->kind));
     if (!cld) goto nada;

     pln = MKPLAN_RDFT(P, &padt, apply_dit);
     if (!mkP(pln, r, p->O, os*m, p->kind[0], plnr))
	  goto nada;

     pln->ios = os*m;
     pln->os = os;
     pln->m = m;
     pln->cld = cld;
     pln->W = 0;

     X(ops_madd)((m - 1)/2, &pln->super.super.ops, &cld->ops,
		 &pln->super.super.ops);

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld);
     X(ifree0)(pln);
     return (plan *) 0;
}

static plan *mkplan_dif(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_rdft *p = (const problem_rdft *) p_;
     P *pln = 0;
     int n, is, os, r, m;
     plan *cld = (plan *) 0;

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

     cld = X(mkplan_d)(plnr,
		       X(mkproblem_rdft_d)(X(mktensor_1d)(m, is, r * os),
					   X(mktensor_1d)(r, m * is, os),
					   p->I, p->O, p->kind));
     if (!cld) goto nada;

     pln = MKPLAN_RDFT(P, &padt, apply_dif);
     if (!mkP(pln, r, p->I, is*m, p->kind[0], plnr)) goto nada;

     pln->ios = is*m;
     pln->os = is;
     pln->m = m;
     pln->cld = cld;
     pln->W = 0;

     X(ops_madd)((m - 1)/2, &pln->super.super.ops, &cld->ops,
		 &pln->super.super.ops);

     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld);
     X(ifree0)(pln);
     return (plan *) 0;
}

/* constructors */

static solver *mksolver_dit(void)
{
     static const solver_adt sadt = { mkplan_dit };
     S *slv = MKSOLVER(S, &sadt);
     slv->kind = R2HC;
     return &(slv->super);
}

static solver *mksolver_dif(void)
{
     static const solver_adt sadt = { mkplan_dif };
     S *slv = MKSOLVER(S, &sadt);
     slv->kind = HC2R;
     return &(slv->super);
}

void X(rdft_rader_hc2hc_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver_dit());
     REGISTER_SOLVER(p, mksolver_dif());
}
