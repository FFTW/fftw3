/* not worth copyrighting */

#include "bench.h"

void aset(bench_real *A, unsigned int n, bench_real x)
{
     unsigned int i;
     for (i = 0; i < n; ++i)
	  A[i] = x;
}
