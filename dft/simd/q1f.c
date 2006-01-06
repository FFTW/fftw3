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
#include "q1f.h"

#if HAVE_SIMD
static int okp(const ct_desc *d,
	       const R *rio, const R *iio, 
	       INT ios, INT vs, INT m, INT mb, INT me, INT dist,
	       const planner *plnr)
{
     return (RIGHT_CPU()
             && ALIGNED(rio)
	     && !NO_SIMDP(plnr)
	     && SIMD_STRIDE_OK(ios)
	     && SIMD_STRIDE_OK(vs)
	     && SIMD_VSTRIDE_OK(dist)
	     && iio == rio + 1
             && (m % VL) == 0
             && (mb % VL) == 0
             && (me % VL) == 0
	     && (!d->s1 || (d->s1 == ios))
	     && (!d->s2 || (d->s2 == vs))
	     && (!d->dist || (d->dist == dist))
	  );
}
const ct_genus GENUS = { okp, VL };
#endif
