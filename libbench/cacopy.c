/* not worth copyrighting */

#include "bench.h"

/* copy A onto B */
void cacopy(bench_complex *A, bench_complex *B, unsigned int n)
{
     unsigned int i;
     for (i = 0; i < n; ++i)
	  B[i] = A[i];
}
