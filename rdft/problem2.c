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

/* $Id: problem2.c,v 1.14 2002-09-01 23:22:54 athena Exp $ */

#include "dft.h"
#include "rdft.h"
#include <stddef.h>

static void destroy(problem *ego_)
{
     problem_rdft2 *ego = (problem_rdft2 *) ego_;
     X(tensor_destroy)(ego->vecsz);
     X(tensor_destroy)(ego->sz);
     X(free)(ego_);
}

static void hash(const problem *p_, md5 *m)
{
     const problem_rdft2 *p = (const problem_rdft2 *) p_;
     X(md5int)(m, p->r == p->rio);
     X(md5int)(m, p->r == p->iio);
     X(md5ptrdiff)(m, p->iio - p->rio);  /* (1) */
     X(md5uint)(m, X(alignment_of)(p->r));
     X(md5uint)(m, X(alignment_of)(p->rio)); 
             /* alignment of imag is implied by (1) */
     X(md5int)(m, p->kind);
     X(tensor_md5)(m, p->sz);
     X(tensor_md5)(m, p->vecsz);
}

static int equal(const problem *ego_, const problem *problem_)
{
     if (ego_->adt == problem_->adt) {
          const problem_rdft2 *e = (const problem_rdft2 *) ego_;
          const problem_rdft2 *p = (const problem_rdft2 *) problem_;

          return (1

		  /* both in-place or both out-of-place */
                  && ((p->r == p->rio) == (e->r == e->rio))
                  && ((p->r == p->iio) == (e->r == e->iio))

		  /* distance between real and imag must be the same (1) */
                  && p->iio - p->rio == e->iio - e->rio

		  /* aligments must match */
		  && X(alignment_of)(p->r) == X(alignment_of)(e->r)
		  && X(alignment_of)(p->rio) == X(alignment_of)(e->rio)
		  /* alignment of imag is implied by (1) */

		  && p->kind == e->kind

		  && X(tensor_equal)(p->sz, e->sz)
                  && X(tensor_equal)(p->vecsz, e->vecsz)
	       );
     }
     return 0;
}

static void print(problem *ego_, printer *p)
{
     problem_rdft2 *ego = (problem_rdft2 *) ego_;
     p->print(p, "(rdft2 %u %td %td %d %T %T)", 
	      X(alignment_of)(ego->r),
	      ego->rio - ego->r, 
	      ego->iio - ego->r,
	      (int)(ego->kind),
	      &ego->sz,
	      &ego->vecsz);
}

static int scan(scanner *sc, problem **p)
{
     tensor sz = { 0, 0 }, vecsz = { 0, 0 };
     uint align;
     ptrdiff_t offr, offi;
     int kind;
     R *r;
     
     if (!sc->scan(sc, "%u %td %td %d %T %T",
		   &align, &offr, &offi, &kind, &sz, &vecsz)) {
	  X(tensor_destroy)(sz);
	  X(tensor_destroy)(vecsz);
	  return 0;
     }
     r = X(ptr_with_alignment)(align);
     *p = X(mkproblem_rdft2_d)(sz, vecsz, r, r + offr, r + offi, kind);
     return 1;
}

static void zero(const problem *ego_)
{
     const problem_rdft2 *ego = (const problem_rdft2 *) ego_;
     tensor sz;
     if (ego->kind == R2HC) {
	  sz = X(tensor_append)(ego->vecsz, ego->sz);
	  X(rdft_zerotens)(sz, ego->r);
     }
     else {
	  tensor sz2;
	  sz2 = X(tensor_copy)(ego->sz);
	  if (sz2.rnk > 0) /* ~half as many complex outputs */
	       sz2.dims[0].n = sz2.dims[0].n / 2 + 1;
	  sz = X(tensor_append)(ego->vecsz, sz2);
	  X(tensor_destroy)(sz2);
	  X(dft_zerotens)(sz, ego->rio, ego->iio);
     }
     X(tensor_destroy)(sz);
}

static const problem_adt padt =
{
     equal,
     hash,
     zero,
     print,
     destroy,
     scan,
     "rdft2"
};

int X(problem_rdft2_p)(const problem *p)
{
     return (p->adt == &padt);
}

problem *X(mkproblem_rdft2)(const tensor sz, const tensor vecsz,
			    R *r, R *rio, R *iio, rdft_kind kind)
{
     problem_rdft2 *ego =
          (problem_rdft2 *)X(mkproblem)(sizeof(problem_rdft2), &padt);

     ego->sz = X(tensor_compress)(sz);
     ego->vecsz = X(tensor_compress_contiguous)(vecsz);
     ego->r = r;
     ego->rio = rio;
     ego->iio = iio;
     ego->kind = kind;

     A(kind == R2HC || kind == HC2R);
     A(FINITE_RNK(ego->sz.rnk));
     return &(ego->super);
}

/* Same as X(mkproblem_rdft2), but also destroy input tensors. */
problem *X(mkproblem_rdft2_d)(tensor sz, tensor vecsz,
			      R *r, R *rio, R *iio, rdft_kind kind)
{
     problem *p;
     p = X(mkproblem_rdft2)(sz, vecsz, r, rio, iio, kind);
     X(tensor_destroy)(vecsz);
     X(tensor_destroy)(sz);
     return p;
}

/* Check if the vecsz/sz strides are consistent with the problem
   being in-place for vecsz.dim[vdim], or for all dimensions
   if vdim == RNK_MINFTY.  We can't just use tensor_inplace_strides
   because rdft transforms have the unfortunate property of
   differing input and output sizes.   This routine is not
   exhaustive; we only return 1 for the most common case.  */
int X(rdft2_inplace_strides)(const problem_rdft2 *p, uint vdim)
{
     uint N, Nc;
     int is, os;
     uint i;
     
     for (i = 0; i + 1 < p->sz.rnk; ++i)
	  if (p->sz.dims[i].is != p->sz.dims[i].os)
	       return 0;

     if (!FINITE_RNK(p->vecsz.rnk) || p->vecsz.rnk == 0)
	  return 1;
     if (!FINITE_RNK(vdim)) { /* check all vector dimensions */
	  for (vdim = 0; vdim < p->vecsz.rnk; ++vdim)
	       if (!X(rdft2_inplace_strides)(p, vdim))
		    return 0;
	  return 1;
     }

     A(vdim < p->vecsz.rnk);
     if (p->sz.rnk == 0)
	  return(p->vecsz.dims[vdim].is == p->vecsz.dims[vdim].os);

     N = X(tensor_sz)(p->sz);
     Nc = (N / p->sz.dims[p->sz.rnk-1].n) * (p->sz.dims[p->sz.rnk-1].n/2 + 1);
     if (p->kind == R2HC) {
	  is = p->sz.dims[p->sz.rnk-1].is;
	  os = p->sz.dims[p->sz.rnk-1].os;
     }
     else {
	  is = p->sz.dims[p->sz.rnk-1].os;
	  os = p->sz.dims[p->sz.rnk-1].is;
     }
     return(p->vecsz.dims[vdim].is == p->vecsz.dims[vdim].os
	    && X(iabs)(p->vecsz.dims[vdim].os)
	    >= X(uimax)(Nc * X(iabs)(os), N * X(iabs)(is)));
}

void X(problem_rdft2_register)(planner *p)
{
     REGISTER_PROBLEM(p, &padt);
}
