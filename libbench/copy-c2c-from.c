/* not worth copyrighting */
#include "bench.h"

void copy_c2c_from(struct problem *p, bench_complex *in)
{
     cacopy(in, p->in, p->size);
}
