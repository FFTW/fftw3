/* not worth copyrighting */

#include "bench.h"

void aset(bench_real *A, int n, bench_real x)
{
     int i;
     for (i = 0; i < n; ++i)
	  A[i] = x;
}
