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

/* header file for fftw3 */
/* $Id: fftw3.h,v 1.8 2003-01-11 14:49:30 athena Exp $ */

#ifndef FFTW3_H
#define FFTW3_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* If <complex.h> is included, use the C99 complex type.  Otherwise
   define a type bit-compatible with C99 complex */
#ifdef _Complex_I
#  define FFTW_DEFINE_COMPLEX(R, C) typedef R _Complex C
#else
#  define FFTW_DEFINE_COMPLEX(R, C) typedef R C[2]
#endif

/*
  huge second-order macro that defines prototypes for all API
  functions.  We expand this macro for each supported precision

  X: name-mangling macro
  R: real data type
  C: complex data type
*/

#define FFTW_DEFINE_API(X, R, C)					  \
									  \
FFTW_DEFINE_COMPLEX(R, C);						  \
									  \
typedef struct X(plan_s) *X(plan);					  \
									  \
void X(execute)(X(plan) p);						  \
									  \
X(plan) X(plan_dft)(unsigned int rank, const unsigned int *n,		  \
		    C *in, const unsigned int *inembed,			  \
		    C *out, const unsigned int *onembed,		  \
		    int sign, unsigned int flags);			  \
									  \
X(plan) X(plan_dft_1d)(unsigned int n, C *in, C *out, int sign,		  \
		       unsigned int flags);				  \
									  \
									  \
X(plan) X(plan_dft_2d)(unsigned int nx, unsigned int ny,		  \
		       C *in, C *out, int sign, unsigned int flags);	  \
									  \
X(plan) X(plan_dft_3d)(unsigned int nx, unsigned int ny, unsigned int nz, \
		       C *in, C *out, int sign, unsigned int flags);	  \
									  \
void X(plan_destroy)(X(plan) p);					  \
void X(cleanup)(void);

/* end of FFTW_DEFINE_API macro */

#define FFTW_CONCAT(prefix, name) prefix ## name
#define FFTW_MANGLE_DOUBLE(name) FFTW_CONCAT(fftw_, name)
#define FFTW_MANGLE_FLOAT(name) FFTW_CONCAT(fftwf_, name)
#define FFTW_MANGLE_LONG_DOUBLE(name) FFTW_CONCAT(fftwl_, name)

FFTW_DEFINE_API(FFTW_MANGLE_DOUBLE, double, fftw_complex)
FFTW_DEFINE_API(FFTW_MANGLE_FLOAT, float, fftwf_complex)
FFTW_DEFINE_API(FFTW_MANGLE_LONG_DOUBLE, long double, fftwl_complex)

/* documented flags */
#define FFTW_DEFAULTS (0U)
#define FFTW_DESTROY_INPUT (1U << 0)
#define FFTW_POSSIBLY_UNALIGNED (1U << 1)
#define FFTW_CONSERVE_MEMORY (1U << 2)
#define FFTW_EXHAUSTIVE (1U << 3) /* NO_EXHAUSTIVE is default */
#define FFTW_PRESERVE_INPUT (1U << 4) /* cancels FFTW_DESTROY_INPUT */
#define FFTW_IMPATIENT (1U << 5)
#define FFTW_ESTIMATE (1U << 6)

/* undocumented beyond-guru flags */
#define FFTW_ESTIMATE_PATIENT (1U << 7)
#define FFTW_BELIEVE_PCOST (1U << 8)
#define FFTW_DFT_R2HC_ICKY (1U << 9)
#define FFTW_NONTHREADED_ICKY (1U << 10)
#define FFTW_NO_BUFFERING (1U << 11)
#define FFTW_NO_INDIRECT_OP (1U << 12)
#define FFTW_ALLOW_LARGE_GENERIC (1U << 13) /* NO_LARGE_GENERIC is default */
#define FFTW_NO_RANK_SPLITS (1U << 14)
#define FFTW_ALLOW_UGLY (1U << 15) /* NO_UGLY is default */
#define FFTW_NO_VRANK_SPLITS (1U << 16)
#define FFTW_NO_VRECURSE (1U << 17)

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

#endif /* FFTW3_H */
