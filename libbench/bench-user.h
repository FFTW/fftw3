/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Steven G. Johnson
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

/* $Id: bench-user.h,v 1.9 2002-09-14 03:07:39 stevenj Exp $ */
#ifndef __BENCH_USER_H__
#define __BENCH_USER_H__

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

/* benchmark program definitions for user code */
#include "config.h"

#if HAVE_STDDEF_H
#include <stddef.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if defined(BENCHFFT_SINGLE)
typedef float bench_real;
#elif defined(BENCHFFT_LDOUBLE)
typedef long double bench_real;
#else
typedef double bench_real;
#endif

typedef struct {
     bench_real re, im;
} bench_complex;

#define c_re(c)  ((c).re)
#define c_im(c)  ((c).im)

#undef DOUBLE_PRECISION
#define DOUBLE_PRECISION (sizeof(bench_real) == sizeof(double))
#undef SINGLE_PRECISION
#define SINGLE_PRECISION (!DOUBLE_PRECISION && sizeof(bench_real) == sizeof(float))
#undef LDOUBLE_PRECISION
#define LDOUBLE_PRECISION (!DOUBLE_PRECISION && sizeof(bench_real) == sizeof(long double))

typedef enum { PROBLEM_COMPLEX, PROBLEM_REAL } problem_kind_t;

#define MAX_RANK 20
struct problem {
     problem_kind_t kind;
     unsigned int rank;
     unsigned int n[MAX_RANK];  
     unsigned int size;  /* total size of input = PROD n[i] */
     unsigned int vrank;
     unsigned int vn[MAX_RANK];  
     unsigned int vsize;  /* total vector size of input = PROD vn[i] */
     unsigned int phys_size;  /* total size of allocated input */
     int sign;
     int in_place;
     int split;
     void *in;
     void *out;
     void *userinfo; /* user can store whatever */
};

extern int can_do(struct problem *p);
extern void setup(struct problem *p);
extern void doit(int iter, struct problem *p);
extern void done(struct problem *p);
extern void verify(const char *param, int rounds, double tol);

extern void problem_alloc(struct problem *p);
extern void problem_free(struct problem *p);
extern void problem_zero(struct problem *p);
extern void problem_ccopy_from(struct problem *p, bench_complex *in);
extern void after_problem_ccopy_from(struct problem *p, bench_complex *in);
extern void problem_ccopy_to(struct problem *p, bench_complex *out);
extern void after_problem_ccopy_to(struct problem *p, bench_complex *out);

void copy_c2r(struct problem *p, bench_complex *in);
void copy_c2r_packed(struct problem *p, bench_complex *in);
void copy_c2r_unpacked(struct problem *p, bench_complex *in);
void copy_r2c(struct problem *p, bench_complex *out);
void copy_r2c_unpacked(struct problem *p, bench_complex *out);
void copy_r2c_packed(struct problem *p, bench_complex *out);

void copy_c2h(struct problem *p, bench_complex *in);
void copy_h2c(struct problem *p, bench_complex *out);

void copy_c2h_1d_packed_strided(unsigned int n,
                                bench_real *pin, int rstride,
                                bench_complex *in, int cstride,
                                bench_real sign_of_r2h_transform);
void copy_c2h_1d_packed(struct problem *p, bench_complex *in,
			bench_real sign_of_r2h_transform);
void copy_h2c_1d_packed_strided(unsigned int n,
                                bench_real *pout, int rstride,
                                bench_complex *out, int cstride,
                                bench_real sign_of_r2h_transform);
void copy_h2c_1d_packed(struct problem *p, bench_complex *out, 
			bench_real sign_of_r2h_transform);
void copy_h2c_1d_fftpack(struct problem *p, bench_complex *out, 
			 bench_real sign_of_r2h_transform);
void copy_c2h_1d_fftpack(struct problem *p, bench_complex *in,
			 bench_real sign_of_r2h_transform);
void copy_c2h_unpacked(struct problem *p, bench_complex *in,
		       bench_real sign_of_r2h_transform);
void copy_h2c_unpacked(struct problem *p, bench_complex *out, 
		       bench_real sign_of_r2h_transform);
void copy_c2h_1d_unpacked_ri(struct problem *p, bench_complex *in,
			     bench_real sign_of_r2h_transform);
void copy_h2c_1d_unpacked_ri(struct problem *p, bench_complex *out, 
			     bench_real sign_of_r2h_transform);
void copy_h2c_1d_halfcomplex(struct problem *p, bench_complex *out, 
			     bench_real sign_of_r2h_transform);
void copy_c2h_1d_halfcomplex(struct problem *p, bench_complex *in,
			     bench_real sign_of_r2h_transform);
void copy_ri2c(bench_real *rin, bench_real *rout, bench_complex *out,
	       unsigned int n);
void copy_c2ri(bench_complex *in, bench_real *rout, bench_real *iout,
	       unsigned int n);
void unnormalize(struct problem *p, bench_complex *out, int which_sign);
void copy_c2c_from(struct problem *p, bench_complex *in);
void copy_c2c_to(struct problem *p, bench_complex *out);

extern int power_of_two(unsigned int n);
extern int problem_power_of_two(struct problem *p, int in_place);
extern int problem_complex_power_of_two(struct problem *p, int in_place);
extern int problem_real_power_of_two(struct problem *p, int in_place);
extern int problem_in_place(struct problem *p);
extern int log_2(unsigned int n);

extern void aset(bench_real *A, unsigned int n, bench_real x);
extern void caset(bench_complex *A, unsigned int n, bench_complex x);
extern void acopy(bench_real *A, bench_real *B, unsigned int n);
extern void cacopy(bench_complex *A, bench_complex *B, unsigned int n);

extern void cascale(bench_complex *A, unsigned int n, bench_complex alpha);
extern void ascale(bench_real *A, unsigned int n, bench_real alpha);

extern void caadd(bench_complex *C, bench_complex *A, 
		  bench_complex *B, unsigned int n);
extern void casub(bench_complex *C, bench_complex *A, 
		  bench_complex *B, unsigned int n);

extern int check_prime_factors(unsigned int n, unsigned int maxprime);

/**************************************************************
 * malloc
 **************************************************************/
extern void *bench_malloc(size_t size);
extern void bench_free(void *ptr);

/**************************************************************
 * alloca
 **************************************************************/
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

extern int verbose;
extern int paranoid;

/**************************************************************
 * assert
 **************************************************************/
extern void bench_assertion_failed(const char *s, int line, const char *file);
#define BENCH_ASSERT(ex)						 \
      (void)((ex) || (bench_assertion_failed(#ex, __LINE__, __FILE__), 0))

#define UNUSED(x) (void)x

/***************************************
 * Documentation strings
 ***************************************/
struct bench_doc {
     const char *key;
     const char *val;
     const char *(*f)(void);
};

extern struct bench_doc bench_doc[];

#ifdef CC
#define CC_DOC BENCH_DOC("cc", CC)
#elif defined(BENCH_CC)
#define CC_DOC BENCH_DOC("cc", BENCH_CC)
#else
#define CC_DOC /* none */
#endif

#ifdef CXX
#define CXX_DOC BENCH_DOC("cxx", CXX)
#elif defined(BENCH_CXX)
#define CXX_DOC BENCH_DOC("cxx", BENCH_CXX)
#else
#define CXX_DOC /* none */
#endif

#ifdef F77
#define F77_DOC BENCH_DOC("f77", F77)
#elif defined(BENCH_F77)
#define F77_DOC BENCH_DOC("f77", BENCH_F77)
#else
#define F77_DOC /* none */
#endif

#ifdef F90
#define F90_DOC BENCH_DOC("f90", F90)
#elif defined(BENCH_F90)
#define F90_DOC BENCH_DOC("f90", BENCH_F90)
#else
#define F90_DOC /* none */
#endif

#define BEGIN_BENCH_DOC						\
struct bench_doc bench_doc[] = {				\
    CC_DOC							\
    CXX_DOC							\
    F77_DOC							\
    F90_DOC

#define BENCH_DOC(key, val) { key, val, 0 },
#define BENCH_DOCF(key, f) { key, 0, f },

#define END_BENCH_DOC				\
     {0, 0, 0}};

#ifdef __cplusplus
}                               /* extern "C" */
#endif                          /* __cplusplus */
    
#endif /* __BENCH_USER_H__ */
