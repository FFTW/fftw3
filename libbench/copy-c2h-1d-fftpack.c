#include "bench.h"

/* 
 * copy a 1D complex array into a 1D hermitian array stored in
 * ``fftpack'' format: r[0] r[1] i[1] r[2] i[2] ... r[n/2]
 */

void copy_c2h_1d_fftpack(struct problem *p, bench_complex *in,
			 bench_real sign_of_r2h_transform)
{
     unsigned int k, n;
     bench_real *pin = p->in;

     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     n = p->n[0];

     *pin++ = c_re(*in);
     ++in;
     
     for (k = 1; k < (n + 1) / 2; ++k) {
	  *pin++ = c_re(*in);
	  *pin++ = -sign_of_r2h_transform * c_im(*in);
	  ++in;
     }
     if (2 * k == n)
	  *pin = c_re(*in);
}
