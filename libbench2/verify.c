/*
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* $Id: verify.c,v 1.12 2003-02-09 07:36:25 stevenj Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "verify.h"

void verify_problem(bench_problem *p, int rounds, double tol)
{
     errors e;

     switch (p->kind) {
	 case PROBLEM_COMPLEX: verify_dft(p, rounds, tol, &e); break;
	 case PROBLEM_REAL: verify_rdft2(p, rounds, tol, &e); break;
	 case PROBLEM_R2R: verify_r2r(p, rounds, tol, &e); break;
     }

     if (verbose)
	  ovtpvt("%g %g %g\n", e.l, e.i, e.s);
}

void verify(const char *param, int rounds, double tol)
{
     bench_problem *p;

     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);

     verify_problem(p, rounds, tol);

     done(p);
     problem_destroy(p);
}

void accuracy(const char *param, int rounds)
{
     bench_problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);
     BENCH_ASSERT(0); /* TODO */
     done(p);
     problem_destroy(p);
}
