/* not worth copyrighting */

#include "bench.h"

/* A = alpha * A  (in place) */
void cascale(bench_complex *A, unsigned int n, bench_complex alpha)
{
     unsigned int i;

     for (i = 0; i < n; ++i) {
	  bench_complex a = A[i];
	  c_re(A[i]) = c_re(a) * c_re(alpha) - c_im(a) * c_im(alpha);
	  c_im(A[i]) = c_re(a) * c_im(alpha) + c_im(a) * c_re(alpha);
     }
}
