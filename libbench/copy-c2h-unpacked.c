#include "bench.h"

/* 
 * copy a ND complex array into a ND hermitian array stored in
 * ``unpacked'' format.
 */

void copy_c2h_unpacked(struct problem *p, bench_complex *in,
		       bench_real sign_of_r2h_transform)
{
     unsigned int k, n, k0, n0, ld;
     bench_complex *pin = p->in;

     BENCH_ASSERT(p->rank >= 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     n = p->n[p->rank - 1];
     ld = 1 + (n / 2);
     n0 = 1;
     for (k0 = 0; k0 < p->rank - 1; ++k0)
	  n0 *= p->n[k0];

     for (k0 = 0; k0 < n0; ++k0) {
	  for (k = 0; k < 1 + (n / 2); ++k) {
	       c_re(pin[k]) = c_re(in[k]);
	       c_im(pin[k]) = -sign_of_r2h_transform * c_im(in[k]);
	  }

	  in += n;
	  pin += ld;
     }
}
