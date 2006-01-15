/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Massachusetts Institute of Technology
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

/* $Id: speed.c,v 1.11 2006-01-15 21:09:53 athena Exp $ */

#include "bench.h"

void speed(const char *param, int setup_only)
{
     double *t;
     int iter = 0, k;
     bench_problem *p;
     double tmin, y;

     t = (double *) bench_malloc(time_repeat * sizeof(double));

     for (k = 0; k < time_repeat; ++k) 
	  t[k] = 0;

     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);

     timer_start(LIBBENCH_TIMER);
     setup(p);
     p->setup_time = timer_stop(LIBBENCH_TIMER);
     
     if (setup_only)
	  goto done;

 start_over:
     for (iter = 1; iter < (1<<30); iter *= 2) {
	  tmin = 1.0e20;
	  for (k = 0; k < time_repeat; ++k) {
	       timer_start(LIBBENCH_TIMER);
	       doit(iter, p);
	       y = timer_stop(LIBBENCH_TIMER);
	       if (y < 0) /* yes, it happens */
		    goto start_over;
	       t[k] = y;
	       if (y < tmin)
		    tmin = y;
	  }
	  
	  if (tmin >= time_min)
	       goto done;
     }

     goto start_over; /* this also happens */

 done:
     done(p);

     if (iter) 
	  for (k = 0; k < time_repeat; ++k) 
	       t[k] /= iter;
     else
	  for (k = 0; k < time_repeat; ++k) 
	       t[k] = 0;

     report(p, t, time_repeat);

     problem_destroy(p);
     bench_free(t);
     return;
}
