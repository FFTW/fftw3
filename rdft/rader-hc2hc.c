/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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
     uint min_prime;
     rdft_kind kind;
} S;

typedef struct {
     plan_rdft super;

     plan *cldr, *cldr0;
     plan *cld;
     R *W;
     R *omega;
     uint m, r, g, ginv;
     int os, ios;
     rdft_kind kind;
} P;

/***************************************************************************/

/* Below, we extensively use the identity that fft(x*)* = ifft(x) in
   order to share data between forward and backward transforms and to
   obviate the necessity of having separate forward and backward
   plans. */

static void apply_aux(uint r, plan_dft *cldr, const R *omega,
		      R *buf, R *ro, R i0, R *io)
{
     R r0;
     uint k;

     /* compute DFT of buf, operating in-place */
     cldr->apply((plan *) cldr, buf, buf+1, buf, buf+1);

     /* set output DC component: */
     ro[0] = (r0 = ro[0]) + buf[0];
     io[0] = i0 + buf[1];

     /* now, multiply by omega: */
     for (k = 0; k < r - 1; ++k) {
	  fftw_real rB, iB, rW, iW;
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

static void apply_dit(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     plan_dft *cldr;
     int os, ios;
     uint j, k, gpower, g, ginv, r, m;
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

     buf = (R *) fftw_malloc(sizeof(R) * (r - 1) * 2, BUFFERS);

     for (j = 2; j < m; j += 2, rio += os, ii -= os, io -= os, W += 2*(r - 1)) {
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

	  /* second half of array must be fiddled to get real/imag parts in correct spots: */
	  for (k = (r+1)/2; k < r; ++k) {
	       R r;
	       r = rio[k * ios];
	       rio[k * ios] = -io[-k * ios];
	       io[-k * ios] = r;
	  }
     }

     /* Avoid funny m/2-th iter by requiring m odd.  This always
	happens anyway because all the factors of 2 get divided out
	first by codelets (Rader is UGLY for small factors). */

     X(free)(buf);
}

/***************************************************************************/

/* shared twiddle and omega lists, keyed by two/three integers. */

typedef struct TL_s { uint k1,k2,k3; R *W; int refcnt; struct TL_s *nxt; } TL;

static TL *TL_insrt(uint k1, uint k2, uint k3, R *W, TL *tl)
{
     TL *t = (TL *) fftw_malloc(sizeof(TL), TWIDDLES);
     t->k1 = k1; t->k2 = k2; t->k3 = k3; t->W = W;
     t->refcnt = 1; t->nxt = tl;
     return t;
}

static TL *TL_fnd(uint k1, uint k2, uint k3, TL *tl)
{
     TL *t = tl;
     while (t && (t->k1 != k1 || t->k2 != k2 || t->k3 != k3))
	  t = t->nxt;
     return t;
}

static TL *TL_dlt(R *W, TL *tl)
{
     if (W) {
	  TL *tp = (TL *) 0, *t = tl;
	  while (t && t->W != W) {
	       tp = t;
	       t = t->nxt;
	  }
	  if (t && --t->refcnt <= 0) {
	       if (tp) tp->nxt = t->nxt; else tl = t->nxt;
	       X(free)(t->W);
	       X(free)(t);
	  }
     }
     return(tl);
}

/***************************************************************************/

static TL *omegas = (TL *) 0;
static TL *twiddles = (TL *) 0;

static R *mkomega(plan *p_, uint n, uint ginv)
{
     plan_dft *p = (plan_dft *) p_;
     R *omega;
     uint i, gpower;
     trigreal scale, twoPiOverN;
     TL *o;

     if ((o = TL_fnd(n, n, ginv, omegas))) {
	  ++o->refcnt;
	  return o->W;
     }

     omega = (R *)fftw_malloc(sizeof(R) * (n - 1) * 2, TWIDDLES);

     scale = ((trigreal)1.0) / (n - 1); /* normalization for convolution */
     twoPiOverN = K2PI / (trigreal) n;

     for (i = 0, gpower = 1; i < n-1; ++i, gpower = MULMOD(gpower, ginv, n)) {
	  omega[2*i] = scale * COS(twoPiOverN * gpower);
	  omega[2*i+1] = FFT_SIGN * scale * SIN(twoPiOverN * gpower);
     }
     A(gpower == 1);

     AWAKE(p_, 1);
     p->apply(p_, omega, omega + 1, omega, omega + 1);
     AWAKE(p_, 0);

     omegas = TL_insrt(n, n, ginv, omega, omegas);
     return omega;
}

static void free_omega(R *omega)
{
     omegas = TL_dlt(omega, omegas);
}

static R *mktwiddle(uint m, uint r, uint g)
{
     trigreal twoPiOverN;
     uint i, j, gpower;
     R *W;
     TL *o;

     if ((o = TL_fnd(m, r, g, twiddles))) {
	  ++o->refcnt;
	  return o->W;
     }

     twoPiOverN = K2PI / (trigreal) (r * m);
     W = (R *)fftw_malloc(sizeof(R) * (r - 1) * ((m-1)/2) * 2, TWIDDLES);
     for (i = 1; i < (m+1)/2; ++i) {
	  for (gpower = 1, j = 0; j < r - 1;
	       ++j, gpower = MULMOD(gpower, g, r)) {
	       uint k = (i - 1) * (r - 1) + j;
	       W[2*k] = COS(twoPiOverN * (i * gpower));
	       W[2*k+1] = FFT_SIGN * SIN(twoPiOverN * (i * gpower));
	  }
	  A(gpower == 1);
     }

     twiddles = TL_insrt(m, r, g, W, twiddles);
     return W;
}

static void free_twiddle(R *twiddle)
{
     twiddles = TL_dlt(twiddle, twiddles);
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
	       ego->omega = mkomega(ego->cldr, ego->r, ego->ginv);
	  if (!ego->W)
	       ego->W = mktwiddle(ego->m, ego->r, ego->g);
     } else {
	  free_omega(ego->omega);
	  ego->omega = 0;
	  free_twiddle(ego->W);
	  ego->W = 0;
     }
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cld);
     X(plan_destroy)(ego->cldr);
     X(plan_destroy)(ego->cldr0);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;

     p->print(p, "(rdft-rader-%s-%u%(%p%)%(%p%)%(%p%))",
	      ego->kind == R2HC ? "r2hc-dit" : "hc2r-dif",
              ego->r, ego->cldr0, ego->cldr, ego->cld);
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (RDFTP(p_)) {
	  const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz.rnk == 1
		  && p->vecsz.rnk == 0
		  && p->sz.dims[0].n > 1
		  && p->sz.dims[0].n % 4 /* make sure n / r = m is odd */
		  && p->kind == ego->kind
		  && !X(is_prime)(p->sz.dims[0].n) /* avoid inf. loops planning cldr0 */
	       );
     }

     return 0;
}

static int score(const solver *ego_, const problem *p_, int flags)
{
     UNUSED(flags);
     if (applicable(ego_, p_)) {
	  const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
	  uint r = X(first_divisor)(p->sz.dims[0].n);
	  if (r < ego->min_prime || r == p->sz.dims[0].n)
	       return UGLY;
	  else
	       return GOOD;
     }
     return BAD;
}

static int mkP(P *pln, uint r, R *O, int ios, rdft_kind kind, planner *plnr)
{
     plan *cldr = (plan *) 0;
     plan *cldr0 = (plan *) 0;
     problem *cldp = 0;
     R *buf = (R *) 0;

     cldp =
          X(mkproblem_rdft_d)(
               X(mktensor_1d)(r, ios, ios),
               X(mktensor_1d)(1, 0, 0),
	       O, O, kind);
     cldr0 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldr0)
          goto nada;

     /* initial allocation for the purpose of planning */
     buf = (R *) fftw_malloc(sizeof(R) * (r - 1) * 2, BUFFERS);

     cldp =
          X(mkproblem_dft_d)(
               X(mktensor_1d)(r - 1, 2, 2),
               X(mktensor_1d)(1, 0, 0),
               buf, buf + 1, buf, buf + 1);
     cldr = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cldr)
          goto nada;

     X(free)(buf);

     pln->cldr = cldr;
     pln->cldr0 = cldr0;
     pln->omega = 0;
     pln->r = r;
     pln->g = X(find_generator)(r);
     pln->ginv = X(power_mod)(pln->g, r - 2, r);
     pln->kind = kind;
     A(MULMOD(pln->g, pln->ginv, r) == 1);

     pln->super.super.ops = X(ops_add)(cldr->ops, cldr->ops);
     pln->super.super.ops.other += (r - 1) * (4 * 2 + 6) + 6;
     pln->super.super.ops.add += (r - 1) * 2 + 4;
     pln->super.super.ops.mul += (r - 1) * 4;

     return 1;

 nada:
     if (buf)
          X(free)(buf);
     if (cldr)
          X(plan_destroy)(cldr);
     if (cldr0)
          X(plan_destroy)(cldr0);
     return 0;
}

static plan *mkplan_dit(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_rdft *p = (const problem_rdft *) p_;
     P *pln = 0;
     uint n, is, os, r, m;
     plan *cld = (plan *) 0;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_))
          goto nada;

     n = p->sz.dims[0].n;
     is = p->sz.dims[0].is;
     os = p->sz.dims[0].os;

     r = X(first_divisor)(n);
     m = n / r;

     {
	  problem *cldp;
	  cldp = X(mkproblem_rdft_d)(X(mktensor_1d)(m, r * is, os),
				     X(mktensor_1d)(r, is, m * os),
				     p->I, p->O, p->kind);
	  cld = MKPLAN(plnr, cldp);
	  X(problem_destroy)(cldp);
	  if (!cld)
	       goto nada;
     }

     pln = MKPLAN_RDFT(P, &padt, apply_dit);
     if (!mkP(pln, r, p->O, os*m, p->kind, plnr))
	  goto nada;

     pln->ios = os*m;
     pln->os = os;
     pln->m = m;
     pln->cld = cld;
     pln->W = 0;

     pln->super.super.ops =
	  X(ops_add)(X(ops_mul)((m - 1)/2, pln->super.super.ops),
		     cld->ops);

     return &(pln->super.super);

 nada:
     if (cld)
          X(plan_destroy)(cld);
     if (pln)
	  X(free)(pln);
     return (plan *) 0;
}

/* constructors */

static solver *mksolver_dit(uint min_prime)
{
     static const solver_adt sadt = { mkplan_dit, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->min_prime = min_prime;
     slv->kind = R2HC;
     return &(slv->super);
}

void X(rdft_rader_hc2hc_register)(planner *p)
{
     /* FIXME: what are good defaults? */
     REGISTER_SOLVER(p, mksolver_dit(53));
}
