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

/* $Id: kr2hcII.c,v 1.2 2002-07-21 02:09:15 stevenj Exp $ */

#include "rdft.h"

void X(kr2hcII_register)(planner *p, kr2hcII codelet, const kr2hcII_desc *desc)
{
     UNUSED(p); UNUSED(codelet); UNUSED(desc);
/* TODO
     solver *s = X(mksolver_rdftII_direct)(codelet, desc);
     REGISTER_SOLVER(p, s);
*/
}
