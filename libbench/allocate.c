/* not worth copyrighting */

/* $Id: allocate.c,v 1.4 2006-01-05 03:04:27 stevenj Exp $ */

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
	  size_t s = p->size * p->vsize;

	  p->phys_size = s;
	  p->in = bench_malloc(s * sizeof(bench_complex));
	  
	  if (p->in_place)
	       p->out = p->in;
	  else
	       p->out = bench_malloc(s * sizeof(bench_complex));
     } else {
	  size_t s = p->vsize;
	  int i;

	  for (i = 0; i < p->rank; ++i)
	       /* slightly overallocate to account for unpacked formats */
	       s *= p->n[i] + 2;

	  p->phys_size = s;
	  p->in = bench_malloc(s * sizeof(bench_real));
	  if (p->in_place)
	       p->out = p->in;
	  else
	       p->out = bench_malloc(s * sizeof(bench_real));
     }
}
