/* not worth copyrighting */

#include "bench.h"

void caset(bench_complex *A, int n, bench_complex x)
{
     int i;
     for (i = 0; i < n; ++i)
	  A[i] = x;
}
