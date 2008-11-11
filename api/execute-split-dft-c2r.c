/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
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

#include "api.h"
#include "rdft.h"

/* guru interface: requires care in alignment, r - i, etcetera. */
void X(execute_split_dft_c2r)(const X(plan) p, R *ri, R *ii, R *out)
WITH_ALIGNED_STACK({
     plan_rdft2 *pln = (plan_rdft2 *) p->pln;
     problem_rdft2 *prb = (problem_rdft2 *) p->prb;
     pln->apply((plan *) pln, out, out + (prb->r1 - prb->r0), ri, ii);
})
