#include "bench.h"

/* undo the normalization */
void unnormalize(struct problem *p, bench_complex *out, int which_sign)
{
     if (p->sign == which_sign) {
	  bench_complex x;
	  c_re(x) = p->size;
	  c_im(x) = 0;

	  cascale(out, p->size, x);
     }
}
