/* not worth copyrighting */
#include "bench.h"

/*
 * convert complex array into separate real and imag arrays 
 */
void copy_c2ri(bench_complex *in, bench_real *rout, bench_real *iout,
	       int n)
{
     int i;
     for (i = 0; i < n; ++i) {
	  rout[i] = c_re(in[i]);
	  iout[i] = c_im(in[i]);
     }
}
