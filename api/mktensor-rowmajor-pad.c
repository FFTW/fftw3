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

#include <string.h>
#include "api.h"

static uint *copy_n(uint rnk, const uint *n)
{
     uint *nc;
     A(FINITE_RNK(rnk));
     nc = (uint *) fftw_malloc(sizeof(uint) * rnk, PROBLEMS);
     memcpy(nc, n, sizeof(uint) * rnk);
     return nc;
}

/* Allocate a "padded" rowmajor array tensor for R2HC rdft2 problems.
   Here, n is the logical size of the real array, and
   (niphys,is)/(nophys,os) is the physical (size,stride) of the
   real/complex array.   The main complication is that we want
   the user to be able simply pass n{i,o}phys == n and have it do
   some approximation of the right thing.  To wit:

   - if inplace and niphys == n, then the actual size of the last
     real-array dimension must be padded to 2*(n/2+1) to leave space for
     the complex output.
     
   - if nophys == n, then the last dimension of nophys is cut in half + 1
     to convert to the halfcomplex output size.

   Ugh.
*/
tensor *X(mktensor_rowmajor_pad)(uint rnk, const uint *n,
				 const uint *niphys, const uint *nophys,
				 int is, int os,
				 int inplace)
{
     uint *niphys_pad = 0;
     uint *nophys_pad = 0;
     tensor *x;

     if (FINITE_RNK(rnk) && rnk > 0) {
	  if (!niphys) {
	       if (inplace) {
		    niphys_pad = copy_n(rnk, n);
		    niphys_pad[rnk-1] = 2 * (n[rnk-1] / 2 + 1);
		    niphys = niphys_pad;
	       }
	       else
		    niphys = n;
	  }
	  if (!nophys) {
	       nophys_pad = copy_n(rnk, n);
	       nophys_pad[rnk-1] = n[rnk-1] / 2 + 1;
	       nophys = nophys_pad;
	  }
     }
     
     x = X(mktensor_rowmajor)(rnk, n, niphys, nophys, is, os);

     X(free0)(niphys_pad);
     X(free0)(nophys_pad);

     return x;
}
