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

/* $Id: vrank2-transpose.c,v 1.24 2003-04-17 23:15:53 athena Exp $ */

/* rank-0, vector-rank-2, square transposition  */

#include "dft.h"

/* transposition routine. TODO: optimize? */
static void t(R *rA, R *iA, int n, int is, int js)
{
     int i, j;
     int im = iA - rA;

     for (i = 1; i < n; ++i) {
	  R *p0 = rA + i * is;
	  R *p1 = rA + i * js;
          for (j = 0; j < i; ++j) {
               R ar = p0[0], ai = p0[im];
               R br = p1[0], bi = p1[im];
               p1[0] = ar; p1[im] = ai; p1 += is;
               p0[0] = br; p0[im] = bi; p0 += js;
          }
     }
}

typedef solver S;

typedef struct {
     plan_dft super;
     int n;
     int s0, s1;
     int m;
     int offset;
     int nd, md, d; /* d = gcd(n,m), nd = n / d, md = m / d */
} P;

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     UNUSED(ro);
     UNUSED(io);
     A(ego->n == ego->m);
     t(ri, ii, ego->n, ego->s0, ego->s1);
}

static void apply_general(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int nd = ego->nd, md = ego->md, d = ego->d;
     R *buf = (R *)MALLOC((sizeof(R) * 2) * nd * md * d, BUFFERS);

     UNUSED(ii); UNUSED(ro); UNUSED(io);
     X(transpose)(ri + ego->offset, nd, md, d, 2, buf);
     X(ifree)(buf);
}

static void apply_slow(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int n = ego->n, m = ego->m;
     R buf[4];
     int move_size = (n + m) / 2;
     char *move;

     UNUSED(ii); UNUSED(ro); UNUSED(io);
     STACK_MALLOC(char *, move, move_size);
     X(transpose_slow)(ri + ego->offset, n, m, 2, move, move_size, buf);
     STACK_FREE(move);
}

static int applicable(const problem *p_, const planner *plnr)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *)p_;
	  const iodim *d = p->vecsz->dims;
          return (1
                  && p->ri == p->ro
                  && p->sz->rnk == 0
                  && p->vecsz->rnk == 2
		  && X(transposable)(d, d+1, 1, X(imin)(d[0].is,d[0].os),
				     p->ri, p->ii)
		  && (!NO_UGLYP(plnr) || d[0].n == d[1].n)
	       );
     }
     return 0;
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dft-transpose-%dx%d)", ego->n, ego->m);
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p;
     P *pln;
     const iodim *d;

     static const plan_adt padt = {
	  X(dft_solve), X(null_awake), print, X(plan_null_destroy)
     };

     UNUSED(ego);

     if (!applicable(p_, plnr))
          return (plan *) 0;
     p = (const problem_dft *) p_;

     d = p->vecsz->dims;
     pln = MKPLAN_DFT(P, &padt, 
		      X(transpose_simplep)(d, d+1, 1, X(imin)(d[0].is,d[0].os),
					   p->ri, p->ii) ? apply :
		      (X(transpose_slowp)(d, d+1, 2) ? apply_slow : 
		       apply_general));
     X(transpose_dims)(d, d+1, &pln->n, &pln->m, &pln->d, &pln->nd, &pln->md);
     pln->offset = (p->ri - p->ii == 1) ? -1 : 0;
     pln->s0 = d[0].is;
     pln->s1 = d[0].os;

     /* (4 loads + 4 stores) * (pln->n \choose 2)
        (FIXME? underestimate for non-square) */
     X(ops_other)(4 * pln->n * (pln->m - 1), &pln->super.super.ops);
     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     return MKSOLVER(S, &sadt);
}

void X(dft_vrank2_transpose_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
