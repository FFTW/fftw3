#include "bench.h"

/* 
 * copy a 1D complex array into a 1D hermitian array stored in
 * ``halfcomplex'' format: r[0] r[1] r[2] ... r[n/2] {i[n/2]} i[n/2-1] ... i[1]
 */

void copy_c2h_1d_halfcomplex(struct problem *p, bench_complex *in,
			     bench_real sign_of_r2h_transform)
{
     unsigned int k, n;
     bench_real *pin = p->in;

     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     n = p->n[0];

     if (n > 0)
	  pin[0] = c_re(in[0]);
     
     for (k = 1; k < (n + 1) / 2; ++k) {
	  pin[k] = c_re(in[k]);
	  pin[n - k] = -sign_of_r2h_transform * c_im(in[k]);
     }

     if (2 * k == n)
	  pin[k] = c_re(in[k]);
}
