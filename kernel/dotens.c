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

/* $Id: dotens.c,v 1.1 2002-07-05 18:49:59 athena Exp $ */

#include "ifftw.h"

static void recur(uint rnk, const iodim *dims, dotens_closure *k, 
		  int indx, int ondx)
{
     if (rnk == 0)
          k->apply(k, indx, ondx);
     else {
          uint i, n = dims[0].n;
          int is = dims[0].is;
          int os = dims[0].os;

          for (i = 0; i < n; ++i) {
               recur(rnk - 1, dims + 1, k, indx, ondx);
	       indx += is; ondx += os;
	  }
     }
}

void X(dotens)(tensor sz, dotens_closure *k)
{
     if (sz.rnk == RNK_MINFTY)
          return;
     recur(sz.rnk, sz.dims, k, 0, 0);
}
