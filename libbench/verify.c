/*
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
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

/* $Id: verify.c,v 1.6 2002-08-15 20:30:03 athena Exp $ */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "bench.h"

/*************************************************
 * complex correctness test
 *************************************************/
#ifdef BENCHFFT_LDOUBLE
#  ifndef HAVE_HYPOTL
static double hypotl(double a, double b)
{
     return sqrt(a * a + b * b);
}
#  else /* HAVE_HYPOTL */
#    if !defined(HAVE_DECL_HYPOTL) || !HAVE_DECL_HYPOTL
extern long double hypotl(long double a, long double b);
#    endif
#  endif
#  define hypot hypotl
#else /* !BENCHFFT_LDOUBLE */
#  ifndef HAVE_HYPOT
static double hypot(double a, double b)
{
     return sqrt(a * a + b * b);
}
#  else /* HAVE_HYPOT */
#    if !defined(HAVE_DECL_HYPOT) || !HAVE_DECL_HYPOT
extern double hypot(double a, double b);
#    endif
#  endif
#endif /* !BENCHFFT_LDOUBLE */

static double dmax(double a, double b)
{
     return a > b ? a : b;
}

static double dmin(double a, double b)
{
     return a > b ? a : b;
}

static double cerror(bench_complex *A, bench_complex *B, unsigned int n)
{
     /* compute the relative error */
     double error = 0.0;
     unsigned int i;
     double mag = 0.0;

     for (i = 0; i < n; ++i) {
	  mag = dmax(mag,
		     dmin(hypot(c_re(A[i]), c_im(A[i])),
			  hypot(c_re(B[i]), c_im(B[i]))));
     }

     for (i = 0; i < n; ++i) {
	  double a;

	  a = hypot(c_re(A[i]) - c_re(B[i]), c_im(A[i]) - c_im(B[i])) / mag;
	  if (a > error) 
	       error = a;

#ifdef HAVE_ISNAN
	  BENCH_ASSERT(!isnan(a));
#endif
     }
     return error;
}

/* generate random inputs */
static void arand(bench_complex *A, unsigned int n)
{
     unsigned int i;

     for (i = 0; i < n; ++i) {
	  c_re(A[i]) = bench_drand();
	  c_im(A[i]) = bench_drand();
     }
}

/* make array real */
static void mkreal(bench_complex *A, unsigned int n)
{
     unsigned int i;

     for (i = 0; i < n; ++i) {
	  c_im(A[i]) = 0.0;
     }
}

static void assign_conj(bench_complex *Ac, bench_complex *A,
			unsigned int rank, unsigned int *n, int size)
{
     if (rank == 0) {
	  c_re(*Ac) = c_re(*A);
	  c_im(*Ac) = -c_im(*A);
     }
     else {
	  unsigned int i, n0 = n[0];
	  rank -= 1;
	  n += 1;
	  size /= n0;
	  assign_conj(Ac, A, rank, n, size);
	  for (i = 1; i < n0; ++i)
	       assign_conj(Ac + (n0 - i) * size, A + i * size, rank, n, size);
     }
}

/* make array hermitian */
static void mkhermitian(bench_complex *A, 
			unsigned int rank, unsigned int *n)
{
     if (rank == 0)
	  c_im(*A) = 0.0;
     else {
	  unsigned int i, n0 = n[0], size;
	  rank -= 1;
	  n += 1;
	  mkhermitian(A, rank, n);
	  for (i = 0, size = 1; i < rank; ++i)
	       size *= n[i];
	  for (i = 1; 2*i < n0; ++i)
	       assign_conj(A + (n0 - i) * size, A + i * size, rank, n, size);
	  if (2*i == n0)
	       mkhermitian(A + i*size, rank, n);
     }
}


/* C = A - B */
static void asub(bench_complex *C, bench_complex *A, bench_complex *B, unsigned int n)
{
     unsigned int i;

     for (i = 0; i < n; ++i) {
	  c_re(C[i]) = c_re(A[i]) - c_re(B[i]);
	  c_im(C[i]) = c_im(A[i]) - c_im(B[i]);
     }
}

/* B = rotate left A */
static void arol(bench_complex *B, bench_complex *A,
		 unsigned int n, unsigned int n_before, unsigned int n_after)
{
     unsigned int i, ib, ia;

     for (ib = 0; ib < n_before; ++ib) {
	  for (i = 0; i < n - 1; ++i)
	       for (ia = 0; ia < n_after; ++ia)
		    B[(ib * n + i) * n_after + ia] =
			 A[(ib * n + i + 1) * n_after + ia];

	  for (ia = 0; ia < n_after; ++ia)
	       B[(ib * n + n - 1) * n_after + ia] = A[ib * n * n_after + ia];
     }
}

#ifdef BENCHFFT_LDOUBLE
   typedef long double trigreal;
#  define COS cosl
#  define SIN sinl
#  define TAN tanl
#  define KTRIG(x) (x##L)
#else
   typedef double trigreal;
#  define COS cos
#  define SIN sin
#  define TAN tan
#  define KTRIG(x) (x)
#endif
#define K2PI KTRIG(6.2831853071795864769252867665590057683943388)

static void aphase_shift(bench_complex *B, bench_complex *A,
			 unsigned int n, 
			 unsigned int n_before, unsigned int n_after,
			 bench_real sign)
{
     unsigned int j, jb, ja;
     trigreal twopin;
     twopin = K2PI / n;

     for (jb = 0; jb < n_before; ++jb)
	  for (j = 0; j < n; ++j) {
	       trigreal s = sign * SIN(j * twopin);
	       trigreal c = COS(j * twopin);

	       for (ja = 0; ja < n_after; ++ja) {
		    unsigned int index = (jb * n + j) * n_after + ja;
		    c_re(B[index]) = c_re(A[index]) * c - c_im(A[index]) * s;
		    c_im(B[index]) = c_re(A[index]) * s + c_im(A[index]) * c;
	       }
	  }
}

static double acmp(bench_complex *A, bench_complex *B, unsigned int n, 
		   const char *test, double tol)
{
     double d = cerror(A, B, n);
     if (d > tol) {
	  fprintf(stderr, "Found relative error %e (%s)\n", d, test);
	  exit(EXIT_FAILURE);
     }
     return d;
}

static void do_fft(struct problem *p, bench_complex *in, bench_complex *out)
{
     problem_ccopy_from(p, in);
     doit(1, p);
     problem_ccopy_to(p, out);
}

/*
 * Implementation of the FFT tester described in
 *
 * Funda Ergün. Testing multivariate linear functions: Overcoming the
 * generator bottleneck. In Proceedings of the Twenty-Seventh Annual
 * ACM Symposium on the Theory of Computing, pages 407-416, Las Vegas,
 * Nevada, 29 May--1 June 1995.
 */

static double linear(struct problem *p,
		     bench_complex *inA,
		     bench_complex *inB,
		     bench_complex *inC,
		     bench_complex *outA,
		     bench_complex *outB,
		     bench_complex *outC,
		     bench_complex *tmp,
		     unsigned int rounds,
		     double tol)
{
     unsigned int N = p->size;
     unsigned int i;
     double e = 0.0;

     /* test 1: check linearity */
     for (i = 0; i < rounds; ++i) {
	  bench_complex alpha, beta;
	  c_re(alpha) = bench_drand();
	  c_re(beta) = bench_drand();

	  if (p->kind == PROBLEM_COMPLEX) {
	       c_im(alpha) = bench_drand();
	       c_im(beta) = bench_drand();
	  } else {
	       c_im(alpha) = c_im(beta) = 0;
	  }
	  arand(inA, N);
	  arand(inB, N);
	  do_fft(p, inA, outA);
	  do_fft(p, inB, outB);

	  cascale(outA, N, alpha);
	  cascale(outB, N, beta);
	  caadd(tmp, outA, outB, N);
	  cascale(inA, N, alpha);
	  cascale(inB, N, beta);
	  caadd(inC, inA, inB, N);
	  do_fft(p, inC, outC);

	  e = dmax(e, acmp(outC, tmp, N, "linearity", tol));
     }
     return e;
}

static double impulse(struct problem *p,
		      bench_complex *inA,
		      bench_complex *inB,
		      bench_complex *inC,
		      bench_complex *outA,
		      bench_complex *outB,
		      bench_complex *outC,
		      bench_complex *tmp,
		      unsigned int rounds,
		      double tol)
{
     unsigned int n = p->size;
     const bench_complex one = {1.0, 0.0};
     const bench_complex zero = {0.0, 0.0};
     unsigned int i;
     double e = 0.0;

     /* test 2: check that the unit impulse is transformed properly */
     caset(inA, n, zero);
     inA[0] = one;
     caset(outA, n, one);

     /* a simple test first, to help with debugging: */
     do_fft(p, inA, outB);
     e = dmax(e, acmp(outB, outA, n, "impulse response", tol));

     for (i = 0; i < rounds; ++i) {
	  arand(inB, n);
	  asub(inC, inA, inB, n);
	  do_fft(p, inB, outB);
	  do_fft(p, inC, outC);
	  caadd(tmp, outB, outC, n);
	  e = dmax(e, acmp(tmp, outA, n, "impulse response", tol));
     }
     return e;
}

enum { TIME_SHIFT, FREQ_SHIFT };

static double tf_shift(struct problem *p,
		       bench_complex *inA,
		       bench_complex *inB,
		       bench_complex *outA,
		       bench_complex *outB,
		       bench_complex *tmp,
		       unsigned int rounds,
		       double tol,
		       int which_shift)
{
     double sign;
     unsigned int n, n_before, n_after, dim;
     unsigned int i;
     double e = 0.0;

     n = p->size;
     sign = p->sign;

     /* test 3: check the time-shift property */
     /* the paper performs more tests, but this code should be fine too */

     n_before = 1;
     n_after = n;
     for (dim = 0; dim < p->rank; ++dim) {
	  unsigned int n_cur = p->n[dim];

	  n_after /= n_cur;

	  for (i = 0; i < rounds; ++i) {
	       arand(inA, n);

	       if (which_shift == TIME_SHIFT) {
		    if (p->kind == PROBLEM_REAL) 
			 mkreal(inA, n);

		    arol(inB, inA, n_cur, n_before, n_after);
		    do_fft(p, inA, outA);
		    do_fft(p, inB, outB);
		    aphase_shift(tmp, outB, n_cur, n_before, n_after, sign);
		    e = dmax(e, acmp(tmp, outA, n, "time shift", tol));
	       } else {
		    if (p->kind == PROBLEM_REAL) 
			 mkhermitian(inA, p->rank, p->n);

		    aphase_shift(inB, inA, n_cur, n_before, n_after, -sign);
		    do_fft(p, inA, outA);
		    do_fft(p, inB, outB);
		    arol(tmp, outB, n_cur, n_before, n_after);
		    e = dmax(e, acmp(tmp, outA, n, "freq shift", tol));
	       }
	  }

	  n_before *= n_cur;
     }
     return e;
}

static void do_verify(struct problem *p, unsigned int rounds, double tol)
{
     bench_complex *inA, *inB, *inC, *outA, *outB, *outC, *tmp;
     unsigned int n = p->size;
     double el, ei, es = 0.0;

     if (rounds == 0)
	  rounds = 20;  /* default value */

     inA = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     inB = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     inC = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     outA = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     outB = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     outC = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     tmp = (bench_complex *) bench_malloc(n * sizeof(bench_complex));

     el = linear(p, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     ei = impulse(p, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);

     if (p->kind == PROBLEM_COMPLEX || p->sign == -1)
	  es = tf_shift(p, inA, inB, outA, outB, tmp, rounds, tol, TIME_SHIFT);
     if (p->kind == PROBLEM_COMPLEX || p->sign == 1)
	  es = dmax(es, tf_shift(p, inA, inB, outA, outB, 
				 tmp, rounds, tol, FREQ_SHIFT));

     if (verbose)
	  ovtpvt("%g %g %g\n", el, ei, es);

     bench_free(tmp);
     bench_free(outC);
     bench_free(outB);
     bench_free(outA);
     bench_free(inC);
     bench_free(inB);
     bench_free(inA);
}

static void do_accuracy(struct problem *p)
{
     extern void mfft(unsigned int n, bench_complex *a, int sign);
     unsigned int n, i;
     bench_complex *a, *b;
     bench_real e1, n1, e2, n2, einf, ninf;

     /* only works for these cases */
     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->vrank == 0);
     BENCH_ASSERT(problem_power_of_two(p, 0) || problem_power_of_two(p, 1));

     n = p->n[0],
     a = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     b = (bench_complex *) bench_malloc(n * sizeof(bench_complex));
     
     arand(a, n);

     if (p->kind == PROBLEM_REAL) {
	  if (p->sign == -1) 
	       mkreal(a, n); 
	  else
	       mkhermitian(a, p->rank, p->n);
     }

     do_fft(p, a, b);
     mfft(n, a, p->sign);

     e1 = e2 = einf = 0.0;
     n1 = n2 = ninf = 0.0;
     for (i = 0; i < n; ++i) {

#         define DO(x1, x2, xinf, var) {			\
              bench_real d = var;				\
	      if (d < 0) d = -d;				\
	      x1 += d; x2 += d * d; if (d > xinf) xinf = d;	\
          }
	  DO(n1, n2, ninf, c_re(b[i]));
	  DO(n1, n2, ninf, c_im(b[i]));
	  DO(e1, e2, einf, c_re(a[i]) - c_re(b[i]));
	  DO(e1, e2, einf, c_im(a[i]) - c_im(b[i]));
#         undef DO
     }
     e1 /= n1;
     e2 = sqrt(e2 / n1);
     einf /= ninf;

     printf("%10d %6.2e %6.2e %6.2e\n",
	    n, (double)e1, (double)e2, (double)einf);
}

void verify(const char *param, int rounds, double tol)
{
     struct problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);
     do_verify(p, rounds, tol);
     done(p);
     problem_destroy(p);
}

void accuracy(const char *param)
{
     struct problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);
     do_accuracy(p);
     done(p);
     problem_destroy(p);
}
