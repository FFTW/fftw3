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

/*
 * Compute transforms of prime sizes using Rader's trick: turn them
 * into convolutions of size n - 1, which you then perform via a pair
 * of FFTs.   Here, in order to take advantage of real data, we first
 * express the original DFT as a DHT (Discrete Hartley Transform), and
 * apply Rader's trick to the DHT.  This file contains only nontwiddle
 * (direct) solvers.
 */

typedef struct {
     solver super;
     uint min_prime;
     rdft_kind kind;
} S;

typedef struct {
     plan_rdft super;

     plan *cld1, *cld2;
     R *omega;
     uint n, g, ginv;
     int is, os;
     plan *cld_omega;
     rdft_kind kind;
} P;

/***************************************************************************/

/* If R2HC_ONLY_CONV is 1, we use a trick to perform the convolution
   purely in terms of R2HC transforms, as opposed to R2HC followed by H2RC.
   This requires a few more operations, but allows us to share the same
   plan/codelets for both Rader children.  (See also r2hc-hc2r.c) */
#define R2HC_ONLY_CONV 1

static int apply_aux(P *ego, uint r, int is, R *I, R *O)
{
     int os;
     uint k, gpower, g;
     R *buf, *omega;
     R r0;

     buf = (R *) fftw_malloc(sizeof(R) * (r - 1), BUFFERS);

     /* First, permute the input, storing in buf: */
     g = ego->g; 
     for (gpower = 1, k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	  buf[k] = I[gpower * is];
     }
     /* gpower == g^(r-1) mod r == 1 */;

     os = ego->os;

     /* compute RDFT of buf, storing in output (except DC): */
     {
	    plan_rdft *cld = (plan_rdft *) ego->cld1;
	    cld->apply((plan *) cld, buf, O + os);
     }

     /* set output DC component: */
     O[0] = (r0 = I[0]) + O[os];

     /* now, multiply by omega: */
     omega = ego->omega;

     O[(0 + 1) * os] *= omega[0];
#if R2HC_ONLY_CONV
     for (k = 1; k < (r - 1)/2; ++k) {
	  E rB, iB, rW, iW, a, b;
	  rW = omega[k];
	  iW = omega[(r-1) - k];
	  rB = O[(k + 1) * os];
	  iB = O[((r-1) - k + 1) * os];
	  a = rW * rB - iW * iB;
	  b = rW * iB + iW * rB;
	  O[(k + 1) * os] = a + b;
	  O[((r-1) - k + 1) * os] = a - b;
     }
#else
     for (k = 1; k < (r - 1)/2; ++k) {
	  E rB, iB, rW, iW;
	  rW = omega[k];
	  iW = omega[(r-1) - k];
	  rB = O[(k + 1) * os];
	  iB = O[((r-1) - k + 1) * os];
	  O[(k + 1) * os] = rW * rB - iW * iB;
	  O[((r-1) - k + 1) * os] = rW * iB + iW * rB;
     }
#endif
     /* Nyquist component: */
     O[(k + 1) * os] *= omega[k]; /* k == (r-1)/2, since r-1 is even */
     
     /* this will add input[0] to all of the outputs after the ifft */
     O[os] += r0;

     /* inverse FFT: */
     {
	    plan_rdft *cld = (plan_rdft *) ego->cld2;
	    cld->apply((plan *) cld, O + os, buf);
     }
     
     /* do inverse permutation to unshuffle the output: */
     A(gpower == 1);
#if R2HC_ONLY_CONV
     O[os] = buf[0];
     gpower = g = ego->ginv;
     for (k = 1; k < (r - 1)/2; ++k, gpower = MULMOD(gpower, g, r)) {
	  O[gpower * os] = buf[k] + buf[r - 1 - k];
     }
     O[gpower * os] = buf[k];
     ++k, gpower = MULMOD(gpower, g, r);
     for (; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	  O[gpower * os] = buf[r - 1 - k] - buf[k];
     }
#else
     g = ego->ginv;
     for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	  O[gpower * os] = buf[k];
     }
#endif
     A(gpower == 1);

     X(free)(buf);

     return os;
}

static void apply_r2hc(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     uint r, k;
     int os;

     r = ego->n;
     os = apply_aux(ego, r, ego->is, I, O);

     /* finally, unscramble DHT back into DFT:
        (sucks that this requires an extra pass) */
     for (k = 1; k < (r+1)/2; ++k) {
	  E rp, rm;
	  rp = O[k * os];
	  rm = O[(r - k) * os];
	  O[k * os] = 0.5 * (rp + rm);
	  O[(r - k) * os] = 0.5 * (rp - rm);
     }
}

static void apply_hc2r(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     uint r, k;
     int is;

     r = ego->n;
     is = ego->is;

     /* before everything, scramble DFT into DHT (destroying input):
        (sucks that this requires an extra pass) */
     for (k = 1; k < (r+1)/2; ++k) {
	  E a, b;
	  a = I[k * is];
	  b = I[(r - k) * is];
	  I[k * is] = a + b;
	  I[(r - k) * is] = a - b;
     }

     apply_aux(ego, r, is, I, O);
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

static R *mkomega(plan *p_, uint n, uint ginv)
{
     plan_rdft *p = (plan_rdft *) p_;
     R *omega;
     uint i, gpower;
     trigreal scale;
     R *buf; 
     TL *o;

     if ((o = TL_fnd(n, n, ginv, omegas))) {
	  ++o->refcnt;
	  return o->W;
     }

     omega = (R *)fftw_malloc(sizeof(R) * (n - 1), TWIDDLES);
     buf = (R *) fftw_malloc(sizeof(R) * (n - 1), BUFFERS);

     scale = n - 1.0; /* normalization for convolution */

     for (i = 0, gpower = 1; i < n-1; ++i, gpower = MULMOD(gpower, ginv, n)) {
	  buf[i] = (X(cos2pi)(gpower, n) + FFT_SIGN * X(sin2pi)(gpower, n)) 
	       / scale;
     }
     A(gpower == 1);

     AWAKE(p_, 1);
     p->apply(p_, buf, omega);
     AWAKE(p_, 0);

     X(free)(buf);

     omegas = TL_insrt(n, n, ginv, omega, omegas);
     return omega;
}

static void free_omega(R *omega)
{
     omegas = TL_dlt(omega, omegas);
}

/***************************************************************************/

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld1, flg);
     AWAKE(ego->cld2, flg);

     if (flg) {
	  if (!ego->omega) 
	       ego->omega = mkomega(ego->cld_omega,ego->n,ego->ginv);
     } else {
	  free_omega(ego->omega);
	  ego->omega = 0;
     }
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cld_omega);
     X(plan_destroy)(ego->cld2);
     X(plan_destroy)(ego->cld1);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;

     p->print(p, "(rdft-rader-dht-%s-%u%ois=%oos=%(%p%)",
              X(rdft_kind_str)(ego->kind), ego->n, ego->is, ego->os, ego->cld1);
     if (ego->cld2 != ego->cld1)
          p->print(p, "%(%p%)", ego->cld2);
     if (ego->cld_omega != ego->cld1 && ego->cld_omega != ego->cld2)
          p->print(p, "%(%p%)", ego->cld_omega);
     p->putchr(p, ')');
}

static int applicable(const solver *ego_, const problem *p_)
{
     if (RDFTP(p_)) {
	  const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz.rnk == 1
		  && p->vecsz.rnk == 0
		  && p->kind[0] == ego->kind
		  && X(is_prime)(p->sz.dims[0].n)
		  && p->sz.dims[0].n > 2
	       );
     }

     return 0;
}

static int score(const solver *ego_, const problem *p_, const planner *plnr)
{
     UNUSED(plnr);
     if (applicable(ego_, p_)) {
	  const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
	  if (p->sz.dims[0].n < ego->min_prime)
	       return UGLY;
	  else
	       return GOOD;
     }
     return BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_rdft *p = (const problem_rdft *) p_;
     P *pln;
     uint n;
     int is, os;
     plan *cld1 = (plan *) 0;
     plan *cld2 = (plan *) 0;
     plan *cld_omega = (plan *) 0;
     problem *cldp = 0;
     R *buf = (R *) 0, *omega = (R *) 0;
     R *O;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_))
	  return (plan *) 0;

     n = p->sz.dims[0].n;
     is = p->sz.dims[0].is;
     os = p->sz.dims[0].os;
     O = p->O;

     /* initial allocation for the purpose of planning */
     buf = (R *) fftw_malloc(sizeof(R) * (n - 1), BUFFERS);

     cldp =
          X(mkproblem_rdft_1_d)(
               X(mktensor_1d)(n - 1, 1, os),
               X(mktensor_1d)(1, 0, 0),
               buf, O + os, R2HC);
     cld1 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld1)
          goto nada;

     cldp =
          X(mkproblem_rdft_1_d)(
               X(mktensor_1d)(n - 1, os, 1),
               X(mktensor_1d)(1, 0, 0),
               O + os, buf, 
#if R2HC_ONLY_CONV
	       R2HC
#else
	       HC2R
#endif
	       );
     cld2 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld2)
          goto nada;

     /* initial allocation for the purpose of planning */
     omega = (R *) fftw_malloc(sizeof(R) * (n - 1), BUFFERS);

     plnr->flags |= ESTIMATE;
     cldp =
          X(mkproblem_rdft_1_d)(
               X(mktensor_1d)(n - 1, 1, 1),
               X(mktensor_1d)(1, 0, 0),
	       buf, omega, R2HC);
     cld_omega = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld_omega)
          goto nada;

     /* deallocate buffers; let awake() or apply() allocate them for real */
     X(free)(omega);
     omega = 0;
     X(free)(buf);
     buf = 0;

     pln = MKPLAN_RDFT(P, &padt, R2HC_KINDP(ego->kind) ? apply_r2hc : apply_hc2r);
     pln->kind = ego->kind;
     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->cld_omega = cld_omega;
     pln->omega = omega;
     pln->n = n;
     pln->is = is;
     pln->os = os;
     pln->g = X(find_generator)(n);
     pln->ginv = X(power_mod)(pln->g, n - 2, n);
     A(MULMOD(pln->g, pln->ginv, n) == 1);

     pln->super.super.ops = X(ops_add)(cld1->ops, cld2->ops);
     pln->super.super.ops.other += (n - 1) * (2 + 3 + 2 + 2) + 3;
     pln->super.super.ops.add += (n - 1) * 2;
     pln->super.super.ops.mul += (n - 1) * (R2HC_KINDP(ego->kind) ? 3 : 2) - 2;
#if R2HC_ONLY_CONV
     pln->super.super.ops.other += (n - 1) - 2;
     pln->super.super.ops.add += 2 * (n - 1) - 4;
#endif

     return &(pln->super.super);

 nada:
     if (buf)
          X(free)(buf);
     if (omega)
          X(free)(omega);
     if (cld_omega)
          X(plan_destroy)(cld_omega);
     if (cld2)
          X(plan_destroy)(cld2);
     if (cld1)
          X(plan_destroy)(cld1);
     return 0;
}

/* constructors */

static solver *mksolver(rdft_kind kind, uint min_prime)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->kind = kind;
     slv->min_prime = min_prime;
     return &(slv->super);
}

void X(rdft_rader_dht_register)(planner *p)
{
     /* FIXME: what are good defaults? */
     REGISTER_SOLVER(p, mksolver(R2HC, RADER_MIN_GOOD));
     REGISTER_SOLVER(p, mksolver(HC2R, RADER_MIN_GOOD));
}
