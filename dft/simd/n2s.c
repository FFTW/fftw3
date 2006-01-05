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

#include "codelet-dft.h"

#if HAVE_SIMD
#include "n2s.h"

static int okp(const kdft_desc *d,
               const R *ri, const R *ii, const R *ro, const R *io,
               INT is, INT os, INT vl, INT ivs, INT ovs, 
	       const planner *plnr)
{
     return (RIGHT_CPU()
	     && !NO_SIMDP(plnr)
             && ALIGNEDA(ri)
             && ALIGNEDA(ii)
             && ALIGNEDA(ro)
             && ALIGNEDA(io)
	     && SIMD_STRIDE_OKA(is)
	     && ivs == 1
	     && os == 1
	     && SIMD_STRIDE_OKA(ovs)
             && (vl % (2 * VL)) == 0
             && (!d->is || (d->is == is))
             && (!d->os || (d->os == os))
             && (!d->ivs || (d->ivs == ivs))
             && (!d->ovs || (d->ovs == ovs))
          );
}

const kdft_genus GENUS = { okp, 2 * VL };
#endif
