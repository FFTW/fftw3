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

/* $Id: rdft2-tensor-max-index.c,v 1.2 2003-01-09 08:19:35 stevenj Exp $ */

#include "rdft.h"

/* like X(tensor_max_index), but takes into account the special n/2+1
   final dimension for the complex output/input of an R2HC/HC2R transform. */
uint X(rdft2_tensor_max_index)(const tensor *sz, rdft_kind k)
{
     uint i;
     uint n = 0;

     A(FINITE_RNK(sz->rnk));
     for (i = 0; i + 1 < sz->rnk; ++i) {
          const iodim *p = sz->dims + i;
          n += (p->n - 1) * X(uimax)(X(iabs)(p->is), X(iabs)(p->os));
     }
     if (i < sz->rnk) {
	  const iodim *p = sz->dims + i;
	  if (k == R2HC)
	       n += X(uimax)((p->n - 1) * X(iabs)(p->is),
			     (p->n/2) * X(iabs)(p->os));
	  else /* HC2R */
	       n += X(uimax)((p->n - 1) * X(iabs)(p->os),
			     (p->n/2) * X(iabs)(p->is));
     }
     return n;
}
