/* not worth copyrighting */

#include "bench.h"

/* copy A onto B */
void acopy(bench_real *A, bench_real *B, unsigned int n)
{
     unsigned int i;
     for (i = 0; i < n; ++i)
	  B[i] = A[i];
}
