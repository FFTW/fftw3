/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
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
#include "n2b.h"

#if HAVE_SIMD
static int okp(const kdft_desc *d,
               const R *ri, const R *ii, const R *ro, const R *io,
               INT is, INT os, INT vl, INT ivs, INT ovs, 
	       const planner *plnr)
{
     return (RIGHT_CPU()
             && ALIGNEDA(ii)
             && ALIGNEDA(io)
	     && !NO_SIMDP(plnr)
	     && SIMD_STRIDE_OKA(is)
	     && SIMD_VSTRIDE_OKA(ivs)
	     && SIMD_VSTRIDE_OKA(os) /* os == 2 enforced by codelet */
	     && SIMD_STRIDE_OKPAIR(ovs)
             && ri == ii + 1
             && ro == io + 1
             && (vl % VL) == 0
             && (!d->is || (d->is == is))
             && (!d->os || (d->os == os))
             && (!d->ivs || (d->ivs == ivs))
             && (!d->ovs || (d->ovs == ovs))
          );
}

const kdft_genus GENUS = { okp, VL };
#endif
