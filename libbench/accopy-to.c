/* not worth copyrighting */
/* $Id: accopy-to.c,v 1.1 2002-06-03 15:44:18 athena Exp $ */
#include "bench.h"

/* default routine, can be overridden by user */
void after_problem_ccopy_to(struct problem *p, bench_complex *out)
{
     UNUSED(p);
     UNUSED(out);
}
