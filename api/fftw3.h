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
/* $Id: fftw3.h,v 1.34 2003-01-15 11:51:34 athena Exp $ */

#ifndef FFTW3_H
#define FFTW3_H

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* If <complex.h> is included, use the C99 complex type.  Otherwise
   define a type bit-compatible with C99 complex */
#ifdef _Complex_I
#  define FFTW_DEFINE_COMPLEX(R, C) typedef R _Complex C
#else
#  define FFTW_DEFINE_COMPLEX(R, C) typedef R C[2]
#endif

#define FFTW_CONCAT(prefix, name) prefix ## name
#define FFTW_MANGLE_DOUBLE(name) FFTW_CONCAT(fftw_, name)
#define FFTW_MANGLE_FLOAT(name) FFTW_CONCAT(fftwf_, name)
#define FFTW_MANGLE_LONG_DOUBLE(name) FFTW_CONCAT(fftwl_, name)


enum FFTW_R2R_KIND {
     FFTW_R2HC=0, FFTW_HC2R=1, FFTW_DHT=2,
     FFTW_REDFT00=3, FFTW_REDFT01=4, FFTW_REDFT10=5, FFTW_REDFT11=6,
     FFTW_RODFT00=7, FFTW_RODFT01=8, FFTW_RODFT10=9, FFTW_RODFT11=10
};

/*
  huge second-order macro that defines prototypes for all API
  functions.  We expand this macro for each supported precision
 
  X: name-mangling macro
  R: real data type
  C: complex data type
*/

#define FFTW_DEFINE_API(X, R, C)					\
									\
FFTW_DEFINE_COMPLEX(R, C);						\
									\
typedef struct X(plan_s) *X(plan);					\
									\
typedef struct {							\
     int n;								\
     int is;			/* input stride */			\
     int os;			/* output stride */			\
} X(iodim);								\
									\
typedef enum FFTW_R2R_KIND X(r2r_kind);					\
									\
void X(execute)(X(plan) p);						\
									\
X(plan) X(plan_dft)(int rank, const int *n,				\
		    C *in, C *out, int sign, int flags);		\
									\
X(plan) X(plan_dft_1d)(int n, C *in, C *out, int sign,			\
		       int flags);					\
X(plan) X(plan_dft_2d)(int nx, int ny,					\
		       C *in, C *out, int sign, int flags);		\
X(plan) X(plan_dft_3d)(int nx, int ny, int nz,				\
		       C *in, C *out, int sign, int flags);		\
									\
X(plan) X(plan_many_dft)(int rank, const int *n,			\
                         int howmany,					\
                         C *in, const int *inembed,			\
                         int istride, int idist,			\
                         C *out, const int *onembed,			\
                         int ostride, int odist,			\
                         int sign, int flags);				\
									\
X(plan) X(plan_guru_dft)(int rank, const X(iodim) *dims,		\
			 int howmany_rank,				\
			 const X(iodim) *howmany_dims,			\
			 R *ri, R *ii, R *ro, R *io,			\
			 int flags);					\
									\
void X(execute_dft)(X(plan) p, R *ri, R *ii, R *ro, R *io);		\
									\
X(plan) X(plan_many_dft_r2c)(int rank, const int *n,			\
                             int howmany,				\
                             R *in, const int *inembed,			\
                             int istride, int idist,			\
                             C *out, const int *onembed,		\
                             int ostride, int odist,			\
                             int flags);				\
									\
X(plan) X(plan_dft_r2c)(int rank, const int *n,				\
                        R *in, C *out, int flags);			\
									\
X(plan) X(plan_dft_r2c_1d)(int n,R *in,C *out,int flags);		\
X(plan) X(plan_dft_r2c_2d)(int nx, int ny,				\
			   R *in, C *out, int flags);			\
X(plan) X(plan_dft_r2c_3d)(int nx, int ny,				\
			   int nz,					\
			   R *in, C *out, int flags);			\
									\
									\
X(plan) X(plan_many_dft_c2r)(int rank, const int *n,			\
			     int howmany,				\
			     C *in, const int *inembed,			\
			     int istride, int idist,			\
			     R *out, const int *onembed,		\
			     int ostride, int odist,			\
			     int flags);				\
									\
X(plan) X(plan_dft_c2r)(int rank, const int *n,				\
                        C *in, R *out, int flags);			\
									\
X(plan) X(plan_dft_c2r_1d)(int n,C *in,R *out,int flags);		\
X(plan) X(plan_dft_c2r_2d)(int nx, int ny,				\
			   C *in, R *out, int flags);			\
X(plan) X(plan_dft_c2r_3d)(int nx, int ny,				\
			   int nz,					\
			   C *in, R *out, int flags);			\
									\
X(plan) X(plan_guru_dft_r2c)(int rank, const X(iodim) *dims,		\
			     int howmany_rank,				\
			     const X(iodim) *howmany_dims,		\
			     R *in, R *ro, R *io,			\
			     int flags);				\
X(plan) X(plan_guru_dft_c2r)(int rank, const X(iodim) *dims,		\
			     int howmany_rank,				\
			     const X(iodim) *howmany_dims,		\
			     R *ri, R *ii, R *out,			\
			     int flags);				\
									\
void X(execute_dft_r2c)(X(plan) p, R *in, R *ro, R *io);		\
void X(execute_dft_c2r)(X(plan) p, R *ri, R *ii, R *out);		\
									\
X(plan) X(plan_many_r2r)(int rank, const int *n,			\
                         int howmany,					\
                         R *in, const int *inembed,			\
                         int istride, int idist,			\
                         R *out, const int *onembed,			\
                         int ostride, int odist,			\
                         const X(r2r_kind) *kind, int flags);		\
									\
X(plan) X(plan_r2r)(int rank, const int *n, R *in, R *out,		\
                    const X(r2r_kind) *kind, int flags);		\
									\
X(plan) X(plan_r2r_1d)(int n, R *in, R *out,				\
                       X(r2r_kind) kind, int flags);			\
X(plan) X(plan_r2r_2d)(int nx, int ny, R *in, R *out,			\
                       X(r2r_kind) kindx, X(r2r_kind) kindy,		\
                       int flags);					\
X(plan) X(plan_r2r_3d)(int nx, int ny, int nz,				\
                       R *in, R *out, X(r2r_kind) kindx,		\
                       X(r2r_kind) kindy, X(r2r_kind) kindz,		\
                       int flags);					\
									\
X(plan) X(plan_guru_r2r)(int rank, const X(iodim) *dims,		\
                         int howmany_rank,				\
                         const X(iodim) *howmany_dims,			\
                         R *in, R *out,					\
                         const X(r2r_kind) *kind, int flags);		\
void X(execute_r2r)(X(plan) p, R *in, R *out);				\
									\
void X(destroy_plan)(X(plan) p);					\
void X(forget_wisdom)(void);						\
void X(cleanup)(void);							\
									\
int X(plan_with_nthreads)(int nthreads);				\
									\
void X(export_wisdom_to_file)(FILE *output_file);			\
char *X(export_wisdom_to_string)(void);					\
void X(export_wisdom)(void (*absorber)(char c, void *), void *data);	\
int X(import_system_wisdom)(void);					\
int X(import_wisdom_from_file)(FILE *input_file);			\
int X(import_wisdom_from_string)(const char *input_string);		\
int X(import_wisdom)(int (*emitter)(void *), void *data);		\
									\
void X(print_plan)(X(plan) p, FILE *output_file);

/* end of FFTW_DEFINE_API macro */

#define FFTW_DEFINE_APIx(X, R) FFTW_DEFINE_API(X, R, X(complex))
FFTW_DEFINE_APIx(FFTW_MANGLE_DOUBLE, double)
FFTW_DEFINE_APIx(FFTW_MANGLE_FLOAT, float)
FFTW_DEFINE_APIx(FFTW_MANGLE_LONG_DOUBLE, long double)

#define FFTW_FORWARD (-1)
#define FFTW_BACKWARD (+1)

/* documented flags */
#define FFTW_DEFAULTS (0U)
#define FFTW_DESTROY_INPUT (1U << 0)
#define FFTW_POSSIBLY_UNALIGNED (1U << 1)
#define FFTW_CONSERVE_MEMORY (1U << 2)
#define FFTW_EXHAUSTIVE (1U << 3) /* NO_EXHAUSTIVE is default */
#define FFTW_PRESERVE_INPUT (1U << 4) /* cancels FFTW_DESTROY_INPUT */
#define FFTW_PATIENT (1U << 5) /* IMPATIENT is default */
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
#define FFTW_NO_VRANK_SPLITS (1U << 15)
#define FFTW_NO_VRECURSE (1U << 16)

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

#endif /* FFTW3_H */
