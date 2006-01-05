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
#include "r2hc.h"

static int okp(const kr2hc_desc *d,
	       const R *I,
	       const R *ro, const R *io,
	       INT is, INT ros, INT ios, INT vl, INT ivs, INT ovs)
{
     UNUSED(I); UNUSED(ro); UNUSED(io); UNUSED(vl);
     return (1
	     && (!d->is || (d->is == is))
	     && (!d->ros || (d->ros == ros))
	     && (!d->ios || (d->ios == ios))
	     && (!d->ivs || (d->ivs == ivs))
	     && (!d->ovs || (d->ovs == ovs))
	  );
}

const kr2hc_genus GENUS = { okp, R2HC, 1 };

#undef GENUS
#include "r2hcII.h"

const kr2hcII_genus GENUS = { okp, R2HCII, 1 };
