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

/* $Id: tensor4.c,v 1.5 2003-01-17 09:23:37 athena Exp $ */

#include "ifftw.h"

int X(tensor_max_index)(const tensor *sz)
{
     int i;
     int n = 0;

     A(FINITE_RNK(sz->rnk));
     for (i = 0; i < sz->rnk; ++i) {
          const iodim *p = sz->dims + i;
          n += (p->n - 1) * X(imax)(X(iabs)(p->is), X(iabs)(p->os));
     }
     return n;
}

#define tensor_min_xstride(sz, xs) {			\
     A(FINITE_RNK(sz->rnk));				\
     if (sz->rnk == 0) return 0;			\
     else {						\
          int i;					\
          int s = X(iabs)(sz->dims[0].xs);		\
          for (i = 1; i < sz->rnk; ++i)			\
               s = X(imin)(s, X(iabs)(sz->dims[i].xs));	\
          return s;					\
     }							\
}

int X(tensor_min_istride)(const tensor *sz) tensor_min_xstride(sz, is)
int X(tensor_min_ostride)(const tensor *sz) tensor_min_xstride(sz, os)

int X(tensor_min_stride)(const tensor *sz)
{
     return X(imin)(X(tensor_min_istride)(sz), X(tensor_min_ostride)(sz));
}

int X(tensor_inplace_strides)(const tensor *sz)
{
     int i;
     A(FINITE_RNK(sz->rnk));
     for (i = 0; i < sz->rnk; ++i) {
          const iodim *p = sz->dims + i;
          if (p->is != p->os)
               return 0;
     }
     return 1;
}

int X(tensor_inplace_strides2)(const tensor *a, const tensor *b)
{
     return X(tensor_inplace_strides(a)) && X(tensor_inplace_strides(b));
}
