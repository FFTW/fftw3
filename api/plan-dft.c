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
#include "dft.h"

X(plan) X(plan_dft)(unsigned int rank, const unsigned int *n,
		    C *in, const unsigned int *inembed,
		    C *out, const unsigned int *onembed,
		    int sign, unsigned int flags)
{
     R *ri, *ii, *ro, *io;

     X(extract_reim)(sign, in, &ri, &ii);
     X(extract_reim)(sign, out, &ro, &io);
     
     return X(mkapiplan)(
	  flags,
	  X(mkproblem_dft_d)(X(mktensor_rowmajor)(rank, n, inembed, onembed,
						  2, 2),
			     X(mktensor_0d)(), 
			     ri, ii, ro, io));
}
