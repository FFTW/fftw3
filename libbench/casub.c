/* not worth copyrighting */

#include "bench.h"

/* C = A - B */
void casub(bench_complex *C, bench_complex *A, bench_complex *B, unsigned int n)
{
     unsigned int i;

     for (i = 0; i < n; ++i) {
	  c_re(C[i]) = c_re(A[i]) - c_re(B[i]);
	  c_im(C[i]) = c_im(A[i]) - c_im(B[i]);
     }
}

