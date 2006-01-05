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

/* $Id: speed.c,v 1.3 2006-01-05 03:04:27 stevenj Exp $ */

#include "config.h"
#include "bench.h"

void speed(const char *param)
{
     double *t;
     int iter, k;
     struct problem *p;
     double tmin, y;

     t = bench_malloc(time_repeat * sizeof(double));

     p = problem_parse(param);
     BENCH_ASSERT(can_do(p));
     problem_alloc(p);
     problem_zero(p);
     setup(p);

 start_over:
     for (iter = 1; iter < (1<<30); iter *= 2) {
	  tmin = 1.0e20;
	  for (k = 0; k < time_repeat; ++k) {
	       timer_start();
	       doit(iter, p);
	       y = timer_stop();
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

     for (k = 0; k < time_repeat; ++k) {
	  t[k] /= iter;
     }

     report(p, t, time_repeat);

     problem_destroy(p);
     bench_free(t);
     return;
}
