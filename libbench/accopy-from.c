/* not worth copyrighting */
/* $Id: accopy-from.c,v 1.2 2006-01-05 03:04:27 stevenj Exp $ */
#include "bench.h"

/* default routine, can be overridden by user */
void after_problem_ccopy_from(struct problem *p, bench_complex *in)
{
     UNUSED(p);
     UNUSED(in);
}
