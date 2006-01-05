/* not worth copyrighting */
/* $Id: accopy-to.c,v 1.2 2006-01-05 03:04:27 stevenj Exp $ */
#include "bench.h"

/* default routine, can be overridden by user */
void after_problem_ccopy_to(struct problem *p, bench_complex *out)
{
     UNUSED(p);
     UNUSED(out);
}
