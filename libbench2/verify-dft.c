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

/* $Id: verify-dft.c,v 1.1 2003-01-18 12:20:18 athena Exp $ */

#include "verify.h"

typedef struct {
     problem *p;
     tensor *probsz;
     tensor *totalsz;
     tensor *pckdsz;
} info;


/*
 * compute dft:
 */

/* copy A into B, using output stride of A and input stride of B */
typedef struct {
     dotens2_closure k;
     C *a; C *b;
} cpy_closure;

static void cpy0(dotens2_closure *k_, 
		 int indxa, int ondxa, int indxb, int ondxb)
{
     cpy_closure *k = (cpy_closure *)k_;
     CASSIGN(k->b[indxb], k->a[ondxa]);
     UNUSED(indxa); UNUSED(ondxb);
}

static void cpy(C *a, const tensor *sza, C *b, const tensor *szb)
{
     cpy_closure k;
     k.k.apply = cpy0;
     k.a = a; k.b = b;
     dotens2(sza, szb, &k.k);
}

static void dofft(void *n_, C *in, C *out)
{
     info *n = (info *)n_;
     C *pin = n->p->in, *pout = n->p->out;

     cpy(in, n->pckdsz, pin, n->totalsz);
     doit(1, n->p);
     cpy(pout, n->totalsz, out, n->pckdsz);
}

/***************************************************************************/

void verify_dft(problem *p, int rounds, double tol)
{
     C *inA, *inB, *inC, *outA, *outB, *outC, *tmp;
     info nfo;
     int n, vecn, N;

     if (rounds == 0)
	  rounds = 20;  /* default value */

     n = tensor_sz(p->sz);
     vecn = tensor_sz(p->vecsz);
     N = n * vecn;

     inA = (C *) bench_malloc(N * sizeof(C));
     inB = (C *) bench_malloc(N * sizeof(C));
     inC = (C *) bench_malloc(N * sizeof(C));
     outA = (C *) bench_malloc(N * sizeof(C));
     outB = (C *) bench_malloc(N * sizeof(C));
     outC = (C *) bench_malloc(N * sizeof(C));
     tmp = (C *) bench_malloc(N * sizeof(C));

     nfo.p = p;
     nfo.probsz = p->sz;
     nfo.totalsz = tensor_append(p->vecsz, p->sz);
     nfo.pckdsz = verify_pack(nfo.totalsz, 1);

     impulse(dofft, &nfo, 
	     n, vecn, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     linear(dofft, &nfo, 0,
	    N, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);

     tf_shift(dofft, &nfo, 0, p->sz,
	      n, vecn, p->sign, inA, inB, outA, outB, tmp, 
	      rounds, tol, TIME_SHIFT);
     tf_shift(dofft, &nfo, 0, p->sz,
	      n, vecn, p->sign, inA, inB, outA, outB, tmp, 
	      rounds, tol, FREQ_SHIFT);

     tensor_destroy(nfo.totalsz);
     tensor_destroy(nfo.pckdsz);
     bench_free(tmp);
     bench_free(outC);
     bench_free(outB);
     bench_free(outA);
     bench_free(inC);
     bench_free(inB);
     bench_free(inA);
}

