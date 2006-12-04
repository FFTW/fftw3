/* not worth copyrighting */


#include "config.h"
#include "bench.h"

void problem_free(struct problem *p)
{
     if (p->out && p->out != p->in)
	  bench_free(p->out);
     if (p->in)
	  bench_free(p->in);
}
