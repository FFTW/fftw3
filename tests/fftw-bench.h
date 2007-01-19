/* declarations of common subroutines, etc. for use with FFTW
   self-test/benchmark program (see bench.c). */

#include "bench-user.h"
#include "fftw3.h"

#define CONCAT(prefix, name) prefix ## name
#if defined(BENCHFFT_SINGLE)
#define FFTW(x) CONCAT(fftwf_, x)
#elif defined(BENCHFFT_LDOUBLE)
#define FFTW(x) CONCAT(fftwl_, x)
#else
#define FFTW(x) CONCAT(fftw_, x)
#endif

extern FFTW(plan) mkplan(bench_problem *p, unsigned flags);
extern void initial_cleanup(void);
extern void final_cleanup(void);
extern int import_wisdom(FILE *f);
extern void export_wisdom(FILE *f);
