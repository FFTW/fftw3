/* not worth copyrighting */

/* $Id: deallocate.c,v 1.1 2002-06-03 15:44:18 athena Exp $ */

#include "config.h"
#include "bench.h"

void problem_free(struct problem *p)
{
     if (p->out && p->out != p->in)
	  bench_free(p->out);
     if (p->in)
	  bench_free(p->in);
}
