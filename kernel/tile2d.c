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

/* out of place 2D copy routines */
#include "ifftw.h"

/* partition [0..N0] x [0..N1] into TILESZ x TILESZ blocks,
   call F on each block */
void X(tile2d)(int n0l, int n0u, int n1l, int n1u, int tilesz,
	       void (*f)(int n0l, int n0u, int n1l, int n1u, void *args),
	       void *args)
{
     int i0, i1;

     for (i1 = n1l; i1 < n1u - tilesz; i1 += tilesz) {
	  for (i0 = n0l; i0 < n0u - tilesz; i0 += tilesz) 
	       f(i0, i0 + tilesz, i1, i1 + tilesz, args);
	  f(i0, n0u, i1, i1 + tilesz, args);
     }

     for (i0 = n0l; i0 < n0u - tilesz; i0 += tilesz) 
	  f(i0, i0 + tilesz, i1, n1u, args);
     f(i0, n0u, i1, n1u, args);
}
