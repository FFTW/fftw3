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

/* this way, we can re-use the ugly padding logic from the r2c transform */
static tensor *swap_strides(tensor *x)
{
     if (FINITE_RNK(x->rnk)) {
	  uint i;
	  for (i = 0; i < x->rnk; ++i) {
	       int s = x->dims[i].is;
	       x->dims[i].is = x->dims[i].os;
	       x->dims[i].os = s;
	  }
     }
     return x;
}

X(plan) X(plan_many_dft_c2r)(unsigned int rank, const unsigned int *n,
			     unsigned int howmany,
			     C *in, const unsigned int *inembed,
			     int istride, int idist,
			     R *out, const unsigned int *onembed,
			     int ostride, int odist,
			     int sign, unsigned int flags)
{
     R *ri, *ii;

     X(extract_reim)(sign, in, &ri, &ii);
     
     return X(mkapiplan)(
	  flags,
	  X(mkproblem_rdft2_d)(
	       swap_strides(X(mktensor_rowmajor_pad)(rank,n,onembed,inembed,
						     ostride, 2*istride,
						     out == ri || out == ii)),
	       howmany == 1 ? X(mktensor_0d)() 
	       : X(mktensor_1d)(howmany, 2*idist, odist), 
	       out, ri, ii, HC2R));
}
