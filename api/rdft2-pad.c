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

#include <string.h>
#include "api.h"

const uint *X(rdft2_pad)(uint rnk, const uint *n, const uint *nembed,
			 int inplace, int cmplx, uint **nfree)
{
     A(FINITE_RNK(rnk));
     *nfree = 0;
     if (!nembed && rnk > 0) {
	  if (inplace || cmplx) {
	       uint *np = (uint *) MALLOC(sizeof(uint) * rnk, PROBLEMS);
	       memcpy(np, n, sizeof(uint) * rnk);
	       np[rnk-1] = (n[rnk-1]/2 + 1) * (1 + !cmplx);
	       nembed = *nfree = np;
	  }
	  else
	       nembed = n;
     }
     return nembed;
}
