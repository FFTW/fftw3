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

/* $Id: vrank3-transpose.c,v 1.3 2002-09-18 21:16:17 athena Exp $ */

/* rank-0, vector-rank-3, square transposition  */

#include "rdft.h"

/* transposition routine. TODO: optimize? */
static void t(R *rA, uint n, int is, int js, uint vn, int vs)
{
     uint i, j, iv;

     for (i = 1; i < n; ++i) {
          for (j = 0; j < i; ++j) {
	       R *rp0 = rA + i * is + j * js;
	       R *rp1 = rA + j * is + i * js;
               for (iv = 0; iv < vn; ++iv) {
                    R ar, br;

                    ar = *rp0;
                    br = *rp1;
                    *rp1 = ar; rp1 += vs;
                    *rp0 = br; rp0 += vs;
               }
          }
     }
}

typedef solver S;

typedef struct {
     plan_rdft super;
     uint n, vl;
     int s0, s1, vs;
} P;

static void apply(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     UNUSED(O);
     t(I, ego->n, ego->s0, ego->s1, ego->vl, ego->vs);
}

static int pickdim(tensor s, uint *pdim0, uint *pdim1)
{
     uint dim0, dim1;

     for (dim0 = 0; dim0 < s.rnk; ++dim0)
          for (dim1 = dim0 + 1; dim1 < s.rnk; ++dim1)
               if (s.dims[dim0].n == s.dims[dim1].n &&
		   s.dims[dim0].is == s.dims[dim1].os &&
		   s.dims[dim0].os == s.dims[dim1].is) {
                    *pdim0 = dim0;
                    *pdim1 = dim1;
                    return 1;
               }
     return 0;
}

/* For dim0 != dim1 in {0,1,2}, return the third element. */
static int other_dim(uint *dim0, uint *dim1, uint *dim2)
{
     *dim2 = 3 - (*dim0 + *dim1);
     return 1;
}

static int applicable0(const problem *p_, uint *dim0, uint *dim1, uint *dim2)
{
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *)p_;
          return (1
                  && p->I == p->O
                  && p->sz.rnk == 0
                  && p->vecsz.rnk == 3
                  && pickdim(p->vecsz, dim0, dim1)
                  && other_dim(dim0, dim1, dim2)

                  /* non-transpose dimension must be in-place */
                  && p->vecsz.dims[*dim2].is == p->vecsz.dims[*dim2].os
	       );
     }
     return 0;
}

static int applicable(const problem *p_, 
		      const planner *plnr, uint *dim0, uint *dim1, uint *dim2)
{
     if (!applicable0(p_, dim0, dim1, dim2)) return 0;

     if (NO_UGLYP(plnr)) {
	  const problem_rdft *p = (const problem_rdft *) p_;
	  if (p->vecsz.dims[*dim2].is > X(imax)(p->vecsz.dims[*dim0].is,
						p->vecsz.dims[*dim0].os))
	       return 0; /* loops are in the wrong order for locality */
     }

     return 1;
}

static void destroy(plan *ego)
{
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(rdft-transpose-%u%v)", ego->n, ego->vl);
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_rdft *p;
     P *pln;
     uint dim0, dim1, dim2;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, destroy
     };

     UNUSED(ego);
     if (!applicable(p_, plnr, &dim0, &dim1, &dim2))
          return (plan *) 0;
     p = (const problem_rdft *) p_;

     pln = MKPLAN_RDFT(P, &padt, apply);
     pln->n = p->vecsz.dims[dim0].n;
     pln->s0 = p->vecsz.dims[dim0].is;
     pln->s1 = p->vecsz.dims[dim0].os;
     pln->vl = p->vecsz.dims[dim2].n;
     pln->vs = p->vecsz.dims[dim2].is; /* == os */

     /* pln->vl * (2 loads + 2 stores) * (pln->n \choose 2) */
     pln->super.super.ops = X(ops_other)(2 * pln->vl * pln->n * (pln->n - 1));

     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     return MKSOLVER(S, &sadt);
}

void X(rdft_vrank3_transpose_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
