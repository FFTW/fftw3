#include "bench-user.h"
#include <math.h>
#include <stdio.h>


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

static void putchr(printer *p, char c)
{
     UNUSED(p);
     putchar(c);
}

static void debug_hook(const plan *pln, const fftw_problem *p_)
{
     problem_dft *p = (problem_dft *) p_;
     printer *pr = FFTW(mkprinter) (sizeof(printer), putchr);
     pr->print(pr, "%P:%(%p%)\n", p, pln);
     FFTW(printer_destroy) (pr);
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
     bench_real *ri, *ii, *ro, *io;
     BENCH_ASSERT(can_do(p));

     plnr = FFTW(mkplanner_score)(0);
     
     FFTW(dft_conf_standard) (plnr);

     if (verbose > 5)
	  FFTW(planner_set_hook) (plnr, debug_hook);

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

     prblm = 
	  FFTW(mkproblem_dft_d)(
	       FFTW(mktensor_rowmajor)(p->rank, p->n, p->n, 2, 2),
	       FFTW(mktensor_rowmajor) (p->vrank, p->vn,
					p->vn, 2 * p->size,
					2 * p->size), 
	       ri, ii, ro, io);
     pln = plnr->adt->mkplan(plnr, prblm);
     BENCH_ASSERT(pln);

     if (verbose) {
	  printer *pr = FFTW(mkprinter) (sizeof(printer), putchr);
	  pr->print(pr, "%p\nnprob %u  nplan %u\n",
		    pln, plnr->nprob, plnr->nplan);
	  if (verbose > 3) 
	       plnr->adt->export(plnr, pr);
	  FFTW(printer_destroy)(pr);
     }
     pln->adt->awake(pln, 1);
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

     FFTW(plan_destroy) (pln);
     FFTW(problem_destroy) (prblm);
     FFTW(planner_destroy) (plnr);

#    ifdef FFTW_DEBUG
     if (verbose >= 1)
	  FFTW(malloc_print_minfo)();
#    endif
}
