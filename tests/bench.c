
#include "bench-user.h"
#include <math.h>
#include <stdio.h>

#include "fftw.h"

/*
  horrible hack for now.  This will go away once we define an interface
  for fftw
 */
#define problem fftw_problem
#include "ifftw.h"
#include "dft.h"
#include "codelet.h"
#undef problem
extern const char *fftw_version;
extern const char *fftw_cc;

/* END HACKS */

static const char *mkvers(void)
{
     return fftw_version;
}

static const char *mkcc(void)
{
     return fftw_cc;
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

static void debug_hook(plan *pln, fftw_problem *p_)
{
     problem_dft *p = (problem_dft *)p_;
     printer *pr = fftw_mkprinter(sizeof(printer), putchr);
     printf("%d: ", fftw_tensor_sz(p->sz));
     pln->adt->print(pln, pr);
     printf("\n");
     fftw_printer_destroy(pr);
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

#if 0
     plnr = fftw_mkplanner_naive();
#else
     plnr = fftw_mkplanner_score();
#endif
     fftw_dft_conf_standard(plnr);

//     fftw_planner_set_hook(plnr, debug_hook);

     if (p->sign == -1) {
	  ri = p->in; ii = ri + 1; ro = p->out; io = ro + 1;
     } else {
	  ii = p->in; ri = ii + 1; io = p->out; ro = io + 1;
     }

     prblm = 
	  fftw_mkproblem_dft_d(
	       fftw_mktensor_rowmajor(p->rank, p->n, p->n, 2, 2),
	       fftw_mktensor_rowmajor(p->vrank, p->vn, p->vn, 
				      2 * p->size, 2 * p->size),
	       ri, ii, ro, io);
     pln = plnr->adt->mkplan(plnr, prblm);
     BENCH_ASSERT(pln);
     
     if (verbose) {
	  printer *pr = fftw_mkprinter(sizeof(printer), putchr);
	  pln->adt->print(pln, pr);
	  fftw_printer_destroy(pr);
	  printf("\n");
     }
     printf("%d\n", plnr->ntry);
     pln->adt->awake(pln, AWAKE);
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
	    fftw_planner_dump(plnr, verbose - 2);
#    endif

     fftw_plan_destroy(pln);
     fftw_problem_destroy(prblm);
     fftw_planner_destroy(plnr);

#    ifdef FFTW_DEBUG
       if (verbose >= 1) 
	    fftw_malloc_print_minfo();
#    endif
}
