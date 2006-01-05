/* not worth copyrighting */

/* $Id: deallocate.c,v 1.2 2006-01-05 03:04:27 stevenj Exp $ */

#include "config.h"
#include "bench.h"

void problem_free(struct problem *p)
{
     if (p->out && p->out != p->in)
	  bench_free(p->out);
     if (p->in)
	  bench_free(p->in);
}
