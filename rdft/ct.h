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

#include "rdft.h"

typedef void (*hc2hcapply) (const plan *ego, R *IO);
typedef struct ct_solver_s ct_solver;
typedef plan *(*mkinferior)(const ct_solver *ego,
			    rdft_kind kind, int r, int m, int s, 
			    int vl, int vs, 
			    R *IO, planner *plnr);

typedef struct {
     plan super;
     hc2hcapply apply;
} plan_hc2hc;

extern plan *X(mkplan_hc2hc)(size_t size, const plan_adt *adt, 
			     hc2hcapply apply);

#define MKPLAN_HC2HC(type, adt, apply) \
  (type *)X(mkplan_hc2hc)(sizeof(type), adt, apply)

struct ct_solver_s {
     solver super;
     int r;

     mkinferior mkcldw;
};

ct_solver *X(mksolver_rdft_ct)(size_t size, int r, mkinferior mkcldw);

solver *X(mksolver_rdft_hc2hc_direct)(khc2hc codelet, const hc2hc_desc *desc);

int X(rdft_ct_mkcldrn)(rdft_kind kind, int r, int m, int s, 
		       R *IO, planner *plnr,
		       plan **cld0p, plan **cldmp);
