/* not worth copyrighting */

#include "bench.h"
#include <math.h>

double mflops(const bench_problem *p, double t)
{
     int size = tensor_sz(p->sz);
     int vsize = tensor_sz(p->vecsz);

     if (size == 0)
	  return 0.0; /* fails because of log(0) */

     switch (p->kind) {
	 case PROBLEM_COMPLEX:
	      return (5.0 * size * vsize * log((double)size) / 
		      (log(2.0) * t * 1.0e6));
	 case PROBLEM_REAL:
	 case PROBLEM_R2R:
	      return (2.5 * vsize * size * log((double) size) / 
		      (log(2.0) * t * 1.0e6));
     }
     BENCH_ASSERT(0 /* can't happen */);
     return 0.0;
}

