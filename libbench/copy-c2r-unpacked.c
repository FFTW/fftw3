#include "bench.h"

void copy_c2r_unpacked(struct problem *p, bench_complex *in)
{
     unsigned int k, n, k0, n0, ld;
     bench_real *pin = p->in;

     BENCH_ASSERT(p->rank >= 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     n = p->n[p->rank - 1];
     ld = 2 * (1 + (n / 2));
     n0 = 1;
     for (k0 = 0; k0 < p->rank - 1; ++k0)
	  n0 *= p->n[k0];

     for (k0 = 0; k0 < n0; ++k0) {
	  for (k = 0; k < n; ++k) {
	       pin[k] = c_re(in[k]);
	  }
	  in += n;
	  pin += ld;
     }
}
