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

/* $Id: problem2.c,v 1.7 2002-08-01 07:03:18 stevenj Exp $ */

#include "dft.h"
#include "rdft.h"
#include <stddef.h>

static void destroy(problem *ego_)
{
     problem_rdft2 *ego = (problem_rdft2 *) ego_;
     X(tensor_destroy)(ego->vecsz);
     X(free)(ego_);
}

static unsigned int hash(const problem *p_)
{
     const problem_rdft2 *p = (const problem_rdft2 *) p_;
     tensor psz = {1, 0};
     psz.dims = (iodim *) &p->sz;
     return (0xBEEFDEAD
	     ^ ((p->r == p->rio) * 31)
	     ^ ((p->r == p->iio) * 29)
	     ^ ((p->iio - p->rio) * 23)
	     ^ (p->kind * 37)
	     ^ (X(tensor_hash)(psz) * 10487)
             ^ (X(tensor_hash)(p->vecsz) * 27197)
	  );
}

static int equal(const problem *ego_, const problem *problem_)
{
     if (ego_->adt == problem_->adt) {
          const problem_rdft2 *e = (const problem_rdft2 *) ego_;
          const problem_rdft2 *p = (const problem_rdft2 *) problem_;
	  tensor psz = {1, 0};
	  tensor esz = {1, 0};
	  psz.dims = (iodim *) &p->sz;
	  esz.dims = (iodim *) &e->sz;

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

		  && X(tensor_equal)(psz, esz)
                  && X(tensor_equal)(p->vecsz, e->vecsz)
	       );
     }
     return 0;
}

static void print(problem *ego_, printer *p)
{
     problem_rdft2 *ego = (problem_rdft2 *) ego_;
     tensor psz = {1, 0};
     psz.dims = &ego->sz;
     p->print(p, "(rdft2 %u %td %td %d %T %T)", 
	      X(alignment_of)(ego->r),
	      ego->rio - ego->r, 
	      ego->iio - ego->r,
	      (int)(ego->kind),
	      &psz,
	      &ego->vecsz);
}

static int scan(scanner *sc, problem **p)
{
     tensor sz = { 0, 0 }, vecsz = { 0, 0 };
     uint align;
     ptrdiff_t offr, offi;
     int kind;
     R *r;
     int ret;

     ret = sc->scan(sc, "%u %td %td %d %T %T",
		    &align, &offr, &offi, &kind, &sz, &vecsz);
     if (ret == EOF || ret < 6 || sz.rnk == 1) {
	  X(tensor_destroy)(sz);
	  X(tensor_destroy)(vecsz);
	  return(ret == EOF ? EOF : 0);
     }
     r = (R *) ((char *) 0 + align);
     *p = X(mkproblem_rdft2_d)(sz.dims[0], vecsz, r, r + offr, r + offi, kind);
     X(tensor_destroy)(sz);
     return 1;
}

static void zero(const problem *ego_)
{
     const problem_rdft2 *ego = (const problem_rdft2 *) ego_;
     tensor sz;
     if (ego->kind == R2HC) {
	  tensor psz = {1, 0};
	  psz.dims = (iodim *) &ego->sz;
	  sz = X(tensor_append)(ego->vecsz, psz);
	  X(rdft_zerotens)(sz, ego->r);
     }
     else {
	  iodim sz2;
	  tensor psz2 = {1, &sz2};
	  sz2 = ego->sz;
	  sz2.n = sz2.n/2 + 1; /* ~half as many complex outputs */
	  sz2.is = sz2.os; /* sz.os is complex stride */
	  sz = X(tensor_append)(ego->vecsz, psz2);
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

problem *X(mkproblem_rdft2)(iodim sz, const tensor vecsz,
			   R *r, R *rio, R *iio, rdft_kind kind)
{
     problem_rdft2 *ego =
          (problem_rdft2 *)X(mkproblem)(sizeof(problem_rdft2), &padt);

     if (sz.n == 1)
	  sz.is = sz.os = 0;
     ego->sz = sz;
     ego->vecsz = X(tensor_compress_contiguous)(vecsz);
     ego->r = r;
     ego->rio = rio;
     ego->iio = iio;
     ego->kind = kind;

     A(kind == R2HC || kind == HC2R);
     A(sz.n > 0);
     return &(ego->super);
}

/* Same as X(mkproblem_rdft2), but also destroy input tensors. */
problem *X(mkproblem_rdft2_d)(iodim sz, tensor vecsz,
			     R *r, R *rio, R *iio, rdft_kind kind)
{
     problem *p;
     p = X(mkproblem_rdft2)(sz, vecsz, r, rio, iio, kind);
     X(tensor_destroy)(vecsz);
     return p;
}

static uint iabs(int i)
{
     return(i > 0 ? i : -i);
}

/* Check if the vecsz/sz strides are consistent with the problem
   being in-place for vecsz.dim[vdim], or for all dimensions
   if vdim == RNK_MINFTY.  We can't just use tensor_inplace_strides
   because rdft transforms have the unfortunate property of
   differing input and output sizes.   This routine is not
   exhaustive; we only return 1 for the most common case.  */
int X(rdft2_inplace_strides)(const problem_rdft2 *p, uint vdim)
{
     if (!FINITE_RNK(p->vecsz.rnk) || p->vecsz.rnk == 0)
	  return 1;
     if (!FINITE_RNK(vdim)) { /* check all vector dimensions */
	  for (vdim = 0; vdim < p->vecsz.rnk; ++vdim)
	       X(rdft2_inplace_strides)(p, vdim);
     }
     A(vdim < p->vecsz.rnk);
     return(p->vecsz.dims[vdim].is == p->vecsz.dims[vdim].os
	    && iabs(p->vecsz.dims[vdim].os)
	     >= X(uimax)((p->sz.n/2 + 1) * iabs(p->sz.os),
			 p->sz.n * iabs(p->sz.is)));
}

void X(problem_rdft2_register)(planner *p)
{
     REGISTER_PROBLEM(p, &padt);
}
