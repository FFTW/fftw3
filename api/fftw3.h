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
/* $Id: fftw3.h,v 1.3 2003-01-09 18:41:51 athena Exp $ */

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

#define FFTW_DEFINE_API(X, R, C)				\
								\
FFTW_DEFINE_COMPLEX(R, C);					\
								\
typedef struct X(plan_s) *X(plan);				\
								\
void X(execute)(X(plan) p);					\
								\
X(plan) X(plan_dft_1d)(unsigned int n, C *in, C *out, int sign,	\
		       unsigned int flags);			\


/* end of FFTW_DEFINE_API macro */

#define FFTW_CONCAT(prefix, name) prefix ## name
#define FFTW_MANGLE_DOUBLE(name) FFTW_CONCAT(fftw_, name)
#define FFTW_MANGLE_FLOAT(name) FFTW_CONCAT(fftwf_, name)
#define FFTW_MANGLE_LONG_DOUBLE(name) FFTW_CONCAT(fftwl_, name)

FFTW_DEFINE_API(FFTW_MANGLE_DOUBLE, double, fftw_complex)
FFTW_DEFINE_API(FFTW_MANGLE_FLOAT, float, fftwf_complex)
FFTW_DEFINE_API(FFTW_MANGLE_LONG_DOUBLE, long double, fftwl_complex)

