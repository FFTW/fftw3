/* not worth copyrighting */

#include "bench.h"
#include <math.h>

double mflops(const struct problem *p, double t)
{
     if (p->size == 0)
	  return 0.0; /* fails because of log(0) */

     switch (p->kind) {
	 case PROBLEM_COMPLEX:
	      return (5.0 * p->vsize * p->size * log((double) p->size) / 
		      (log(2.0) * t * 1.0e6));
	 case PROBLEM_REAL:
	      return (2.5 * p->vsize * p->size * log((double) p->size) / 
		      (log(2.0) * t * 1.0e6));
     }
     BENCH_ASSERT(0 /* can't happen */);
     return 0.0;
}

