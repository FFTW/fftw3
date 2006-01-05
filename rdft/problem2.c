/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* $Id: problem2.c,v 1.37 2006-01-05 03:04:27 stevenj Exp $ */

#include "dft.h"
#include "rdft.h"
#include <stddef.h>

static void destroy(problem *ego_)
{
     problem_rdft2 *ego = (problem_rdft2 *) ego_;
     X(tensor_destroy2)(ego->vecsz, ego->sz);
     X(ifree)(ego_);
}

static void hash(const problem *p_, md5 *m)
{
     const problem_rdft2 *p = (const problem_rdft2 *) p_;
     X(md5puts)(m, "rdft2");
     X(md5int)(m, p->r == p->rio);
     X(md5int)(m, p->r == p->iio);
     X(md5INT)(m, p->iio - p->rio);
     X(md5int)(m, X(alignment_of)(p->r));
     X(md5int)(m, X(alignment_of)(p->rio)); 
     X(md5int)(m, X(alignment_of)(p->iio)); 
     X(md5int)(m, p->kind);
     X(tensor_md5)(m, p->sz);
     X(tensor_md5)(m, p->vecsz);
}

static void print(problem *ego_, printer *p)
{
     problem_rdft2 *ego = (problem_rdft2 *) ego_;
     p->print(p, "(rdft2 %d %D %D %d %T %T)", 
	      X(alignment_of)(ego->r),
	      (INT)(ego->rio - ego->r), 
	      (INT)(ego->iio - ego->r),
	      (int)(ego->kind),
	      ego->sz,
	      ego->vecsz);
}

static void zero(const problem *ego_)
{
     const problem_rdft2 *ego = (const problem_rdft2 *) ego_;
     tensor *sz;
     if (ego->kind == R2HC) {
	  sz = X(tensor_append)(ego->vecsz, ego->sz);
	  X(rdft_zerotens)(sz, UNTAINT(ego->r));
     }
     else {
	  tensor *sz2 = X(tensor_copy)(ego->sz);
	  if (sz2->rnk > 0) /* ~half as many complex outputs */
	       sz2->dims[0].n = sz2->dims[0].n / 2 + 1;
	  sz = X(tensor_append)(ego->vecsz, sz2);
	  X(tensor_destroy)(sz2);
	  X(dft_zerotens)(sz, UNTAINT(ego->rio), UNTAINT(ego->iio));
     }
     X(tensor_destroy)(sz);
}

static const problem_adt padt =
{
     PROBLEM_RDFT2,
     hash,
     zero,
     print,
     destroy
};

problem *X(mkproblem_rdft2)(const tensor *sz, const tensor *vecsz,
			    R *r, R *rio, R *iio, rdft_kind kind)
{
     problem_rdft2 *ego =
          (problem_rdft2 *)X(mkproblem)(sizeof(problem_rdft2), &padt);

     A(X(tensor_kosherp)(sz));
     A(X(tensor_kosherp)(vecsz));
     A(FINITE_RNK(sz->rnk));

     if (UNTAINT(r) == UNTAINT(rio))
	  r = rio = JOIN_TAINT(r, rio);
     if (UNTAINT(r) == UNTAINT(iio))
	  r = iio = JOIN_TAINT(r, iio);

     /* correctness condition: */
     A(TAINTOF(rio) == TAINTOF(iio));

     if (sz->rnk > 1) { /* have to compress rnk-1 dims separately, ugh */
	  tensor *szc = X(tensor_copy_except)(sz, sz->rnk - 1);
	  tensor *szr = X(tensor_copy_sub)(sz, sz->rnk - 1, 1);
	  tensor *szcc = X(tensor_compress)(szc);
	  if (szcc->rnk > 0)
	       ego->sz = X(tensor_append)(szcc, szr);
	  else
	       ego->sz = X(tensor_compress)(szr);
	  X(tensor_destroy2)(szc, szr); X(tensor_destroy)(szcc);
     }
     else
	  ego->sz = X(tensor_compress)(sz);
     ego->vecsz = X(tensor_compress_contiguous)(vecsz);
     ego->r = r;
     ego->rio = rio;
     ego->iio = iio;
     ego->kind = kind;

     A(kind == R2HC || kind == HC2R);
     A(FINITE_RNK(ego->sz->rnk));
     return &(ego->super);
}

/* Same as X(mkproblem_rdft2), but also destroy input tensors. */
problem *X(mkproblem_rdft2_d)(tensor *sz, tensor *vecsz,
			      R *r, R *rio, R *iio, rdft_kind kind)
{
     problem *p;
     p = X(mkproblem_rdft2)(sz, vecsz, r, rio, iio, kind);
     X(tensor_destroy2)(vecsz, sz);
     return p;
}
