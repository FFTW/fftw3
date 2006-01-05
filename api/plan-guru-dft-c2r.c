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

#include "api.h"
#include "rdft.h"

X(plan) X(plan_guru_dft_c2r)(int rank, const X(iodim) *dims,
			     int howmany_rank, const X(iodim) *howmany_dims,
			     C *in, R *out, unsigned flags)
{
     R *ri, *ii;

     if (!X(guru_kosherp)(rank, dims, howmany_rank, howmany_dims)) return 0;

     X(extract_reim)(FFT_SIGN, in, &ri, &ii);

     if (out != ri)
	  flags |= FFTW_DESTROY_INPUT;
     return X(mkapiplan)(
	  0, flags, 
	  X(mkproblem_rdft2_d)(X(mktensor_iodims)(rank, dims, 2, 1),
			       X(mktensor_iodims)(howmany_rank, howmany_dims,
						  2, 1),
			       TAINT_UNALIGNED(out, flags),
			       TAINT_UNALIGNED(ri, flags),
			       TAINT_UNALIGNED(ii, flags), HC2R));
}
