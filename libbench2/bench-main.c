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

/* $Id: bench-main.c,v 1.8 2003-04-02 01:57:39 athena Exp $ */

#include "getopt.h"
#include "bench.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int verbose;

static struct option long_options[] =
{
  {"accuracy", required_argument, 0, 'a'},
  {"accuracy-rounds", required_argument, 0, 405},
  {"impulse-accuracy-rounds", required_argument, 0, 406},
  {"can-do", required_argument, 0, 'd'},
  {"help", no_argument, 0, 'h'},
  {"info", required_argument, 0, 'i'},
  {"info-all", no_argument, 0, 'I'},
  {"print-precision", no_argument, 0, 402},
  {"print-time-min", no_argument, 0, 400},
  {"random-seed", required_argument, 0, 404},
  {"report-avg-mflops", no_argument, 0, 302},
  {"report-avg-time", no_argument, 0, 312},
  {"report-benchmark", no_argument, 0, 320},
  {"report-max-mflops", no_argument, 0, 301},
  {"report-mflops", no_argument, 0, 300},
  {"report-min-time", no_argument, 0, 311},
  {"report-time", no_argument, 0, 310},
  {"speed", required_argument, 0, 's'},
  {"time-min", required_argument, 0, 't'},
  {"time-repeat", required_argument, 0, 'r'},
  {"user-option", required_argument, 0, 'o'},
  {"verbose", optional_argument, 0, 'v'},
  {"verify", required_argument, 0, 'y'},
  {"verify-rounds", required_argument, 0, 401},
  {"verify-tolerance", required_argument, 0, 403},
  {0, no_argument, 0, 0}
};

static void check_alignment(void)
{
     double x;
     BENCH_ASSERT((((long)&x) & 0x7) == 0);
}

int bench_main(int argc, char *argv[])
{
     double tmin = 0.0;
     double tol;
     int repeat = 0;
     int rounds = 10;
     int iarounds = 0;
     int arounds = 1; /* this is too low for precise results */
     int c;
     int index;
     char *short_options = make_short_options(long_options);

     check_alignment();

     report = report_time; /* default */
     verbose = 0;

     tol = SINGLE_PRECISION ? 1.0e-3 : 1.0e-10;
     bench_srand(1);

     while ((c = getopt_long (argc, argv, short_options,
			      long_options, &index)) != -1) {
	  switch (c) {
	      case 't' :
		   tmin = strtod(optarg, 0);
		   break;
	      case 'r':
		   repeat = atoi(optarg);
		   break;
	      case 's':
		   timer_init(tmin, repeat);
		   speed(optarg);
		   break;
	      case 'd':
		   report_can_do(optarg);
		   break;
	      case 'o':
		   useropt(optarg);
		   break;
	      case 'v':
		   if (optarg)
			verbose = atoi(optarg);
		   else
			++verbose;
		   break;
	      case 'y':
		   verify(optarg, rounds, tol);
		   break;
	      case 'a':
		   accuracy(optarg, arounds, iarounds);
		   break;
	      case 'i':
		   report_info(optarg);
		   break;
	      case 'I':
		   report_info_all();
		   break;
	      case 'h':
		   usage(argv[0], long_options);
		   break;

	      case 300: /* --report-mflops */
		   report = report_mflops;
		   break;
	      case 301: /* --report-max-mflops */
		   report = report_max_mflops;
		   break;
	      case 302: /* --report-avg-mflops */
		   report = report_avg_mflops;
		   break;

	      case 310: /* --report-time */
		   report = report_time;
		   break;
	      case 311: /* --report-min-time */
		   report = report_min_time;
		   break;
	      case 312: /* --report-avg-time */
		   report = report_avg_time;
		   break;

 	      case 320: /* --report-benchmark */
		   report = report_benchmark;
		   break;

	      case 400: /* --print-time-min */
		   timer_init(tmin, repeat);
		   ovtpvt("%g\n", time_min);
		   break;

	      case 401: /* --verify-rounds */
		   rounds = atoi(optarg);
		   break;

	      case 402: /* --print-precision */
		   if (SINGLE_PRECISION)
			ovtpvt("single\n");
		   else if (LDOUBLE_PRECISION)
			ovtpvt("long-double\n");
		   else if (DOUBLE_PRECISION)
			ovtpvt("double\n");
		   else 
			ovtpvt("unknown %d\n", sizeof(bench_real));
		   break;

	      case 403: /* --verify-tolerance */
		   tol = strtod(optarg, 0);
		   break;

	      case 404: /* --random-seed */
		   bench_srand(atoi(optarg));
		   break;

	      case 405: /* --accuracy-rounds */
		   arounds = atoi(optarg);
		   break;
		   
	      case 406: /* --impulse-accuracy-rounds */
		   iarounds = atoi(optarg);
		   break;
		   
	      case '?':
		   /* `getopt_long' already printed an error message. */
		   break;

	      default:
		   abort ();
	  }
     }

     /* Print any remaining command line arguments (not options). */
     if (optind < argc) {
	  fprintf(stderr, "unrecognized non-option arguments: ");
	  while (optind < argc)
	       fprintf(stderr, "%s ", argv[optind++]);
	  fprintf(stderr, "\n");
     }

     cleanup();
     bench_free(short_options);
     return 0;
}
