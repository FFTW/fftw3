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

/* C analogues to Fortran integer type: */
typedef int fint;
typedef unsigned int ufint;


static ulong *reverse_n(ufint rnk, const ufint *n)
{
     ulong *nrev;
     ufint i;
     A(FINITE_RNK(rnk));
     nrev = MALLOC(sizeof(ulong) * rnk, PROBLEMS);
     for (i = 0; i < rnk; ++i)
	  nrev[rnk-i-1] = n[i];
     return nrev;
}

static X(iodim) *make_dims(ufint rnk, const ufint *n,
			   const fint *is, const fint *os)
{
     X(iodim) *dims;
     ufint i;
     A(FINITE_RNK(rnk));
     dims = MALLOC(sizeof(X(iodim)) * rnk, PROBLEMS);
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
			     unsigned fint *rank, const unsigned fint *n,
			     C *in, C *out, fint *sign, unsigned fint *flags)
{
     ulong *nrev = reverse_n(*rank, n);
     *p = X(plan_dft)(*rank, nrev, in, out, *sign, *flags);
     X(ifree0)(nrev);
}

void F77(plan_dft_1d, PLAN_DFT_1D)(
     X(plan) *p,
     unsigned fint *n,
     C *in, C *out, fint *sign, unsigned fint *flags)
{
     *p = X(plan_dft_1d)(*n, in, out, *sign, *flags);
}

void F77(plan_dft_2d, PLAN_DFT_2D)(
     X(plan) *p,
     unsigned fint *nx, unsigned fint *ny,
     C *in, C *out, fint *sign, unsigned fint *flags)
{
     *p = X(plan_dft_2d)(*ny, *nx, in, out, *sign, *flags);
}

void F77(plan_dft_3d, PLAN_DFT_3D)(
     X(plan) *p,
     unsigned fint *nx, unsigned fint *ny, unsigned fint *nz,
     C *in, C *out, fint *sign, unsigned fint *flags)
{
     *p = X(plan_dft_3d)(*nz, *ny, *nx, in, out, *sign, *flags);
}

void F77(plan_many_dft, PLAN_MANY_DFT)(
     X(plan) *p,
     unsigned fint *rank, const unsigned fint *n,
     unsigned fint *howmany,
     C *in, const unsigned fint *inembed,
     fint *istride, fint *idist,
     C *out, const unsigned fint *onembed,
     fint *ostride, fint *odist,
     fint *sign, unsigned fint *flags)
{
     ulong *nrev = reverse_n(*rank, n);
     ulong *inembedrev = reverse_n(*rank, inembed);
     ulong *onembedrev = reverse_n(*rank, onembed);
     *p = X(plan_many_dft)(*rank, nrev, *howmany,
			   in, inembedrev, *istride, *idist,
			   out, onembedrev, *ostride, *odist,
			   *sign, *flags);
     X(ifree0)(onembedrev);
     X(ifree0)(inembedrev);
     X(ifree0)(nrev);
}

void F77(plan_guru_dft, PLAN_GURU_DFT)(
     X(plan) *p,
     unsigned fint *rank,
     const unsigned fint *n, const fint *is, const fint *os,
     unsigned fint *howmany_rank,
     const unsigned fint *h_n, const fint *h_is, const fint *h_os,
     R *ri, R *ii, R *ro, R *io,
     unsigned fint *flags)
{
     X(iodim) *dims = make_dims(*rank, n, is, os);
     X(iodim) *howmany_dims = make_dims(*howmany_rank, h_n, h_is, h_os);
     *p = X(plan_guru_dft)(*rank, dims, *howmany_rank, howmany_dims,
			   ri, ii, ro, io,
			   *flags);
     X(ifree0)(howmany_dims);
     X(ifree0)(dims);
}

void F77(execute_dft, EXECUTE_DFT)(X(plan) *p, R *ri, R *ii, R *ro, R *io)
{
     X(execute_dft)(*p, ri, ii, ro, io);
}
