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

/* $Id: vrank3-transpose.c,v 1.1 2002-06-08 20:40:58 athena Exp $ */

/* rank-0, vector-rank-2, square transposition  */

#include "dft.h"

/* transposition routine. TODO: optimize? */
static void t(R *rA, R *iA, uint n, int is, int js, uint vn, int vs)
{
     uint i, j, iv;

     for (i = 1; i < n; ++i) {
	  for (j = 0; j < i; ++j) {
	       for (iv = 0; iv < vn; ++iv) {
		    R ar, ai, br, bi;

		    ar = rA[i * is + j * js + iv * vs];
		    ai = iA[i * is + j * js + iv * vs];
		    br = rA[j * is + i * js + iv * vs];
		    bi = iA[j * is + i * js + iv * vs];

		    rA[j * is + i * js + iv * vs] = ar;
		    iA[j * is + i * js + iv * vs] = ai;
		    rA[i * is + j * js + iv * vs] = br;
		    iA[i * is + j * js + iv * vs] = bi;
	       }
	  }
     }
}

typedef solver S;

typedef struct {
     plan_dft super;
     uint n, vl;
     int s0, s1, vs;
} P;

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     UNUSED(ro);
     UNUSED(io);
     t(ri, ii, ego->n, ego->s0, ego->s1, ego->vl, ego->vs);
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

static int applicable(const problem *p_, uint *dim0, uint *dim1, uint *dim2)
{
     if (DFTP(p_)) {
	  const problem_dft *p = (const problem_dft *)p_;
	  return (1
		  && p->ri == p->ro 
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

static int score(const solver *ego, const problem *p_)
{
     uint dim0, dim1, dim2;
     const problem_dft *p;
     UNUSED(ego);

     if (!applicable(p_, &dim0, &dim1, &dim2)) 
	  return BAD;

     p = (const problem_dft *) p_;
     if (p->vecsz.dims[dim2].is > fftw_imax(p->vecsz.dims[dim0].is, 
					    p->vecsz.dims[dim0].os))
	  return UGLY;		/* loops are in the wrong order for locality */

     return GOOD;
}

static void destroy(plan *ego)
{
     fftw_free(ego);
}

static void print(plan *ego_, plan_printf prntf)
{
     P *ego = (P *) ego_;
     prntf("(DFT-TRANSPOSE-%u-x%u)", ego->n, ego->vl);
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p;
     P *pln;
     uint dim0, dim1, dim2;

     static const plan_adt padt = { 
	  fftw_dft_solve, fftw_null_awake, print, destroy 
     };

     UNUSED(plnr); UNUSED(ego);

     if (!applicable(p_, &dim0, &dim1, &dim2)) 
	  return (plan *) 0;
     p = (const problem_dft *) p_;

     pln = MKPLAN_DFT(P, &padt, apply);
     pln->n = p->vecsz.dims[dim0].n;
     pln->s0 = p->vecsz.dims[dim0].is;
     pln->s1 = p->vecsz.dims[dim0].os;
     pln->vl = p->vecsz.dims[dim2].n;
     pln->vs = p->vecsz.dims[dim2].is; /* == os */

     pln->super.super.cost = 1.0;	/* FIXME? */
     pln->super.super.flops = fftw_flops_zero;
     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     return MKSOLVER(S, &sadt);
}

void fftw_dft_vrank3_transpose_register(planner *p)
{
     p->adt->register_solver(p, mksolver());
}
