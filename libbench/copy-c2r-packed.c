#include "bench.h"

void copy_c2r_packed(struct problem *p, bench_complex *in)
{
     unsigned int i;
     unsigned int n = p->size;
     bench_real *pin = p->in;

     for (i = 0; i < n; ++i)
	  pin[i] = c_re(in[i]);
}
