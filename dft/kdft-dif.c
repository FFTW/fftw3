/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* $Id: kdft-dif.c,v 1.7 2003-05-15 23:09:07 athena Exp $ */

#include "dft.h"

void (*X(kdft_dif_register_hook))(planner *, kdftw, const ct_desc *) = 0;

void X(kdft_dif_register)(planner *p, kdftw codelet, const ct_desc *desc)
{
     REGISTER_SOLVER(p, X(mksolver_dft_directw)(codelet, desc, DECDIF));
     REGISTER_SOLVER(p, X(mksolver_dft_directwbuf)(codelet, desc, DECDIF));
     if (X(kdft_dif_register_hook))
	  X(kdft_dif_register_hook)(p, codelet, desc);
}
