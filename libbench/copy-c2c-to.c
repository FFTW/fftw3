/* not worth copyrighting */
#include "bench.h"

void copy_c2c_to(struct problem *p, bench_complex *out)
{
      cacopy(p->out, out, p->size);
}
