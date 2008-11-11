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
#include "n.h"

static int okp(const kdft_desc *d,
	       const R *ri, const R *ii, 
	       const R *ro, const R *io,
	       INT is, INT os, INT vl, INT ivs, INT ovs,
	       const planner *plnr)
{
     UNUSED(ri); UNUSED(ii); UNUSED(ro); UNUSED(io); UNUSED(vl); UNUSED(plnr);
     return (1
	     && (!d->is || (d->is == is))
	     && (!d->os || (d->os == os))
	     && (!d->ivs || (d->ivs == ivs))
	     && (!d->ovs || (d->ovs == ovs))
	  );
}

const kdft_genus GENUS = { okp, 1 };
