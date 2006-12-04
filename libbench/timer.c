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


#include "config.h"
#include "bench.h"
#include <stdio.h>

/* 
 * System-dependent timing functions:
 */
#ifdef HAVE_SYS_TIME_H
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

double time_min;
int time_repeat;

#if defined(HAVE_GETTIMEOFDAY) && !defined(HAVE_TIMER)
typedef struct timeval mytime;

static mytime get_time(void)
{
     struct timeval tv;
     gettimeofday(&tv, 0);
     return tv;
}

static double elapsed(mytime t1, mytime t0)
{
     return (double)(t1.tv_sec - t0.tv_sec) +
	  (double)(t1.tv_usec - t0.tv_usec) * 1.0E-6;
}

#define HAVE_TIMER
#endif

#if !defined(HAVE_TIMER) && (defined(__WIN32__) || defined(WIN32) || defined(_WINDOWS))
#include <windows.h>
typedef LARGE_INTEGER mytime;

static mytime get_time(void)
{
     mytime tv;
     QueryPerformanceCounter(&tv);
     return tv;
}

static double elapsed(mytime t1, mytime t0)
{
     LARGE_INTEGER freq;
     QueryPerformanceFrequency(&freq);
     return ((double) (t1.QuadPart - t0.QuadPart)) /
	  ((double) freq.QuadPart);
}

#define HAVE_TIMER
#endif

#ifndef HAVE_TIMER
#error "timer not defined"
#endif

/*
 * Routines to calibrate the slow timer.  Derived from Larry McVoy's
 * lmbench, distributed under the GNU General Public License.
 *
 *

From: "Staelin, Carl" <staelin@exch.hpl.hp.com>
To: Larry McVoy <lm@bitmover.com>, athena@fftw.org, stevenj@alum.mit.edu
Date: Sat, 7 Jul 2001 23:50:49 -0700 

Matteo,

You have my permission to use the enough_duration, 
duration, time_N, find_N, test_time, and 
compute_enough from lib_timing.c routines
under the LGPL license.  You may also use the
BENCH* macros from bench.h under the LGPL
if you find them useful.

*/


typedef char *TYPE;
static const double tmin_try = 1.0e-6; /* seconds */
static const double tmax_try = 1.0;    /* seconds */
static const double tolerance = 0.001;

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
     mytime t0, t1;

     t0 = get_time();
     p = work(n, p);
     t1 = get_time();
     return (elapsed(t1, t0));
}

static double time_n(int n)
{
     int     i;
     double  tmin;

     tmin = duration(n);
     for (i = 1; i < time_repeat; ++i) {
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
	       n = k + 1;
	  }
	  t = time_n(n);
     }
     return (-1);
}

/* Verify that small modifications affect the runtime proportionally */
static int acceptable(double tmin)
{
     int n;
     unsigned i;
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

static double calibrate(void)
{
     double tmin;

     for (tmin = tmin_try; tmin < tmax_try && !acceptable(tmin); tmin *= 2.0)
	  ;
     return tmin;
}


void timer_init(double tmin, int repeat)
{
     if (!repeat)
	  repeat = 8;
     time_repeat = repeat;

     if (tmin > 0)
	  time_min = tmin;
     else
	  time_min = calibrate();
}

static mytime t0;

void timer_start(void)
{
     t0 = get_time();
}

double timer_stop(void)
{
     mytime t1;
     t1 = get_time();
     return elapsed(t1, t0);
}

