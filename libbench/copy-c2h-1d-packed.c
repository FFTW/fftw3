#include "bench.h"

/* 
 * copy a 1D complex array into a 1D hermitian array stored in
 * ``packed'' format:
 *
 *    a[2*k] = R[k], 0<=k<n/2
 *    a[2*k+1] = I[k], 0<k<n/2
 *    a[1] = R[n/2]
 * 
 * Works only for even N.
 */

void copy_c2h_1d_packed_strided(unsigned int n,
				bench_real *pin, int rstride,
				bench_complex *in, int cstride,
				bench_real sign_of_r2h_transform)
{
     unsigned int k;

     BENCH_ASSERT((n & 1) == 0); /* even n */

     for (k = 0; k < n / 2; ++k) {
	  pin[2 * k * rstride] = c_re(in[k * cstride]);
	  pin[(2 * k + 1) * rstride] 
	       = -sign_of_r2h_transform * c_im(in[k * cstride]);
     }
     if (k > 0)
	  pin[1 * rstride] = c_re(in[k * cstride]);
}

void copy_c2h_1d_packed(struct problem *p, bench_complex *in,
			bench_real sign_of_r2h_transform)
{
     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     copy_c2h_1d_packed_strided(p->n[0], (bench_real *) p->in, 1, in, 1,
				sign_of_r2h_transform);
}
