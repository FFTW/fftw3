#include "bench.h"

/* 
 * copy a 1D packed hermitian array into a 1D complex array
 * Works only for even N.
 */

void copy_h2c_1d_packed_strided(unsigned int n,
				bench_real *pout, int rstride,
				bench_complex *out, int cstride,
				bench_real sign_of_r2h_transform)
{
     unsigned int k;

     BENCH_ASSERT((n & 1) == 0); /* even n */

     if (n > 0) {
	  c_re(out[0]) = pout[0];
	  c_re(out[(n/2)*cstride]) = pout[1*rstride];
	  c_im(out[0]) = 0.0;
	  c_im(out[(n/2)*cstride]) = 0.0;
     }

     for (k = 1; k < n / 2; ++k) {
	  bench_real re, im;
	  re = pout[2 * k * rstride];
	  im = pout[(2 * k + 1) * rstride];
	  c_re(out[k*cstride]) = re;
	  c_im(out[k*cstride]) = - sign_of_r2h_transform * im;
	  c_re(out[(n - k)*cstride]) = re;
	  c_im(out[(n - k)*cstride]) = sign_of_r2h_transform * im;
     }
}

void copy_h2c_1d_packed(struct problem *p, bench_complex *out, 
			bench_real sign_of_r2h_transform)
{
     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     copy_h2c_1d_packed_strided(p->n[0], (bench_real *) p->out, 1, out, 1,
				sign_of_r2h_transform);
}

