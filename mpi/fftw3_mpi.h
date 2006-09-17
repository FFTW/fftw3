/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
 *
 * The following statement of license applies *only* to this header file,
 * and *not* to the other files distributed with FFTW or derived therefrom:
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***************************** NOTE TO USERS *********************************
 *
 *                 THIS IS A HEADER FILE, NOT A MANUAL
 *
 *    If you want to know how to use FFTW, please read the manual,
 *    online at http://www.fftw.org/doc/ and also included with FFTW.
 *    For a quick start, see the manual's tutorial section.
 *
 *   (Reading header files to learn how to use a library is a habit
 *    stemming from code lacking a proper manual.  Arguably, it's a
 *    *bad* habit in most cases, because header files can contain
 *    interfaces that are not part of the public, stable API.)
 *
 ****************************************************************************/

#ifndef FFTW3_MPI_H
#define FFTW3_MPI_H

#include "fftw3.h"
#include <mpi.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
  huge second-order macro that defines prototypes for all API
  functions.  We expand this macro for each supported precision
 
  X: name-mangling macro
  R: real data type
  C: complex data type
*/

#define FFTW_MPI_DEFINE_API(X, R, C)				\
								\
FFTW_EXTERN void X(mpi_cleanup)(void);				\
								\
FFTW_EXTERN ptrdiff_t X(mpi_local_size_2d)			\
     (ptrdiff_t nx, ptrdiff_t ny, MPI_Comm comm,		\
      ptrdiff_t *local_nx, ptrdiff_t *local_x_start);		\
FFTW_EXTERN ptrdiff_t X(mpi_local_size_2d_transposed)		\
     (ptrdiff_t nx, ptrdiff_t ny,				\
      ptrdiff_t xblock, ptrdiff_t yblock,			\
      MPI_Comm comm,						\
      ptrdiff_t *local_nx, ptrdiff_t *local_x_start,		\
      ptrdiff_t *local_ny, ptrdiff_t *local_y_start);		\
								\
								\
FFTW_EXTERN X(plan) X(mpi_plan_transpose)			\
     (ptrdiff_t nx, ptrdiff_t ny,				\
      R *in, R *out, MPI_Comm comm, unsigned flags);		\
FFTW_EXTERN X(plan) X(mpi_plan_many_transpose)			\
     (ptrdiff_t nx, ptrdiff_t ny,				\
      ptrdiff_t howmany, ptrdiff_t xblock, ptrdiff_t yblock,	\
      R *in, R *out, MPI_Comm comm, unsigned flags);

/* end of FFTW_MPI_DEFINE_API macro */

FFTW_MPI_DEFINE_API(FFTW_MANGLE_DOUBLE, double, fftw_complex)
FFTW_MPI_DEFINE_API(FFTW_MANGLE_FLOAT, float, fftwf_complex)
FFTW_MPI_DEFINE_API(FFTW_MANGLE_LONG_DOUBLE, long double, fftwl_complex)

#define FFTW_MPI_DEFAULT_BLOCK (0)

/* MPI-specific flags */
#define FFTW_MPI_TRANSPOSED (1U << 27)
#define FFTW_MPI_SCRAMBLED_IN (1U << 28)
#define FFTW_MPI_SCRAMBLED_OUT (1U << 29)

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

#endif /* FFTW3_MPI_H */
