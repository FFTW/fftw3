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

/* $Id: report.c,v 1.4 2006-01-05 03:04:27 stevenj Exp $ */

#include "config.h"
#include "bench.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void (*report)(const struct problem *p, double *t, int st);

/* report WHAT */
enum {
     W_TIME = 0x1,
     W_MFLOPS = 0x2
};

/* report HOW */
enum { H_ALL, H_MIN, H_MAX, H_AVG };

#undef min
#undef max /* you never know */

struct stats {
     double min;
     double max;
     double avg;
     double median;
};

static void mkstat(double *t, int st, struct stats *a)
{
     int i, j;
     
     a->min = t[0];
     a->max = t[0];
     a->avg = 0.0;

     for (i = 0; i < st; ++i) {
	  if (t[i] < a->min)
	       a->min = t[i];
	  if (t[i] > a->max)
	       a->max = t[i];
	  a->avg += t[i];
     }
     a->avg /= (double)st;

     /* compute median --- silly bubblesort algorithm */
     for (i = st - 1; i > 1; --i) {
	  for (j = 0; j < i - 1; ++j) {
	       double t0, t1;
	       if ((t0 = t[j]) > (t1 = t[j + 1])) {
		    t[j] = t0;
		    t[j + 1] = t1;
	       }
	  } 
     }
     a->median = t[st / 2];
}

static void report_generic(const struct problem *p, double *t, int st,
			   int what, int how)
{
     struct stats s;

     mkstat(t, st, &s);

     if (what & W_TIME) {
	  switch (how) {
	      case H_ALL:
		   ovtpvt("(%g %g %g %g)\n", s.min, s.avg, s.max, s.median);
		   break;
	      case H_MIN:
		   ovtpvt("%g\n", s.min);
		   break;
	      case H_MAX:
		   ovtpvt("%g\n", s.max);
		   break;
	      case H_AVG:
		   ovtpvt("%g\n", s.avg);
		   break;
	  }
     }

     if (what & W_MFLOPS) {
	  switch (how) {
	      case H_ALL:
		   ovtpvt("(%g %g %g)\n", 
			  mflops(p, s.max), mflops(p, s.avg), 
			  mflops(p, s.min));
		   break;
	      case H_MIN:
		   ovtpvt("%g\n", mflops(p, s.max));
		   break;
	      case H_MAX:
		   ovtpvt("%g\n", mflops(p, s.max));
		   break;
	      case H_AVG:
		   ovtpvt("%g\n", mflops(p, s.avg));
		   break;
	  }
     }
}

void report_mflops(const struct problem *p, double *t, int st)
{
     report_generic(p, t, st, W_MFLOPS, H_ALL);
}

void report_max_mflops(const struct problem *p, double *t, int st)
{
     report_generic(p, t, st, W_MFLOPS, H_MAX);
}

void report_avg_mflops(const struct problem *p, double *t, int st)
{
     report_generic(p, t, st, W_MFLOPS, H_AVG);
}

void report_time(const struct problem *p, double *t, int st)
{
     report_generic(p, t, st, W_TIME, H_ALL);
}

void report_min_time(const struct problem *p, double *t, int st)
{
     report_generic(p, t, st, W_TIME, H_MIN);
}

void report_avg_time(const struct problem *p, double *t, int st)
{
     report_generic(p, t, st, W_TIME, H_AVG);
}

void report_benchmark(const struct problem *p, double *t, int st)
{
     struct stats s;
     mkstat(t, st, &s);
     ovtpvt("%.5g %.8g\n", mflops(p, s.min), s.min);
}
