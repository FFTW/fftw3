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

/* $Id: ifftw.h,v 1.3 2002-06-03 12:09:05 athena Exp $ */

/* FFTW internal header file */
#ifndef __IFFTW_H__
#define __IFFTW_H__

#include "fftw.h"
#include "config.h"

#include <stdlib.h>		/* size_t */

/* dummy use of unused parameters to avoid compiler warnings */
#define UNUSED(x) (void)x

/* shorthand */
typedef fftw_real R;

/* forward declarations */
typedef struct problem_s problem;
typedef struct plan_s plan;
typedef struct solver_s solver;
typedef struct planner_s planner;

/* assert.c: */
extern void fftw_assertion_failed(const char *s, int line, char *file);

/* always check */
#define CK(ex)						 \
      (void)((ex) || (fftw_assertion_failed(#ex, __LINE__, __FILE__), 0))

#ifdef FFTW_DEBUG
/* check only if debug enabled */
#define A(ex)						 \
      (void)((ex) || (fftw_assertion_failed(#ex, __LINE__, __FILE__), 0))
#else
#define A(ex) /* nothing */
#endif

/* alloc.c: */

/* objects allocated by malloc, for statistical purposes */
enum fftw_malloc_what {
     PLANS,
     SOLVERS,
     PROBLEMS,
     BUFFERS,
     HASHT,
     TENSORS,
     PLANNERS,
     TWIDDLES,
     OTHER,
     MALLOC_WHAT_LAST		/* must be last */
};

extern void fftw_free(void *ptr);

#ifdef FFTW_DEBUG
extern void *fftw_malloc_debug(size_t n, enum fftw_malloc_what what,
			       const char *file, int line);

#define fftw_malloc(n, what) \
     fftw_malloc_debug(n, what, __FILE__, __LINE__)

void fftw_malloc_print_minfo(void);

#else
extern void *fftw_malloc_plain(size_t sz);

#define fftw_malloc(n, what) \
     fftw_malloc_plain(n)
#endif

/* flops.c: */
/*
 * flops counter.  The total number of additions is add + fma
 * and the total number of multiplications is mul + fma.
 * Total flops = add + mul + 2 * fma
 */
typedef struct {
     int add;
     int mul;
     int fma;
} flopcnt;

flopcnt fftw_flops_add(flopcnt a, flopcnt b);
flopcnt fftw_flops_mul(int a, flopcnt b);
extern const flopcnt fftw_flops_zero;

/* minmax.c: */
int fftw_imax(int a, int b);
int fftw_imin(int a, int b);

/* rand.c: */
double fftw_drand(void);

/* tensor.c: */
typedef struct {
     int n;
     int is;			/* input stride */
     int os;			/* output stride */
} iodim;

typedef struct {
     int rnk;
     iodim *dims;
} tensor;

tensor fftw_mktensor(int rnk);
tensor fftw_mktensor_1d(int n, int is, int os);
tensor fftw_mktensor_2d(int n0, int is0, int os0,
			int n1, int is1, int os1);
tensor fftw_mktensor_rowmajor(int rnk,
			      const int *n,
			      const int *nphys, int is, int os);
int fftw_tensor_equal(const tensor a, const tensor b);
int fftw_tensor_sz(const tensor sz);
int fftw_tensor_hash(const tensor t);
int fftw_tensor_max_index(const tensor sz);
int fftw_tensor_min_stride(const tensor sz);
int fftw_tensor_inplace_strides(const tensor sz);
tensor fftw_tensor_copy(const tensor sz);
tensor fftw_tensor_copy_except(const tensor sz, int except_dim);
tensor fftw_tensor_copy_sub(const tensor sz, int start_dim, int rnk);
tensor fftw_tensor_compress(const tensor sz);
tensor fftw_tensor_compress_contiguous(const tensor sz);
tensor fftw_tensor_append(const tensor a, const tensor b);
void fftw_tensor_split(const tensor sz, tensor *a, int a_rnk, tensor *b);
void fftw_tensor_destroy(tensor sz);


/* problem.c: */
typedef struct {
     int (*equal) (const problem *ego, const problem *p);
     unsigned int (*hash) (const problem *ego);
     void (*zero) (const problem *ego);
     void (*destroy) (problem *ego);
} problem_adt;

struct problem_s {
     const problem_adt *adt;
     int refcnt;
};

problem *fftw_mkproblem(size_t sz, const problem_adt *adt);
void fftw_problem_destroy(problem *ego);
problem *fftw_problem_dup(problem *ego);

/* plan.c: */
typedef int (*plan_printf)(const char *format, ...);

enum awake { SLEEP, AWAKE };

typedef struct {
     void (*solve)(plan *ego, const problem *p);
     void (*awake)(plan *ego, int awaken);
     void (*print)(plan *ego, plan_printf prntf);
     void (*destroy)(plan *ego);
} plan_adt;

struct plan_s {
     const plan_adt *adt;
     int refcnt;
     flopcnt flops; 
     double cost;
};

plan *fftw_mkplan(size_t size, const plan_adt *adt);
void fftw_plan_use(plan *ego);
void fftw_plan_destroy(plan *ego);

/* solver.c: */
enum score {
     BAD,   /* solver cannot solve problem */
     UGLY,  /* we are 99% sure that this solver is suboptimal */
     GOOD
};

typedef struct {
     plan *(*mkplan)(const solver *ego, const problem *p, planner *planner);
     enum score (*score)(const solver *ego, const problem *p);
} solver_adt;

struct solver_s {
     const solver_adt *adt;
     int refcnt;
};

solver *fftw_mksolver(size_t size, const solver_adt *adt);
void fftw_solver_use(solver *ego);

  /* shorthand */
#define MKSOLVER(type, adt) (type *)fftw_mksolver(sizeof(type), adt)

/* stride.c: */

/* If PRECOMPUTE_ARRAY_INDICES is defined, precompute all strides. */
#if defined(__i386__)
#define PRECOMPUTE_ARRAY_INDICES
#endif

#ifdef PRECOMPUTE_ARRAY_INDICES

typedef int *stride;
#define WS(stride, i)  (stride[i])
extern stride fftw_mkstride(int n, int stride);
void fftw_stride_destroy(stride p);

#else

typedef int stride;
#define WS(stride, i)  (stride * i)
#define fftw_stride_make(n, stride) stride
#define fftw_stride_destroy(p) {}

#endif /* PRECOMPUTE_ARRAY_INDICES */

/* awake.c: */
void fftw_null_awake(plan *ego, int awake);

/* square.c: */
int fftw_square(int x);

#endif /* __IFFTW_H__ */
