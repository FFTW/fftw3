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

/* $Id: ifftw.h,v 1.19 2002-06-10 14:55:40 athena Exp $ */

/* FFTW internal header file */
#ifndef __IFFTW_H__
#define __IFFTW_H__

#include "config.h"
#include "fftw3.h"

#include <stdlib.h>		/* size_t */

/* dummy use of unused parameters to avoid compiler warnings */
#define UNUSED(x) (void)x

/* shorthands */
typedef fftw_real R;
#define X FFTW

#ifndef HAVE_UINT
typedef unsigned int uint;
#endif

/* forward declarations */
typedef struct problem_s problem;
typedef struct plan_s plan;
typedef struct solver_s solver;
typedef struct planner_s planner;
typedef struct printer_s printer;

/*-----------------------------------------------------------------------*/
/* assert.c: */
extern void X(assertion_failed)(const char *s, int line, char *file);

/* always check */
#define CK(ex)						 \
      (void)((ex) || (X(assertion_failed)(#ex, __LINE__, __FILE__), 0))

#ifdef FFTW_DEBUG
/* check only if debug enabled */
#define A(ex)						 \
      (void)((ex) || (X(assertion_failed)(#ex, __LINE__, __FILE__), 0))
#else
#define A(ex) /* nothing */
#endif

/*-----------------------------------------------------------------------*/
/* alloc.c: */

/* objects allocated by malloc, for statistical purposes */
enum fftw_malloc_what {
     EVERYTHING,
     PLANS,
     SOLVERS,
     PROBLEMS,
     BUFFERS,
     HASHT,
     TENSORS,
     PLANNERS,
     PAIRS,
     TWIDDLES,
     OTHER,
     MALLOC_WHAT_LAST		/* must be last */
};

extern void X(free)(void *ptr);

#ifdef FFTW_DEBUG
extern void *X(malloc_debug)(size_t n, enum fftw_malloc_what what,
			     const char *file, int line);

#define fftw_malloc(n, what) \
     X(malloc_debug)(n, what, __FILE__, __LINE__)

void X(malloc_print_minfo)(void);

#else
extern void *X(malloc_plain)(size_t sz);

#define fftw_malloc(n, what)  X(malloc_plain)(n)
#endif

/*-----------------------------------------------------------------------*/
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

flopcnt X(flops_add)(flopcnt a, flopcnt b);
flopcnt X(flops_mul)(uint a, flopcnt b);
extern const flopcnt X(flops_zero);

/*-----------------------------------------------------------------------*/
/* minmax.c: */
int X(imax)(int a, int b);
int X(imin)(int a, int b);
uint X(uimax)(uint a, uint b);
uint X(uimin)(uint a, uint b);

/*-----------------------------------------------------------------------*/
/* tensor.c: */
typedef struct {
     uint n;
     int is;			/* input stride */
     int os;			/* output stride */
} iodim;

typedef struct {
     uint rnk;
     iodim *dims;
} tensor;

/*
  Definition of rank -infinity.
  This definition has the property that if you want rank 0 or 1,
  you can simply test for rank <= 1.  This is a common case.
 
  A tensor of rank -infinity has size 0.
*/
#define RNK_MINFTY  ((uint) -1)
#define FINITE_RNK(rnk) ((rnk) != RNK_MINFTY)

tensor X(mktensor)(uint rnk);
tensor X(mktensor_1d)(uint n, int is, int os);
tensor X(mktensor_2d)(uint n0, int is0, int os0,
                      uint n1, int is1, int os1);
tensor X(mktensor_rowmajor)(uint rnk, const uint *n, const uint *nphys,
                            int is, int os);
int X(tensor_equal)(const tensor a, const tensor b);
uint X(tensor_sz)(const tensor sz);
uint X(tensor_hash)(const tensor t);
int X(tensor_max_index)(const tensor sz);
int X(tensor_min_stride)(const tensor sz);
int X(tensor_inplace_strides)(const tensor sz);
tensor X(tensor_copy)(const tensor sz);
tensor X(tensor_copy_except)(const tensor sz, uint except_dim);
tensor X(tensor_copy_sub)(const tensor sz, uint start_dim, uint rnk);
tensor X(tensor_compress)(const tensor sz);
tensor X(tensor_compress_contiguous)(const tensor sz);
tensor X(tensor_append)(const tensor a, const tensor b);
void X(tensor_split)(const tensor sz, tensor *a, uint a_rnk, tensor *b);
void X(tensor_destroy)(tensor sz);

/*-----------------------------------------------------------------------*/
/* problem.c: */
typedef struct {
     int (*equal) (const problem *ego, const problem *p);
     uint (*hash) (const problem *ego);
     void (*zero) (const problem *ego);
     void (*destroy) (problem *ego);
} problem_adt;

struct problem_s {
     const problem_adt *adt;
     int refcnt;
};

problem *X(mkproblem)(size_t sz, const problem_adt *adt);
void X(problem_destroy)(problem *ego);
problem *X(problem_dup)(problem *ego);

/*-----------------------------------------------------------------------*/
/* print.c */
struct printer_s {
     void (*print)(printer *p, const char *format, ...);
     void (*putchr)(printer *p, char c);
     uint indent;
};

printer *X(mkprinter)(size_t size, void (*putchr)(printer *p, char c));
void X(printer_destroy)(printer *p);

/*-----------------------------------------------------------------------*/
/* plan.c: */
enum awake { SLEEP, AWAKE };

typedef struct {
     void (*solve)(plan *ego, const problem *p);
     void (*awake)(plan *ego, int awaken);
     void (*print)(plan *ego, printer *p);
     void (*destroy)(plan *ego);
} plan_adt;

struct plan_s {
     const plan_adt *adt;
     int refcnt;
     flopcnt flops;
     double cost;
};

plan *X(mkplan)(size_t size, const plan_adt *adt);
void X(plan_use)(plan *ego);
void X(plan_destroy)(plan *ego);

/*-----------------------------------------------------------------------*/
/* solver.c: */
enum {
     BAD,   /* solver cannot solve problem */
     UGLY,  /* we are 99% sure that this solver is suboptimal */
     GOOD
};

typedef struct {
     plan *(*mkplan)(const solver *ego, const problem *p, planner *plnr);
     int (*score)(const solver *ego, const problem *p);
} solver_adt;

struct solver_s {
     const solver_adt *adt;
     int refcnt;
};

solver *X(mksolver)(size_t size, const solver_adt *adt);
void X(solver_use)(solver *ego);
void X(solver_destroy)(solver *ego);

/* shorthand */
#define MKSOLVER(type, adt) (type *)X(mksolver)(sizeof(type), adt)

/*-----------------------------------------------------------------------*/
/* planner.c */

typedef struct pair_s pair; /* opaque */
typedef struct solutions_s solutions; /* opaque */

typedef struct {
     solver *(*car)(pair *cons);
     pair *(*cdr)(pair *cons);
     pair *(*solvers)(planner *ego);
     void (*register_solver)(planner *ego, solver *s);
     plan *(*mkplan)(planner *ego, problem *p);
} planner_adt;

struct planner_s {
     const planner_adt *adt;
     int ntry;  /* number of plans evaluated */
     void (*hook)(plan *plan, problem *p);

     pair *solvers;
     solutions **sols;
     void (*destroy)(planner *ego);
     plan *(*inferior_mkplan)(planner *ego, problem *p);
     uint hashsiz;
     uint cnt;
     int memoize;               /* If TRUE, turn memoization on */
     int memoize_failures;	/* if TRUE, also memoize unfeasible problems */
};

planner *X(mkplanner)(size_t sz, plan *(*mkplan)(planner *, problem *),
                      void (*destroy) (planner *));
void X(planner_destroy)(planner *ego);
void X(planner_set_hook)(planner *p, void (*hook)(plan *, problem *));

#ifdef FFTW_DEBUG
void X(planner_dump)(planner *ego, int verbose);
#endif

/*
  Iterate over all solvers.   Read:
 
  @article{ baker93iterators,
  author = "Henry G. Baker, Jr.",
  title = "Iterators: Signs of Weakness in Object-Oriented Languages",
  journal = "{ACM} {OOPS} Messenger",
  volume = "4",
  number = "3",
  pages = "18--25"
  }
*/
#define FORALL_SOLVERS(ego, s, what)					 \
{									 \
     pair *_l_;								 \
     for (_l_ = ego->adt->solvers(ego); _l_; _l_ = ego->adt->cdr(_l_)) { \
	  solver *s = ego->adt->car(_l_);				 \
	  what;								 \
     }									 \
}

/* various planners */
planner *X(mkplanner_naive)(void);
planner *X(mkplanner_estimate)(void);
planner *X(mkplanner_score)(void);

/*-----------------------------------------------------------------------*/
/* stride.c: */

/* If PRECOMPUTE_ARRAY_INDICES is defined, precompute all strides. */
#if defined(__i386__)
#define PRECOMPUTE_ARRAY_INDICES
#endif

#ifdef PRECOMPUTE_ARRAY_INDICES

typedef int *stride;
#define WS(stride, i)  (stride[i])
extern stride X(mkstride)(int n, int stride);
void X(stride_destroy)(stride p);

#else

typedef int stride;
#define WS(stride, i)  (stride * i)
#define X(mkstride)(n, stride) stride
#define X(stride_destroy)(p) {}

#endif /* PRECOMPUTE_ARRAY_INDICES */

/*-----------------------------------------------------------------------*/
/* solvtab.c */

typedef void (*const solvtab[])(planner *);
void X(solvtab_exec)(solvtab tbl, planner *p);

/*-----------------------------------------------------------------------*/
/* twiddle.c */
/* little language to express twiddle factors computation */
enum { TW_COS = 0, TW_SIN = 1, TW_TAN = 2, TW_NEXT = 3};

typedef struct {
     uint op:3;
     uint v:4;
     int i:25;
} tw_instr;

typedef struct twid_s {
     R *W;                     /* array of twiddle factors */
     uint r, m;                /* radix, N / r */
     int refcnt;
     const tw_instr *instr;
     struct twid_s *cdr;
} twid;

twid *X(mktwiddle)(const tw_instr *instr, uint r, uint m);
void X(twiddle_destroy)(twid *p);
uint X(twiddle_length)(const tw_instr *p);

/*-----------------------------------------------------------------------*/
/* misc stuff */
void X(null_awake)(plan *ego, int awake);
int X(square)(int x);
double X(measure_execution_time)(plan *pln, const problem *p);

#endif /* __IFFTW_H__ */
