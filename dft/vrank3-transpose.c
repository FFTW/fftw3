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

/* $Id: vrank3-transpose.c,v 1.24 2003-03-29 07:58:39 stevenj Exp $ */

/* rank-0, vector-rank-3, square transposition  */

#include "dft.h"

/* transposition routine. TODO: optimize? */
static void t(R *rA, R *iA, int n, int is, int js, int vn, int vs)
{
     int i, j, iv;
     int im = iA - rA;

     for (i = 1; i < n; ++i) {
          for (j = 0; j < i; ++j) {
	       R *p0 = rA + i * is + j * js;
	       R *p1 = rA + j * is + i * js;
               for (iv = 0; iv < vn; ++iv) {
                    R ar = p0[0], ai = p0[im];
                    R br = p1[0], bi = p1[im];
                    p1[0] = ar; p1[im] = ai; p1 += vs;
                    p0[0] = br; p0[im] = bi; p0 += vs;
               }
          }
     }
}

typedef solver S;

typedef struct {
     plan_dft super;
     int n, vl;
     int s0, s1, vs;
     int m;
     int nd, md, d; /* d = gcd(n,m), nd = n / d, md = m / d */
} P;

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     UNUSED(ro);
     UNUSED(io);
     A(ego->n == ego->m);
     t(ri, ii, ego->n, ego->s0, ego->s1, ego->vl, ego->vs);
}

static void apply_general(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int nd = ego->nd, md = ego->md, d = ego->d, vl = ego->vl;
     R *buf = (R *)MALLOC((sizeof(R) * 2) * vl * nd * md * d, BUFFERS);

     UNUSED(ii); UNUSED(ro); UNUSED(io);
     X(transpose)(ri, nd, md, d, 2*vl, buf);
     X(ifree)(buf);
}

static void apply_slow(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int n = ego->n, m = ego->m, vl = ego->vl;
     R *buf = (R *)MALLOC((sizeof(R) * 4) * vl, BUFFERS);
     int move_size = (n + m) / 2;
     char *move;

     UNUSED(ii); UNUSED(ro); UNUSED(io);
     STACK_MALLOC(char *, move, move_size);
     X(transpose_slow)(ri, n, m, 2*vl, move, move_size, buf);
     STACK_FREE(move);
     X(ifree)(buf);
}

static int pickdim(const tensor *s, int *pdim0, int *pdim1, int *pdim2)
{
     int dim0, dim1;

     for (dim0 = 0; dim0 < s->rnk; ++dim0)
          for (dim1 = dim0 + 1; dim1 < s->rnk; ++dim1) {
	       int dim2 = 3 - dim0 - dim1;
               if (s->dims[dim2].is == s->dims[dim2].os
		   && X(transposable)(s->dims + dim0, s->dims + dim1, 
				      s->dims[dim2].n, 2, s->dims[dim2].is)) {
                    *pdim0 = dim0;
                    *pdim1 = dim1;
		    *pdim2 = dim2;
                    return 1;
               }
	  }
     return 0;
}

static int applicable0(const problem *p_, int *dim0, int *dim1, int *dim2)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *)p_;
          return (1
                  && p->ri == p->ro
                  && p->sz->rnk == 0
                  && p->vecsz->rnk == 3
                  && pickdim(p->vecsz, dim0, dim1, dim2)
	       );
     }
     return 0;
}

static int applicable(const problem *p_, const planner *plnr, 
		      int *dim0, int *dim1, int *dim2)
{
     const problem_dft *p;

     if (!applicable0(p_, dim0, dim1, dim2))
          return 0;

     p = (const problem_dft *) p_;

     if (NO_UGLYP(plnr))
	  if (p->vecsz->dims[*dim2].is > X(imax)(p->vecsz->dims[*dim0].is,
						p->vecsz->dims[*dim0].os))
	       /* loops are in the wrong order for locality */
	       return 0;	

     return 1;
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dft-transpose-%dX%d%v)", ego->n, ego->m, ego->vl);
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p;
     P *pln;
     const iodim *d;
     int dim0, dim1, dim2;
     int vl;

     static const plan_adt padt = {
	  X(dft_solve), X(null_awake), print, X(plan_null_destroy)
     };

     UNUSED(plnr);
     UNUSED(ego);

     if (!applicable(p_, plnr, &dim0, &dim1, &dim2))
          return (plan *) 0;
     p = (const problem_dft *) p_;

     d = p->vecsz->dims;
     vl = d[dim2].n;
     pln = MKPLAN_DFT(P, &padt,
                      X(transpose_simplep)(d+dim0, d+dim1, 2*vl) ? apply :
                      (X(transpose_slowp)(d+dim0, d+dim1, 2*vl) ? apply_slow
                       : apply_general));
     X(transpose_dims)(d+dim0, d+dim1, 
		       &pln->n, &pln->m, &pln->d, &pln->nd, &pln->md);
     pln->s0 = d[dim0].is;
     pln->s1 = d[dim0].os;
     pln->vl = vl;
     pln->vs = d[dim2].is; /* == os */

     /* pln->vl * (4 loads + 4 stores) * (pln->n \choose 2) 
        (FIXME? underestimate for non-square) */
     X(ops_other)(4 * pln->vl * pln->n * (pln->m - 1), &pln->super.super.ops);

     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     return MKSOLVER(S, &sadt);
}

void X(dft_vrank3_transpose_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
