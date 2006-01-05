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

/* $Id: rdft2-inplace-strides.c,v 1.7 2006-01-05 03:04:27 stevenj Exp $ */

#include "rdft.h"

/* Check if the vecsz/sz strides are consistent with the problem
   being in-place for vecsz.dim[vdim], or for all dimensions
   if vdim == RNK_MINFTY.  We can't just use tensor_inplace_strides
   because rdft transforms have the unfortunate property of
   differing input and output sizes.   This routine is not
   exhaustive; we only return 1 for the most common case.  */
int X(rdft2_inplace_strides)(const problem_rdft2 *p, int vdim)
{
     INT N, Nc;
     INT is, os;
     int i;
     
     for (i = 0; i + 1 < p->sz->rnk; ++i)
	  if (p->sz->dims[i].is != p->sz->dims[i].os)
	       return 0;

     if (!FINITE_RNK(p->vecsz->rnk) || p->vecsz->rnk == 0)
	  return 1;
     if (!FINITE_RNK(vdim)) { /* check all vector dimensions */
	  for (vdim = 0; vdim < p->vecsz->rnk; ++vdim)
	       if (!X(rdft2_inplace_strides)(p, vdim))
		    return 0;
	  return 1;
     }

     A(vdim < p->vecsz->rnk);
     if (p->sz->rnk == 0)
	  return(p->vecsz->dims[vdim].is == p->vecsz->dims[vdim].os);

     N = X(tensor_sz)(p->sz);
     Nc = (N / p->sz->dims[p->sz->rnk-1].n) *
	  (p->sz->dims[p->sz->rnk-1].n/2 + 1);
     X(rdft2_strides)(p->kind, p->sz->dims + p->sz->rnk - 1, &is, &os);
     return(p->vecsz->dims[vdim].is == p->vecsz->dims[vdim].os
	    && X(iabs)(p->vecsz->dims[vdim].os)
	    >= X(imax)(Nc * X(iabs)(os), N * X(iabs)(is)));
}
