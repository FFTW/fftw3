/* not worth copyrighting */
/* $Id: ccopy-to.c,v 1.1 2002-06-03 15:44:18 athena Exp $ */
#include "bench.h"

/* default routine, can be overridden by user */
void problem_ccopy_to(struct problem *p, bench_complex *out)
{
     if (p->kind == PROBLEM_COMPLEX)
	  copy_c2c_to(p, out);
     else {
	  if (p->sign == -1) {
	       /* forward real->hermitian transform */
	       copy_h2c(p, out);
	  } else {
	       /* backward hermitian->real transform */
	       copy_r2c(p, out);
	  }
     }
     after_problem_ccopy_to(p, out);
}
