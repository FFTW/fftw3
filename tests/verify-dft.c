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

/* $Id: verify-dft.c,v 1.1 2002-09-02 15:46:57 athena Exp $ */

#include "dft.h"
#include "debug.h"
#include "verify.h"

typedef struct {
     plan *pln;
     const problem_dft *p;
     tensor probsz;
     tensor totalsz;
     tensor pckdsz;
} info;


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

static void dofft(void *n_, C *in, C *out)
{
     info *n = (info *)n_;
     cpy(&in->r, &in->i, n->pckdsz, n->p->ri, n->p->ii, n->totalsz);
     n->pln->adt->solve(n->pln, &(n->p->super));
     cpy(n->p->ro, n->p->io, n->totalsz, &out->r, &out->i, n->pckdsz);
}

/***************************************************************************/

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
     nfo.pckdsz = verify_pack(nfo.totalsz, 2);

     impulse(dofft, &nfo, 
	     n, vecn, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);
     linear(dofft, &nfo, 0,
	    N, inA, inB, inC, outA, outB, outC, tmp, rounds, tol);

     tf_shift(dofft, &nfo, 0, p->sz,
	      n, vecn, inA, inB, outA, outB, tmp, 
	      rounds, tol, TIME_SHIFT);
     tf_shift(dofft, &nfo, 0, p->sz,
	      n, vecn, inA, inB, outA, outB, tmp, 
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
