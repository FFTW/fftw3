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

/* $Id: bench-user.h,v 1.2 2003-01-18 12:20:18 athena Exp $ */
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

typedef bench_real bench_complex[2];

#define c_re(c)  ((c)[0])
#define c_im(c)  ((c)[1])

#undef DOUBLE_PRECISION
#define DOUBLE_PRECISION (sizeof(bench_real) == sizeof(double))
#undef SINGLE_PRECISION
#define SINGLE_PRECISION (!DOUBLE_PRECISION && sizeof(bench_real) == sizeof(float))
#undef LDOUBLE_PRECISION
#define LDOUBLE_PRECISION (!DOUBLE_PRECISION && sizeof(bench_real) == sizeof(long double))

typedef enum { PROBLEM_COMPLEX, PROBLEM_REAL } problem_kind_t;

typedef struct {
     int n;
     int is;			/* input stride */
     int os;			/* output stride */
} iodim;

typedef struct {
     int rnk;
     iodim *dims;
} tensor;

tensor *mktensor(int rnk);
void tensor_destroy(tensor *sz);
int tensor_sz(const tensor *sz);
tensor *tensor_compress(const tensor *sz);
int tensor_unitstridep(tensor *t);
int tensor_rowmajorp(tensor *t);
tensor *tensor_append(const tensor *a, const tensor *b);
tensor *tensor_copy(const tensor *sz);
void tensor_ibounds(tensor *t, int *lbp, int *ubp);
void tensor_obounds(tensor *t, int *lbp, int *ubp);

/*
  Definition of rank -infinity.
  This definition has the property that if you want rank 0 or 1,
  you can simply test for rank <= 1.  This is a common case.
 
  A tensor of rank -infinity has size 0.
*/
#define RNK_MINFTY  ((int)(((unsigned) -1) >> 1))
#define FINITE_RNK(rnk) ((rnk) != RNK_MINFTY)

struct problem {
     problem_kind_t kind;
     tensor *sz;
     tensor *vecsz;
     int sign;
     int in_place;
     int split;
     void *in, *out;
     void *inphys, *outphys;
     int iphyssz, ophyssz;
     void *userinfo; /* user can store whatever */
};

typedef struct problem problem;

extern int verbose;
extern void timer_start(void);
extern double timer_stop(void);

extern int can_do(struct problem *p);
extern void setup(struct problem *p);
extern void doit(int iter, struct problem *p);
extern void done(struct problem *p);
extern void verify(const char *param, int rounds, double tol);

extern void problem_alloc(struct problem *p);
extern void problem_free(struct problem *p);
extern void problem_zero(struct problem *p);

extern int power_of_two(int n);
extern int log_2(int n);

/**************************************************************
 * malloc
 **************************************************************/
extern void *bench_malloc(size_t size);
extern void bench_free(void *ptr);
extern void bench_free0(void *ptr);

/**************************************************************
 * alloca
 **************************************************************/
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

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
