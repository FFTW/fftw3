#include "bench.h"

void copy_r2c_packed(struct problem *p, bench_complex *out)
{
     unsigned int i;
     unsigned int n = p->size;
     bench_real *pout = p->out;

     for (i = 0; i < n; ++i) {
	  c_re(out[i]) = pout[i];
	  c_im(out[i]) = 0.0;
     }
}
