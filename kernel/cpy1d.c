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

/* out of place 1D copy routine */
#include "ifftw.h"

void X(cpy1d)(R *I, R *O, int n0, int is0, int os0, int vl)
{
     int i0, v;

     A(I != O);
     switch (vl) {
	 case 1:
	      for (; n0 > 0; --n0, I += is0, O += os0)
		   *O = *I;
	      break;
	 case 2:
	      for (; n0 > 0; --n0, I += is0, O += os0) {
		   R x0 = I[0];
		   R x1 = I[1];
		   O[0] = x0;
		   O[1] = x1;
	      }
	      break;
	 default:
	      for (i0 = 0; i0 < n0; ++i0)
		   for (v = 0; v < vl; ++v) {
			R x0 = I[i0 * is0 + v];
			O[i0 * os0 + v] = x0;
		   }
	      break;
     }
}
