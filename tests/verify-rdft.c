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

/* $Id: verify-rdft.c,v 1.8 2003-01-13 09:20:37 athena Exp $ */

#include "rdft.h"
#include "debug.h"
#include "verify.h"

typedef struct {
     plan *pln;
     const problem_rdft *p;
     const problem_rdft2 *p2;
     tensor *probsz;
     tensor *totalsz;
     tensor *pckdsz;
     tensor *pckdvecsz;
     tensor *probsz2;
     tensor *totalsz2;
     tensor *pckdsz2;
} info;


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

/* copy halfcomplex A[n] into packed-complex B[n], using output stride
   of A and input stride of B */
typedef struct {
     dotens2_closure k;
     uint n;
     int as;
     R *a;
     R *rb, *ib;
} cpyhc_closure;

static void cpyhc0(dotens2_closure *k_, 
		  int indxa, int ondxa, int indxb, int ondxb)
{
     cpyhc_closure *k = (cpyhc_closure *)k_;
     uint i, n = k->n;
     int as = k->as;
     R *a = k->a + ondxa, *rb = k->rb + indxb, *ib = k->ib + indxb;
     UNUSED(indxa); UNUSED(ondxb);

     if (n > 0) {
	  rb[0] = a[0];
	  ib[0] = 0;
     }
     for (i = 1; i < (n + 1) / 2; ++i) {
	  rb[2*i] = a[as*i];
	  ib[2*i] = a[as*(n-i)];
	  rb[2*(n-i)] = rb[2*i];
	  ib[2*(n-i)] = -ib[2*i];
     }
     if (2 * i == n) {
	  rb[2*i] = a[as*i];
	  ib[2*i] = 0;
     }
}

static void cpyhc(R *a, const tensor *sza, const tensor *vecsza, 
		  R *rb, R *ib, const tensor *szb)
{
     cpyhc_closure k;
     A(sza->rnk <= 1);
     k.k.apply = cpyhc0;
     k.n = X(tensor_sz(sza));
     if (!FINITE_RNK(sza->rnk) || sza->rnk == 0)
	  k.as = 0;
     else
	  k.as = sza->dims[0].os;
     k.a = a; k.rb = rb; k.ib = ib;
     X(dotens2)(vecsza, szb, &k.k);
}

/* icpyhc is the inverse of cpyhc, conjugated for hc2r transform */

static void icpyhc0(dotens2_closure *k_, 
		  int indxa, int ondxa, int indxb, int ondxb)
{
     cpyhc_closure *k = (cpyhc_closure *)k_;
     uint i, n = k->n;
     int as = k->as;
     R *a = k->a + indxa, *rb = k->rb + ondxb, *ib = k->ib + ondxb;
     UNUSED(ondxa); UNUSED(indxb);

     if (n > 0) {
	  a[0] = rb[0];
     }
     for (i = 1; i < (n + 1) / 2; ++i) {
	  a[as*i] = rb[2*i];
	  a[as*(n-i)] = -ib[2*i];
     }
     if (2 * i == n) {
	  a[as*i] = rb[2*i];
     }
}

static void icpyhc(R *a, const tensor *sza, const tensor *vecsza,
		   R *rb, R *ib, const tensor *szb)
{
     cpyhc_closure k;
     A(sza->rnk <= 1); /* TODO: support multidimensions? */
     k.k.apply = icpyhc0;
     k.n = X(tensor_sz(sza));
     if (!FINITE_RNK(sza->rnk) || sza->rnk == 0)
	  k.as = 0;
     else
	  k.as = sza->dims[0].is;
     k.a = a; k.rb = rb; k.ib = ib;
     X(dotens2)(vecsza, szb, &k.k);
}

/* copy unpacked halfcomplex A[n] into packed-complex B[n], using output stride
   of A and input stride of B */
typedef struct {
     dotens2_closure k;
     uint n;
     int as;
     R *ra, *ia;
     R *rb, *ib;
} cpyhc2_closure;

static void cpyhc20(dotens2_closure *k_, 
		  int indxa, int ondxa, int indxb, int ondxb)
{
     cpyhc2_closure *k = (cpyhc2_closure *)k_;
     uint i, n = k->n;
     int as = k->as;
     R *ra = k->ra + ondxa, *ia = k->ia + ondxa;
     R *rb = k->rb + indxb, *ib = k->ib + indxb;
     UNUSED(indxa); UNUSED(ondxb);

     if (n > 0) {
	  rb[0] = ra[0];
	  ib[0] = 0;
     }
     for (i = 1; i < (n + 1) / 2; ++i) {
	  rb[2*i] = ra[as*i];
	  ib[2*i] = ia[as*i];
	  rb[2*(n-i)] = rb[2*i];
	  ib[2*(n-i)] = -ib[2*i];
     }
     if (2 * i == n) {
	  rb[2*i] = ra[as*i];
	  ib[2*i] = 0;
     }
}

static void cpyhc2(R *ra, R *ia, const tensor *sza, const tensor *vecsza, 
		   R *rb, R *ib, const tensor *szb)
{
     cpyhc2_closure k;
     A(sza->rnk <= 1); /* TODO: support multidimensions? */
     k.k.apply = cpyhc20;
     k.n = X(tensor_sz(sza));
     if (!FINITE_RNK(sza->rnk) || sza->rnk == 0)
	  k.as = 0;
     else
	  k.as = sza->dims[0].os;
     k.ra = ra; k.ia = ia; k.rb = rb; k.ib = ib;
     X(dotens2)(vecsza, szb, &k.k);
}

/* icpyhc2 is the inverse of cpyhc2, conjugated for hc2r transform */

static void icpyhc20(dotens2_closure *k_, 
		  int indxa, int ondxa, int indxb, int ondxb)
{
     cpyhc2_closure *k = (cpyhc2_closure *)k_;
     uint i, n = k->n;
     int as = k->as;
     R *ra = k->ra + indxa, *ia = k->ia + indxa;
     R *rb = k->rb + ondxb, *ib = k->ib + ondxb;
     UNUSED(ondxa); UNUSED(indxb);

     if (n > 0) {
	  ra[0] = rb[0];
	  ia[0] = 0;
     }
     for (i = 1; i < (n + 1) / 2; ++i) {
	  ra[as*i] = rb[2*i];
	  ia[as*i] = -ib[2*i];
     }
     if (2 * i == n) {
	  ra[as*i] = rb[2*i];
	  ia[as*i] = 0;
     }
}

static void icpyhc2(R *ra, R *ia, const tensor *sza, const tensor *vecsza,
		    R *rb, R *ib, const tensor *szb)
{
     cpyhc2_closure k;
     A(sza->rnk <= 1);
     k.k.apply = icpyhc20;
     k.n = X(tensor_sz(sza));
     if (!FINITE_RNK(sza->rnk) || sza->rnk == 0)
	  k.as = 0;
     else
	  k.as = sza->dims[0].is;
     k.ra = ra; k.ia = ia; k.rb = rb; k.ib = ib;
     X(dotens2)(vecsza, szb, &k.k);
}

static void dofft(void *n_, C *in, C *out)
{
     info *n = (info *)n_;
     if (n->p) {
	  switch (n->p->kind[0]) {
	      case R2HC:
		   cpyr(&in->r, n->pckdsz, n->p->I, n->totalsz);
		   n->pln->adt->solve(n->pln, &(n->p->super));
		   cpyhc(n->p->O, n->probsz, n->p->vecsz, 
			 &out->r, &out->i, n->pckdvecsz);
		   break;
	      case HC2R:
		   icpyhc(n->p->I, n->probsz, n->p->vecsz, 
			  &in->r, &in->i, n->pckdvecsz);
		   n->pln->adt->solve(n->pln, &(n->p->super));
		   mkreal(out, X(tensor_sz)(n->pckdsz));
		   cpyr(n->p->O, n->totalsz, &out->r, n->pckdsz);
		   break;
	      default:
		   A(0); /* not implemented */
	  }
     }
     else if (n->p2) {
	  switch (n->p2->kind) {
	      case R2HC:
		   cpyr(&in->r, n->pckdsz, n->p2->r, n->totalsz);
		   n->pln->adt->solve(n->pln, &(n->p2->super));
		   cpyhc2(n->p2->rio, n->p2->iio, n->probsz2, n->totalsz2, 
			  &out->r, &out->i, n->pckdsz2);
		   break;
	      case HC2R:
		   icpyhc2(n->p2->rio, n->p2->iio, n->probsz2, n->totalsz2, 
			   &in->r, &in->i, n->pckdsz2);
		   n->pln->adt->solve(n->pln, &(n->p2->super));
		   mkreal(out, X(tensor_sz)(n->pckdsz));
		   cpyr(n->p2->r, n->totalsz, &out->r, n->pckdsz);
		   break;
	      default:
		   A(0); /* not implemented */
	  }
     }
     else {
	  A(0);
     }
}

/***************************************************************************/

static void really_verify(plan *pln, const problem_rdft *p, 
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

     inA = (C *) MALLOC(N * sizeof(C), OTHER);
     inB = (C *) MALLOC(N * sizeof(C), OTHER);
     inC = (C *) MALLOC(N * sizeof(C), OTHER);
     outA = (C *) MALLOC(N * sizeof(C), OTHER);
     outB = (C *) MALLOC(N * sizeof(C), OTHER);
     outC = (C *) MALLOC(N * sizeof(C), OTHER);
     tmp = (C *) MALLOC(N * sizeof(C), OTHER);

     nfo.pln = pln;
     nfo.p = p;
     nfo.p2 = 0;
     nfo.probsz = p->sz;
     nfo.totalsz = X(tensor_append)(p->vecsz, p->sz);
     nfo.pckdsz = verify_pack(nfo.totalsz, 2);
     nfo.pckdvecsz = verify_pack(p->vecsz, 2 * X(tensor_sz)(p->sz));
     nfo.probsz2 = nfo.totalsz2 = nfo.pckdsz2 = 0;

     impulse(dofft, &nfo, 
	     n, vecn, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     linear(dofft, &nfo, 1,
	    N, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);

     if (nfo.p->kind[0] == R2HC)
	  tf_shift(dofft, &nfo, 0, p->sz,
		   n, vecn, inA, inB, outA, outB, tmp, 
		   rounds, tol, TIME_SHIFT);
     if (nfo.p->kind[0] == HC2R)
	  tf_shift(dofft, &nfo, 0, p->sz,
		   n, vecn, inA, inB, outA, outB, tmp, 
		   rounds, tol, FREQ_SHIFT);

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

static void really_verify2(plan *pln, const problem_rdft2 *p, 
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

     inA = (C *) MALLOC(N * sizeof(C), OTHER);
     inB = (C *) MALLOC(N * sizeof(C), OTHER);
     inC = (C *) MALLOC(N * sizeof(C), OTHER);
     outA = (C *) MALLOC(N * sizeof(C), OTHER);
     outB = (C *) MALLOC(N * sizeof(C), OTHER);
     outC = (C *) MALLOC(N * sizeof(C), OTHER);
     tmp = (C *) MALLOC(N * sizeof(C), OTHER);

     nfo.pln = pln;
     nfo.p = 0;
     nfo.p2 = p;
     nfo.probsz = p->sz;
     nfo.totalsz = X(tensor_append)(p->vecsz, p->sz);
     nfo.pckdsz = verify_pack(nfo.totalsz, 2);
     nfo.pckdvecsz = verify_pack(p->vecsz, 2 * X(tensor_sz)(p->sz));
     nfo.probsz2 = X(tensor_copy_sub)(p->sz, p->sz->rnk - 1, 1);
     nfo.totalsz2 = X(tensor_copy_sub)(nfo.totalsz, 0, nfo.totalsz->rnk - 1);
     nfo.pckdsz2 = X(tensor_copy_sub)(nfo.pckdsz, 0, nfo.pckdsz->rnk - 1);

     impulse(dofft, &nfo, 
	     n, vecn, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     linear(dofft, &nfo, 1,
	    N, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);


     if (nfo.p2->kind == R2HC)
	  tf_shift(dofft, &nfo, 0, p->sz,
		   n, vecn, inA, inB, outA, outB, tmp, 
		   rounds, tol, TIME_SHIFT);
     if (nfo.p2->kind == HC2R)
	  tf_shift(dofft, &nfo, 0, p->sz,
		   n, vecn, inA, inB, outA, outB, tmp, 
		   rounds, tol, FREQ_SHIFT);

     X(tensor_destroy)(nfo.totalsz);
     X(tensor_destroy)(nfo.pckdsz);
     X(tensor_destroy)(nfo.pckdvecsz);
     X(tensor_destroy)(nfo.probsz2);
     X(tensor_destroy)(nfo.totalsz2);
     X(tensor_destroy)(nfo.pckdsz2);
     X(ifree)(tmp);
     X(ifree)(outC);
     X(ifree)(outB);
     X(ifree)(outA);
     X(ifree)(inC);
     X(ifree)(inB);
     X(ifree)(inA);
}

void X(rdft_verify)(plan *pln, const problem_rdft *p, uint rounds)
{
     if (p->sz->rnk == 1 && (p->kind[0] == R2HC || p->kind[0] == HC2R)) {
	  AWAKE(pln, 1);
	  really_verify(pln, p, rounds, 
			sizeof(R) == sizeof(float) ? 1.0e-2 : 1.0e-7);
	  AWAKE(pln, 0);
     }
}

void X(rdft2_verify)(plan *pln, const problem_rdft2 *p, uint rounds)
{
     if (p->kind == R2HC || p->kind == HC2R) {
	  AWAKE(pln, 1);
	  really_verify2(pln, p, rounds, 
			 sizeof(R) == sizeof(float) ? 1.0e-2 : 1.0e-7);
	  AWAKE(pln, 0);
     }
}
