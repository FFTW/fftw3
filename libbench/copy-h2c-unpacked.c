#include "bench.h"

/* 
 * copy a ND unpacked hermitian array into a ND complex array,
 * assuming that the last dimension is the one cut into half.
 */

static void cp(bench_complex *in, bench_complex *out,
	       unsigned int rank,
	       unsigned int *n,
	       unsigned int nb,
	       unsigned int mnb,
	       bench_real sign)
{
     if (rank == 1) {
	  unsigned int k0;
	  unsigned int n0 = n[0];
	  unsigned int ld = 1 + (n[0] / 2);

	  for (k0 = 0; k0 < n0; ++k0) {
	       bench_complex a;
	       if (2 * k0 <= n0) {
		    a = in[ld * nb + k0];
		    c_im(a) = -sign * c_im(a);
	       } else {
		    a = in[ld * mnb + n0 - k0];
		    c_im(a) = sign * c_im(a);
	       }
	       out[n0 * nb + k0] = a;
	  }
     } else {
	  unsigned int ki;
	  unsigned int ni = n[0];
	  cp(in, out, rank - 1, n + 1, nb * ni, mnb * ni, sign);
	  for (ki = 1; ki < ni; ++ki) {
	       cp(in, out, rank - 1, n + 1, nb * ni + ki, 
		  mnb * ni + (ni - ki), sign);
	  }
     }
}

void copy_h2c_unpacked(struct problem *p, bench_complex *out, 
		       bench_real sign_of_r2h_transform)
{
     bench_complex *pout = p->out;

     BENCH_ASSERT(p->kind == PROBLEM_REAL);
     BENCH_ASSERT(p->rank >= 1);

     cp(pout, out, p->rank, p->n, 0, 0, sign_of_r2h_transform);
}
