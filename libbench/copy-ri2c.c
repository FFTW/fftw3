/* not worth copyrighting */
#include "bench.h"

/*
 * convert real/imag arrays into one complex array 
 */
void copy_ri2c(bench_real *rin, bench_real *iin, bench_complex *out,
	       unsigned int n)
{
     unsigned int i;
     for (i = 0; i < n; ++i) {
	  c_re(out[i]) = rin[i];
	  c_im(out[i]) = iin[i];
     }
}
