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

/* $Id: stride.c,v 1.1.1.1 2002-06-02 18:42:32 athena Exp $ */
#include "ifftw.h"

#ifdef PRECOMPUTE_ARRAY_INDICES
stride fftw_mkstride(int n, int s)
{
     int i;
     int *p = (int *) fftw_malloc(n * sizeof(int), OTHER);

     for (i = 0; i < n; ++i)
	  p[i] = s * i;

     return p;
}

void fftw_stride_destroy(stride p)
{
     fftw_free(p);
}

#endif
