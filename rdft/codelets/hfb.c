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

#include "codelet-rdft.h"
#include "hf.h"

static int okp(const hc2hc_desc *d,
	       const R *rio, const R *iio, 
	       INT ios, INT vs, INT m, INT dist)
{
     UNUSED(rio); UNUSED(iio); UNUSED(m);
     return (1
	     && (!d->s1 || (d->s1 == ios))
	     && (!d->s2 || (d->s2 == vs))
	     && (!d->dist || (d->dist == dist))
	  );
}

const hc2hc_genus GENUS = { okp, R2HC, 1 };

#undef GENUS
#include "hb.h"

const hc2hc_genus GENUS = { okp, HC2R, 1 };
