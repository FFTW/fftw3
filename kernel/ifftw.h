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

/* $Id: ifftw.h,v 1.98 2002-08-30 22:07:52 stevenj Exp $ */

/* FFTW internal header file */
#ifndef __IFFTW_H__
#define __IFFTW_H__

#include "config.h"

#include <stdlib.h>		/* size_t */
#include <stdarg.h>		/* va_list */
#include <stdio.h>              /* FILE */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>		/* uint, maybe */
#endif

/* determine precision and name-mangling scheme */
#define FFTW_MANGLE(prefix, name) prefix ## name
#if defined(FFTW_SINGLE)
typedef float fftw_real;
#define FFTW(name) FFTW_MANGLE(fftwf_, name)
#elif defined(FFTW_LDOUBLE)
typedef long double fftw_real;
#define FFTW(name) FFTW_MANGLE(fftwl_, name)
#else
typedef double fftw_real;
#define FFTW(name) FFTW_MANGLE(fftw_, name)
#endif

/* dummy use of unused parameters to avoid compiler warnings */
#define UNUSED(x) (void)x

/* shorthands */
typedef fftw_real R;
#define X FFTW

#define FFT_SIGN (-1)  /* sign convention for forward transforms */

/* get rid of that object-oriented stink: */
#define DESTROY(thing) ((thing)->adt->destroy)(thing)
#define REGISTER_SOLVER(p, s) ((p)->adt->register_solver)((p), (s))
#define REGISTER_PROBLEM(p, padt) ((p)->adt->register_problem)((p), (padt))
#define MKPLAN(plnr, prblm) ((plnr)->adt->mkplan)((plnr), (prblm))

#define STRINGIZEx(x) #x
#define STRINGIZE(x) STRINGIZEx(x)

#ifndef HAVE_K7
#define HAVE_K7 0
#endif

#if defined(HAVE_SSE) || defined(HAVE_SSE2) || defined(HAVE_ALTIVEC) || defined(HAVE_3DNOW)
#define HAVE_SIMD 1
#else
#define HAVE_SIMD 0
#endif

/* forward declarations */
typedef struct problem_s problem;
typedef struct plan_s plan;
typedef struct solver_s solver;
typedef struct planner_s planner;
typedef struct printer_s printer;
typedef struct scanner_s scanner;

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

extern void X(debug)(const char *format, ...);
#define D X(debug)

/*-----------------------------------------------------------------------*/
/* alloc.c: */
#define MIN_ALIGNMENT 16

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
     SLVPAIRS,
     TWIDDLES,
     STRIDES,
     OTHER,
     MALLOC_WHAT_LAST		/* must be last */
};

extern void X(free)(void *ptr);

#ifdef FFTW_DEBUG

extern void *X(malloc_debug)(size_t n, enum fftw_malloc_what what,
			     const char *file, int line);
#define fftw_malloc(n, what) X(malloc_debug)(n, what, __FILE__, __LINE__)
void X(malloc_print_minfo)(void);

#else /* ! FFTW_DEBUG */

extern void *X(malloc_plain)(size_t sz);
#define fftw_malloc(n, what)  X(malloc_plain)(n)

#endif

#if defined(FFTW_DEBUG) && defined(HAVE_THREADS)
extern int X(in_thread);
#  define IN_THREAD X(in_thread)
#  define THREAD_ON { int in_thread_save = X(in_thread); X(in_thread) = 1
#  define THREAD_OFF X(in_thread) = in_thread_save; }
#else
#  define IN_THREAD 0
#  define THREAD_ON 
#  define THREAD_OFF 
#endif

/*-----------------------------------------------------------------------*/
/* alloca: */
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif /* HAVE_ALLOCA_H */

#ifdef HAVE_ALLOCA
/* use alloca if available */
#define STACK_MALLOC(T, p, x)				\
{							\
     p = (T)alloca((x) + MIN_ALIGNMENT);		\
     p = (T)(((unsigned long)p + (MIN_ALIGNMENT - 1)) &	\
           (~(unsigned long)(MIN_ALIGNMENT - 1)));	\
}
#define STACK_FREE(x) 

#else /* ! HAVE_ALLOCA */
/* use malloc instead of alloca */
#define STACK_MALLOC(T, p, x) p = (T)fftw_malloc(x, OTHER)
#define STACK_FREE(x) X(free)(x)
#endif /* ! HAVE_ALLOCA */


/*-----------------------------------------------------------------------*/
/* ops.c: */
/*
 * ops counter.  The total number of additions is add + fma
 * and the total number of multiplications is mul + fma.
 * Total flops = add + mul + 2 * fma
 */
typedef struct {
     uint add;
     uint mul;
     uint fma;
     uint other;
} opcnt;

opcnt X(ops_add)(opcnt a, opcnt b);
opcnt X(ops_add3)(opcnt a, opcnt b, opcnt c);
opcnt X(ops_mul)(uint a, opcnt b);
opcnt X(ops_other)(uint o);
extern const opcnt X(ops_zero);

/*-----------------------------------------------------------------------*/
/* minmax.c: */
int X(imax)(int a, int b);
int X(imin)(int a, int b);
uint X(uimax)(uint a, uint b);
uint X(uimin)(uint a, uint b);

/*-----------------------------------------------------------------------*/
/* iabs.c: */
uint X(iabs)(int a);

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
tensor X(mktensor_rowmajor)(uint rnk, const uint *n,
			    const uint *niphys, const uint *nophys,
                            int is, int os);
int X(tensor_equal)(const tensor a, const tensor b);
uint X(tensor_sz)(const tensor sz);
uint X(tensor_hash)(const tensor t);
uint X(tensor_max_index)(const tensor sz);
uint X(tensor_min_istride)(const tensor sz);
uint X(tensor_min_ostride)(const tensor sz);
uint X(tensor_min_stride)(const tensor sz);
int X(tensor_inplace_strides)(const tensor sz);
tensor X(tensor_copy)(const tensor sz);
typedef enum { INPLACE_IS, INPLACE_OS } inplace_kind;
tensor X(tensor_copy_inplace)(const tensor sz, inplace_kind k);
tensor X(tensor_copy_except)(const tensor sz, uint except_dim);
tensor X(tensor_copy_sub)(const tensor sz, uint start_dim, uint rnk);
tensor X(tensor_compress)(const tensor sz);
tensor X(tensor_compress_contiguous)(const tensor sz);
tensor X(tensor_append)(const tensor a, const tensor b);
void X(tensor_split)(const tensor sz, tensor *a, uint a_rnk, tensor *b);
void X(tensor_tornk1)(const tensor *t, uint *n, int *is, int *os);
void X(tensor_destroy)(tensor sz);
void X(tensor_print)(tensor sz, printer *p);
int X(tensor_scan)(tensor *x, scanner *sc);

/*-----------------------------------------------------------------------*/
/* dotens.c: */
typedef struct dotens_closure_s {
     void (*apply)(struct dotens_closure_s *k, int indx, int ondx);
} dotens_closure;

void X(dotens)(tensor sz, dotens_closure *k);

/*-----------------------------------------------------------------------*/
/* dotens2.c: */
typedef struct dotens2_closure_s {
     void (*apply)(struct dotens2_closure_s *k, 
		   int indx0, int ondx0, int indx1, int ondx1);
} dotens2_closure;

void X(dotens2)(tensor sz0, tensor sz1, dotens2_closure *k);
/*-----------------------------------------------------------------------*/
/* problem.c: */
typedef struct {
     int (*equal) (const problem *ego, const problem *p);
     uint (*hash) (const problem *ego);
     void (*zero) (const problem *ego);
     void (*print) (problem *ego, printer *p);
     void (*destroy) (problem *ego);
     int (*scan)(scanner *sc, problem **p);
     const char *nam;
} problem_adt;

struct problem_s {
     const problem_adt *adt;
     int refcnt;
};

typedef struct prbpair_s {
     const problem_adt *adt;
     const char *reg_nam;
     int mark;
     struct prbpair_s *cdr;
} prbpair;

problem *X(mkproblem)(size_t sz, const problem_adt *adt);
void X(problem_destroy)(problem *ego);
problem *X(problem_dup)(problem *ego);

/*-----------------------------------------------------------------------*/
/* print.c */
struct printer_s {
     void (*print)(printer *p, const char *format, ...);
     void (*vprint)(printer *p, const char *format, va_list ap);
     void (*putchr)(printer *p, char c);
     uint indent;
     uint indent_incr;
};

printer *X(mkprinter)(size_t size, void (*putchr)(printer *p, char c));
void X(printer_destroy)(printer *p);

/*-----------------------------------------------------------------------*/
/* printers.c */

printer *X(mkprinter_file)(FILE *f);
printer *X(mkprinter_cnt)(uint *cnt);
printer *X(mkprinter_str)(char *s);

/*-----------------------------------------------------------------------*/
/* scan.c */
struct scanner_s {
     int (*scan)(scanner *sc, const char *format, ...);
     int (*vscan)(scanner *sc, const char *format, va_list ap);
     int (*getchr)(scanner *sc);
     int ungotc;
     const prbpair *problems;
};

scanner *X(mkscanner)(size_t size, int (*getchr)(scanner *sc),
		      const prbpair *probs);
void X(scanner_destroy)(scanner *sc);
int X(scanner_getchr)(scanner *sc);
void X(scanner_ungetchr)(scanner *sc, int c);

/*-----------------------------------------------------------------------*/
/* scanners.c */

scanner *X(mkscanner_file)(FILE *f, const prbpair *probs);
scanner *X(mkscanner_str)(const char *s, const prbpair *probs);

/*-----------------------------------------------------------------------*/
/* traverse.c */
typedef struct visit_closure_s {
     void (*visit)(struct visit_closure_s *, plan *);
} visit_closure;

void X(traverse_plan)(plan *p, int recur, visit_closure *k);

/*-----------------------------------------------------------------------*/
/* plan.c: */
typedef struct {
     void (*solve)(plan *ego, const problem *p);
     void (*awake)(plan *ego, int flag);
     void (*print)(plan *ego, printer *p);
     void (*destroy)(plan *ego);
} plan_adt;

struct plan_s {
     const plan_adt *adt;
     int refcnt;
     int awake_refcnt;
     opcnt ops;
     double pcost;
     int blessed;
     int score;
};

plan *X(mkplan)(size_t size, const plan_adt *adt);
void X(plan_use)(plan *ego);
void X(plan_destroy)(plan *ego);
void X(plan_awake)(plan *ego, int flag);
#define AWAKE(plan, flag) X(plan_awake)(plan, flag)
void X(plan_bless)(plan *ego);

/*-----------------------------------------------------------------------*/
/* solver.c: */
enum {
     BAD,   /* solver cannot solve problem */
     UGLY,  /* we are 99% sure that this solver is suboptimal */
     GOOD
};

typedef struct {
     plan *(*mkplan)(const solver *ego, const problem *p, planner *plnr);
     int (*score)(const solver *ego, const problem *p, const planner *plnr);
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

typedef struct slvpair_s {
     solver *slv;
     const char *reg_nam;
     int id;
     struct slvpair_s *cdr;
} slvpair;

typedef struct solutions_s solutions; /* opaque */

/* planner flags */
enum { IMPATIENT = 0x1, PATIENT = 0x0,
       ESTIMATE = 0x2, 
       CLASSIC_VRECURSE = 0x4,
       FORCE_VRECURSE = 0x8, 
       DESTROY_INPUT = 0x10,
       POSSIBLY_UNALIGNED = 0x20,
       FORBID_DHT_R2HC = 0x40
};

#define NONPATIENCE_FLAGS(flags) ((flags) & ~(ESTIMATE | IMPATIENT | PATIENT))
#define IMPATIENCE(flags) ((flags) & (ESTIMATE | IMPATIENT | PATIENT))

typedef enum { FORGET_PLANS, FORGET_ACCURSED, FORGET_EVERYTHING } amnesia;

typedef struct {
     void (*register_solver)(planner *ego, solver *s);
     void (*register_problem)(planner *ego, const problem_adt *adt);
     plan *(*mkplan)(planner *ego, problem *p);
     void (*forget)(planner *ego, amnesia a);
     void (*exprt)(planner *ego, printer *p); /* export is a reserved word
						 in C++.  Idiots. */
     int (*imprt)(planner *ego, scanner *sc);
     void (*exprt_conf)(planner *ego, printer *p);
     plan *(*slv_mkplan)(planner *ego, problem *p, solver *s);
} planner_adt;

struct planner_s {
     const planner_adt *adt;
     uint nplan;    /* number of plans evaluated */
     uint nprob;    /* number of problems evaluated */
     void (*hook)(plan *plan, const problem *p);

     const char *cur_reg_nam;
     slvpair *solvers, **last_solver_cdr;
     solutions **sols;
     prbpair *problems;
     void (*destroy)(planner *ego);
     void (*inferior_mkplan)(planner *ego, problem *p, plan **, slvpair **);
     uint hashsiz;
     uint cnt;
     int flags;
     uint nthr;
     int idcnt;
};

planner *X(mkplanner)(size_t sz,
		      void (*mkplan)(planner *ego, problem *p, 
				     plan **, slvpair **),
                      void (*destroy) (planner *), 
		      int flags);
void X(planner_destroy)(planner *ego);
void X(planner_set_hook)(planner *p, void (*hook)(plan *, const problem *));
void X(evaluate_plan)(planner *ego, plan *pln, const problem *p);

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
#define FORALL_SOLVERS(ego, s, p, what)		\
{						\
     slvpair *p;				\
     for (p = ego->solvers; p; p = p->cdr) {	\
	  solver *s = p->slv;			\
	  what;					\
     }						\
}

/* various planners */
planner *X(mkplanner_naive)(int flags);
planner *X(mkplanner_score)(int flags);

#define NO_VRECURSE(flags) (((flags) & IMPATIENT) && !((flags) & (CLASSIC_VRECURSE | FORCE_VRECURSE)))
#define CLASSIC_VRECURSE_RESET(plnr) { if ((plnr)->flags & CLASSIC_VRECURSE) (plnr)->flags &= ~FORCE_VRECURSE; }

/*-----------------------------------------------------------------------*/
/* stride.c: */

/* If PRECOMPUTE_ARRAY_INDICES is defined, precompute all strides. */
#if defined(__i386__) && !HAVE_K7
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
#define fftwf_mkstride(n, stride) stride
#define fftw_mkstride(n, stride) stride
#define fftwl_mkstride(n, stride) stride
#define fftwf_stride_destroy(p) {}
#define fftw_stride_destroy(p) {}
#define fftwl_stride_destroy(p) {}

#endif /* PRECOMPUTE_ARRAY_INDICES */

/*-----------------------------------------------------------------------*/
/* solvtab.c */

struct solvtab_s { void (*reg)(planner *); const char *reg_nam; };
typedef struct solvtab_s solvtab[];
void X(solvtab_exec)(const solvtab tbl, planner *p);
#define SOLVTAB(s) { s, STRINGIZE(s) }
#define SOLVTAB_END { 0, 0 }

/*-----------------------------------------------------------------------*/
/* pickdim.c */
int X(pickdim)(int which_dim, const int *buddies, uint nbuddies,
	       const tensor sz, int oop, uint *dp);

/*-----------------------------------------------------------------------*/
/* twiddle.c */
/* little language to express twiddle factors computation */
enum { TW_COS = 0, TW_SIN = 1, TW_TAN = 2, TW_NEXT = 3,
       TW_FULL = 4, TW_GENERIC = 5 };

typedef struct {
     unsigned char op;
     unsigned char v;
     short i;
} tw_instr;

typedef struct twid_s {
     R *W;                     /* array of twiddle factors */
     uint n, r, m;                /* transform order, radix, # twiddle rows */
     int refcnt;
     const tw_instr *instr;
     struct twid_s *cdr;
} twid;

void X(mktwiddle)(twid **pp, const tw_instr *instr, uint n, uint r, uint m);
void X(twiddle_destroy)(twid **pp);
uint X(twiddle_length)(uint r, const tw_instr *p);
void X(twiddle_awake)(int flg, twid **pp, 
		      const tw_instr *instr, uint n, uint r, uint m);

/*-----------------------------------------------------------------------*/
/* trig.c */
#ifdef FFTW_LDOUBLE
typedef long double trigreal;
#else
typedef double trigreal;
#endif

extern trigreal X(cos2pi)(int, uint);
extern trigreal X(sin2pi)(int, uint);
extern trigreal X(tan2pi)(int, uint);

/*-----------------------------------------------------------------------*/
/* primes.c: */

#if defined(FFTW_ENABLE_UNSAFE_MULMOD)
#  define MULMOD(x,y,p) (((x) * (y)) % (p))
#elif ((SIZEOF_UNSIGNED_INT != 0) && (SIZEOF_UNSIGNED_LONG_LONG >= 2 * SIZEOF_UNSIGNED_INT))
#  define MULMOD(x,y,p) ((uint) ((((unsigned long long) (x))    \
                                  * ((unsigned long long) (y))) \
				 % ((unsigned long long) (p))))
#else /* 'long long' unavailable */
#  define SAFE_MULMOD 1
uint X(safe_mulmod)(uint x, uint y, uint p);
#  define MULMOD(x,y,p) X(safe_mulmod)(x,y,p)
#endif

uint X(power_mod)(uint n, uint m, uint p);
uint X(find_generator)(uint p);
uint X(first_divisor)(uint n);
int X(is_prime)(uint n);

#define RADER_MIN_GOOD 71 /* min prime for which Rader becomes GOOD */

/*-----------------------------------------------------------------------*/
/* misc stuff */
void X(null_awake)(plan *ego, int awake);
int X(square)(int x);
double X(measure_execution_time)(plan *pln, const problem *p);
uint X(alignment_of)(R *p);
R *X(ptr_with_alignment)(uint algn);
extern const char *const FFTW(version);
extern const char *const FFTW(cc);
extern const char *const FFTW(codelet_optim);

/*-----------------------------------------------------------------------*/
/* macros used in codelets to reduce source code size */

typedef R E;  /* internal precision of codelets. */

#ifdef FFTW_LDOUBLE
#  define K(x) ((E) x##L)
#else
#  define K(x) ((E) x)
#endif
#define DK(name, value) const E name = K(value)

/* FMA macros */

#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ E FMA(E a, E b, E c)
{
     return a * b + c;
}

static __inline__ E FMS(E a, E b, E c)
{
     return a * b - c;
}

static __inline__ E FNMS(E a, E b, E c)
{
     return -(a * b - c);
}

#else
#define FMA(a, b, c) (((a) * (b)) + (c))
#define FMS(a, b, c) (((a) * (b)) - (c))
#define FNMS(a, b, c) ((c) - ((a) * (b)))
#endif

#endif /* __IFFTW_H__ */
