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

/* $Id: verify-lib.c,v 1.1 2003-01-18 12:20:18 athena Exp $ */

#include "verify.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Utility functions:
 */
static double dabs(double x) { return (x < 0.0) ? -x : x; }
static double dmax(double x, double y) { return (x > y) ? x : y; }
static double dmin(double x, double y) { return (x < y) ? x : y; }
static double norm2(double x, double y) { return dmax(dabs(x), dabs(y)); }

static double aerror(C *a, C *b, int n)
{
     if (n > 0) {
	  /* compute the relative Linf error */
	  double e = 0.0, mag = 0.0;
	  int i;

	  for (i = 0; i < n; ++i) {
	       e = dmax(e, norm2(c_re(a[i]) - c_re(b[i]),
				 c_im(a[i]) - c_im(b[i])));
	       mag = dmax(mag, 
			  dmin(norm2(c_re(a[i]), c_im(a[i])),
			       norm2(c_re(b[i]), c_im(b[i]))));
	  }
	  e /= mag;

#ifdef HAVE_ISNAN
	  BENCH_ASSERT(!isnan(e));
#endif
	  return e;
     } else
	  return 0.0;
}

#ifdef HAVE_DRAND48
#  if defined(HAVE_DECL_DRAND48) && !HAVE_DECL_DRAND48
extern double drand48(void);
#  endif
double mydrand(void)
{
     return drand48() - 0.5;
}
#else
double mydrand(void)
{
     double d = rand();
     return (d / (double) RAND_MAX) - 0.5;
}
#endif

void arand(C *a, int n)
{
     int i;

     /* generate random inputs */
     for (i = 0; i < n; ++i) {
	  c_re(a[i]) = mydrand();
	  c_im(a[i]) = mydrand();
     }
}

/* make array real */
void mkreal(C *A, int n)
{
     int i;

     for (i = 0; i < n; ++i) {
          c_im(A[i]) = 0.0;
     }
}

static void assign_conj(C *Ac, C *A, int rank, const iodim *dim, int size)
{
     if (rank == 0) {
          c_re(*Ac) = c_re(*A);
          c_im(*Ac) = -c_im(*A);
     }
     else {
          int i, n0 = dim[0].n;
          rank -= 1;
          dim += 1;
          size /= n0;
          assign_conj(Ac, A, rank, dim, size);
          for (i = 1; i < n0; ++i)
               assign_conj(Ac + (n0 - i) * size, A + i * size, rank, dim,size);
     }
}

/* make array hermitian */
void mkhermitian(C *A, int rank, const iodim *dim)
{
     if (rank == 0)
          c_im(*A) = 0.0;
     else {
          int i, n0 = dim[0].n, size;
          rank -= 1;
          dim += 1;
          mkhermitian(A, rank, dim);
          for (i = 0, size = 1; i < rank; ++i)
               size *= dim[i].n;
          for (i = 1; 2*i < n0; ++i)
               assign_conj(A + (n0 - i) * size, A + i * size, rank, dim, size);
          if (2*i == n0)
               mkhermitian(A + i*size, rank, dim);
     }
}

/* C = A + B */
void aadd(C *c, C *a, C *b, int n)
{
     int i;

     for (i = 0; i < n; ++i) {
	  c_re(c[i]) = c_re(a[i]) + c_re(b[i]);
	  c_im(c[i]) = c_im(a[i]) + c_im(b[i]);
     }
}

/* C = A - B */
void asub(C *c, C *a, C *b, int n)
{
     int i;

     for (i = 0; i < n; ++i) {
	  c_re(c[i]) = c_re(a[i]) - c_re(b[i]);
	  c_im(c[i]) = c_im(a[i]) - c_im(b[i]);
     }
}

/* B = rotate left A (complex) */
void arol(C *b, C *a, int n, int nb, int na)
{
     int i, ib, ia;

     for (ib = 0; ib < nb; ++ib) {
	  for (i = 0; i < n - 1; ++i)
	       for (ia = 0; ia < na; ++ia) {
		    C *pb = b + (ib * n + i) * na + ia;
		    C *pa = a + (ib * n + i + 1) * na + ia;
		    c_re(*pb) = c_re(*pa);
		    c_im(*pb) = c_im(*pa);
	       }

	  for (ia = 0; ia < na; ++ia) {
	       C *pb = b + (ib * n + n - 1) * na + ia;
	       C *pa = a + ib * n * na + ia;
	       c_re(*pb) = c_re(*pa);
	       c_im(*pb) = c_im(*pa);
	  }
     }
}

void aphase_shift(C *b, C *a, int n, int nb, int na, double sign)
{
     int j, jb, ja;

     for (jb = 0; jb < nb; ++jb)
	  for (j = 0; j < n; ++j) {
	       double c = cos((2*M_PI*j)/n);
	       double s = sign * sin((2*M_PI*j)/n);

	       for (ja = 0; ja < na; ++ja) {
		    int k = (jb * n + j) * na + ja;
		    c_re(b[k]) = c_re(a[k]) * c - c_im(a[k]) * s;
		    c_im(b[k]) = c_re(a[k]) * s + c_im(a[k]) * c;
	       }
	  }
}

/* A = alpha * A  (complex, in place) */
void ascale(C *a, C alpha, int n)
{
     int i;

     for (i = 0; i < n; ++i) {
	  R xr = c_re(a[i]), xi = c_im(a[i]);
	  c_re(a[i]) = xr * c_re(alpha) - xi * c_im(alpha);
	  c_im(a[i]) = xr * c_im(alpha) + xi * c_re(alpha);
     }
}


double acmp(C *a, C *b, int n, const char *test, double tol)
{
     double d = aerror(a, b, n);
     if (d > tol) {
	  fprintf(stderr, "Found relative error %e (%s)\n", d, test);

	  {
	       int i;
	       for (i = 0; i < n; ++i) 
		    fprintf(stderr,
			    "%8d %16.12f %16.12f   %16.12f %16.12f\n", i, 
			   (double) c_re(a[i]), (double) c_im(a[i]),
			   (double) c_re(b[i]), (double) c_im(b[i]));
	  }

	  exit(EXIT_FAILURE);
     }
     return d;
}


/*
 * Implementation of the FFT tester described in
 *
 * Funda Ergün. Testing multivariate linear functions: Overcoming the
 * generator bottleneck. In Proceedings of the Twenty-Seventh Annual
 * ACM Symposium on the Theory of Computing, pages 407-416, Las Vegas,
 * Nevada, 29 May--1 June 1995.
 */

static void impulse0(void (*dofft)(void *nfo, C *in, C *out),
		     void *nfo, 
		     int n, int vecn, 
		     C *inA, C *inB, C *inC,
		     C *outA, C *outB, C *outC,
		     C *tmp, int rounds, double tol)
{
     int N = n * vecn;
     int j;

     dofft(nfo, inA, tmp);
     acmp(tmp, outA, N, "impulse 1", tol);

     for (j = 0; j < rounds; ++j) {
	  arand(inB, N);
	  asub(inC, inA, inB, N);
	  dofft(nfo, inB, outB);
	  dofft(nfo, inC, outC);
	  aadd(tmp, outB, outC, N);
	  acmp(tmp, outA, N, "impulse", tol);
     }
}

void impulse(void (*dofft)(void *nfo, C *in, C *out),
	     void *nfo, 
	     int n, int vecn, 
	     C *inA, C *inB, C *inC,
	     C *outA, C *outB, C *outC,
	     C *tmp, int rounds, double tol)
{
     int N = n * vecn;
     C pls;
     int i;

     /* check that the unit impulse is transformed properly */
     c_re(pls) = 1.0;
     c_im(pls) = 0.0;
     
     for (i = 0; i < N; ++i) {
	  /* pls */
	  c_re(inA[i]) = c_im(inA[i]) = 0.0;
	  CASSIGN(outA[i], pls);
     }
     for (i = 0; i < vecn; ++i)
	  CASSIGN(inA[i * n], pls);

     impulse0(dofft, nfo, n, vecn, inA, inB, inC, outA, outB, outC,
	      tmp, rounds, tol);

     c_re(pls) = n;
     for (i = 0; i < vecn; ++i)
	  CASSIGN(inA[i * n], pls);

     impulse0(dofft, nfo, n, vecn, outA, inB, inC, inA, outB, outC,
	      tmp, rounds, tol);
}

void linear(void (*dofft)(void *nfo, C *in, C *out),
	    void *nfo, int realp,
	    int n, C *inA, C *inB, C *inC, C *outA,
	    C *outB, C *outC, C *tmp, int rounds, double tol)
{
     int j;

     for (j = 0; j < rounds; ++j) {
	  C alpha, beta;
	  c_re(alpha) = mydrand();
	  c_im(alpha) = realp ? 0.0 : mydrand();
	  c_re(beta) = mydrand();
	  c_im(beta) = realp ? 0.0 : mydrand();
	  arand(inA, n);
	  arand(inB, n);
	  dofft(nfo, inA, outA);
	  dofft(nfo, inB, outB);

	  ascale(outA, alpha, n);
	  ascale(outB, beta, n);
	  aadd(tmp, outA, outB, n);
	  ascale(inA, alpha, n);
	  ascale(inB, beta, n);
	  aadd(inC, inA, inB, n);
	  dofft(nfo, inC, outC);

	  acmp(outC, tmp, n, "linear", tol);
     }

}



void tf_shift(void (*dofft)(void *nfo, C *in, C *out),
	      void *nfo, int realp, const tensor *sz,
	      int n, int vecn, double sign,
	      C *inA, C *inB, C *outA, C *outB, C *tmp,
	      int rounds, double tol, int which_shift)
{
     int nb, na, dim, N = n * vecn;
     int i, j;

     /* test 3: check the time-shift property */
     /* the paper performs more tests, but this code should be fine too */

     nb = 1;
     na = n;

     /* check shifts across all SZ dimensions */
     for (dim = 0; dim < sz->rnk; ++dim) {
	  int ncur = sz->dims[dim].n;

	  na /= ncur;

	  for (j = 0; j < rounds; ++j) {
	       arand(inA, N);

	       if (which_shift == TIME_SHIFT) {
		    for (i = 0; i < vecn; ++i) {
			 if (realp) mkreal(inA + i * n, n);
			 arol(inB + i * n, inA + i * n, ncur, nb, na);
		    }
		    dofft(nfo, inA, outA);
		    dofft(nfo, inB, outB);
		    for (i = 0; i < vecn; ++i) 
			 aphase_shift(tmp + i * n, outB + i * n, ncur, 
				      nb, na, sign);
		    acmp(tmp, outA, N, "time shift", tol);
	       } else {
		    for (i = 0; i < vecn; ++i) {
			 if (realp) 
			      mkhermitian(inA + i * n, sz->rnk, sz->dims);
			 aphase_shift(inB + i * n, inA + i * n, ncur,
				      nb, na, -sign);
		    }
		    dofft(nfo, inA, outA);
		    dofft(nfo, inB, outB);
		    for (i = 0; i < vecn; ++i) 
			 arol(tmp + i * n, outB + i * n, ncur, nb, na);
		    acmp(tmp, outA, N, "freq shift", tol);
	       }
	  }

	  nb *= ncur;
     }
}


/* Make a copy of the size tensor, with the same dimensions, but with
   the strides corresponding to a "packed" row-major array with the
   given stride. */
tensor *verify_pack(const tensor *sz, int s)
{
     tensor *x = tensor_copy(sz);
     if (FINITE_RNK(x->rnk) && x->rnk > 0) {
	  int i;
	  x->dims[x->rnk - 1].is = s;
	  x->dims[x->rnk - 1].os = s;
	  for (i = x->rnk - 1; i > 0; --i) {
	       x->dims[i - 1].is = x->dims[i].is * x->dims[i].n;
	       x->dims[i - 1].os = x->dims[i].os * x->dims[i].n;
	  }
     }
     return x;
}
