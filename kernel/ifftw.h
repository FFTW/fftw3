/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* $Id: ifftw.h,v 1.209 2003-04-02 01:57:39 athena Exp $ */

/* FFTW internal header file */
#ifndef __IFFTW_H__
#define __IFFTW_H__

#include "config.h"

#include <stdlib.h>		/* size_t */
#include <stdarg.h>		/* va_list */
#include <stddef.h>             /* ptrdiff_t */

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if HAVE_STDINT_H
# include <stdint.h>             /* uintptr_t, maybe */
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>           /* uintptr_t, maybe */
#endif

/* determine precision and name-mangling scheme */
#define CONCAT(prefix, name) prefix ## name
#if defined(FFTW_SINGLE)
typedef float R;
#define X(name) CONCAT(fftwf_, name)
#elif defined(FFTW_LDOUBLE)
typedef long double R;
#define X(name) CONCAT(fftwl_, name)
#else
typedef double R;
#define X(name) CONCAT(fftw_, name)
#endif

/* dummy use of unused parameters to silence compiler warnings */
#define UNUSED(x) (void)x

#define FFT_SIGN (-1)  /* sign convention for forward transforms */

/* get rid of that object-oriented stink: */
#define REGISTER_SOLVER(p, s) X(solver_register)(p, s)

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
/* alloca: */
#if HAVE_SIMD
#define MIN_ALIGNMENT 16
#endif

#ifdef HAVE_ALLOCA
   /* use alloca if available */

#ifndef alloca
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   ifdef _AIX
 #pragma alloca
#   else
#    ifndef alloca /* predefined by HP cc +Olibcalls */
void *alloca(size_t);
#    endif
#   endif
#  endif
# endif
#endif
#endif

#  ifdef MIN_ALIGNMENT
#    define STACK_MALLOC(T, p, x)				\
     {								\
         p = (T)alloca((x) + MIN_ALIGNMENT);			\
         p = (T)(((uintptr_t)p + (MIN_ALIGNMENT - 1)) &	\
               (~(uintptr_t)(MIN_ALIGNMENT - 1)));		\
     }
#    define STACK_FREE(x) 
#  else /* HAVE_ALLOCA && !defined(MIN_ALIGNMENT) */
#    define STACK_MALLOC(T, p, x) p = (T)alloca(x) 
#    define STACK_FREE(x) 
#  endif

#else /* ! HAVE_ALLOCA */
   /* use malloc instead of alloca */
#  define STACK_MALLOC(T, p, x) p = (T)MALLOC(x, OTHER)
#  define STACK_FREE(x) X(ifree)(x)
#endif /* ! HAVE_ALLOCA */

/*-----------------------------------------------------------------------*/
/* define uintptr_t if it is not already defined */

#ifndef HAVE_UINTPTR_T
#  if SIZEOF_VOID_P == 0
#    error sizeof void* is unknown!
#  elif SIZEOF_UNSIGNED_INT == SIZEOF_VOID_P
     typedef unsigned int uintptr_t;
#  elif SIZEOF_UNSIGNED_LONG == SIZEOF_VOID_P
     typedef unsigned long uintptr_t;
#  elif SIZEOF_UNSIGNED_LONG_LONG == SIZEOF_VOID_P
     typedef unsigned long long uintptr_t;
#  else
#    error no unsigned integer type matches void* sizeof!
#  endif
#endif

/*-----------------------------------------------------------------------*/
/* assert.c: */
extern void X(assertion_failed)(const char *s, int line, const char *file);

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

/* objects allocated by malloc, for statistical purposes */
enum malloc_tag {
     EVERYTHING,
     PLANS,
     SOLVERS,
     PROBLEMS,
     BUFFERS,
     HASHT,
     TENSORS,
     PLANNERS,
     SLVDESCS,
     TWIDDLES,
     STRIDES,
     OTHER,
     MALLOC_WHAT_LAST		/* must be last */
};

extern void X(ifree)(void *ptr);
extern void X(ifree0)(void *ptr);

#ifdef FFTW_DEBUG_MALLOC

extern void *X(malloc_debug)(size_t n, enum malloc_tag what,
			     const char *file, int line);
#define MALLOC(n, what) X(malloc_debug)(n, what, __FILE__, __LINE__)
#define NATIVE_MALLOC(n, what) MALLOC(n, what)
void X(malloc_print_minfo)(int vrbose);

#else /* ! FFTW_DEBUG_MALLOC */

extern void *X(malloc_plain)(size_t sz);
#define MALLOC(n, what)  X(malloc_plain)(n)
#define NATIVE_MALLOC(n, what) malloc(n)

#endif

#if defined(FFTW_DEBUG) && defined(FFTW_DEBUG_MALLOC) && defined(HAVE_THREADS)
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
/* ops.c: */
/*
 * ops counter.  The total number of additions is add + fma
 * and the total number of multiplications is mul + fma.
 * Total flops = add + mul + 2 * fma
 */
typedef struct {
     int add;
     int mul;
     int fma;
     int other;
} opcnt;

void X(ops_zero)(opcnt *dst);
void X(ops_other)(int o, opcnt *dst);
void X(ops_cpy)(const opcnt *src, opcnt *dst);

void X(ops_add)(const opcnt *a, const opcnt *b, opcnt *dst);
void X(ops_add2)(const opcnt *a, opcnt *dst);

/* dst = m * a + b */
void X(ops_madd)(int m, const opcnt *a, const opcnt *b, opcnt *dst);

/* dst += m * a */
void X(ops_madd2)(int m, const opcnt *a, opcnt *dst);


/*-----------------------------------------------------------------------*/
/* minmax.c: */
int X(imax)(int a, int b);
int X(imin)(int a, int b);

/*-----------------------------------------------------------------------*/
/* iabs.c: */
int X(iabs)(int a);

/*-----------------------------------------------------------------------*/
/* md5.c */

#if SIZEOF_UNSIGNED_INT >= 4
typedef unsigned int md5uint;
#else
typedef unsigned long md5uint; /* at least 32 bits as per C standard */
#endif

typedef md5uint md5sig[4];

typedef struct {
     md5sig s; /* state and signature */

     /* fields not meant to be used outside md5.c: */
     unsigned char c[64]; /* stuff not yet processed */
     unsigned l;  /* total length.  Should be 64 bits long, but this is
		     good enough for us */
} md5;

void X(md5begin)(md5 *p);
void X(md5putb)(md5 *p, const void *d_, int len);
void X(md5puts)(md5 *p, const char *s);
void X(md5putc)(md5 *p, unsigned char c);
void X(md5int)(md5 *p, int i);
void X(md5unsigned)(md5 *p, unsigned i);
void X(md5ptrdiff)(md5 *p, ptrdiff_t d);
void X(md5end)(md5 *p);

/*-----------------------------------------------------------------------*/
/* tensor.c: */
#define STRUCT_HACK_KR
#undef STRUCT_HACK_C99

typedef struct {
     int n;
     int is;			/* input stride */
     int os;			/* output stride */
} iodim;

typedef struct {
     int rnk;
#if defined(STRUCT_HACK_KR)
     iodim dims[1];
#elif defined(STRUCT_HACK_C99)
     iodim dims[];
#else
     iodim *dims;
#endif
} tensor;

/*
  Definition of rank -infinity.
  This definition has the property that if you want rank 0 or 1,
  you can simply test for rank <= 1.  This is a common case.
 
  A tensor of rank -infinity has size 0.
*/
#define RNK_MINFTY  ((int)(((unsigned) -1) >> 1))
#define FINITE_RNK(rnk) ((rnk) != RNK_MINFTY)

typedef enum { INPLACE_IS, INPLACE_OS } inplace_kind;

tensor *X(mktensor)(int rnk);
tensor *X(mktensor_0d)(void);
tensor *X(mktensor_1d)(int n, int is, int os);
tensor *X(mktensor_2d)(int n0, int is0, int os0,
                      int n1, int is1, int os1);
int X(tensor_sz)(const tensor *sz);
void X(tensor_md5)(md5 *p, const tensor *t);
int X(tensor_max_index)(const tensor *sz);
int X(tensor_min_istride)(const tensor *sz);
int X(tensor_min_ostride)(const tensor *sz);
int X(tensor_min_stride)(const tensor *sz);
int X(tensor_inplace_strides)(const tensor *sz);
int X(tensor_inplace_strides2)(const tensor *a, const tensor *b);
tensor *X(tensor_copy)(const tensor *sz);
int X(tensor_kosherp)(const tensor *x);

tensor *X(tensor_copy_inplace)(const tensor *sz, inplace_kind k);
tensor *X(tensor_copy_except)(const tensor *sz, int except_dim);
tensor *X(tensor_copy_sub)(const tensor *sz, int start_dim, int rnk);
tensor *X(tensor_compress)(const tensor *sz);
tensor *X(tensor_compress_contiguous)(const tensor *sz);
tensor *X(tensor_append)(const tensor *a, const tensor *b);
void X(tensor_split)(const tensor *sz, tensor **a, int a_rnk, tensor **b);
int X(tensor_tornk1)(const tensor *t, int *n, int *is, int *os);
void X(tensor_destroy)(tensor *sz);
void X(tensor_destroy2)(tensor *a, tensor *b);
void X(tensor_destroy4)(tensor *a, tensor *b, tensor *c, tensor *d);
void X(tensor_print)(const tensor *sz, printer *p);
int X(dimcmp)(const iodim *a, const iodim *b);

/*-----------------------------------------------------------------------*/
/* problem.c: */
typedef struct {
     void (*hash) (const problem *ego, md5 *p);
     void (*zero) (const problem *ego);
     void (*print) (problem *ego, printer *p);
     void (*destroy) (problem *ego);
} problem_adt;

struct problem_s {
     const problem_adt *adt;
};

problem *X(mkproblem)(size_t sz, const problem_adt *adt);
void X(problem_destroy)(problem *ego);

/*-----------------------------------------------------------------------*/
/* print.c */
struct printer_s {
     void (*print)(printer *p, const char *format, ...);
     void (*vprint)(printer *p, const char *format, va_list ap);
     void (*putchr)(printer *p, char c);
     void (*cleanup)(printer *p);
     int indent;
     int indent_incr;
};

printer *X(mkprinter)(size_t size, 
		      void (*putchr)(printer *p, char c),
		      void (*cleanup)(printer *p));
void X(printer_destroy)(printer *p);

/*-----------------------------------------------------------------------*/
/* scan.c */
struct scanner_s {
     int (*scan)(scanner *sc, const char *format, ...);
     int (*vscan)(scanner *sc, const char *format, va_list ap);
     int (*getchr)(scanner *sc);
     int ungotc;
};

scanner *X(mkscanner)(size_t size, int (*getchr)(scanner *sc));
void X(scanner_destroy)(scanner *sc);

/*-----------------------------------------------------------------------*/
/* plan.c: */
typedef struct {
     void (*solve)(const plan *ego, const problem *p);
     void (*awake)(plan *ego, int flag);
     void (*print)(const plan *ego, printer *p);
     void (*destroy)(plan *ego);
} plan_adt;

struct plan_s {
     const plan_adt *adt;
     int awake_refcnt;
     opcnt ops;
     double pcost;
};

plan *X(mkplan)(size_t size, const plan_adt *adt);
void X(plan_destroy_internal)(plan *ego);
void X(plan_awake)(plan *ego, int flag);
#define AWAKE(plan, flag) X(plan_awake)(plan, flag)
void X(plan_null_destroy)(plan *ego);

/*-----------------------------------------------------------------------*/
/* solver.c: */
typedef struct {
     plan *(*mkplan)(const solver *ego, const problem *p, planner *plnr);
} solver_adt;

struct solver_s {
     const solver_adt *adt;
     int refcnt;
};

solver *X(mksolver)(size_t size, const solver_adt *adt);
void X(solver_use)(solver *ego);
void X(solver_destroy)(solver *ego);
void X(solver_register)(planner *plnr, solver *s);

/* shorthand */
#define MKSOLVER(type, adt) (type *)X(mksolver)(sizeof(type), adt)

/*-----------------------------------------------------------------------*/
/* planner.c */

typedef struct slvdesc_s {
     solver *slv;
     const char *reg_nam;
     unsigned nam_hash;
     int reg_id;
} slvdesc;

typedef struct solution_s solution; /* opaque */

/* values for problem_flags: */
enum { 
     DESTROY_INPUT = 0x1,
     UNALIGNED = 0x2,
     CONSERVE_MEMORY = 0x4,
     NO_DHT_R2HC = 0x8
};

#define DESTROY_INPUTP(plnr) ((plnr)->problem_flags & DESTROY_INPUT)
#define UNALIGNEDP(plnr) ((plnr)->problem_flags & UNALIGNED)
#define CONSERVE_MEMORYP(plnr) ((plnr)->problem_flags & CONSERVE_MEMORY)
#define NO_DHT_R2HCP(plnr) ((plnr)->problem_flags & NO_DHT_R2HC)

/* values for planner_flags: */
enum {
     /* impatience flags  */

     BELIEVE_PCOST = 0x1,
     DFT_R2HC_ICKY = 0x2,
     NONTHREADED_ICKY = 0x4,
     NO_BUFFERING = 0x8,
     NO_EXHAUSTIVE = 0x10,
     NO_INDIRECT_OP = 0x20,
     NO_LARGE_GENERIC = 0x40,
     NO_RANK_SPLITS = 0x80,
     NO_VRANK_SPLITS = 0x100,
     NO_VRECURSE = 0x200,
     
     /* flags that control the search */
     NO_UGLY = 0x400,  /* avoid plans we are 99% sure are suboptimal */
     NO_SEARCH = 0x800,  /* avoid searching altogether---use wisdom entries 
			    only */

     ESTIMATE = 0x1000,
     IMPATIENCE_FLAGS = (ESTIMATE | (ESTIMATE - 1)),
     
     BLESSING = 0x4000,  /* save this entry */
     H_VALID = 0x8000,    /* valid hastable entry */
     NONIMPATIENCE_FLAGS = BLESSING
};

#define BELIEVE_PCOSTP(plnr) ((plnr)->planner_flags & BELIEVE_PCOST)
#define DFT_R2HC_ICKYP(plnr) ((plnr)->planner_flags & DFT_R2HC_ICKY)
#define ESTIMATEP(plnr) ((plnr)->planner_flags & ESTIMATE)
#define NONTHREADED_ICKYP(plnr) (((plnr)->planner_flags & NONTHREADED_ICKY) \
                                  && (plnr)->nthr > 1)
#define NO_BUFFERINGP(plnr) ((plnr)->planner_flags & NO_BUFFERING)
#define NO_EXHAUSTIVEP(plnr) ((plnr)->planner_flags & NO_EXHAUSTIVE)
#define NO_INDIRECT_OP_P(plnr) ((plnr)->planner_flags & NO_INDIRECT_OP)
#define NO_LARGE_GENERICP(plnr) ((plnr)->planner_flags & NO_LARGE_GENERIC)
#define NO_RANK_SPLITSP(plnr) ((plnr)->planner_flags & NO_RANK_SPLITS)
#define NO_UGLYP(plnr) ((plnr)->planner_flags & NO_UGLY)
#define NO_SEARCHP(plnr) ((plnr)->planner_flags & NO_SEARCH)
#define NO_VRANK_SPLITSP(plnr) ((plnr)->planner_flags & NO_VRANK_SPLITS)
#define NO_VRECURSEP(plnr) ((plnr)->planner_flags & NO_VRECURSE)

typedef enum { FORGET_ACCURSED, FORGET_EVERYTHING } amnesia;

typedef struct {
     void (*register_solver)(planner *ego, solver *s);
     plan *(*mkplan)(planner *ego, problem *p);
     void (*forget)(planner *ego, amnesia a);
     void (*exprt)(planner *ego, printer *p); /* ``export'' is a reserved
						 word in C++. */
     int (*imprt)(planner *ego, scanner *sc);
} planner_adt;

struct planner_s {
     const planner_adt *adt;
     void (*hook)(plan *pln, const problem *p, int optimalp);

     /* solver descriptors */
     slvdesc *slvdescs;
     unsigned nslvdesc, slvdescsiz;
     const char *cur_reg_nam;
     int cur_reg_id;

     /* hash table of solutions */
     solution *solutions;
     unsigned hashsiz, nelem;

     int nthr;
     unsigned problem_flags;
     unsigned short planner_flags; /* matches type of solution.flags in
				      planner.c */
     /* various statistics */
     int nplan;    /* number of plans evaluated */
     double pcost, epcost; /* total pcost of measured/estimated plans */
     int nprob;    /* number of problems evaluated */
     int lookup, succ_lookup, lookup_iter;
     int insert, insert_iter, insert_unknown;
     int nrehash;
};

planner *X(mkplanner)(void);
void X(planner_destroy)(planner *ego);

#ifdef FFTW_DEBUG
void X(planner_dump)(planner *ego, int vrbose);
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
#define FORALL_SOLVERS(ego, s, p, what)			\
{							\
     unsigned _cnt;					\
     for (_cnt = 0; _cnt < ego->nslvdesc; ++_cnt) {	\
	  slvdesc *p = ego->slvdescs + _cnt;		\
	  solver *s = p->slv;				\
	  what;						\
     }							\
}

/* make plan, destroy problem */
plan *X(mkplan_d)(planner *ego, problem *p);

/*-----------------------------------------------------------------------*/
/* stride.c: */

/* If PRECOMPUTE_ARRAY_INDICES is defined, precompute all strides. */
#if defined(__i386__) && !HAVE_K7 && !defined(FFTW_LDOUBLE)
#define PRECOMPUTE_ARRAY_INDICES
#endif

#ifdef PRECOMPUTE_ARRAY_INDICES
typedef int *stride;
#define WS(stride, i)  (stride[i])
extern stride X(mkstride)(int n, int s);
void X(stride_destroy)(stride p);

#else

typedef int stride;
#define WS(stride, i)  (stride * i)
#define fftwf_mkstride(n, stride) stride
#define fftw_mkstride(n, stride) stride
#define fftwl_mkstride(n, stride) stride
#define fftwf_stride_destroy(p) ((void) p)
#define fftw_stride_destroy(p) ((void) p)
#define fftwl_stride_destroy(p) ((void) p)

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
int X(pickdim)(int which_dim, const int *buddies, int nbuddies,
	       const tensor *sz, int oop, int *dp);

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
     int n, r, m;                /* transform order, radix, # twiddle rows */
     int refcnt;
     const tw_instr *instr;
     struct twid_s *cdr;
} twid;

void X(mktwiddle)(twid **pp, const tw_instr *instr, int n, int r, int m);
void X(twiddle_destroy)(twid **pp);
int X(twiddle_length)(int r, const tw_instr *p);
void X(twiddle_awake)(int flg, twid **pp, 
		      const tw_instr *instr, int n, int r, int m);

/*-----------------------------------------------------------------------*/
/* trig.c */
#ifdef FFTW_LDOUBLE
typedef long double trigreal;
#else
typedef double trigreal;
#endif

extern trigreal X(cos2pi)(int, int);
extern trigreal X(sin2pi)(int, int);
extern trigreal X(tan2pi)(int, int);
extern trigreal X(sincos)(trigreal m, trigreal n, int sinp);

/*-----------------------------------------------------------------------*/
/* primes.c: */

#if defined(FFTW_ENABLE_UNSAFE_MULMOD)
#  define MULMOD(x,y,p) (((x) * (y)) % (p))
#elif ((SIZEOF_INT != 0) && (SIZEOF_LONG >= 2 * SIZEOF_INT))
#  define MULMOD(x,y,p) ((int) ((((long) (x)) * ((long) (y))) % ((long) (p))))
#elif ((SIZEOF_INT != 0) && (SIZEOF_LONG_LONG >= 2 * SIZEOF_INT))
#  define MULMOD(x,y,p) ((int) ((((long long) (x)) * ((long long) (y))) \
				 % ((long long) (p))))
#else /* 'long long' unavailable */
#  define SAFE_MULMOD 1
int X(safe_mulmod)(int x, int y, int p);
#  define MULMOD(x,y,p) X(safe_mulmod)(x,y,p)
#endif

int X(power_mod)(int n, int m, int p);
int X(find_generator)(int p);
int X(first_divisor)(int n);
int X(is_prime)(int n);
int X(next_prime)(int n);

#define GENERIC_MIN_BAD 71 /* min prime for which generic becomes bad */

/*-----------------------------------------------------------------------*/
/* rader.c: */
typedef struct rader_tls rader_tl;

void X(rader_tl_insert)(int k1, int k2, int k3, R *W, rader_tl **tl);
R *X(rader_tl_find)(int k1, int k2, int k3, rader_tl *t);
void X(rader_tl_delete)(R *W, rader_tl **tl);

/*-----------------------------------------------------------------------*/
/* transpose.c: */

void X(transpose)(R *A, int n, int m, int d, int N, R *buf);
void X(transpose_slow)(R *a, int nx, int ny, int N,
		       char *move, int move_size, R *buf);
int X(transposable)(const iodim *a, const iodim *b,
		    int vl, int s, R *ri, R *ii);
void X(transpose_dims)(const iodim *a, const iodim *b,
                       int *n, int *m, int *d, int *nd, int *md);
int X(transpose_simplep)(const iodim *a, const iodim *b, int vl, int s,
			 R *ri, R *ii);
int X(transpose_slowp)(const iodim *a, const iodim *b, int N);

/*-----------------------------------------------------------------------*/
/* misc stuff */
void X(null_awake)(plan *ego, int awake);
int X(square)(int x);
double X(measure_execution_time)(plan *pln, const problem *p);
int X(alignment_of)(R *p);
R *X(most_unaligned)(R *p1, R *p2);
void X(most_unaligned_complex)(R *r, R *i, R **rp, R **ip, int s);
unsigned X(hash)(const char *s);
int X(compute_nbuf)(int n, int vl, int nbuf, int maxbufsz);
int X(ct_uglyp)(int min_n, int n, int r);
void X(with_aligned_stack)(void (*f)(void *), void *p);

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

/* inline nontwiddle codelets, at the discretion of the compiler */
#define MAYBE_INLINE inline

#endif /* __IFFTW_H__ */
