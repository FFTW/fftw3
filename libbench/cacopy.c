/* not worth copyrighting */

#include "bench.h"

/* copy A onto B */
void cacopy(bench_complex *A, bench_complex *B, int n)
{
     int i;
     for (i = 0; i < n; ++i)
	  B[i] = A[i];
}
