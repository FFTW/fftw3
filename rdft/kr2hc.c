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

/* $Id: kr2hc.c,v 1.6 2006-01-05 03:04:27 stevenj Exp $ */

#include "rdft.h"

void X(kr2hc_register)(planner *p, kr2hc codelet, const kr2hc_desc *desc)
{
     REGISTER_SOLVER(p, X(mksolver_rdft_r2hc_direct)(codelet, desc));
     REGISTER_SOLVER(p, X(mksolver_rdft2_r2hc_direct)(codelet, desc));
}
