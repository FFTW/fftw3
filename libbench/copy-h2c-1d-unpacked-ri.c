#include "bench.h"

/* 
 * copy a 1D unpacked hermitian ``ri'' array into a 1D complex array
 * (A unpacked hermitian ``ri'' array consists of the first 1 + n/2
 * elements of a complex array stored as R R R R R ... I I I I I ...)
 * 
 * This monstrosity is used by the intel library.
 */

void copy_h2c_1d_unpacked_ri(struct problem *p, bench_complex *out, 
			     bench_real sign_of_r2h_transform)
{
     unsigned int k, n;
     bench_real *rpout, *ipout;
 
     BENCH_ASSERT(p->rank == 1);
     BENCH_ASSERT(p->kind == PROBLEM_REAL);

     n = p->n[0];
     rpout = p->out;
     ipout = rpout + (n/2) + 1; 

     if (n > 0) {
	  c_re(out[0]) = rpout[0];
	  c_im(out[0]) = - sign_of_r2h_transform * ipout[0];
     }

     for (k = 1; k < 1 + (n / 2); ++k) {
	  c_re(out[k]) = rpout[k];
	  c_im(out[k]) = - sign_of_r2h_transform * ipout[k];
	  c_re(out[n - k]) = rpout[k];
	  c_im(out[n - k]) = sign_of_r2h_transform * ipout[k];
     }
}
