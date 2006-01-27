/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* $Id: timer.c,v 1.27 2006-01-27 02:10:50 athena Exp $ */

#include "ifftw.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifndef WITH_SLOW_TIMER
#  include "cycle.h"
#endif

#ifndef FFTW_TIME_LIMIT
#define FFTW_TIME_LIMIT 2.0  /* don't run for more than two seconds */
#endif

#if defined(HAVE_GETTIMEOFDAY)
crude_time X(get_crude_time)(void)
{
     crude_time tv;
     gettimeofday(&tv, 0);
     return tv;
}

#define elapsed_sec(t1,t0) ((double)(t1.tv_sec - t0.tv_sec) +		\
			    (double)(t1.tv_usec - t0.tv_usec) * 1.0E-6)

double X(elapsed_since)(crude_time t0)
{
     crude_time t1;
     gettimeofday(&t1, 0);
     return elapsed_sec(t1, t0);
}

#  define TIME_MIN_SEC 1.0e-2 /* from fftw2 */

#else /* !HAVE_GETTIMEOFDAY */

/* Note that the only system where we are likely to need to fall back
   on the clock() function is Windows, for which CLOCKS_PER_SEC is 1000
   and thus the clock wraps once every 50 days.  This should hopefully
   be longer than the time required to create any single plan! */
crude_time X(get_crude_time)(void) { return clock(); }

#define elapsed_sec(t1,t0) ((double) ((t1) - (t0)) / CLOCKS_PER_SEC)

double X(elapsed_since)(crude_time t0)
{
     return elapsed_sec(clock(), t0);
}

#  define TIME_MIN_SEC 2.0e-1 /* from fftw2 */

#endif /* !HAVE_GETTIMEOFDAY */

#ifdef WITH_SLOW_TIMER
/* excruciatingly slow; only use this if there is no choice! */
typedef crude_time ticks;
#  define getticks X(get_crude_time)
#  define elapsed(t1,t0) elapsed_sec(t1,t0)
#  define TIME_MIN TIME_MIN_SEC
#  define TIME_REPEAT 4 /* from fftw2 */
#  define HAVE_TICK_COUNTER
#endif

#ifdef HAVE_TICK_COUNTER

#  ifndef TIME_MIN
#    define TIME_MIN 100.0
#  endif

#  ifndef TIME_REPEAT
#    define TIME_REPEAT 8
#  endif

  static double measure(plan *pln, const problem *p, int iter)
  {
       ticks t0, t1;
       int i;

       t0 = getticks();
       for (i = 0; i < iter; ++i) 
	    pln->adt->solve(pln, p);
       t1 = getticks();
       return elapsed(t1, t0);
  }


  double X(measure_execution_time)(plan *pln, const problem *p)
  {
       int iter;
       int repeat;

       X(plan_awake)(pln, AWAKE_ZERO);
       p->adt->zero(p);

  start_over:
       for (iter = 1; iter; iter *= 2) {
	    double tmin = 1.0E10, tmax = -1.0E10;
	    crude_time begin = X(get_crude_time)();

	    /* repeat the measurement TIME_REPEAT times */
	    for (repeat = 0; repeat < TIME_REPEAT; ++repeat) {
		 double t = measure(pln, p, iter);

		 if (t < 0)
		      goto start_over;

		 if (t < tmin)
		      tmin = t;
		 if (t > tmax)
		      tmax = t;

		 /* do not run for too long */
		 if (X(elapsed_since)(begin) > FFTW_TIME_LIMIT)
		      break;
	    }

	    if (tmin >= TIME_MIN) {
		 tmin /= (double) iter;
		 tmax /= (double) iter;
		 X(plan_awake)(pln, SLEEPY);
		 return tmin;
	    }
       }
       goto start_over; /* may happen if timer is screwed up */
  }

#else /* no cycle counter */

  double X(measure_execution_time)(plan *pln, const problem *p)
  {
       UNUSED(p);
       UNUSED(pln);
       return -1.0;
  }

#endif
