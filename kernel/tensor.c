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

/* $Id: tensor.c,v 1.30 2002-09-25 01:27:49 athena Exp $ */

#include "ifftw.h"

tensor *X(mktensor)(uint rnk) 
{
     tensor *x;

#if defined(STRUCT_HACK_KR)
     if (FINITE_RNK(rnk) && rnk > 1)
	  x = (tensor *)fftw_malloc(sizeof(tensor) + (rnk - 1) * sizeof(iodim),
				    TENSORS);
     else
	  x = (tensor *)fftw_malloc(sizeof(tensor), TENSORS);
#elif defined(STRUCT_HACK_C99)
     if (FINITE_RNK(rnk))
	  x = (tensor *)fftw_malloc(sizeof(tensor) + rnk * sizeof(iodim),
				    TENSORS);
     else
	  x = (tensor *)fftw_malloc(sizeof(tensor), TENSORS);
#else
     x = (tensor *)fftw_malloc(sizeof(tensor), TENSORS);
     if (FINITE_RNK(rnk) && rnk > 0)
          x->dims = (iodim *)fftw_malloc(sizeof(iodim) * rnk, TENSORS);
     else
          x->dims = 0;
#endif

     x->rnk = rnk;
     return x;
}

void X(tensor_destroy)(tensor *sz)
{
#if !defined(STRUCT_HACK_C99) && !defined(STRUCT_HACK_KR)
     X(free0)(sz->dims);
#endif
     X(free)(sz);
}

uint X(tensor_sz)(const tensor *sz)
{
     uint i, n = 1;

     if (!FINITE_RNK(sz->rnk))
          return 0;

     for (i = 0; i < sz->rnk; ++i)
          n *= sz->dims[i].n;
     return n;
}

void X(tensor_md5)(md5 *p, const tensor *t)
{
     uint i;
     X(md5uint)(p, t->rnk);
     if (FINITE_RNK(t->rnk)) {
	  for (i = 0; i < t->rnk; ++i) {
	       const iodim *q = t->dims + i;
	       X(md5uint)(p, q->n);
	       X(md5int)(p, q->is);
	       X(md5int)(p, q->os);
	  }
     }
}

/* treat a (rank <= 1)-tensor as a rank-1 tensor, extracting
   appropriate n, is, and os components */
void X(tensor_tornk1)(const tensor *t, uint *n, int *is, int *os)
{
     A(t->rnk <= 1);
     if (t->rnk == 1) {
	  iodim *vd = t->dims;
          *n = vd[0].n;
          *is = vd[0].is;
          *os = vd[0].os;
     } else {
          *n = 1U;
          *is = *os = 0;
     }
}

void X(tensor_print)(const tensor *x, printer *p)
{
     p->print(p, "(t:%d", FINITE_RNK(x->rnk) ? (int) x->rnk : -1);
     if (FINITE_RNK(x->rnk)) {
	  uint i;
	  for (i = 0; i < x->rnk; ++i) {
	       const iodim *d = x->dims + i;
	       p->print(p, " (%u %d %d)", d->n, d->is, d->os);
	  }
     }
     p->print(p, ")");
}
