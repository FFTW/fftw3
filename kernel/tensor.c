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

/* $Id: tensor.c,v 1.11 2002-06-13 15:54:02 athena Exp $ */

#include "ifftw.h"

static inline int imax(int a, int b)
{
     return (a > b) ? a : b;
}

static inline int imin(int a, int b)
{
     return (a < b) ? a : b;
}

static void talloc(tensor *x, uint rnk)
{
     x->rnk = rnk;
     if (FINITE_RNK(rnk) && rnk > 0)
          x->dims = (iodim *)fftw_malloc(sizeof(iodim) * rnk, TENSORS);
     else
          x->dims = 0;
}

void X(tensor_destroy)(tensor sz)
{
     if (sz.dims)
          X(free)(sz.dims);
}

tensor X(mktensor)(uint rnk)
{
     tensor x;
     talloc(&x, rnk);
     return x;
}

tensor X(mktensor_1d)(uint n, int is, int os)
{
     tensor x;
     talloc(&x, 1);
     x.dims[0].n = n;
     x.dims[0].is = is;
     x.dims[0].os = os;
     return x;
}

tensor X(mktensor_2d)(uint n0, int is0, int os0,
                      uint n1, int is1, int os1)
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

tensor X(mktensor_rowmajor)(uint rnk, const uint *n, const uint *nphys,
                            int is, int os)
{
     tensor x;
     talloc(&x, rnk);

     if (FINITE_RNK(rnk) && rnk > 0) {
          uint i;

          A(n && nphys);
          x.dims[rnk - 1].is = is;
          x.dims[rnk - 1].os = os;
          x.dims[rnk - 1].n = n[rnk - 1];
          for (i = rnk - 1; i > 0; --i) {
               x.dims[i - 1].is = x.dims[i].is * nphys[i];
               x.dims[i - 1].os = x.dims[i].os * nphys[i];
               x.dims[i - 1].n = n[i - 1];
          }
     }
     return x;
}

int X(tensor_equal)(const tensor a, const tensor b)
{
     uint i;

     if (a.rnk != b.rnk)
          return 0;

     if (!FINITE_RNK(a.rnk))
          return 1;

     for (i = 0; i < a.rnk; ++i) {
          iodim *ad = a.dims + i, *bd = b.dims + i;
          if (ad->n != bd->n || ad->is != bd->is || ad->os != bd->os)
               return 0;
     }
     return 1;
}

uint X(tensor_sz)(const tensor sz)
{
     uint i, n = 1;

     if (!FINITE_RNK(sz.rnk))
          return 0;

     for (i = 0; i < sz.rnk; ++i)
          n *= sz.dims[i].n;
     return n;
}

uint X(tensor_hash)(const tensor t)
{
     uint i;
     uint h = 1023 * t.rnk;

     /* FIXME: find a decent hash function */
     if (FINITE_RNK(t.rnk)) {
	  for (i = 0; i < t.rnk; ++i) {
	       iodim *p = t.dims + i;
	       h = (h * 17) ^ p->n ^ (7 * p->is) ^ (13 * p->os);
	  }
     }

     return h;
}

int X(tensor_max_index)(const tensor sz)
{
     uint i;
     int n = 0;

     A(FINITE_RNK(sz.rnk));
     for (i = 0; i < sz.rnk; ++i) {
          iodim *p = sz.dims + i;
          n += (p->n - 1) * imax(p->is, p->os);
     }
     return n;
}

int X(tensor_min_stride)(const tensor sz)
{
     A(FINITE_RNK(sz.rnk));
     if (sz.rnk == 0)
          return 0;
     else {
          uint i;
          int s = imin(sz.dims[0].is, sz.dims[0].os);
          for (i = 1; i < sz.rnk; ++i) {
               iodim *p = sz.dims + i;
               s = imin(s, imin(p->is, p->os));
          }
          return s;
     }
}

int X(tensor_inplace_strides)(const tensor sz)
{
     uint i;
     A(FINITE_RNK(sz.rnk));
     for (i = 0; i < sz.rnk; ++i) {
          iodim *p = sz.dims + i;
          if (p->is != p->os)
               return 0;
     }
     return 1;
}

static void dimcpy(iodim *dst, const iodim *src, uint rnk)
{
     uint i;
     if (FINITE_RNK(rnk))
          for (i = 0; i < rnk; ++i)
               dst[i] = src[i];
}

tensor X(tensor_copy)(const tensor sz)
{
     tensor x;

     talloc(&x, sz.rnk);
     dimcpy(x.dims, sz.dims, sz.rnk);
     return x;
}

/* Like X(tensor_copy), but copy all of the dimensions *except*
   except_dim. */
tensor X(tensor_copy_except)(const tensor sz, uint except_dim)
{
     tensor x;

     A(FINITE_RNK(sz.rnk) && sz.rnk >= 1 && except_dim < sz.rnk);
     talloc(&x, sz.rnk - 1);
     dimcpy(x.dims, sz.dims, except_dim);
     dimcpy(x.dims + except_dim, sz.dims + except_dim + 1,
            x.rnk - except_dim);
     return x;
}

/* Like X(tensor_copy), but copy only rnk dimensions starting
   with start_dim. */
tensor X(tensor_copy_sub)(const tensor sz, uint start_dim, uint rnk)
{
     tensor x;

     A(FINITE_RNK(sz.rnk) && start_dim + rnk <= sz.rnk);
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
     return (int)(a->n - b->n);	        /* larger n's go later */
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
tensor X(tensor_compress)(const tensor sz)
{
     uint i, rnk;
     tensor x;

     A(FINITE_RNK(sz.rnk));
     for (i = rnk = 0; i < sz.rnk; ++i) {
          A(sz.dims[i].n > 0);
          if (sz.dims[i].n != 1)
               ++rnk;
     }

     talloc(&x, rnk);
     for (i = rnk = 0; i < sz.rnk; ++i) {
          if (sz.dims[i].n != 1)
               x.dims[rnk++] = sz.dims[i];
     }

     qsort(x.dims, (size_t)x.rnk, sizeof(iodim), cmp_iodim);

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
tensor X(tensor_compress_contiguous)(const tensor sz)
{
     uint i, rnk;
     tensor sz2, x;

     if (X(tensor_sz)(sz) == 0) {
          talloc(&x, RNK_MINFTY);
          return x;
     }

     sz2 = X(tensor_compress)(sz);
     A(FINITE_RNK(sz2.rnk));

     if (sz2.rnk < 2)		/* nothing to compress */
          return sz2;

     for (i = rnk = 1; i < sz2.rnk; ++i)
          if (!strides_contig(sz2.dims + i - 1, sz2.dims + i))
               ++rnk;

     talloc(&x, rnk);
     x.dims[0] = sz2.dims[0];
     for (i = rnk = 1; i < sz2.rnk; ++i) {
          if (strides_contig(sz2.dims + i - 1, sz2.dims + i)) {
               x.dims[rnk - 1].n *= sz2.dims[i].n;
               x.dims[rnk - 1].is = sz2.dims[i].is;
               x.dims[rnk - 1].os = sz2.dims[i].os;
          } else {
               A(rnk < x.rnk);
               x.dims[rnk++] = sz2.dims[i];
          }
     }

     X(tensor_destroy)(sz2);
     return x;
}

tensor X(tensor_append)(const tensor a, const tensor b)
{
     tensor x;

     if (!FINITE_RNK(a.rnk) || !FINITE_RNK(b.rnk)) {
          talloc(&x, RNK_MINFTY);
     } else {
          talloc(&x, a.rnk + b.rnk);
          dimcpy(x.dims, a.dims, a.rnk);
          dimcpy(x.dims + a.rnk, b.dims, b.rnk);
     }
     return x;
}

/* The inverse of X(tensor_append): splits the sz tensor into
   tensor a followed by tensor b, where a's rank is arnk. */
void X(tensor_split)(const tensor sz, tensor *a, uint arnk, tensor *b)
{
     A(FINITE_RNK(sz.rnk) && FINITE_RNK(arnk));

     *a = X(tensor_copy_sub)(sz, 0, arnk);
     *b = X(tensor_copy_sub)(sz, arnk, sz.rnk - arnk);
}

void X(tensor_print)(tensor x, printer *p)
{
     p->print(p, "(t");
     if (FINITE_RNK(x.rnk)) {
	  uint i;
	  for (i = 0; i < x.rnk; ++i) {
	       iodim *d = x.dims + i;
	       p->print(p, " (%u %d %d)", d->n, d->is, d->os);
	  }
     } else {
	  p->print(p, " (0 0 0)");
     }
     p->print(p, ")");
}
