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

/* $Id: k7.c,v 1.1 2002-06-30 18:37:55 athena Exp $ */

#include "dft.h"

int X(kdft_k7_mokp)(const kdft_desc *d,
		    const R *ri, const R *ii, const R *ro, const R *io,
		    int is, int os, uint vl, int ivs, int ovs)
{
     return (1
	     && ii == ri + 1 
	     && io == ro + 1
	     && (!d->is || (d->is == is))
	     && (!d->os || (d->is == os))
	  );
}

int X(kdft_k7_pokp)(const kdft_desc *d,
		    const R *ri, const R *ii, const R *ro, const R *io,
		    int is, int os, uint vl, int ivs, int ovs)
{
     return (1
	     && ri == ii + 1 
	     && ro == io + 1
	     && (!d->is || (d->is == is))
	     && (!d->os || (d->is == os))
	  );
}

int X(kdft_ct_k7_mokp)(const ct_desc *d,
		       const R *rio, const R *iio, 
		       int ios, int vs, uint m, int dist)
{
     return (1
	     && iio == rio + 1
	     && (!d->s1 || (d->s1 == ios))
	     && (!d->s2 || (d->s2 == vs))
	  );
}

int X(kdft_ct_k7_pokp)(const ct_desc *d,
		       const R *rio, const R *iio, 
		       int ios, int vs, uint m, int dist)
{
     return (1
	     && rio == iio + 1
	     && (!d->s1 || (d->s1 == ios))
	     && (!d->s2 || (d->s2 == vs))
	  );
}
