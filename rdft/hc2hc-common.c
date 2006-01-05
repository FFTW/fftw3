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

/* common hc2hc routines */

#include "hc2hc.h"

/* A note on mstart and mcount for real solvers:

   For the complex solvers, we have a loop from 0 to m-1 of twiddle
   codelets, and we specify a subset from mstart to mstart+mcount-1.

   For the real solvers, it is more complicated since we typically perform
   the "logical" loop from 0 to m-1 as:

       i) a real nontwiddle (DC-component) transform (0)
       ii) (m-1)/2 *pairs* of twiddle transforms
       iii) if m is even, the Nyquist-component transform (m/2)

   Thus, there are (m+2)/2 transforms to perform, counting a pair
   in (ii) as one unit.  These transforms are indexed from 0 to
   (m+2)/2-1, with the 0th being (i) and the last (if m is even)
   being (iii).  We specify a subset of these indices from
   mstart to mstart+mcount-1.

   Therefore, for a given (mstart,mcount), the number of (ii) iterations
   to perform is therefore:
       #ii = mcount - (mstart==0) - (m%2 == 0 && mstart+mcount == (m+2)/2)
   The "m" parameter passed to the twiddle codelet for (ii), however,
   is for historical reasons 1 + 2 * #ii.  Sigh.
*/

/* generic routine that produces cld0 and cldm, used by inferior
   solvers */
int X(hc2hc_mkcldrn)(rdft_kind kind, INT r, INT m, INT s,
		       INT mstart, INT mcount,
		       R *IO, planner *plnr,
		       plan **cld0p, plan **cldmp)
{
     tensor *radix = X(mktensor_1d)(r, m * s, m * s);
     tensor *null = X(mktensor_0d)();
     INT imid = s * (m/2);
     plan *cld0 = 0, *cldm = 0;

     A(R2HC_KINDP(kind) || HC2R_KINDP(kind));
     A(mstart >= 0 && mcount > 0 && mstart + mcount <= (m + 2) / 2);

     cld0 = X(mkplan_d)(plnr, 
			X(mkproblem_rdft_1)(mstart == 0 ? radix : null, 
					    null, IO, IO, kind));
     if (!cld0) goto nada;

     cldm = X(mkplan_d)(plnr,
			X(mkproblem_rdft_1)(
			     (m%2 || mstart+mcount < (m+2)/2) ? null : radix, 
			     null, IO + imid, IO + imid, 
			     R2HC_KINDP(kind) ? R2HCII : HC2RIII));
     if (!cldm) goto nada;

     X(tensor_destroy2)(null, radix);
     *cld0p = cld0;
     *cldmp = cldm;
     return 1;

 nada:
     X(tensor_destroy2)(null, radix);
     X(plan_destroy_internal)(cld0);
     X(plan_destroy_internal)(cldm);
     return 0;
}
