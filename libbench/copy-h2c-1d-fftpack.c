#include "bench.h"

/* 
 * copy a 1D fftpack-style hermitian array into a 1D complex array.
 */

void copy_h2c_1d_fftpack(struct problem *p, bench_complex *out, 
			 bench_real sign_of_r2h_transform)
{
     unsigned int k, n;
     bench_real *pout = p->out;

     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     n = p->n[0];

     c_re(out[0]) = *pout++;
     c_im(out[0]) = 0;
     
     for (k = 1; k < (n + 1) / 2; ++k) {
	  c_re(out[k]) = *pout++;
	  c_im(out[k]) = -sign_of_r2h_transform * *pout++;
	  c_re(out[n - k]) = c_re(out[k]);
	  c_im(out[n - k]) = -c_im(out[k]);
     }
     if (2 * k == n) {
	  c_re(out[k]) = *pout++;
	  c_im(out[k]) = 0;
     }
}
