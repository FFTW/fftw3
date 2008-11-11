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

#if HAVE_SIMD
#include "ts.h"

static int okp(const ct_desc *d,
	       const R *rio, const R *iio, 
	       INT rs, INT vs, INT m, INT mb, INT me, INT ms,
	       const planner *plnr)
{
     UNUSED(rio);
     UNUSED(iio);
     return (RIGHT_CPU()
	     && !NO_SIMDP(plnr)
	     && ALIGNEDA(rio)
	     && ALIGNEDA(iio)
	     && SIMD_STRIDE_OKA(rs)
	     && ms == 1
             && (m % (2 * VL)) == 0
             && (mb % (2 * VL)) == 0
             && (me % (2 * VL)) == 0
	     && (!d->rs || (d->rs == rs))
	     && (!d->vs || (d->vs == vs))
	     && (!d->ms || (d->ms == ms))
	  );
}

const ct_genus GENUS = { okp, 2 * VL };

#endif
