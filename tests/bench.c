
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
extern const char *X(version);
extern const char *X(cc);

/* END HACKS */

static const char *mkvers(void)
{
     return X(version);
}

static const char *mkcc(void)
{
     return X(cc);
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
     printer *pr = X(mkprinter)(sizeof(printer), putchr);
     printf("%d: ", X(tensor_sz)(p->sz));
     pln->adt->print(pln, pr);
     printf("\n");
     X(printer_destroy)(pr);
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

     plnr = X(mkplanner_naive)();
#else

     plnr = X(mkplanner_score)();
#endif

     X(dft_conf_standard)(plnr);

     //     X(planner_set_hook)(plnr, debug_hook);

     if (p->sign == -1)
     {
          ri = p->in;
          ii = ri + 1;
          ro = p->out;
          io = ro + 1;
     } else
     {
          ii = p->in;
          ri = ii + 1;
          io = p->out;
          ro = io + 1;
     }

     prblm =
          X(mkproblem_dft_d)(
               X(mktensor_rowmajor)(p->rank, p->n, p->n, 2, 2),
               X(mktensor_rowmajor)(p->vrank, p->vn, p->vn,
                                    2 * p->size, 2 * p->size),
               ri, ii, ro, io);
     pln = plnr->adt->mkplan(plnr, prblm);
     BENCH_ASSERT(pln);

     if (verbose)
     {
          printer *pr = X(mkprinter)(sizeof(printer), putchr);
          pln->adt->print(pln, pr);
          X(printer_destroy)(pr);
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
     for (i = 0; i < iter; ++i)
     {
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

     X(plan_destroy)(pln);
     X(problem_destroy)(prblm);
     X(planner_destroy)(plnr);

#    ifdef FFTW_DEBUG

     if (verbose >= 1)
          fftw_malloc_print_minfo();
#    endif
}
