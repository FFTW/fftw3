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

/* $Id: verify-reodft.c,v 1.12 2003-01-13 09:20:37 athena Exp $ */

#include "reodft.h"
#include "debug.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
     plan *pln;
     const problem_rdft *p;
     tensor *probsz;
     tensor *totalsz;
     tensor *pckdsz;
     tensor *pckdvecsz;
} info;

/*
 * Utility functions:
 */

static double dabs(double x) { return (x < 0.0) ? -x : x; }
static double dmax(double x, double y) { return (x > y) ? x : y; }
static double dmin(double x, double y) { return (x < y) ? x : y; }

static double aerror(R *a, R *b, uint n)
{
     if (n > 0) {
          /* compute the relative Linf error */
          double e = 0.0, mag = 0.0;
          uint i;

          for (i = 0; i < n; ++i) {
               e = dmax(e, dabs(a[i] - b[i]));
               mag = dmax(mag, dmin(dabs(a[i]), dabs(b[i])));
          }
	  if (mag == 0.0 && e == 0.0)
	       e = 0.0;
	  else
	       e /= mag;

#ifdef HAVE_ISNAN
          A(!isnan(e));
#endif
          return e;
     } else
          return 0.0;
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

static trigreal cos00(int i, int j, uint n)
{
     return X(cos2pi)(i * j, n);
}

static trigreal cos01(int i, int j, uint n)
{
     return cos00(i, 2*j + 1, 2*n);
}

static trigreal cos10(int i, int j, uint n)
{
     return cos00(2*i + 1, j, 2*n);
}

static trigreal cos11(int i, int j, uint n)
{
     return cos00(2*i + 1, 2*j + 1, 4*n);
}

static trigreal sin00(int i, int j, uint n)
{
     return X(sin2pi)(i * j, n);
}

static trigreal sin01(int i, int j, uint n)
{
     return sin00(i, 2*j + 1, 2*n);
}

static trigreal sin10(int i, int j, uint n)
{
     return sin00(2*i + 1, j, 2*n);
}

static trigreal sin11(int i, int j, uint n)
{
     return sin00(2*i + 1, 2*j + 1, 4*n);
}

typedef trigreal (*trigfun)(int, int, uint);

static void arand(R *a, uint n)
{
     uint i;

     /* generate random inputs */
     for (i = 0; i < n; ++i) {
	  a[i] = mydrand();
     }
}

/* C = A + B */
static void aadd(R *c, R *a, R *b, uint n)
{
     uint i;

     for (i = 0; i < n; ++i) {
	  c[i] = a[i] + b[i];
     }
}

/* C = A - B */
static void asub(R *c, R *a, R *b, uint n)
{
     uint i;

     for (i = 0; i < n; ++i) {
	  c[i] = a[i] - b[i];
     }
}

/* B = rotate left A + rotate right A */
static void arolr(R *b, R *a, uint n, uint nb, uint na, 
		  int isL0, int isL1, int isR0, int isR1)
{
     uint i, ib, ia;

     for (ib = 0; ib < nb; ++ib) {
	  for (i = 0; i < n - 1; ++i)
	       for (ia = 0; ia < na; ++ia)
		    b[(ib * n + i) * na + ia] =
			 a[(ib * n + i + 1) * na + ia];

	  for (ia = 0; ia < na; ++ia)
	       b[(ib * n + n - 1) * na + ia] = 
		    isR0 * a[(ib * n + n - 1) * na + ia]
		    + (n > 1 ? isR1 * a[(ib * n + n - 2) * na + ia] : 0);

	  for (i = 1; i < n; ++i)
	       for (ia = 0; ia < na; ++ia)
		    b[(ib * n + i) * na + ia] +=
			 a[(ib * n + i - 1) * na + ia];

	  for (ia = 0; ia < na; ++ia)
	       b[(ib * n) * na + ia] += 
		    isL0 * a[(ib * n) * na + ia]
		    + (n > 1 ? isL1 * a[(ib * n + 1) * na + ia] : 0);
     }
}

static void aphase_shift(R *b, R *a, uint n, uint nb, uint na,
			 uint n0, int k0, trigfun t)
{
     uint j, jb, ja;
 
     for (jb = 0; jb < nb; ++jb)
          for (j = 0; j < n; ++j) {
               trigreal c = 2.0 * t(1, j + k0, n0);

               for (ja = 0; ja < na; ++ja) {
                    uint k = (jb * n + j) * na + ja;
                    b[k] = a[k] * c;
               }
          }
}

/* A = alpha * A  (real, in place) */
static void ascale(R *a, R alpha, uint n)
{
     uint i;

     for (i = 0; i < n; ++i) {
	  a[i] *= alpha;
     }
}

/*
 * compute rdft:
 */

/* copy real A into real B, using output stride of A and input stride of B */
typedef struct {
     dotens2_closure k;
     R *ra;
     R *rb;
} cpyr_closure;

static void cpyr0(dotens2_closure *k_, 
		  int indxa, int ondxa, int indxb, int ondxb)
{
     cpyr_closure *k = (cpyr_closure *)k_;
     k->rb[indxb] = k->ra[ondxa];
     UNUSED(indxa); UNUSED(ondxb);
}

static void cpyr(R *ra, const tensor *sza, R *rb, const tensor *szb)
{
     cpyr_closure k;
     k.k.apply = cpyr0;
     k.ra = ra; k.rb = rb;
     X(dotens2)(sza, szb, &k.k);
}

static void dofft(info *n, R *in, R *out)
{
     cpyr(in, n->pckdsz, n->p->I, n->totalsz);
     n->pln->adt->solve(n->pln, &(n->p->super));
     cpyr(n->p->O, n->totalsz, out, n->pckdsz);
}

static double acmp(R *a, R *b, uint n, const char *test, double tol)
{
     double d = aerror(a, b, n);
     if (d > tol) {
	  fprintf(stderr, "Found relative error %e (%s)\n", d, test);
	  {
	       uint i;
	       for (i = 0; i < n; ++i)
		    fprintf(stderr, "%8d %16.12f   %16.12f\n", i, 
			    (double) a[i],
			    (double) b[i]);
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

static void linear(uint n, info *nfo, R *inA, R *inB, R *inC, R *outA,
		   R *outB, R *outC, R *tmp, uint rounds, double tol)
{
     uint j;

     for (j = 0; j < rounds; ++j) {
	  R alpha, beta;
	  alpha = mydrand();
	  beta = mydrand();
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

static void impulse(uint n0, int i0, int k0, trigfun t, R impulse_amp,
		    uint n, uint vecn, info *nfo, 
		    R *inA, R *inB, R *inC,
		    R *outA, R *outB, R *outC,
		    R *tmp, uint rounds, double tol)
{
     uint N = n * vecn;
     uint i;
     uint j;

     /* test 2: check that the unit impulse is transformed properly */

     for (i = 0; i < N; ++i) {
	  /* pls */
	  inA[i] = 0.0;
	  
	  /* transform of the pls */
	  outA[i] = impulse_amp * t(i0, (i % n) + k0, n0);
     }
     for (i = 0; i < vecn; ++i)
	  inA[i * n] = 1.0;

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

enum { TIME_SHIFT, FREQ_SHIFT };

static void tf_shift(uint n, uint vecn, info *nfo, 
		     R *inA, R *inB, R *outA, R *outB, R *tmp,
		     uint rounds, double tol, int which_shift,
		     int isL0, int isL1, int isR0, int isR1, 
		     uint n0, int k0, trigfun t)
{
     double sign;
     uint nb, na, dim, N = n * vecn;
     uint i, j;
     const tensor *sz = nfo->probsz;

     sign = -1.0;

     /* test 3: check the time-shift property */
     /* the paper performs more tests, but this code should be fine too */

     nb = 1;
     na = n;

     /* check shifts across all SZ dimensions */
     for (dim = 0; dim < sz->rnk; ++dim) {
	  uint ncur = sz->dims[dim].n;

	  na /= ncur;

	  for (j = 0; j < rounds; ++j) {
	       arand(inA, N);

	       if (which_shift == TIME_SHIFT) {
		    for (i = 0; i < vecn; ++i) {
			 arolr(inB + i * n, inA + i*n, ncur, nb,na, 
			       isL0, isL1, isR0, isR1);
		    }
		    dofft(nfo, inA, outA);
		    dofft(nfo, inB, outB);
		    for (i = 0; i < vecn; ++i) 
			 aphase_shift(tmp + i * n, outA + i * n, ncur, 
				      nb, na, n0, k0, t);
		    acmp(tmp, outB, N, "time shift", tol);
	       } else {
		    for (i = 0; i < vecn; ++i) {
			 aphase_shift(inB + i * n, inA + i * n, ncur,
				      nb, na, n0, k0, t);
		    }
		    dofft(nfo, inA, outA);
		    dofft(nfo, inB, outB);
		    for (i = 0; i < vecn; ++i) 
			 arolr(tmp + i * n, outA + i*n, ncur, nb,na,
			       isL0, isL1, isR0, isR1);
		    acmp(tmp, outB, N, "freq shift", tol);
	       }
	  }

	  nb *= ncur;
     }
}


/* Make a copy of the size tensor, with the same dimensions, but with
   the strides corresponding to a "packed" row-major array with the
   given stride. */
static tensor *pack(const tensor *sz, int s)
{
     tensor *x = X(tensor_copy)(sz);
     if (FINITE_RNK(x->rnk) && x->rnk > 0) {
	  uint i;
	  x->dims[x->rnk - 1].is = s;
	  x->dims[x->rnk - 1].os = s;
	  for (i = x->rnk - 1; i > 0; --i) {
	       x->dims[i - 1].is = x->dims[i].is * x->dims[i].n;
	       x->dims[i - 1].os = x->dims[i].os * x->dims[i].n;
	  }
     }
     return x;
}

static void really_verify(plan *pln, const problem_rdft *p, 
			  uint rounds, double tol)
{
     R *inA, *inB, *inC, *outA, *outB, *outC, *tmp;
     info nfo;
     uint n, vecn, N;
     uint n0;
     int i0, k0;
     int isL0t = 0, isL1t = 0, isR0t = 0, isR1t = 0;
     int isL0f = 0, isL1f = 0, isR0f = 0, isR1f = 0;
     double impulse_amp = 1.0;
     trigfun ti, tst, tsf;

     if (rounds == 0)
	  rounds = 20;  /* default value */

     A(p->sz->rnk == 1);
     n = p->sz->dims[0].n;
     n0 = 2 * (n + (p->kind[0] == REDFT00 ? -1 : 
		    (p->kind[0] == RODFT00 ? 1 : 0)));
     vecn = X(tensor_sz)(p->vecsz);
     N = n * vecn;

     switch (p->kind[0]) {
	 case REDFT00:
	      isL1t = isR1t = isL1f = isR1f = 1;
	      i0 = k0 = 0;
	      ti = tst = tsf = cos00;
	      break;
	 case REDFT01:
	      isL1t = isL0f = isR0f = 1;
	      i0 = k0 = 0;
	      ti = tst = cos01;
	      tsf = cos00;
	      break;
	 case REDFT10:
	      isL1f = isL0t = isR0t = 1;
	      i0 = k0 = 0;
	      ti = cos10; impulse_amp = 2.0;
	      tst = cos00;
	      tsf = cos01;
	      break;
	 case REDFT11:
	      isL0t = isL0f = 1;
	      isR0t = isR0f = -1;
	      i0 = k0 = 0;
	      ti = cos11; impulse_amp = 2.0;
	      tst = tsf = cos01;
	      break;
	 case RODFT00:
	      i0 = k0 = 1;
	      ti = sin00; impulse_amp = 2.0;
	      tst = tsf = cos00;
	      break;
	 case RODFT01:
	      isR1t = 1;
	      isL0f = isR0f = -1;
	      i0 = 1; k0 = 0;
	      ti = sin01; impulse_amp = n == 1 ? 1.0 : 2.0;
	      tst = cos01;
	      tsf = cos00;
	      break;
	 case RODFT10:
	      isR1f = 1;
	      isL0t = isR0t = -1;
	      i0 = 0; k0 = 1;
	      ti = sin10; impulse_amp = 2.0;
	      tst = cos00;
	      tsf = cos01;
	      break;
	 case RODFT11:
	      isL0t = isL0f = -1;
	      isR0t = isR0f = 1;
	      i0 = k0 = 0;
	      ti = sin11; impulse_amp = 2.0;
	      tst = tsf = cos01;
	      break;
	 default:
	      A(0);
	      return;
     }

     inA = (R *) MALLOC(N * sizeof(R), OTHER);
     inB = (R *) MALLOC(N * sizeof(R), OTHER);
     inC = (R *) MALLOC(N * sizeof(R), OTHER);
     outA = (R *) MALLOC(N * sizeof(R), OTHER);
     outB = (R *) MALLOC(N * sizeof(R), OTHER);
     outC = (R *) MALLOC(N * sizeof(R), OTHER);
     tmp = (R *) MALLOC(N * sizeof(R), OTHER);

     nfo.pln = pln;
     nfo.p = p;
     nfo.probsz = p->sz;
     nfo.totalsz = X(tensor_append)(p->vecsz, nfo.probsz);
     nfo.pckdsz = pack(nfo.totalsz, 1);
     nfo.pckdvecsz = pack(p->vecsz, X(tensor_sz)(nfo.probsz));

     linear(N, &nfo, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     impulse(n0, i0, k0, ti, impulse_amp,
	     n, vecn, &nfo, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     tf_shift(n, vecn, &nfo, inA, inB, outA, outB, tmp, 
	      rounds, tol, TIME_SHIFT,
	      isL0t, isL1t, isR0t, isR1t, n0, k0, tst);

     tf_shift(n, vecn, &nfo, inA, inB, outA, outB, tmp, 
	      rounds, tol, FREQ_SHIFT, 
	      isL0f, isL1f, isR0f, isR1f, n0, i0, tsf);

     X(tensor_destroy)(nfo.totalsz);
     X(tensor_destroy)(nfo.pckdsz);
     X(tensor_destroy)(nfo.pckdvecsz);
     X(ifree)(tmp);
     X(ifree)(outC);
     X(ifree)(outB);
     X(ifree)(outA);
     X(ifree)(inC);
     X(ifree)(inB);
     X(ifree)(inA);
}

void X(reodft_verify)(plan *pln, const problem_rdft *p, uint rounds)
{
     if (p->sz->rnk == 1 && REODFT_KINDP(p->kind[0])) {
	  AWAKE(pln, 1);
	  really_verify(pln, p, rounds, 
			sizeof(R) == sizeof(float) ? 1.0e-2 : 1.0e-7);
	  AWAKE(pln, 0);
     }
}
