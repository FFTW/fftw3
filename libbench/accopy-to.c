/* not worth copyrighting */
#include "bench.h"

/* default routine, can be overridden by user */
void after_problem_ccopy_to(struct problem *p, bench_complex *out)
{
     UNUSED(p);
     UNUSED(out);
}
