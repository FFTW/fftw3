#include "bench-user.h"
#include <math.h>
#include <stdio.h>

extern void timer_start(void);
extern double timer_stop(void);

/*
  horrible hack for now.  This will go away once we define an interface
  for fftw
*/
#define problem fftw_problem
#include "ifftw.h"
#include "dft.h"
#include "codelet.h"
#undef problem
extern const char *FFTW(version);
extern const char *FFTW(cc);

/* END HACKS */

static const char *mkvers(void)
{
     return FFTW(version);
}

static const char *mkcc(void)
{
     return FFTW(cc);
}

BEGIN_BENCH_DOC
BENCH_DOC("name", "fftw3")
BENCH_DOCF("version", mkvers) 
BENCH_DOCF("fftw-compiled-by", mkcc)
END_BENCH_DOC 

static bench_real *ri, *ii, *ro, *io;
static int is, os;

void copy_c2c_from(struct problem *p, bench_complex *in)
{
     unsigned int i;
     if (p->sign == -1) {
	  for (i = 0; i < p->size; ++i) {
	       ri[i * is] = c_re(in[i]);
	       ii[i * is] = c_im(in[i]);
	  }
     } else {
	  for (i = 0; i < p->size; ++i) {
	       ii[i * is] = c_re(in[i]);
	       ri[i * is] = c_im(in[i]);
	  }
     }
}

void copy_c2c_to(struct problem *p, bench_complex *out)
{
     unsigned int i;
     if (p->sign == -1) {
	  for (i = 0; i < p->size; ++i) {
	       c_re(out[i]) = ro[i * os];
	       c_im(out[i]) = io[i * os];
	  }
     } else {
	  for (i = 0; i < p->size; ++i) {
	       c_re(out[i]) = io[i * os];
	       c_im(out[i]) = ro[i * os];
	  }
     }
}

static void putchr(printer *p, char c)
{
     UNUSED(p);
     putchar(c);
}

static void increment(plan *p, void *closure)
{
     int *count = (int *) closure;
     UNUSED(p);
     ++*count;
}

static void hook(plan *pln, const fftw_problem *p_)
{
     const problem_dft *p = (const problem_dft *) p_;

     if (verbose > 5) {
	  printer *pr = FFTW(mkprinter) (sizeof(printer), putchr);
	  pr->print(pr, "%P:%(%p%)\n", p, pln);
	  FFTW(printer_destroy) (pr);
	  printf("%g\n", pln->pcost);
     }

     if (paranoid) {
	  X(dft_verify)(pln, p, 5);
     }
}

int can_do(struct problem *p)
{
     return (sizeof(fftw_real) == sizeof(bench_real) &&
	     p->kind == PROBLEM_COMPLEX);
}

static planner *plnr;
static fftw_problem *prblm;
static plan *pln;

void setup(struct problem *p)
{
     double tplan;

     BENCH_ASSERT(can_do(p));

     plnr = FFTW(mkplanner_score)(0);
     FFTW(dft_conf_standard) (plnr);
     FFTW(planner_set_hook) (plnr, hook);
     /* plnr->flags |= CLASSIC | CLASSIC_VRECURSE; */

     if (p->split) {
	  is = os = 1;
	  if (p->sign == -1) {
	       ri = p->in;
	       ii = ri + p->size;
	       ro = p->out;
	       io = ro + p->size;
	  } else {
	       ii = p->in;
	       ri = ii + p->size;
	       io = p->out;
	       ro = io + p->size;
	  }
     } else {
	  is = os = 2;
	  if (p->sign == -1) {
	       ri = p->in;
	       ii = ri + 1;
	       ro = p->out;
	       io = ro + 1;
	  } else {
	       ii = p->in;
	       ri = ii + 1;
	       io = p->out;
	       ro = io + 1;
	  }
     }

     prblm = 
	  FFTW(mkproblem_dft_d)(
	       FFTW(mktensor_rowmajor)(p->rank, p->n, p->n, is, os),
	       FFTW(mktensor_rowmajor) (p->vrank, p->vn, p->vn,
					is * p->size, os * p->size), 
	       ri, ii, ro, io);
     timer_start();
     pln = plnr->adt->mkplan(plnr, prblm);
     tplan = timer_stop();
     BENCH_ASSERT(pln);
     X(plan_bless)(pln);

     if (verbose) {
	  printer *pr = FFTW(mkprinter) (sizeof(printer), putchr);
	  pr->print(pr, "%p\nnprob %u  nplan %u\n",
		    pln, plnr->nprob, plnr->nplan);
	  pr->print(pr, "%d add, %d mul, %d fma, %d other\n",
		    pln->ops.add, pln->ops.mul, pln->ops.fma, pln->ops.other);
	  pr->print(pr, "planner time: %g s\n", tplan);
	  if (verbose > 1) {
	       int count = 0;
	       FFTW(traverse_plan)(pln, increment, (void *) &count);
	       printf("number of child plans: %d\n", count - 1);
	  }
	  if (verbose > 3) 
	       plnr->adt->exprt(plnr, pr);
	  FFTW(printer_destroy)(pr);
     }
     AWAKE(pln, 1);
}

void doit(int iter, struct problem *p)
{
     int i;
     plan *PLN = pln;
     fftw_problem *PRBLM = prblm;

     UNUSED(p);
     for (i = 0; i < iter; ++i) {
	  PLN->adt->solve(PLN, PRBLM);
     }
}

void done(struct problem *p)
{
     UNUSED(p);

#    ifdef FFTW_DEBUG
     if (verbose >= 2)
	  FFTW(planner_dump)(plnr, verbose - 2);
#    endif

     AWAKE(pln, 0);
     FFTW(plan_destroy) (pln);
     FFTW(problem_destroy) (prblm);
     FFTW(planner_destroy) (plnr);

#    ifdef FFTW_DEBUG
     if (verbose >= 1)
	  FFTW(malloc_print_minfo)();
#    endif
}
