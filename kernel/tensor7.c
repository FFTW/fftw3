/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* $Id: tensor7.c,v 1.5 2003-03-15 20:29:43 stevenj Exp $ */

#include "ifftw.h"

/* total order among iodim's */
int X(dimcmp)(const iodim *a, const iodim *b)
{
     if (b->is != a->is)
          return (b->is - a->is);	/* shorter strides go later */
     if (b->os != a->os)
          return (b->os - a->os);	/* shorter strides go later */
     return (int)(a->n - b->n);	        /* larger n's go later */
}

/* Like tensor_copy, but eliminate n == 1 dimensions, which
   never affect any transform or transform vector.
 
   Also, we sort the tensor into a canonical order of decreasing
   is.  In general, processing a loop/array in order of
   decreasing stride will improve locality; sorting also makes the
   analysis in fftw_tensor_contiguous (below) easier.  The choice
   of is over os is mostly arbitrary, and hopefully
   shouldn't affect things much.  Normally, either the os will be
   in the same order as is (for e.g. multi-dimensional
   transforms) or will be in opposite order (e.g. for Cooley-Tukey
   recursion).  (Both forward and backwards traversal of the tensor
   are considered e.g. by vrank-geq1, so sorting in increasing
   vs. decreasing order is not really important.) */
tensor *X(tensor_compress)(const tensor *sz)
{
     int i, rnk;
     tensor *x;

     A(FINITE_RNK(sz->rnk));
     for (i = rnk = 0; i < sz->rnk; ++i) {
          A(sz->dims[i].n > 0);
          if (sz->dims[i].n != 1)
               ++rnk;
     }

     x = X(mktensor)(rnk);
     for (i = rnk = 0; i < sz->rnk; ++i) {
          if (sz->dims[i].n != 1)
               x->dims[rnk++] = sz->dims[i];
     }

     if (rnk) {
	  /* God knows how qsort() behaves if n==0 */
	  qsort(x->dims, (size_t)x->rnk, sizeof(iodim),
		(int (*)(const void *, const void *))X(dimcmp));
     }

     return x;
}

/* Return whether the strides of a and b are such that they form an
   effective contiguous 1d array.  Assumes that a.is >= b.is. */
static int strides_contig(iodim *a, iodim *b)
{
     return (a->is == b->is * (int)b->n &&
             a->os == b->os * (int)b->n);
}

/* Like tensor_compress, but also compress into one dimension any
   group of dimensions that form a contiguous block of indices with
   some stride.  (This can safely be done for transform vector sizes.) */
tensor *X(tensor_compress_contiguous)(const tensor *sz)
{
     int i, rnk;
     tensor *sz2, *x;

     if (X(tensor_sz)(sz) == 0) 
	  return X(mktensor)(RNK_MINFTY);

     sz2 = X(tensor_compress)(sz);
     A(FINITE_RNK(sz2->rnk));

     if (sz2->rnk < 2)		/* nothing to compress */
          return sz2;

     for (i = rnk = 1; i < sz2->rnk; ++i)
          if (!strides_contig(sz2->dims + i - 1, sz2->dims + i))
               ++rnk;

     x = X(mktensor)(rnk);
     x->dims[0] = sz2->dims[0];
     for (i = rnk = 1; i < sz2->rnk; ++i) {
          if (strides_contig(sz2->dims + i - 1, sz2->dims + i)) {
               x->dims[rnk - 1].n *= sz2->dims[i].n;
               x->dims[rnk - 1].is = sz2->dims[i].is;
               x->dims[rnk - 1].os = sz2->dims[i].os;
          } else {
               A(rnk < x->rnk);
               x->dims[rnk++] = sz2->dims[i];
          }
     }

     X(tensor_destroy)(sz2);
     return x;
}

/* The inverse of X(tensor_append): splits the sz tensor into
   tensor a followed by tensor b, where a's rank is arnk. */
void X(tensor_split)(const tensor *sz, tensor **a, int arnk, tensor **b)
{
     A(FINITE_RNK(sz->rnk) && FINITE_RNK(arnk));

     *a = X(tensor_copy_sub)(sz, 0, arnk);
     *b = X(tensor_copy_sub)(sz, arnk, sz->rnk - arnk);
}
