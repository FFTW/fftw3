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

/* $Id: tensor.c,v 1.1.1.1 2002-06-02 18:42:32 athena Exp $ */

#include "ifftw.h"

static inline int imax(int a, int b)
{
     return (a > b) ? a : b;
}

static inline int imin(int a, int b)
{
     return (a < b) ? a : b;
}

static void talloc(tensor *x, int rnk)
{
     A(rnk >= 0);
     x->rnk = rnk;
     x->dims = (iodim *)fftw_malloc(sizeof(iodim) * rnk, TENSORS);
}

void fftw_tensor_destroy(tensor sz)
{
     fftw_free(sz.dims);
}

tensor fftw_mktensor(int rnk)
{
     tensor x;
     talloc(&x, rnk);
     return x;
}

tensor fftw_mktensor_1d(int n, int is, int os)
{
     tensor x;
     talloc(&x, 1);
     x.dims[0].n = n;
     x.dims[0].is = is;
     x.dims[0].os = os;
     return x;
}

tensor fftw_mktensor_2d(int n0, int is0, int os0, int n1, int is1, int os1)
{
     tensor x;
     talloc(&x, 2);
     x.dims[0].n = n0;
     x.dims[0].is = is0;
     x.dims[0].os = os0;
     x.dims[1].n = n1;
     x.dims[1].is = is1;
     x.dims[1].os = os1;
     return x;
}

tensor fftw_mktensor_rowmajor(int rnk, const int *n, const int *nphys,
			      int is, int os)
{
     tensor x;
     talloc(&x, rnk);

     if (rnk > 0) {
	  int i;

	  A(n && nphys);
	  x.dims[rnk - 1].is = is;
	  x.dims[rnk - 1].os = os;
	  x.dims[rnk - 1].n = n[rnk - 1];
	  for (i = rnk - 2; i >= 0; --i) {
	       x.dims[i].is = x.dims[i + 1].is * nphys[i + 1];
	       x.dims[i].os = x.dims[i + 1].os * nphys[i + 1];
	       x.dims[i].n = n[i];
	  }
     }
     return x;
}

int fftw_tensor_equal(const tensor a, const tensor b)
{
     int i;

     if (a.rnk != b.rnk)
	  return 0;

     for (i = 0; i < a.rnk; ++i) {
	  iodim *ad = a.dims + i, *bd = b.dims + i;
	  if (ad->n != bd->n || ad->is != bd->is || ad->os != bd->os)
	       return 0;
     }
     return 1;
}

int fftw_tensor_sz(const tensor sz)
{
     int i, n = 1;
     for (i = 0; i < sz.rnk; ++i)
	  n *= sz.dims[i].n;
     return n;
}

int fftw_tensor_hash(const tensor t)
{
     int i, h = 1023 * t.rnk;

     /* FIXME: find a decent hash function */
     for (i = 0; i < t.rnk; ++i) {
	  iodim *p = t.dims + i;
	  h = h * 13131 + p->n + 7 * p->is + 13 * p->os;
     }

     return h;
}

int fftw_tensor_max_index(const tensor sz)
{
     int i, n = 0;
     for (i = 0; i < sz.rnk; ++i) {
	  iodim *p = sz.dims + i;
	  n += (p->n - 1) * imax(p->is, p->os);
     }
     return n;
}

int fftw_tensor_min_stride(const tensor sz)
{
     if (sz.rnk == 0)
	  return 0;
     else {
	  int i, s = imin(sz.dims[0].is, sz.dims[0].os);
	  for (i = 1; i < sz.rnk; ++i) {
	       iodim *p = sz.dims + i;
	       s = imin(s, imin(p->is, p->os));
	  }
	  return s;
     }
}

int fftw_tensor_inplace_strides(const tensor sz)
{
     int i;
     for (i = 0; i < sz.rnk; ++i) {
	  iodim *p = sz.dims + i;
	  if (p->is != p->os)
	       return 0;
     }
     return 1;
}

static void dimcpy(iodim *dst, const iodim *src, int n)
{
     int i;
     for (i = 0; i < n; ++i)
	  dst[i] = src[i];
}

tensor fftw_tensor_copy(const tensor sz)
{
     tensor x;

     talloc(&x, sz.rnk);
     dimcpy(x.dims, sz.dims, sz.rnk);
     return x;
}

/* Like fftw_tensor_copy, but copy all of the dimensions *except*
   except_dim. */
tensor fftw_tensor_copy_except(const tensor sz, int except_dim)
{
     tensor x;

     A(sz.rnk >= 1 && except_dim >= 0 && except_dim < sz.rnk);
     talloc(&x, sz.rnk - 1);
     dimcpy(x.dims, sz.dims, except_dim);
     dimcpy(x.dims + except_dim, sz.dims + except_dim + 1, 
	    x.rnk - except_dim);
     return x;
}

/* Like fftw_tensor_copy, but copy only rnk dimensions starting
   with start_dim. */
tensor fftw_tensor_copy_sub(const tensor sz, int start_dim, int rnk)
{
     tensor x;

     A(start_dim >= 0 && sz.rnk >= 0 && start_dim + rnk <= sz.rnk);
     talloc(&x, rnk);
     dimcpy(x.dims, sz.dims + start_dim, rnk);
     return x;
}

/* qsort comparison function */
static int cmp_iodim(const void *av, const void *bv)
{
     const iodim *a = (const iodim *) av, *b = (const iodim *) bv;
     if (b->is != a->is)
	  return (b->is - a->is);	/* shorter strides go later */
     if (b->os != a->os)
	  return (b->os - a->os);	/* shorter strides go later */
     return (a->n - b->n);	/* larger n's go later */
}

/* Like tensor_copy, but eliminate n == 1 dimensions, which
   never effect any transform or transform vector.

   Also, we sort the tensor into a canonical order of decreasing
   is.  In general, processing a loop/array in order of
   decreasing stride will improve locality; sorting also makes the
   analysis in fftw_tensor_contiguous (below) easier.  The choice
   of is over os is mostly arbitrary, and hopefully
   shouldn't affect things much.  Normally, either the os will be
   in the same order as is (for e.g. multi-dimensional
   transforms) or will be in opposite order (e.g. for Cooley-Tukey
   recursion).  (Both forward and backwards traversal of the tensor
   are considered e.g. by ridft-vecloop, so sorting in increasing
   vs. decreasing order is not really important.) */
tensor fftw_tensor_compress(const tensor sz)
{
     int i, rnk;
     tensor x;

     A(sz.rnk >= 0);
     for (i = 0, rnk = 0; i < sz.rnk; ++i) {
	  A(sz.dims[i].n > 0);
	  if (sz.dims[i].n != 1)
	       ++rnk;
     }

     talloc(&x, rnk);
     for (i = 0, rnk = 0; i < sz.rnk; ++i) {
	  if (sz.dims[i].n != 1)
	       x.dims[rnk++] = sz.dims[i];
     }

     qsort(x.dims, x.rnk, sizeof(iodim), cmp_iodim);

     return x;
}

/* Return whether the strides of a and b are such that they form an
   effective contiguous 1d array.  Assumes that a.is >= b.is. */
static int strides_contig(iodim *a, iodim *b)
{
     return (a->is == b->is * b->n && a->os == b->os * b->n);
}

/* Like tensor_compress, but also compress into one dimension any
   group of dimensions that form a contiguous block of indices with
   some stride.  (This can safely be done for transform vector sizes.) */
tensor fftw_tensor_compress_contiguous(const tensor sz)
{
     int i, rnk;
     tensor sz2, x;

     sz2 = fftw_tensor_compress(sz);

     if (sz2.rnk < 2)		/* nothing to compress */
	  return sz2;

     for (i = 1, rnk = 1; i < sz2.rnk; ++i)
	  if (!strides_contig(sz2.dims + i - 1, sz2.dims + i))
	       ++rnk;

     talloc(&x, rnk);
     x.dims[0] = sz2.dims[0];
     for (i = 1, rnk = 1; i < sz2.rnk; ++i) {
	  if (strides_contig(sz2.dims + i - 1, sz2.dims + i)) {
	       x.dims[rnk - 1].n *= sz2.dims[i].n;
	       x.dims[rnk - 1].is = sz2.dims[i].is;
	       x.dims[rnk - 1].os = sz2.dims[i].os;
	  } else {
	       A(rnk < x.rnk);
	       x.dims[rnk++] = sz2.dims[i];
	  }
     }

     fftw_tensor_destroy(sz2);
     return x;
}

tensor fftw_tensor_append(const tensor a, const tensor b)
{
     tensor x;

     A(a.rnk >= 0 && b.rnk >= 0);
     talloc(&x, a.rnk + b.rnk);
     dimcpy(x.dims, a.dims, a.rnk);
     dimcpy(x.dims + a.rnk, b.dims, b.rnk);
     return x;
}

/* The inverse of fftw_tensor_append: splits the sz tensor into
   tensor a followed by tensor b, where a's rank is arnk. */
void fftw_tensor_split(const tensor sz, tensor *a, int arnk, tensor *b)
{
     *a = fftw_tensor_copy_sub(sz, 0, arnk);
     *b = fftw_tensor_copy_sub(sz, arnk, sz.rnk - arnk);
}
