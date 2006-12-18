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

static int okp(const kr2c_desc *d,
	       const R *R0, const R *R1, const R *Cr, const R *Ci,
	       INT rs, INT csr, INT csi, INT vl, INT ivs, INT ovs)
{
     UNUSED(R0); UNUSED(R1); UNUSED(Cr); UNUSED(Ci); UNUSED(vl);
     return (1
	     && (!d->rs || (d->rs == rs))
	     && (!d->csr || (d->csr == csr))
	     && (!d->csi || (d->csi == csi))
	     && (!d->ivs || (d->ivs == ivs))
	     && (!d->ovs || (d->ovs == ovs))
	  );
}

#include "r2cf.h"
const kr2c_genus GENUS = { okp, R2HC, 1 };
#undef GENUS

#include "r2cfII.h"
const kr2c_genus GENUS = { okp, R2HCII, 1 };
#undef GENUS

#include "r2cb.h"
const kr2c_genus GENUS = { okp, HC2R, 1 };
#undef GENUS

#include "r2cbIII.h"
const kr2c_genus GENUS = { okp, HC2RIII, 1 };
#undef GENUS
