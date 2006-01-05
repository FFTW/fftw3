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

X(plan) X(plan_many_dft_r2c)(int rank, const int *n,
			     int howmany,
			     R *in, const int *inembed,
			     int istride, int idist,
			     C *out, const int *onembed,
			     int ostride, int odist, unsigned flags)
{
     R *ro, *io;
     int *nfi, *nfo;
     int inplace;
     X(plan) p;

     if (!X(many_kosherp)(rank, n, howmany)) return 0;

     X(extract_reim)(FFT_SIGN, out, &ro, &io);
     inplace = in == ro;

     p = X(mkapiplan)(
	  0, flags, 
	  X(mkproblem_rdft2_d)(
	       X(mktensor_rowmajor)(
		    rank, n,
		    X(rdft2_pad)(rank, n, inembed, inplace, 0, &nfi),
		    X(rdft2_pad)(rank, n, onembed, inplace, 1, &nfo),
		    istride, 2 * ostride), 
	       X(mktensor_1d)(howmany, idist, 2 * odist),
	       TAINT_UNALIGNED(in, flags),
	       TAINT_UNALIGNED(ro, flags), TAINT_UNALIGNED(io, flags),
	       R2HC));

     X(ifree0)(nfi);
     X(ifree0)(nfo);
     return p;
}
