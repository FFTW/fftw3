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

X(plan) X(plan_many_dft_r2c)(unsigned int rank, const unsigned int *n,
			     unsigned int howmany,
			     R *in, const unsigned int *inembed,
			     int istride, int idist,
			     C *out, const unsigned int *onembed,
			     int ostride, int odist,
			     unsigned int flags)
{
     R *ro, *io;

     X(extract_reim)(FFT_SIGN, out, &ro, &io);
     
     return X(mkapiplan)(
	  flags,
	  X(mkproblem_rdft2_d)(X(mktensor_rowmajor_pad)(rank,n,inembed,onembed,
							istride, 2*ostride,
							in == ro || in == io),
			       X(mktensor_1d)(howmany, idist, 2*odist), 
			       in, ro, io, R2HC));
}
