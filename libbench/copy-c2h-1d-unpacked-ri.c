#include "bench.h"

/* 
 * copy a 1D complex array into a 1D hermitian array stored in
 * ``unpacked'' format.
 */

void copy_c2h_1d_unpacked_ri(struct problem *p, bench_complex *in,
			     bench_real sign_of_r2h_transform)
{
     unsigned int k, n;
     bench_real *rpin, *ipin;

     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     n = p->n[0];
     rpin = p->in;
     ipin = rpin + (n/2) + 1;

     for (k = 0; k < 1 + (n / 2); ++k) {
	  rpin[k] = c_re(in[k]);
	  ipin[k] = -sign_of_r2h_transform * c_im(in[k]);
     }
}
