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

/* $Id: vrank2-transpose.c,v 1.18 2003-02-28 23:28:58 stevenj Exp $ */

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
} P;

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     UNUSED(ro);
     UNUSED(io);
     t(ri, ii, ego->n, ego->s0, ego->s1);
}

static int applicable(const problem *p_)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *)p_;
	  const iodim *d = p->vecsz->dims;
          return (1
                  && p->ri == p->ro
                  && p->sz->rnk == 0
                  && p->vecsz->rnk == 2
                  && !X(dimcmp)(d + 0, d + 1)
	       );
     }
     return 0;
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dft-transpose-%d)", ego->n);
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p;
     P *pln;

     static const plan_adt padt = {
	  X(dft_solve), X(null_awake), print, X(plan_null_destroy)
     };

     UNUSED(plnr);
     UNUSED(ego);

     if (!applicable(p_))
          return (plan *) 0;
     p = (const problem_dft *) p_;

     pln = MKPLAN_DFT(P, &padt, apply);
     pln->n = p->vecsz->dims[0].n;
     pln->s0 = p->vecsz->dims[0].is;
     pln->s1 = p->vecsz->dims[0].os;

     /* (4 loads + 4 stores) * (pln->n \choose 2) */
     X(ops_other)(4 * pln->n * (pln->n - 1), &pln->super.super.ops);
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
