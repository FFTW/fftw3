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

/*
 * Compute transforms of prime sizes using Rader's trick: turn them
 * into convolutions of size n - 1, which you then perform via a pair
 * of FFTs. 
 */

/***************************************************************************/

/* Rader's algorithm requires lots of modular arithmetic, and if we
   aren't careful we can have errors due to integer overflows. */

#if defined(FFTW_ENABLE_UNSAFE_MULMOD)
#  define MULMOD(x,y,p) (((x) * (y)) % (p))
#elif (HAVE_UINT && ((SIZEOF_UINT != 0) && \
                     (SIZEOF_UNSIGNED_LONG_LONG >= 2 * SIZEOF_UINT))) \
   || (!HAVE_UINT && ((SIZEOF_UNSIGNED_INT != 0) && \
                     (SIZEOF_UNSIGNED_LONG_LONG >= 2 * SIZEOF_UNSIGNED_INT)))
#  define MULMOD(x,y,p) ((uint) ((((unsigned long long) (x))    \
                                  * ((unsigned long long) (y))) \
				 % ((unsigned long long) (p))))
#else /* 'long long' unavailable */
#  define MULMOD(x,y,p) safe_mulmod(x,y,p)

#  include <limits.h>

/* compute (x * y) mod p, but watch out for integer overflows; we must
   have x, y >= 0, p > 0.  This routine is slow. */
static uint safe_mulmod(uint x, uint y, uint p)
{
     if (y == 0 || x <= UINT_MAX / y)
	  return((x * y) % p);
     else {
	  uint y2 = y/2;
	  return((fftw_safe_mulmod(x, y2, p) +
		  fftw_safe_mulmod(x, y - y2, p)) % p);
     }
}
#endif /* safe_mulmod ('long long' unavailable) */

/***************************************************************************/

/* Compute n^m mod p, where m >= 0 and p > 0.  If we really cared, we
   could make this tail-recursive. */
static uint power_mod(uint n, uint m, uint p)
{
     A(p > 0);
     if (m == 0)
	  return 1;
     else if (m % 2 == 0) {
	  uint x = power_mod(n, m / 2, p);
	  return MULMOD(x, x, p);
     }
     else
	  return MULMOD(n, power_mod(n, m - 1, p), p);
}

/* Find the period of n in the multiplicative group mod p (p prime).
   That is, return the smallest m such that n^m == 1 mod p. */
static uint period(uint n, uint p)
{
     uint prod = n, period = 1;

     while (prod != 1) {
	  prod = MULMOD(prod, n, p);
	  ++period;
	  A(prod != 0);
     }
     return period;
}

/* Find a generator for the multiplicative group mod p, where p is
   prime.  The generators are dense enough that this takes O(p)
   time, not O(p^2) as you might naively expect.   (There are even
   faster ways to find a generator; c.f. Knuth.) */
static uint find_generator(uint p)
{
     uint g;

     for (g = 1; g < p; ++g)
	  if (period(g, p) == p - 1)
	       break;
     A(g != p);
     return g;
}

/* Return whether n is prime.  (It would be only slightly faster to
   search a static table of primes; there are 6542 primes < 2^16.)  */
static int isprime(uint n)
{
     uint i;
     if (n <= 1 || (n % 2 == 0 && n > 2))
	  return 0;
     for (i = 3; i*i <= n; i += 2)
	  if (n % i == 0)
	       return 0;
     return 1;
}

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
     uint is, os, r;
     uint k, gpower, g;
     R *buf, *omega;
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
	    plan *cld0 = ego->cld1;
	    plan_dft *cld = (plan_dft *) cld0;
	    cld->apply(cld0, buf, buf+1, ro+os, io+os);
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
	    plan *cld0 = ego->cld2;
	    plan_dft *cld = (plan_dft *) cld0;
	    cld->apply(cld0, ro+os, io+os, buf, buf+1);
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

/***************************************************************************/

#if 0
static fftw_complex *compute_rader_twiddle(int n, uint r, uint g)
{
     fftw_trig_real_t twoPiOverN;
     uint m = n / r;
     uint i, j, gpower;
     fftw_complex *W;

     twoPiOverN = FFTW_K2PI / (fftw_trig_real_t) n;
     W = (fftw_complex *) fftw_malloc((r - 1) * m * sizeof(fftw_complex));
     for (i = 0; i < m; ++i)
	  for (gpower = 1, j = 0; j < r - 1;
	       ++j, gpower = MULMOD(gpower, g, r)) {
	       uint k = i * (r - 1) + j;
	       fftw_trig_real_t ij = (fftw_trig_real_t) (i * gpower);
	       c_re(W[k]) = FFTW_TRIG_COS(twoPiOverN * ij);
	       c_im(W[k]) = - FFTW_TRIG_SIN(twoPiOverN * ij);
	  }

     return W;
}
#endif

/* change here if you need higher precision */
typedef double trigreal;
#define COS cos
#define SIN sin
#define K2PI ((trigreal)6.2831853071795864769252867665590057683943388)

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
	  buf[2*i+1] = -scale * SIN(twoPiOverN * gpower);
     }
     A(gpower == 1);

     omega = (R *)fftw_malloc(sizeof(R) * (n - 1) * 2, TWIDDLES);

     p->apply(p_, buf, buf + 1, omega, omega + 1);
     return omega;
}

/***************************************************************************/

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld1, flg);
     AWAKE(ego->cld2, flg);

     if (flg) {
          if (!ego->buf)
               ego->buf = 
		    (R *)fftw_malloc(sizeof(R) * (ego->n - 1) * 2, BUFFERS);
	  if (!ego->omega) {
	       AWAKE(ego->cld_omega, 1);
	       ego->omega = mkomega(ego->cld_omega,ego->buf,ego->n,ego->ginv);
	       if (ego->cld_omega != ego->cld1) /* HACK ALERT */
		    AWAKE(ego->cld_omega, 0);
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

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;

     p->print(p, "(dft-rader-%u%ois=%oos=%(%p%)%(%p%))", 
	      ego->n, ego->is, ego->os, ego->cld1, ego->cld2);
}

static int applicable(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
	       && p->sz.rnk == 1
	       && p->vecsz.rnk == 0
	       && isprime(p->sz.dims[0].n)
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

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     P *pln;
     plan *cld1 = (plan *) 0;
     plan *cld2 = (plan *) 0;
     plan *cld_omega = (plan *) 0;
     problem *cldp = 0;
     const problem_dft *p = (const problem_dft *) p_;
     R *buf = (R *) 0, *omega = (R *) 0;
     uint n, is, os;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_))
          goto nada;

     n = p->sz.dims[0].n;
     is = p->sz.dims[0].is;
     os = p->sz.dims[0].os;

     /* initial allocation for the purpose of planning */
     buf = (R *) fftw_malloc(sizeof(R) * (n - 1) * 2, BUFFERS);

     cldp =
          X(mkproblem_dft_d)(
               X(mktensor_1d)(n - 1, 2, os),
               X(mktensor_1d)(1, 0, 0),
               buf, buf + 1, p->ro + os, p->io + os);
     cld1 = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     if (!cld1)
          goto nada;

     cldp =
          X(mkproblem_dft_d)(
               X(mktensor_1d)(n - 1, os, 2),
               X(mktensor_1d)(1, 0, 0),
               p->ro + os, p->io + os, buf, buf + 1);
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

     pln = MKPLAN_DFT(P, &padt, apply);
     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->cld_omega = cld_omega;
     pln->buf = buf;
     pln->omega = omega;
     pln->n = n;
     pln->is = is;
     pln->os = os;
     pln->g = find_generator(n);
     pln->ginv = power_mod(pln->g, n - 2, n);
     A(MULMOD(pln->g, pln->ginv, n) == 1);

     pln->super.super.ops = X(ops_add)(cld1->ops, cld2->ops);
     pln->super.super.ops.other += (n - 1) * (4 * 2 + 6) + 6;
     pln->super.super.ops.add += (n - 1) * 2 + 4;
     pln->super.super.ops.mul += (n - 1) * 4;

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
     return (plan *) 0;
}

/* constructor */
solver *mksolver(uint min_prime)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->min_prime = min_prime;
     return &(slv->super);
}

void X(dft_rader_register)(planner *p)
{
     /* FIXME: what are good defaults? */
     REGISTER_SOLVER(p, mksolver(53));
}
