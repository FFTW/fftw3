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

/* $Id: zero.c,v 1.2 2002-09-22 13:49:08 athena Exp $ */

#include "dft.h"

/* fill a complex array with zeros. */
static void recur(const iodim *dims, uint rnk, R *ri, R *ii)
{
     if (rnk == RNK_MINFTY)
          return;
     else if (rnk == 0)
          ri[0] = ii[0] = 0.0;
     else if (rnk > 0) {
          uint i, n = dims[0].n;
          int is = dims[0].is;

	  if (rnk == 1) {
	       /* this case is redundant but faster */
	       for (i = 0; i < n; ++i)
		    ri[i * is] = ii[i * is] = 0.0;
	  } else {
	       for (i = 0; i < n; ++i)
		    recur(dims + 1, rnk - 1, ri + i * is, ii + i * is);
	  }
     }
}


void X(dft_zerotens)(tensor *sz, R *ri, R *ii)
{
     recur(sz->dims, sz->rnk, ri, ii);
}
