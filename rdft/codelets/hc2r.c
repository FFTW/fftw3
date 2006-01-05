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
#include "hc2r.h"

static int okp(const khc2r_desc *d,
	       const R *ri, const R *ii,
	       const R *O,
	       INT ris, INT iis, INT os, INT vl, INT ivs, INT ovs)
{
     UNUSED(ri); UNUSED(ii); UNUSED(O); UNUSED(vl);
     return (1
	     && (!d->ris || (d->ris == ris))
	     && (!d->iis || (d->iis == iis))
	     && (!d->os || (d->os == os))
	     && (!d->ivs || (d->ivs == ivs))
	     && (!d->ovs || (d->ovs == ovs))
	  );
}

const khc2r_genus GENUS = { okp, HC2R, 1 };

#undef GENUS
#include "hc2rIII.h"

const khc2rIII_genus GENUS = { okp, HC2RIII, 1 };
