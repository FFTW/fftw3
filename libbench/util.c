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

#include "config.h"
#include "bench.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

void bench_assertion_failed(const char *s, int line, char *file)
{
     fflush(stdout);
     fprintf(stderr, "bench: %s:%d: assertion failed: %s\n", file, line, s);
     exit(EXIT_FAILURE);
}

int bench_square(int x) 
{
     return x * x;
}

#ifdef HAVE_DRAND48
#  if !defined(HAVE_DECL_DRAND48) || !HAVE_DECL_DRAND48
extern double drand48(void);
#  endif
double bench_drand(void)
{
     return drand48() - 0.5;
}
#else
double bench_drand(void)
{
     double d = rand();
     return (d / (double) RAND_MAX) - 0.5;
}
#endif

/**********************************************************
 *   DEBUGGING CODE
 **********************************************************/
#ifdef BENCH_DEBUG
static int bench_malloc_cnt = 0;

/*
 * debugging malloc/free.  Initialize every malloced and freed area to
 * random values, just to make sure we are not using uninitialized
 * pointers.  Also check for writes past the ends of allocated blocks,
 * and a couple of other things.
 *
 * This code is a quick and dirty hack -- use at your own risk.
 */

static int bench_malloc_total = 0, bench_malloc_max = 0, bench_malloc_cnt_max = 0;

#define MAGIC ((size_t)0xABadCafe)
#define PAD_FACTOR 2
#define TWO_SIZE_T (2 * sizeof(size_t))

#define VERBOSE_ALLOCATION 0

#if VERBOSE_ALLOCATION
#define WHEN_VERBOSE(a) a
#else
#define WHEN_VERBOSE(a)
#endif

void *bench_malloc(size_t n)
{
     char *p;
     size_t i;

     bench_malloc_total += n;

     if (bench_malloc_total > bench_malloc_max)
	  bench_malloc_max = bench_malloc_total;

     p = (char *) malloc(PAD_FACTOR * n + TWO_SIZE_T);
     BENCH_ASSERT(p);

     /* store the size in a known position */
     ((size_t *) p)[0] = n;
     ((size_t *) p)[1] = MAGIC;
     for (i = 0; i < PAD_FACTOR * n; i++)
	  p[i + TWO_SIZE_T] = (char) (i ^ 0xDEADBEEF);

     ++bench_malloc_cnt;

     if (bench_malloc_cnt > bench_malloc_cnt_max)
	  bench_malloc_cnt_max = bench_malloc_cnt;

     /* skip the size we stored previously */
     return (void *) (p + TWO_SIZE_T);
}

void bench_free(void *p)
{
     char *q;

     BENCH_ASSERT(p);

     q = ((char *) p) - TWO_SIZE_T;
     BENCH_ASSERT(q);

     {
	  size_t n = ((size_t *) q)[0];
	  size_t magic = ((size_t *) q)[1];
	  size_t i;

	  ((size_t *) q)[0] = 0; /* set to zero to detect duplicate free's */

	  BENCH_ASSERT(magic == MAGIC);
	  ((size_t *) q)[1] = ~MAGIC;

	  bench_malloc_total -= n;
	  BENCH_ASSERT(bench_malloc_total >= 0);

	  /* check for writing past end of array: */
	  for (i = n; i < PAD_FACTOR * n; ++i)
	       if (q[i + TWO_SIZE_T] != (char) (i ^ 0xDEADBEEF)) {
		    BENCH_ASSERT(0 /* array bounds overwritten */);
	       }
	  for (i = 0; i < PAD_FACTOR * n; ++i)
	       q[i + TWO_SIZE_T] = (char) (i ^ 0xBEEFDEAD);

	  --bench_malloc_cnt;

	  BENCH_ASSERT(bench_malloc_cnt >= 0);

	  BENCH_ASSERT(
	       (bench_malloc_cnt == 0 && bench_malloc_total == 0) ||
	       (bench_malloc_cnt > 0 && bench_malloc_total > 0));

	  free(q);
     }
}

#else
/**********************************************************
 *   NON DEBUGGING CODE
 **********************************************************/
/* production version, no hacks */

void *bench_malloc(size_t n)
{
     void *p;
     if (n == 0) n = 1;

#ifdef HAVE_MEMALIGN
     {
	  const int alignment = 16; /* power of 2 */
	  p = memalign(alignment, n);
     }
#else
     p = malloc(n);
#endif

     BENCH_ASSERT(p);
     return p;
}

void bench_free(void *p)
{
     free(p);
}

#endif
