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

/* common hc2hc routines */

#include "ct.h"

/* generic routine that produces cld0 and cldm, used by inferior
   solvers */
int X(rdft_ct_mkcldrn)(rdft_kind kind, int r, int m, int s, 
		       R *IO, planner *plnr,
		       plan **cld0p, plan **cldmp)
{
     tensor *radix = X(mktensor_1d)(r, m * s, m * s);
     tensor *null = X(mktensor_0d)();
     int imid = s * (m/2);
     plan *cld0 = 0, *cldm = 0;

     A(R2HC_KINDP(kind) || HC2R_KINDP(kind));

     cld0 = X(mkplan_d)(plnr, 
			X(mkproblem_rdft_1)(radix, null, IO, IO, kind));
     if (!cld0) goto nada;

     cldm = X(mkplan_d)(plnr,
			X(mkproblem_rdft_1)(
			     m%2 ? null : radix, null, IO + imid, IO + imid, 
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
