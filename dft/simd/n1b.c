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

#include "codelet.h"
#include "n1b.h"

#if HAVE_SIMD
static int okp(const kdft_desc *d,
               const R *ri, const R *ii, const R *ro, const R *io,
               int is, int os, uint vl, int ivs, int ovs)
{
     return (RIGHT_CPU()
             && ALIGNED(ii)
             && ALIGNED(io)
	     && VEC_OKSTRIDE(is)
	     && VEC_OKSTRIDE(os)
	     && VEC_OKSTRIDE(ivs)
	     && VEC_OKSTRIDE(ovs)
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
