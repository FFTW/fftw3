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
#include "rdft.h"
#undef problem
extern const char *const FFTW(version);
extern const char *const FFTW(cc);
extern const char *const FFTW(codelet_optim);

/* END HACKS */

static const char *mkvers(void)
{
     return FFTW(version);
}

static const char *mkcc(void)
{
     return FFTW(cc);
}

static const char *mkcodelet_optim(void)
{
     return FFTW(codelet_optim);
}

BEGIN_BENCH_DOC
BENCH_DOC("name", "fftw3")
BENCH_DOCF("version", mkvers) 
BENCH_DOCF("fftw-compiled-by", mkcc)
BENCH_DOCF("codelet-optim", mkcodelet_optim)
END_BENCH_DOC 

static bench_real *ri, *ii, *ro, *io;
static int is, os;

void copy_c2c_from(struct problem *p, bench_complex *in)
{
     unsigned int i;
     if (p->sign == FFT_SIGN) {
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
     if (p->sign == FFT_SIGN) {
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

void copy_h2c(struct problem *p, bench_complex *out)
{
     if (p->split)
	  copy_h2c_1d_halfcomplex(p, out, FFT_SIGN);
     else
	  copy_h2c_unpacked(p, out, FFT_SIGN);
}

void copy_c2h(struct problem *p, bench_complex *in)
{
     if (p->split)
	  copy_c2h_1d_halfcomplex(p, in, FFT_SIGN);
     else
	  copy_c2h_unpacked(p, in, FFT_SIGN);
}

typedef struct {
     visit_closure super;
     int count;
} increment_closure;

static void increment(visit_closure *k_, plan *p)
{
     increment_closure *k = (increment_closure *)k_;
     UNUSED(p);
     ++k->count;
}

static void hook(plan *pln, const fftw_problem *p_)
{
     if (verbose > 5) {
	  printer *pr = FFTW(mkprinter_file) (stdout);
	  pr->print(pr, "%P:%(%p%)\n", p_, pln);
	  FFTW(printer_destroy) (pr);
	  printf("cost %g  score %d\n\n", pln->pcost, pln->score);
     }

     if (paranoid) {
	  if (DFTP(p_))
	       X(dft_verify)(pln, (const problem_dft *) p_, 5);
	  else if (RDFTP(p_))
	       X(rdft_verify)(pln, (const problem_rdft *) p_, 5);
     }
}

int can_do(struct problem *p)
{
     return (sizeof(fftw_real) == sizeof(bench_real) &&
	     p->kind == PROBLEM_COMPLEX || p->kind == PROBLEM_REAL);
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
     FFTW(rdft_conf_standard) (plnr);
     FFTW(planner_set_hook) (plnr, hook);
     /* plnr->flags |= CLASSIC | CLASSIC_VRECURSE; */

#if 0
     {
	  FILE *f;
	  if ((f = fopen("wis.dat", "r"))) {
	       scanner *sc = X(mkscanner_file)(f, plnr->problems);
	       if (!plnr->adt->imprt(plnr, sc))
		    fprintf(stderr, "bench: ERROR reading wis.dat!\n");
	       X(scanner_destroy)(sc);
	       fclose(f);
	  }
     }
#endif

     if (p->kind == PROBLEM_REAL) {
	  if (p->split) {
	       is = os = 1;
	       ri = ii = p->in;
	       ro = io = p->out;
	  }
	  else if (p->sign == FFT_SIGN) {
	       is = 1; os = 2;
	       ri = ii = p->in;
	       ro = p->out; io = ro + 1;
	  }
	  else {
	       is = 2; os = 1;
	       ri = ii = p->out;
	       ro = p->in; io = ro + 1;
	  }
     }
     else if (p->split) {
	  is = os = 1;
	  if (p->sign == FFT_SIGN) {
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
	  if (p->sign == FFT_SIGN) {
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

     if (p->kind == PROBLEM_COMPLEX)
	  prblm = 
	       FFTW(mkproblem_dft_d)(
		    FFTW(mktensor_rowmajor)(p->rank, p->n, p->n, is, os),
		    FFTW(mktensor_rowmajor) (p->vrank, p->vn, p->vn,
					     is * p->size, os * p->size), 
		    ri, ii, ro, io);
     else if (p->split)
	  prblm = 
	       FFTW(mkproblem_rdft_d)(
		    FFTW(mktensor_rowmajor)(p->rank, p->n, p->n, is, os),
		    FFTW(mktensor_rowmajor) (p->vrank, p->vn, p->vn,
					     is * p->size, os * p->size), 
		    ri, ro, p->sign);
     else if (p->rank == 1) {
	  iodim sz = { 0, 1, 2 };
	  sz.n = p->n[0];
	  prblm = 
	       FFTW(mkproblem_rdft2_d)(
		    sz,
		    FFTW(mktensor_rowmajor) (p->vrank, p->vn, p->vn,
					     is * p->size, os * p->size), 
		    ri, ro, io, p->sign);
     }
     else
	  BENCH_ASSERT(0);
     timer_start();
     pln = plnr->adt->mkplan(plnr, prblm);
     tplan = timer_stop();
     BENCH_ASSERT(pln);
     X(plan_bless)(pln);

     if (verbose) {
	  printer *pr = FFTW(mkprinter_file) (stdout);
	  pr->print(pr, "%p\nnprob %u  nplan %u\n",
		    pln, plnr->nprob, plnr->nplan);
	  pr->print(pr, "%d add, %d mul, %d fma, %d other\n",
		    pln->ops.add, pln->ops.mul, pln->ops.fma, pln->ops.other);
	  pr->print(pr, "planner time: %g s\n", tplan);
	  if (verbose > 1) {
	       increment_closure k;
	       k.super.visit = increment;
	       k.count = 0;
	       FFTW(traverse_plan)(pln, 0, &k.super);
	       printf("number of child plans: %d\n", k.count - 1);
	  }
	  if (verbose > 3) {
	       plnr->adt->exprt(plnr, pr);
	       pr->print(pr, "\n");
	  }
	  if (verbose > 4) 
	       plnr->adt->exprt_conf(plnr, pr);
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

#if 0
     {
	  FILE *f;
	  if ((f = fopen("wis.dat", "w"))) {
	       printer *pr = X(mkprinter_file)(f);
	       plnr->adt->exprt(plnr, pr);
	       X(printer_destroy)(pr);
	       fclose(f);
	  }
     }
#endif

     FFTW(planner_destroy) (plnr);

#    ifdef FFTW_DEBUG
     if (verbose >= 1)
	  FFTW(malloc_print_minfo)();
#    endif
}
