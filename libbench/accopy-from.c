/* not worth copyrighting */
/* $Id: accopy-from.c,v 1.1 2002-06-03 15:44:18 athena Exp $ */
#include "bench.h"

/* default routine, can be overridden by user */
void after_problem_ccopy_from(struct problem *p, bench_complex *in)
{
     UNUSED(p);
     UNUSED(in);
}
