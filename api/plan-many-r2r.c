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

#include "api.h"
#include "rdft.h"

rdft_kind *X(map_r2r_kind)(uint rank, const X(r2r_kind) *kind);

#define N0(nembed) ((nembed) ? (nembed) : n)

X(plan) X(plan_many_r2r)(unsigned int rank, const unsigned int *n,
			 unsigned int howmany,
			 R *in, const unsigned int *inembed,
			 int istride, int idist,
			 R *out, const unsigned int *onembed,
			 int ostride, int odist,
			 const X(r2r_kind) *kind, unsigned int flags)
{
     X(plan) p;
     rdft_kind *k = X(map_r2r_kind)(rank, kind);
     
     p = X(mkapiplan)(
	  flags,
	  X(mkproblem_rdft_d)(X(mktensor_rowmajor)(rank, n, 
						   N0(inembed), N0(onembed),
						   istride, ostride),
			      X(mktensor_1d)(howmany, idist, odist), 
			      in, out, k));
     X(free0)(k);
     return p;
}
