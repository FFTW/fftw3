/* fftw hook to be used in the benchmark program.  
   
   We keep it in a separate file because 

   1) bench.c is supposed to test the API---we do not want to #include
      "ifftw.h" and accidentally use internal symbols/macros.
   2) this code is a royal mess.  The messiness is due to
      A) confusion between internal fftw tensors and bench_tensor's
         (which we want to keep separate because the benchmark
	  program tests other routines too)
      B) despite A), our desire to recycle the libbench verifier.
*/

#include <stdio.h>
#include "bench-user.h"
#include "api.h"
#include "dft.h"

extern int paranoid; /* in bench.c */
extern X(plan) the_plan; /* in bench.c */

/*
  transform an fftw tensor into a bench_tensor.
*/
static bench_tensor *fftw_tensor_to_bench_tensor(tensor *t)
{
     bench_tensor *bt = mktensor(t->rnk);

     if (FINITE_RNK(t->rnk)) {
	  int i;
	  for (i = 0; i < t->rnk; ++i) {
	       bt->dims[i].n = t->dims[i].n;
	       bt->dims[i].is = t->dims[i].is;
	       bt->dims[i].os = t->dims[i].os;
	  }
     }
     return bt;
}

/*
  transform an fftw problem into a bench_problem.
*/
static bench_problem *fftw_problem_to_bench_problem(const problem *p_)
{
     bench_problem *bp = 0;
     if (DFTP(p_)) {
	  const problem_dft *p = (const problem_dft *) p_;
	  
	  if (!p->ri || !p->ii)
	       abort();

	  bp = bench_malloc(sizeof(bench_problem));

	  bp->kind = PROBLEM_COMPLEX;
	  bp->sign = FFT_SIGN;
	  bp->split = 1; /* tensor strides are in R's, not C's */
	  bp->in = p->ri;
	  bp->out = p->ro;
	  bp->ini = p->ii;
	  bp->outi = p->io;
	  bp->inphys = bp->outphys = 0;
	  bp->iphyssz = bp->ophyssz = 0;
	  bp->in_place = p->ri == p->ro;
	  bp->userinfo = 0;
	  bp->sz = fftw_tensor_to_bench_tensor(p->sz);
	  bp->vecsz = fftw_tensor_to_bench_tensor(p->vecsz);
     }
     else {
	  /* TODO */
     }
     return bp;
}

static int use_hook = 0;

static void hook(plan *pln, const problem *p_, int optimalp)
{
     int rounds = 5;
     double tol = SINGLE_PRECISION ? 1.0e-3 : 1.0e-10;
     UNUSED(optimalp);

     if (!use_hook)
	  return;

     if (verbose > 5) {
	  printer *pr = X(mkprinter_file)(stdout);
	  pr->print(pr, "%P:%(%p%)\n", p_, pln);
	  X(printer_destroy)(pr);
	  printf("cost %g  \n\n", pln->pcost);
     }

     if (paranoid) {
	  bench_problem *bp;

	  bp = fftw_problem_to_bench_problem(p_);
	  if (bp) {
	       X(plan) the_plan_save = the_plan;

	       the_plan = MALLOC(sizeof(apiplan), PLANS);
	       the_plan->pln = pln;
	       the_plan->prb = (problem *) p_;

	       AWAKE(pln, 1);
	       verify_problem(bp, rounds, tol);
	       AWAKE(pln, 0);

	       X(ifree)(the_plan);
	       the_plan = the_plan_save;

	       problem_free(bp);
	  }

     }
}

void install_hook(void)
{
     planner *plnr = X(the_planner)();
     plnr->hook = hook;
     use_hook = 1;
}

void uninstall_hook(void)
{
     planner *plnr = X(the_planner)();
     use_hook = 0;
}
