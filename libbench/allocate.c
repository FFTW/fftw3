/* not worth copyrighting */

/* $Id: allocate.c,v 1.1 2002-06-03 15:44:18 athena Exp $ */

#include "config.h"
#include "bench.h"

/*
 * Allocate I/O arrays for a problem.
 *
 * This is the default routine that can be overridden by the user in
 * complicated cases.
 */
void problem_alloc(struct problem *p)
{
     if (p->kind == PROBLEM_COMPLEX) {
	  p->in = bench_malloc(p->size * sizeof(bench_complex));
	  
	  if (p->in_place)
	       p->out = p->in;
	  else
	       p->out = bench_malloc(p->size * sizeof(bench_complex));
	  p->phys_size = p->size;
     } else {
	  size_t s = 1;
	  unsigned int i;

	  for (i = 0; i < p->rank; ++i)
	       /* slightly overallocate to account for unpacked formats */
	       s *= p->n[i] + 2;

	  p->in = bench_malloc(s * sizeof(bench_real));
	  
	  if (p->in_place)
	       p->out = p->in;
	  else
	       p->out = bench_malloc(s * sizeof(bench_real));
	  p->phys_size = s;
     }
}
