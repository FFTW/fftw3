/* not worth copyrighting */

#include "bench.h"

/* A = alpha * A  (in place) */
void ascale(bench_real *A, unsigned int n, bench_real alpha)
{
     unsigned int i;

     for (i = 0; i < n; ++i) {
	  A[i] *= alpha;
     }
}
