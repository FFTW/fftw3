/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* $Id: stride.c,v 1.11 2006-01-05 03:04:27 stevenj Exp $ */
#include "ifftw.h"

const INT X(a_stride_guaranteed_to_be_zero) = 0;

#ifdef PRECOMPUTE_ARRAY_INDICES
stride X(mkstride)(INT n, INT s)
{
     int i;
     INT *p = (INT *) MALLOC(n * sizeof(INT), STRIDES);

     for (i = 0; i < n; ++i)
          p[i] = s * i;

     return p;
}

void X(stride_destroy)(stride p)
{
     X(ifree0)(p);
}

#endif
