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

/* $Id: verify.c,v 1.6 2002-08-09 23:26:57 athena Exp $ */

#include "dft.h"
#include <math.h>
#include <stdlib.h>

typedef struct {
     R r;
     R i;
} C;

typedef struct {
     plan *pln;
     const problem_dft *p;
     tensor probsz;
     tensor totalsz;
     tensor pckdsz;
} info;

/*
 * Utility functions:
 */
#ifdef FFTW_LDOUBLE
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
#else /* !FFTW_LDOUBLE */
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
#endif /* !FFTW_LDOUBLE */

static inline double cerror(C a, C b, double tol)
{
     double x;
     double mag;
     x = hypot(a.r - b.r, a.i - b.i);
     mag = 0.5 * (hypot(a.r, a.i) + hypot(b.r, b.i)) + tol;
     x /= mag;
#ifdef HAVE_ISNAN
     A(!isnan(x));
#endif
     return x;
}

static double aerror(C *a, C *b, uint n, double tol)
{
     /* compute the relative error */
     double e = 0.0;
     uint i;

     for (i = 0; i < n; ++i) {
	  double x = cerror(a[i], b[i], tol);
	  if (x > e) e = x;
     }
     return e;
}

#ifdef HAVE_DRAND48
static double mydrand(void)
{
     return drand48() - 0.5;
}
#else
static double mydrand(void)
{
     double d = rand();
     return (d / (double) RAND_MAX) - 0.5;
}
#endif

static void arand(C *a, uint n)
{
     uint i;

     /* generate random inputs */
     for (i = 0; i < n; ++i) {
	  a[i].r = mydrand();
	  a[i].i = mydrand();
     }
}

/* C = A + B */
static void aadd(C *c, C *a, C *b, uint n)
{
     uint i;

     for (i = 0; i < n; ++i) {
	  c[i].r = a[i].r + b[i].r;
	  c[i].i = a[i].i + b[i].i;
     }
}

/* C = A - B */
static void asub(C *c, C *a, C *b, uint n)
{
     uint i;

     for (i = 0; i < n; ++i) {
	  c[i].r = a[i].r - b[i].r;
	  c[i].i = a[i].i - b[i].i;
     }
}

/* B = rotate left A (complex) */
static void arol(C *b, C *a, uint n, uint nb, uint na)
{
     uint i, ib, ia;

     for (ib = 0; ib < nb; ++ib) {
	  for (i = 0; i < n - 1; ++i)
	       for (ia = 0; ia < na; ++ia)
		    b[(ib * n + i) * na + ia] =
			 a[(ib * n + i + 1) * na + ia];

	  for (ia = 0; ia < na; ++ia)
	       b[(ib * n + n - 1) * na + ia] = a[ib * n * na + ia];
     }
}

static void aphase_shift(C *b, C *a, uint n, uint nb, uint na, double sign)
{
     uint j, jb, ja;

     for (jb = 0; jb < nb; ++jb)
	  for (j = 0; j < n; ++j) {
	       trigreal c = X(cos2pi)(j, n);
	       trigreal s = sign * X(sin2pi)(j, n);

	       for (ja = 0; ja < na; ++ja) {
		    uint k = (jb * n + j) * na + ja;
		    b[k].r = a[k].r * c - a[k].i * s;
		    b[k].i = a[k].r * s + a[k].i * c;
	       }
	  }
}

/* A = alpha * A  (complex, in place) */
static void ascale(C *a, C alpha, uint n)
{
     uint i;

     for (i = 0; i < n; ++i) {
	  C x = a[i];
	  a[i].r = x.r * alpha.r - x.i * alpha.i;
	  a[i].i = x.r * alpha.i + x.i * alpha.r;
     }
}

/*
 * compute dft:
 */

/* copy A into B, using output stride of A and input stride of B */
typedef struct {
     dotens2_closure k;
     R *ra; R *ia;
     R *rb; R *ib;
} cpy_closure;

static void cpy0(dotens2_closure *k_, 
		 int indxa, int ondxa, int indxb, int ondxb)
{
     cpy_closure *k = (cpy_closure *)k_;
     k->rb[indxb] = k->ra[ondxa];
     k->ib[indxb] = k->ia[ondxa];
     UNUSED(indxa); UNUSED(ondxb);
}

static void cpy(R *ra, R *ia, tensor sza, R *rb, R *ib, tensor szb)
{
     cpy_closure k;
     k.k.apply = cpy0;
     k.ra = ra; k.ia = ia; k.rb = rb; k.ib = ib;
     X(dotens2)(sza, szb, &k.k);
}

static void dofft(info *n, C *in, C *out)
{
     cpy(&in->r, &in->i, n->pckdsz, n->p->ri, n->p->ii, n->totalsz);
     n->pln->adt->solve(n->pln, &(n->p->super));
     cpy(n->p->ro, n->p->io, n->totalsz, &out->r, &out->i, n->pckdsz);
}

static double acmp(info *nfo, C *a, C *b, uint n, const char *test, double tol)
{
     double d = aerror(a, b, n, tol);
     if (d > tol) {
	  fprintf(stderr, "Found relative error %e (%s)\n", d, test);
	  D("%P\n", nfo->p);
	  D("%p\n", nfo->pln);
	  {
	       uint i;
	       for (i = 0; i < n; ++i) 
		    printf("%8d %16.12f %16.12f   %16.12f %16.12f %e\n", i, 
			   (double) a[i].r, (double) a[i].i,
			   (double) b[i].r, (double) b[i].i,
			   cerror(a[i], b[i], tol));
	  }
	  exit(EXIT_FAILURE);
     }
     return d;
}

/***************************************************************************/

/*
 * Implementation of the FFT tester described in
 *
 * Funda Ergün. Testing multivariate linear functions: Overcoming the
 * generator bottleneck. In Proceedings of the Twenty-Seventh Annual
 * ACM Symposium on the Theory of Computing, pages 407-416, Las Vegas,
 * Nevada, 29 May--1 June 1995.
 */

static void linear(uint n, info *nfo, C *inA, C *inB, C *inC, C *outA,
		   C *outB, C *outC, C *tmp, uint rounds, double tol)
{
     uint j;

     for (j = 0; j < rounds; ++j) {
	  C alpha, beta;
	  alpha.r = mydrand();
	  alpha.i = mydrand();
	  beta.r = mydrand();
	  beta.i = mydrand();
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

	  acmp(nfo, outC, tmp, n, "linear", tol);
     }

}

static void impulse(uint n, uint vecn, info *nfo, 
		    C *inA, C *inB, C *inC,
		    C *outA, C *outB, C *outC,
		    C *tmp, uint rounds, double tol)
{
     uint N = n * vecn;
     C pls;
     uint i;
     uint j;

     /* test 2: check that the unit impulse is transformed properly */

     pls.r = 1.0;
     pls.i = 0.0;
     
     for (i = 0; i < N; ++i) {
	  /* pls */
	  inA[i].r = inA[i].i = 0.0;
	  
	  /* transform of the pls */
	  outA[i] = pls;
     }
     for (i = 0; i < vecn; ++i)
	  inA[i * n] = pls;

     for (j = 0; j < rounds; ++j) {
	  arand(inB, N);
	  asub(inC, inA, inB, N);
	  dofft(nfo, inB, outB);
	  dofft(nfo, inC, outC);
	  aadd(tmp, outB, outC, N);
	  acmp(nfo, tmp, outA, N, "impulse", tol);
     }
}


enum { TIME_SHIFT, FREQ_SHIFT };

static void tf_shift(uint n, uint vecn, info *nfo, 
		     C *inA, C *inB, C *outA, C *outB, C *tmp,
		     uint rounds, double tol, int which_shift)
{
     double sign;
     uint nb, na, dim, N = n * vecn;
     uint i, j;
     tensor sz = nfo->probsz;

     sign = -1.0;

     /* test 3: check the time-shift property */
     /* the paper performs more tests, but this code should be fine too */

     nb = 1;
     na = n;

     /* check shifts across all SZ dimensions */
     for (dim = 0; dim < sz.rnk; ++dim) {
	  uint ncur = sz.dims[dim].n;

	  na /= ncur;

	  for (j = 0; j < rounds; ++j) {
	       arand(inA, N);

	       if (which_shift == TIME_SHIFT) {
		    for (i = 0; i < vecn; ++i) 
			 arol(inB + i * n, inA + i * n, ncur, nb, na);
		    dofft(nfo, inA, outA);
		    dofft(nfo, inB, outB);
		    for (i = 0; i < vecn; ++i) 
			 aphase_shift(tmp + i * n, outB + i * n, ncur, 
				      nb, na, sign);
		    acmp(nfo, tmp, outA, N, "time shift", tol);
	       } else {
		    for (i = 0; i < vecn; ++i) 
			 aphase_shift(inB + i * n, inA + i * n, ncur,
				      nb, na, -sign);
		    dofft(nfo, inA, outA);
		    dofft(nfo, inB, outB);
		    for (i = 0; i < vecn; ++i) 
			 arol(tmp + i * n, outB + i * n, ncur, nb, na);
		    acmp(nfo, tmp, outA, N, "freq shift", tol);
	       }
	  }

	  nb *= ncur;
     }
}


/* Make a copy of the size tensor, with the same dimensions, but with
   the strides corresponding to a "packed" row-major array with the
   given stride. */
static tensor pack(tensor sz, int s)
{
     tensor x = X(tensor_copy)(sz);
     if (FINITE_RNK(x.rnk) && x.rnk > 0) {
	  uint i;
	  x.dims[x.rnk - 1].is = s;
	  x.dims[x.rnk - 1].os = s;
	  for (i = x.rnk - 1; i > 0; --i) {
	       x.dims[i - 1].is = x.dims[i].is * x.dims[i].n;
	       x.dims[i - 1].os = x.dims[i].os * x.dims[i].n;
	  }
     }
     return x;
}

static void really_verify(plan *pln, const problem_dft *p, 
			  uint rounds, double tol)
{
     C *inA, *inB, *inC, *outA, *outB, *outC, *tmp;
     info nfo;
     uint n, vecn, N;

     if (rounds == 0)
	  rounds = 20;  /* default value */

     n = X(tensor_sz)(p->sz);
     vecn = X(tensor_sz)(p->vecsz);
     N = n * vecn;

     inA = (C *) fftw_malloc(N * sizeof(C), OTHER);
     inB = (C *) fftw_malloc(N * sizeof(C), OTHER);
     inC = (C *) fftw_malloc(N * sizeof(C), OTHER);
     outA = (C *) fftw_malloc(N * sizeof(C), OTHER);
     outB = (C *) fftw_malloc(N * sizeof(C), OTHER);
     outC = (C *) fftw_malloc(N * sizeof(C), OTHER);
     tmp = (C *) fftw_malloc(N * sizeof(C), OTHER);

     nfo.pln = pln;
     nfo.p = p;
     nfo.probsz = p->sz;
     nfo.totalsz = X(tensor_append)(p->vecsz, p->sz);
     nfo.pckdsz = pack(nfo.totalsz, 2);

     impulse(n, vecn, &nfo, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     linear(N, &nfo, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);

     tf_shift(n, vecn, &nfo, inA, inB, outA, outB, tmp, 
	      rounds, tol, TIME_SHIFT);
     tf_shift(n, vecn, &nfo, inA, inB, outA, outB, tmp, 
	      rounds, tol, FREQ_SHIFT);

     X(tensor_destroy)(nfo.totalsz);
     X(tensor_destroy)(nfo.pckdsz);
     X(free)(tmp);
     X(free)(outC);
     X(free)(outB);
     X(free)(outA);
     X(free)(inC);
     X(free)(inB);
     X(free)(inA);
}

void X(dft_verify)(plan *pln, const problem_dft *p, uint rounds)
{
     AWAKE(pln, 1);
     really_verify(pln, p, rounds, 
		   sizeof(R) == sizeof(float) ? 1.0e-2 : 1.0e-7);
     AWAKE(pln, 0);
}
