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

#include "dft.h"

/*
 * Compute transforms of prime sizes using Rader's trick: turn them
 * into convolutions of size n - 1, which you then perform via a pair
 * of FFTs.   This file contains both nontwiddle (direct) and 
 * twiddle (DIT Cooley-Tukey) solvers.
 */

typedef struct {
     solver super;
     uint min_prime;
} S;

typedef struct {
     plan_dft super;

     plan *cld1, *cld2;
     R *buf, *omega;
     uint n, is, os, g, ginv;
     plan *cld_omega;
} P;

typedef struct {
     P super;
     plan *cld;
     R *W;
     int os;
     uint m;
} P_dit;

/***************************************************************************/

/* Below, we extensively use the identity that fft(x*)* = ifft(x) in
   order to share data between forward and backward transforms and to
   obviate the necessity of having separate forward and backward
   plans.  (Although we compute separate plans in principle anyway due
   to the differing strides, etcetera.)

   Of course, since the new FFTW gives us separate pointers to
   the real and imaginary parts, we could have instead used the
   fft(r,i) = ifft(i,r) form of this identity, but it was easier to
   reuse the code from our old version. */

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     int is, os;
     uint k, gpower, g, r;
     R *buf;
     const R *omega;
     R r0, i0;

     /* First, permute the input, storing in buf: */
     r = ego->n; is = ego->is; g = ego->g; buf = ego->buf;
     for (gpower = 1, k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	  R rA, iA;
	  rA = ri[gpower * is];
	  iA = ii[gpower * is];
	  buf[2*k] = rA; buf[2*k + 1] = iA;
     }
     /* gpower == g^(r-1) mod r == 1 */;

     os = ego->os;

     /* compute DFT of buf, storing in output (except DC): */
     {
	    plan *cld1 = ego->cld1;
	    plan_dft *cld = (plan_dft *) cld1;
	    cld->apply(cld1, buf, buf+1, ro+os, io+os);
     }

     /* set output DC component: */
     r0 = ri[0]; i0 = ii[0];
     ro[0] = r0 + ro[os];
     io[0] = i0 + io[os];

     /* now, multiply by omega: */
     omega = ego->omega;
     for (k = 0; k < r - 1; ++k) {
	  fftw_real rB, iB, rW, iW;
	  rW = omega[2*k];
	  iW = omega[2*k+1];
	  rB = ro[(k+1)*os];
	  iB = io[(k+1)*os];
	  ro[(k+1)*os] = rW * rB - iW * iB;
	  io[(k+1)*os] = -(rW * iB + iW * rB);
     }
     
     /* this will add input[0] to all of the outputs after the ifft */
     ro[os] += r0;
     io[os] -= i0;

     /* inverse FFT: */
     {
	    plan *cld2 = ego->cld2;
	    plan_dft *cld = (plan_dft *) cld2;
	    cld->apply(cld2, ro+os, io+os, buf, buf+1);
     }
     
     /* finally, do inverse permutation to unshuffle the output: */
     g = ego->ginv;
     A(gpower == 1);
     for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	  ro[gpower * os] = buf[2*k];
	  io[gpower * os] = -buf[2*k+1];
     }
     A(gpower == 1);
}

static void apply_dit(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P_dit *ego_dit = (P_dit *) ego_;
     P *ego;
     plan *cld1, *cld2;
     int os, osm;
     uint j, k, gpower, g, ginv, r, m;
     R *buf;
     const R *omega, *W;
     R r0, i0;

     {
	   plan *cld0 = ego_dit->cld;
	   plan_dft *cld = (plan_dft *) cld0;
	   cld->apply(cld0, ri, ii, ro, io);
     }

     ego = (P *) ego_;
     cld1 = ego->cld1;
     cld2 = ego->cld2;
     r = ego->n;
     m = ego_dit->m;
     g = ego->g; 
     ginv = ego->ginv;
     buf = ego->buf;
     omega = ego->omega;
     W = ego_dit->W;
     os = ego_dit->os;
     osm = ego->os;
     gpower = 1;

     for (j = 0; j < m; ++j, ro += os, io += os, W += 2*(r - 1)) {
	  /* First, permute the input and multiply by W, storing in buf: */
	  A(gpower == 1);
	  for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	       R rA, iA, rW, iW;
	       rA = ro[gpower * osm];
	       iA = io[gpower * osm];
	       rW = W[2*k];
	       iW = W[2*k+1];
	       buf[2*k] = rW * rA - iW * iA;
	       buf[2*k + 1] = rW * iA + iW * rA;
	  }
	  /* gpower == g^(r-1) mod r == 1 */;
	  
	  
	  /* compute DFT of buf, storing in output (except DC): */
	  {
	       plan_dft *cld = (plan_dft *) cld1;
	       cld->apply(cld1, buf, buf+1, ro+osm, io+osm);
	  }
	  
	  /* set output DC component: */
	  ro[0] = (r0 = ro[0]) + ro[osm];
	  io[0] = (i0 = io[0]) + io[osm];

	  /* now, multiply by omega: */
	  for (k = 0; k < r - 1; ++k) {
	       fftw_real rB, iB, rW, iW;
	       rW = omega[2*k];
	       iW = omega[2*k+1];
	       rB = ro[(k+1)*osm];
	       iB = io[(k+1)*osm];
	       ro[(k+1)*osm] = rW * rB - iW * iB;
	       io[(k+1)*osm] = -(rW * iB + iW * rB);
	  }
	  
	  /* this will add input[0] to all of the outputs after the ifft */
	  ro[osm] += r0;
	  io[osm] -= i0;
	  
	  /* inverse FFT: */
	  {
	       plan_dft *cld = (plan_dft *) cld2;
	       cld->apply(cld2, ro+osm, io+osm, buf, buf+1);
	  }
	  
	  /* finally, do inverse permutation to unshuffle the output: */
	  A(gpower == 1);
	  for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, ginv, r)) {
	       ro[gpower * osm] = buf[2*k];
	       io[gpower * osm] = -buf[2*k+1];
	  }
	  A(gpower == 1);
     }
}

/***************************************************************************/

/* FIXME: share Rader omega and twiddle arrays between plans */

static R *mkomega(plan *p_, R *buf, uint n, uint ginv)
{
     plan_dft *p = (plan_dft *) p_;
     R *omega;
     uint i, gpower;
     trigreal scale, twoPiOverN;

     scale = ((trigreal)1.0) / (n - 1); /* normalization for convolution */
     twoPiOverN = K2PI / (trigreal) n;

     for (i = 0, gpower = 1; i < n-1; ++i, gpower = MULMOD(gpower, ginv, n)) {
	  buf[2*i] = scale * COS(twoPiOverN * gpower);
	  buf[2*i+1] = FFT_SIGN * scale * SIN(twoPiOverN * gpower);
     }
     A(gpower == 1);

     omega = (R *)fftw_malloc(sizeof(R) * (n - 1) * 2, TWIDDLES);

     p->apply(p_, buf, buf + 1, omega, omega + 1);
     return omega;
}

static R *mktwiddle(uint m, uint r, uint g)
{
     trigreal twoPiOverN;
     uint i, j, gpower;
     R *W;

     twoPiOverN = K2PI / (trigreal) (r * m);
     W = (R *)fftw_malloc(sizeof(R) * (r - 1) * m * 2, TWIDDLES);
     for (i = 0; i < m; ++i) {
	  for (gpower = 1, j = 0; j < r - 1;
	       ++j, gpower = MULMOD(gpower, g, r)) {
	       uint k = i * (r - 1) + j;
	       W[2*k] = COS(twoPiOverN * (i * gpower));
	       W[2*k+1] = FFT_SIGN * SIN(twoPiOverN * (i * gpower));
	  }
	  A(gpower == 1);
     }

     return W;
}

/***************************************************************************/

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld1, flg);
     AWAKE(ego->cld2, flg);
     AWAKE(ego->cld_omega, flg);

     if (flg) {
          if (!ego->buf)
               ego->buf = 
		    (R *)fftw_malloc(sizeof(R) * (ego->n - 1) * 2, BUFFERS);
	  if (!ego->omega) {
	       ego->omega = mkomega(ego->cld_omega,ego->buf,ego->n,ego->ginv);
#if 0
	       /* HACK ALERT!  We'd really like to put the cld_omega
		  plan to sleep here, since we don't need it any more.
	          However, doing so causes crashes since cld_omega may
	          be the same as cld1/cld2 or (worse) some other plan
	          entirely (e.g. a rader-dit plan that is parent or child
	          to this one). */
	       AWAKE(ego->cld_omega, 0);
#endif
	  }
     } else {
	  if (ego->omega)
	       X(free)(ego->omega);
	  ego->omega = 0;
	  if (ego->buf)
	       X(free)(ego->buf);
	  ego->buf = 0;
     }
}

static void awake_dit(plan *ego_, int flg)
{
     P_dit *ego = (P_dit *) ego_;

     AWAKE(ego->cld, flg);
     if (flg)
	  ego->W = mktwiddle(ego->m, ego->super.n, ego->super.g);
     else {
	  X(free)(ego->W);
	  ego->W = 0;
     }

     awake(ego_, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     if (ego->omega)
	  X(free)(ego->omega);
     if (ego->buf)
	  X(free)(ego->buf);
     X(plan_destroy)(ego->cld_omega);
     X(plan_destroy)(ego->cld2);
     X(plan_destroy)(ego->cld1);
     X(free)(ego);
}

static void destroy_dit(plan *ego_)
{
     P_dit *ego = (P_dit *) ego_;
     if (ego->W)
	  X(free)(ego->W);
     X(plan_destroy)(ego->cld);
     destroy(ego_);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;

     p->print(p, "(dft-rader-%u%ois=%oos=%(%p%)%(%p%))", 
	      ego->n, ego->is, ego->os, ego->cld1, ego->cld2);
}

static void print_dit(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     P_dit *ego_dit = (P_dit *) ego_;

     p->print(p, "(dft-rader-dit-%u%(%p%)%(%p%)%(%p%))", 
	      ego->n, ego->cld1, ego->cld2, ego_dit->cld);
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
	       && p->sz.rnk == 1
	       && p->vecsz.rnk == 0
	       && X(is_prime)(p->sz.dims[0].n)
	       );
     }

     return 0;
}

static int applicable_dit(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
	       && p->sz.rnk == 1
	       && p->vecsz.rnk == 0
	       && p->sz.dims[0].n > 1
	       );
     }

     return 0;
}

static int score(const solver *ego_, const problem *p_, int flags)
{
     UNUSED(flags);
     if (applicable(ego_, p_)) {
	  const S *ego = (const S *) ego_;
          const problem_dft *p = (const problem_dft *) p_;
	  if (p->sz.dims[0].n < ego->min_prime)
	       return UGLY;
	  else
	       return GOOD;
     }
     return BAD;
}

static int score_dit(const solver *ego_, const problem *p_, int flags)
{
     UNUSED(flags);
     if (applicable_dit(ego_, p_)) {
	  const S *ego = (const S *) ego_;
          const problem_dft *p = (const problem_dft *) p_;
	  int r = X(first_divisor)(p->sz.dims[0].n);
	  if (r < ego->min_prime || r == p->sz.dims[0].n)
	       return UGLY;
	  else
	       return GOOD;
     }
     return BAD;
}

static int mkP(P *pln, uint n, int is, int os, R *ro, R *io,
	       planner *plnr)
{
     plan *cld1 = (plan *) 0;
     plan *cld2 = (plan *) 0;
     plan *cld_omega = (plan *) 0;
     problem *cldp = 0;
     R *buf = (R *) 0, *omega = (R *) 0;

     /* initial allocation for the purpose of planning */
     buf = (R *) fftw_malloc(sizeof(R) * (n - 1) * 2, BUFFERS);

     cldp =
          X(mkproblem_dft_d)(
               X(mktensor_1d)(n - 1, 2, os),
               X(mktensor_1d)(1, 0, 0),
               buf, buf + 1, ro + os, io + os);
     cld1 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld1)
          goto nada;

     cldp =
          X(mkproblem_dft_d)(
               X(mktensor_1d)(n - 1, os, 2),
               X(mktensor_1d)(1, 0, 0),
               ro + os, io + os, buf, buf + 1);
     cld2 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld2)
          goto nada;

     /* initial allocation for the purpose of planning */
     omega = (R *) fftw_malloc(sizeof(R) * (n - 1) * 2, BUFFERS);

     /* FIXME: would be nice to use an estimating planner for cld_omega... */
     cldp =
          X(mkproblem_dft_d)(
               X(mktensor_1d)(n - 1, 2, 2),
               X(mktensor_1d)(1, 0, 0),
	       buf, buf + 1, omega, omega + 1);
     cld_omega = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld_omega)
          goto nada;

     /* deallocate buffers; let awake() allocate them for real */
     X(free)(omega);
     omega = 0;
     X(free)(buf);
     buf = 0;

     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->cld_omega = cld_omega;
     pln->buf = buf;
     pln->omega = omega;
     pln->n = n;
     pln->is = is;
     pln->os = os;
     pln->g = X(find_generator)(n);
     pln->ginv = X(power_mod)(pln->g, n - 2, n);
     A(MULMOD(pln->g, pln->ginv, n) == 1);

     pln->super.super.ops = X(ops_add)(cld1->ops, cld2->ops);
     pln->super.super.ops.other += (n - 1) * (4 * 2 + 6) + 6;
     pln->super.super.ops.add += (n - 1) * 2 + 4;
     pln->super.super.ops.mul += (n - 1) * 4;

     return 1;

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

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     P *pln;
     uint n, is, os;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_))
	  return (plan *) 0;

     n = p->sz.dims[0].n;
     is = p->sz.dims[0].is;
     os = p->sz.dims[0].os;

     pln = MKPLAN_DFT(P, &padt, apply);
     if (!mkP(pln, n, is, os, p->ro, p->io, plnr)) {
	  X(free)(pln);
	  return (plan *) 0;
     }
     return &(pln->super.super);
}

static plan *mkplan_dit(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     P_dit *pln;
     uint n, is, os, r, m;
     plan *cld = (plan *) 0;

     static const plan_adt padt = {
	  X(dft_solve), awake_dit, print_dit, destroy_dit
     };

     if (!applicable_dit(ego, p_))
          goto nada;

     n = p->sz.dims[0].n;
     is = p->sz.dims[0].is;
     os = p->sz.dims[0].os;

     r = X(first_divisor)(n);
     m = n / r;

     {
	  problem *cldp;
	  cldp = X(mkproblem_dft_d)(X(mktensor_1d)(m, r * is, os),
				    X(mktensor_1d)(r, is, m * os),
				    p->ri, p->ii, p->ro, p->io);
	  cld = MKPLAN(plnr, cldp);
	  X(problem_destroy)(cldp);
	  if (!cld)
	       goto nada;
     }

     pln = MKPLAN_DFT(P_dit, &padt, apply_dit);
     if (!mkP(&pln->super, r, os*m, os*m, p->ro, p->io, plnr))
	  goto nada;

     pln->os = os;
     pln->m = m;
     pln->cld = cld;

     pln->super.super.super.ops.add += 2 * (r-1);
     pln->super.super.super.ops.mul += 4 * (r-1);
     pln->super.super.super.ops.other += 2 * (r-1);
     pln->super.super.super.ops =
	  X(ops_add)(X(ops_mul)(m, pln->super.super.super.ops),
		     cld->ops);

     return &(pln->super.super.super);

 nada:
     if (cld)
          X(plan_destroy)(cld);
     X(free)(pln);
     return (plan *) 0;
}

/* constructors */

solver *mksolver(uint min_prime)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->min_prime = min_prime;
     return &(slv->super);
}

solver *mksolver_dit(uint min_prime)
{
     static const solver_adt sadt = { mkplan_dit, score_dit };
     S *slv = MKSOLVER(S, &sadt);
     slv->min_prime = min_prime;
     return &(slv->super);
}

void X(dft_rader_register)(planner *p)
{
     /* FIXME: what are good defaults? */
     REGISTER_SOLVER(p, mksolver(53));
     REGISTER_SOLVER(p, mksolver_dit(53));
}
