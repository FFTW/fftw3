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

#include "api.h"

X(plan) X(plan_dft_c2r)(unsigned int rank, const unsigned int *n,
			C *in, const unsigned int *inembed,
			R *out, const unsigned int *onembed,
			unsigned int flags)
{
     return X(plan_many_dft_c2r)(rank, n, 1,
				 in, inembed, 1, 1,
				 out, onembed, 1, 1,
				 flags);
}
