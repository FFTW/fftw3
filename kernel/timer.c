/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

/* $Id: timer.c,v 1.3 2002-06-04 21:49:39 athena Exp $ */

#include "ifftw.h"
#include <stdio.h>

#ifndef FFTW_TIME_LIMIT
#define FFTW_TIME_LIMIT 2.0  /* don't run for more than two seconds */
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_BSDGETTIMEOFDAY
#ifndef HAVE_GETTIMEOFDAY
#define gettimeofday BSDgettimeofday
#define HAVE_GETTIMEOFDAY 1
#endif
#endif

/* tick-based timers */
#include "cycle.h"

#ifdef HAVE_TICK_COUNTER /* nominal values good for all tick counters */
#  ifndef TIME_MIN_TICK
#    define TIME_MIN_TICK 100.0
#  endif

#  define TIME_REPEAT_TICK 8
#endif


/* 
   Second-based times. These timers may have lower resolution but it
   is possible to convert them to seconds.
 */

static double time_min_seconds = 0.0; /* to be set in calibrate() */
static int time_repeat_seconds = 0; /* to be set in calibrate() */

#if defined(HAVE_GETTIMEOFDAY) && !defined(HAVE_SECONDS_TIMER)
   typedef struct timeval seconds;

   static seconds getseconds(void)
   {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv;
   }

   static double elapsed_sec(seconds t1, seconds t0)
   {
	return (double)(t1.tv_sec - t0.tv_sec) +
	     (double)(t1.tv_usec - t0.tv_usec) * 1.0E-6;
   }

#  define HAVE_SECONDS_TIMER
#endif

/* If second-based timers are available but tick-based timers are not,
   use second-based */
#if defined(HAVE_SECONDS_TIMER) && !defined(HAVE_TICK_COUNTER)
#  define ticks seconds
#  define getseticks getseconds
#  define elapsed elapsed_sec
#  define TIME_MIN_TICK time_min_seconds
#  define TIME_REPEAT_TICK time_repeat_seconds
#  define HAVE_TICK_COUNTER
#endif

#if !defined(HAVE_TICK_COUNTER) || !defined(HAVE_SECONDS_TIMER)
#error "Don't know how to time on this system"
#endif


#define MKMEAS(nam, tim, gettim, elaps)				\
static double nam(plan *pln, const problem *p, int iter)	\
{								\
     tim t0, t1;						\
     int i;							\
								\
     t0 = gettim();						\
     for (i = 0; i < iter; ++i) {				\
	  pln->adt->solve(pln, p);				\
     }								\
     t1 = gettim();						\
     return elaps(t1, t0);					\
}

MKMEAS(measure_tick, ticks, getticks, elapsed)
MKMEAS(measure_sec, seconds, getseconds, elapsed_sec)

/*
 * Routines to calibrate the slow timer.  Derived from Larry McVoy's
 * lmbench, distributed under the GNU General Public License.
 */

typedef char *TYPE;
static const double tmin_try = 1.0e-6; /* seconds */
static const double tmax_try = 1.0;    /* seconds */
static const double tolerance = 0.0025;

static TYPE **work(int n, TYPE **p)
{
#define	ENOUGH_DURATION_TEN(one)	one one one one one one one one one one
     while (n-- > 0) {
	  ENOUGH_DURATION_TEN(p = (TYPE **) *p;);
     }
     return (p);
}

/* do N units of work */
static double duration(int n)
{
     TYPE   *x = (TYPE *)&x;
     TYPE  **p = (TYPE **)&x;
     seconds t0, t1;

     t0 = getseconds();
     p = work(n, p);
     t1 = getseconds();
     return (elapsed_sec(t1, t0));
}

static double time_n(int n)
{
     int     i;
     double  tmin;

     tmin = duration(n);
     for (i = 1; i < time_repeat_seconds; ++i) {
	  double t = duration(n);
	  if (t < tmin)
	       tmin = t;
     }
     return tmin;
}

/* return the amount of work needed to run TMIN seconds */
static int find_n(double tmin)
{
     int tries;
     int n = 10000;
     double t;
	
     t = time_n(n);

     for (tries = 0; tries < 10; ++tries) {
	  if (0.98 * tmin < t && t < 1.02 * tmin)
	       return n;
	  if (t < tmin_try)
	       n *= 10;
	  else {
	       double k = n;

	       k /= t;
	       k *= tmin;
	       n = ((int)k) + 1;
	  }
	  t = time_n(n);
     }
     return (-1);
}

/* Verify that small modifications affect the runtime proportionally */
static int acceptable(double tmin)
{
     int n;
     unsigned int i;
     static const double test_points[] = { 1.015, 1.02, 1.035 };
     double baseline;
     
     n = find_n(tmin);
     if (n <= 0)
	  return 0;

     baseline = time_n(n);

     for (i = 0; i < sizeof(test_points) / sizeof(double); ++i) {
	  double usecs = time_n((int)((double) n * test_points[i]));
	  double expected = baseline * test_points[i];
	  double diff = expected > usecs ? expected - usecs : usecs - expected;
	  if (diff / expected > tolerance)
	       return 0;
     }
     return 1;
}

static void calibrate(void)
{
     double tmin;

     time_repeat_seconds = 8;
     for (tmin = tmin_try; tmin < tmax_try && !acceptable(tmin); tmin *= 2.0)
	  ;
     time_min_seconds = tmin;
}

/*
 * This routine measures the execution time of a plan.
 * If SECONDSP, the returned runtime is in seconds.  This
 * choice may require the use of slow timers.  Otherwise,
 * the time is returned in some arbitrary unit if this
 * allows the use of fast timers.
 */
double fftw_measure_execution_time(plan *pln, const problem *p, int secondsp)
{
     seconds begin, now;
     double t, tmax, tmin;
     int iter;
     int repeat;
     double (*meas)(plan *pln, const problem *p, int iter);
     int time_repeat;
     double time_min;

     if (secondsp) {
	  if (!time_repeat_seconds)
	       calibrate();

	  meas = measure_sec;
	  time_repeat = time_repeat_seconds;
	  time_min = time_min_seconds;
     } else {
	  meas = measure_tick;
	  time_repeat = TIME_REPEAT_TICK;
	  time_min = TIME_MIN_TICK;
     }

     pln->adt->awake(pln, AWAKE);
     p->adt->zero(p);

 start_over:
     for (iter = 1; iter; iter *= 2) {
	  tmin = 1.0E10;
	  tmax = -1.0E10;

	  begin = getseconds();
	  /* repeat the measurement TIME_REPEAT times */
	  for (repeat = 0; repeat < time_repeat; ++repeat) {
	       t = meas(pln, p, iter);

	       if (t < 0)
		    goto start_over;

	       if (t < tmin) tmin = t;
	       if (t > tmax) tmax = t;

	       /* do not run for too long */
	       now = getseconds();
	       t = elapsed_sec(now, begin);

	       if (t > FFTW_TIME_LIMIT)
		    break;
	  }

	  if (tmin >= time_min) {
	       tmin /= (double) iter;
	       tmax /= (double) iter;
	       pln->adt->awake(pln, SLEEP);
	       return tmin;
	  }
     }
     goto start_over; /* may happen if timer is screwed up */
}
