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

/* $Id: timer.c,v 1.11 2002-09-02 21:33:49 stevenj Exp $ */

#include "ifftw.h"

#ifndef FFTW_TIME_LIMIT
#define FFTW_TIME_LIMIT 2.0  /* don't run for more than two seconds */
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
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

#include "cycle.h"

#ifndef TIME_MIN
#  define TIME_MIN 100.0
#endif

#ifndef TIME_REPEAT
#  define TIME_REPEAT 8
#endif

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

#ifndef HAVE_SECONDS_TIMER
#  include <time.h>

typedef clock_t seconds;

static seconds getseconds(void) { return clock(); }

static double elapsed_sec(seconds t1, seconds t0)
{
     return ((double) (t1 - t0)) / CLOCKS_PER_SEC;
}

#  define HAVE_SECONDS_TIMER
#endif

#if !defined(HAVE_TICK_COUNTER) || !defined(HAVE_SECONDS_TIMER)
#error "Don't know how to time on this system"
#endif

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
     seconds begin, now;
     double t, tmax, tmin;
     int iter;
     int repeat;

     AWAKE(pln, 1);
     p->adt->zero(p);

 start_over:
     for (iter = 1; iter; iter *= 2) {
          tmin = 1.0E10;
          tmax = -1.0E10;

          begin = getseconds();
          /* repeat the measurement TIME_REPEAT times */
          for (repeat = 0; repeat < TIME_REPEAT; ++repeat) {
               t = measure(pln, p, iter);

               if (t < 0)
                    goto start_over;

               if (t < tmin)
                    tmin = t;
               if (t > tmax)
                    tmax = t;

               /* do not run for too long */
               now = getseconds();
               t = elapsed_sec(now, begin);

               if (t > FFTW_TIME_LIMIT)
                    break;
          }

          if (tmin >= TIME_MIN) {
               tmin /= (double) iter;
               tmax /= (double) iter;
               AWAKE(pln, 0);
	       return tmin;
          }
     }
     goto start_over; /* may happen if timer is screwed up */
}
