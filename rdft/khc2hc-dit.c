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

/* $Id: khc2hc-dit.c,v 1.5 2002-08-29 05:44:33 stevenj Exp $ */

#include "rdft.h"

void (*X(khc2hc_dit_register_hook))(planner *, khc2hc, const hc2hc_desc *)=0;

void X(khc2hc_dit_register)(planner *p, khc2hc codelet, const hc2hc_desc *desc)
{
     REGISTER_SOLVER(p, X(mksolver_rdft_hc2hc_dit)(codelet, desc));
     REGISTER_SOLVER(p, X(mksolver_rdft_hc2hc_ditbuf)(codelet, desc));
     if (X(khc2hc_dit_register_hook))
	  X(khc2hc_dit_register_hook)(p, codelet, desc);
}
