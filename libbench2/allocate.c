/* not worth copyrighting */

/* $Id: allocate.c,v 1.3 2003-01-18 21:13:15 athena Exp $ */

#include "config.h"
#include "bench.h"

static void bounds(bench_problem *p, int *ilb, int *iub, int *olb, int *oub)
{
     bench_tensor *t = tensor_append(p->sz, p->vecsz);
     tensor_ibounds(t, ilb, iub);
     tensor_obounds(t, olb, oub);
     tensor_destroy(t);
}

/*
 * Allocate I/O arrays for a problem.
 *
 * This is the default routine that can be overridden by the user in
 * complicated cases.
 */
void problem_alloc(bench_problem *p)
{
     if (p->kind == PROBLEM_COMPLEX) {
	  int ilb, iub, olb, oub;
	  int isz, osz;
	  bench_complex *in, *out;

	  bounds(p, &ilb, &iub, &olb, &oub);

	  p->iphyssz = isz = iub - ilb;
	  p->inphys = in = bench_malloc(isz * sizeof(bench_complex));
	  p->in = in - ilb;
	  
	  if (p->in_place) {
	       p->out = p->in;
	       p->outphys = p->inphys;
	       p->ophyssz = p->iphyssz;
	  } else {
	       p->ophyssz = osz = oub - olb;
	       p->outphys = out = bench_malloc(osz * sizeof(bench_complex));
	       p->out = out - olb;
	  }
     } else {
	  BENCH_ASSERT(0); /* TODO */
     }
}

void problem_free(bench_problem *p)
{
     if (p->outphys && p->outphys != p->inphys)
	  bench_free(p->outphys);
     if (p->inphys)
	  bench_free(p->inphys);
     tensor_destroy(p->sz);
     tensor_destroy(p->vecsz);
}
