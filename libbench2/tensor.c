/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Steven G. Johnson
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

/* $Id: tensor.c,v 1.3 2003-01-18 12:20:18 athena Exp $ */
#include "bench.h"
#include <stdlib.h>

tensor *mktensor(int rnk) 
{
     tensor *x;

     BENCH_ASSERT(rnk >= 0);

     x = (tensor *)bench_malloc(sizeof(tensor));
     if (FINITE_RNK(rnk) && rnk > 0)
          x->dims = (iodim *)bench_malloc(sizeof(iodim) * rnk);
     else
          x->dims = 0;

     x->rnk = rnk;
     return x;
}

void tensor_destroy(tensor *sz)
{
     bench_free0(sz->dims);
     bench_free(sz);
}

int tensor_sz(const tensor *sz)
{
     int i, n = 1;

     if (!FINITE_RNK(sz->rnk))
          return 0;

     for (i = 0; i < sz->rnk; ++i)
          n *= sz->dims[i].n;
     return n;
}


/* total order among iodim's */
static int dimcmp(const iodim *a, const iodim *b)
{
     if (b->is != a->is)
          return (b->is - a->is);	/* shorter strides go later */
     if (b->os != a->os)
          return (b->os - a->os);	/* shorter strides go later */
     return (int)(a->n - b->n);	        /* larger n's go later */
}

tensor *tensor_compress(const tensor *sz)
{
     int i, rnk;
     tensor *x;

     BENCH_ASSERT(FINITE_RNK(sz->rnk));
     for (i = rnk = 0; i < sz->rnk; ++i) {
          BENCH_ASSERT(sz->dims[i].n > 0);
          if (sz->dims[i].n != 1)
               ++rnk;
     }

     x = mktensor(rnk);
     for (i = rnk = 0; i < sz->rnk; ++i) {
          if (sz->dims[i].n != 1)
               x->dims[rnk++] = sz->dims[i];
     }

     if (rnk) {
	  /* God knows how qsort() behaves if n==0 */
	  qsort(x->dims, (size_t)x->rnk, sizeof(iodim),
		(int (*)(const void *, const void *))dimcmp);
     }

     return x;
}

int tensor_unitstridep(tensor *t)
{
     BENCH_ASSERT(FINITE_RNK(t->rnk));
     return (t->rnk == 0 ||
	     (t->dims[t->rnk - 1].is == 1 && t->dims[t->rnk - 1].os == 1));
}

int tensor_rowmajorp(tensor *t)
{
     int i;

     BENCH_ASSERT(FINITE_RNK(t->rnk));

     i = t->rnk - 1;
     while (--i >= 0) {
	  iodim *d = t->dims + i;
	  if (d[0].is != d[1].is * d[0].n)
	       return 0;
	  if (d[0].os != d[1].os * d[0].n)
	       return 0;
     }
     return 1;
}

static void dimcpy(iodim *dst, const iodim *src, int rnk)
{
     int i;
     if (FINITE_RNK(rnk))
          for (i = 0; i < rnk; ++i)
               dst[i] = src[i];
}

tensor *tensor_append(const tensor *a, const tensor *b)
{
     if (!FINITE_RNK(a->rnk) || !FINITE_RNK(b->rnk)) {
          return mktensor(RNK_MINFTY);
     } else {
	  tensor *x = mktensor(a->rnk + b->rnk);
          dimcpy(x->dims, a->dims, a->rnk);
          dimcpy(x->dims + a->rnk, b->dims, b->rnk);
	  return x;
     }
}

static int imax(int a, int b)
{
     return (a > b) ? a : b;
}

static int imin(int a, int b)
{
     return (a < b) ? a : b;
}

#define DEFBOUNDS(name, xs)			\
void name(tensor *t, int *lbp, int *ubp)	\
{						\
     int lb = 0;				\
     int ub = 1;				\
     int i;					\
						\
     BENCH_ASSERT(FINITE_RNK(t->rnk));		\
						\
     for (i = 0; i < t->rnk; ++i) {		\
	  iodim *d = t->dims + i;		\
	  int n = d->n;				\
	  int s = d->xs;			\
	  lb = imin(lb, lb + s * (n - 1));	\
	  ub = imax(ub, ub + s * (n - 1));	\
     }						\
						\
     *lbp = lb;					\
     *ubp = ub;					\
}

DEFBOUNDS(tensor_ibounds, is)
DEFBOUNDS(tensor_obounds, os)

tensor *tensor_copy(const tensor *sz)
{
     tensor *x = mktensor(sz->rnk);
     dimcpy(x->dims, sz->dims, sz->rnk);
     return x;
}
