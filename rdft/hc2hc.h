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

typedef struct {
     solver super;
     const char *nam;
     const hc2hc_desc *desc;
     khc2hc k;
} solver_hc2hc;

typedef struct {
     plan_rdft super;
     khc2hc k;
     plan *cld0, *cldm; /* children for 0th and middle butterflies */
     plan *cld;
     R *W;
     int n, r, m, vl;
     int is, os, ivs, ovs, iios;
     stride ios, vs;
     const solver_hc2hc *slv;
     twid *td;
} plan_hc2hc;

/* data type describing a generic Cooley-Tukey solver */
typedef struct
{
     size_t pln_size;
     void (*mkcldrn)(const solver_hc2hc *, const problem_rdft *p,
		     problem **cldp, problem **cld0p, problem **cldmp);
     void (*finish)(plan_hc2hc *ego);
     int (*applicable)(const solver_hc2hc *ego, const problem *p,
		       const planner *plnr);
     rdftapply apply;
} hc2hcadt;

int X(rdft_hc2hc_applicable)(const solver_hc2hc *ego, const problem *p_);

plan *X(mkplan_rdft_hc2hc)(const solver_hc2hc *ego,
			   const problem *p_,
			   planner *plnr,
			   const hc2hcadt *adt);

solver *X(mksolver_rdft_hc2hc)(khc2hc k, const hc2hc_desc *desc,
			       const char *nam, const solver_adt *adt);

void X(rdft_mkcldrn_dit)(const solver_hc2hc *, const problem_rdft *p,
			 problem **cldp, problem **cld0p, problem **cldmp);
void X(rdft_mkcldrn_dif)(const solver_hc2hc *, const problem_rdft *p,
			 problem **cldp, problem **cld0p, problem **cldmp);
