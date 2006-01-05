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

/* $Id: tensor9.c,v 1.3 2006-01-05 03:04:27 stevenj Exp $ */

#include "ifftw.h"

int X(tensor_kosherp)(const tensor *x)
{
     int i;

     if (x->rnk < 0) return 0;

     if (FINITE_RNK(x->rnk)) {
	  for (i = 0; i < x->rnk; ++i)
	       if (x->dims[i].n < 0)
		    return 0;
     }
     return 1;
}
