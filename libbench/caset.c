/* not worth copyrighting */

#include "bench.h"

void caset(bench_complex *A, unsigned int n, bench_complex x)
{
     unsigned int i;
     for (i = 0; i < n; ++i)
	  A[i] = x;
}
