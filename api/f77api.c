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

#include "api.h"

/* Fortran-like (e.g. as in BLAS) type prefixes for F77 interface */
#if defined(FFTW_SINGLE)
#  define x77(name) CONCAT(sfftw_, name)
#  define X77(NAME) CONCAT(SFFTW_, NAME)
#elif defined(FFTW_LDOUBLE)
/* FIXME: what is best?  BLAS uses D..._X, apparently.  Ugh. */
#  define x77(name) CONCAT(lfftw_, name)
#  define X77(NAME) CONCAT(LFFTW_, NAME)
#else
#  define x77(name) CONCAT(dfftw_, name)
#  define X77(NAME) CONCAT(DFFTW_, NAME)
#endif

#define F77x(a, A) F77_FUNC_(a, A)
#define F77(a, A) F77x(x77(a), X77(A))

static uint *reverse_n(uint rnk, const uint *n)
{
     uint *nrev, i;
     A(FINITE_RNK(rnk));
     nrev = fftw_malloc(sizeof(uint) * rnk, PROBLEMS);
     for (i = 0; i < rnk; ++i)
	  nrev[rnk-i-1] = n[i];
     return nrev;
}

static X(iodim) *make_dims(uint rnk, const uint *n,
			   const int *is, const int *os)
{
     X(iodim) *dims;
     uint i;
     A(FINITE_RNK(rnk));
     dims = fftw_malloc(sizeof(X(iodim)) * rnk, PROBLEMS);
     for (i = 0; i < rnk; ++i) {
          dims[i].n = n[i];
          dims[i].is = is[i];
          dims[i].os = os[i];
     }
     return dims;
}


void F77(execute, EXECUTE)(X(plan) *p)
{
     X(execute)(*p);
}

void F77(plan_dft, PLAN_DFT)(X(plan) *p,
			     unsigned int *rank, const unsigned int *n,
			     C *in, C *out, int *sign, unsigned int *flags)
{
     unsigned int *nrev = reverse_n(*rank, n);
     *p = X(plan_dft)(*rank, nrev, in, out, *sign, *flags);
     X(free0)(nrev);
}

void F77(plan_dft_1d, PLAN_DFT_1D)(
     X(plan) *p,
     unsigned int *n,
     C *in, C *out, int *sign, unsigned int *flags)
{
     *p = X(plan_dft_1d)(*n, in, out, *sign, *flags);
}

void F77(plan_dft_2d, PLAN_DFT_2D)(
     X(plan) *p,
     unsigned int *nx, unsigned int *ny,
     C *in, C *out, int *sign, unsigned int *flags)
{
     *p = X(plan_dft_2d)(*ny, *nx, in, out, *sign, *flags);
}

void F77(plan_dft_3d, PLAN_DFT_3D)(
     X(plan) *p,
     unsigned int *nx, unsigned int *ny, unsigned int *nz,
     C *in, C *out, int *sign, unsigned int *flags)
{
     *p = X(plan_dft_3d)(*nz, *ny, *nx, in, out, *sign, *flags);
}

void F77(plan_many_dft, PLAN_MANY_DFT)(
     X(plan) *p,
     unsigned int *rank, const unsigned int *n,
     unsigned int *howmany,
     C *in, const unsigned int *inembed,
     int *istride, int *idist,
     C *out, const unsigned int *onembed,
     int *ostride, int *odist,
     int *sign, unsigned int *flags)
{
     unsigned int *nrev = reverse_n(*rank, n);
     unsigned int *inembedrev = reverse_n(*rank, inembed);
     unsigned int *onembedrev = reverse_n(*rank, onembed);
     *p = X(plan_many_dft)(*rank, nrev, *howmany,
			   in, inembedrev, *istride, *idist,
			   out, onembedrev, *ostride, *odist,
			   *sign, *flags);
     X(free0)(onembedrev);
     X(free0)(inembedrev);
     X(free0)(nrev);
}

void F77(plan_guru_dft, PLAN_GURU_DFT)(
     X(plan) *p,
     unsigned int *rank, const unsigned int *n, const int *is, const int *os,
     unsigned int *howmany_rank,
     const unsigned int *h_n, const int *h_is, const int *h_os,
     R *ri, R *ii, R *ro, R *io,
     unsigned int *flags)
{
     X(iodim) *dims = make_dims(*rank, n, is, os);
     X(iodim) *howmany_dims = make_dims(*howmany_rank, h_n, h_is, h_os);
     *p = X(plan_guru_dft)(*rank, dims, *howmany_rank, howmany_dims,
			   ri, ii, ro, io,
			   *flags);
     X(free0)(howmany_dims);
     X(free0)(dims);
}

void F77(execute_dft, EXECUTE_DFT)(X(plan) *p, R *ri, R *ii, R *ro, R *io)
{
     X(execute_dft)(*p, ri, ii, ro, io);
}
