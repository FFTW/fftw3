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

/* $Id: verify.c,v 1.2 2003-01-18 12:20:18 athena Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "bench.h"

void verify(const char *param, int rounds, double tol)
{
     struct problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);

     switch (p->kind) {
	 case PROBLEM_COMPLEX: 
	      verify_dft(p, rounds, tol);
	      break;
	 case PROBLEM_REAL:	  /* TODO */
	 default: BENCH_ASSERT(0); return 0;
     }
     done(p);
     problem_destroy(p);
}

void accuracy(const char *param, int rounds)
{
     struct problem *p;
     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);
     BENCH_ASSERT(0); /* TODO */
     done(p);
     problem_destroy(p);
}
