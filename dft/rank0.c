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

/* $Id: rank0.c,v 1.11 2002-06-12 11:47:41 athena Exp $ */

/* plans for rank-0 DFTs (copy operations) */

#include "dft.h"

#ifdef HAVE_STRING_H
#include <string.h>		/* for memcpy() */
#endif


typedef struct {
     dftapply apply;
     int (*applicable)(const problem_dft *p);
     const char *nam;
} rnk0adt;

typedef struct {
     solver super;
     const rnk0adt *adt;
} S;

typedef struct {
     plan_dft super;
     uint vl;
     int ivs, ovs;
     const S *slv;
} P;

/* generic applicability function */
static int applicable(const solver *ego_, const problem *p_)
{
     if (DFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_dft *p = (const problem_dft *) p_;
          return (1
                  && p->sz.rnk == 0
                  && ego->adt->applicable(p)
	       );
     }
     return 0;
}

/*-----------------------------------------------------------------------*/
/* rank-0 dft, vl == 1: just a copy */
static void apply_1(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     UNUSED(ego_);
     *ro = *ri;
     *io = *ii;
}

static int applicable_1(const problem_dft *p)
{
     return (p->vecsz.rnk == 0);
}

static const rnk0adt adt_cpy1 =
{
     apply_1, applicable_1, "dft-rank0-cpy1"
};

/*-----------------------------------------------------------------------*/
/* rank-0 dft, vl > 1: just a copy loop (unroll 4) */
static void apply_vec(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     uint i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;

     for (i = 4; i <= vl; i += 4) {
          R r0, r1, r2, r3;
          R i0, i1, i2, i3;
          r0 = *ri; ri += ivs;
          i0 = *ii; ii += ivs;
          r1 = *ri; ri += ivs;
          i1 = *ii; ii += ivs;
          r2 = *ri; ri += ivs;
          i2 = *ii; ii += ivs;
          r3 = *ri; ri += ivs;
          i3 = *ii; ii += ivs;
          *ro = r0; ro += ovs;
          *io = i0; io += ovs;
          *ro = r1; ro += ovs;
          *io = i1; io += ovs;
          *ro = r2; ro += ovs;
          *io = i2; io += ovs;
	  *ro = r3; ro += ovs;
          *io = i3; io += ovs;
     }
     for (; i < vl + 4; ++i) {
          R r0, i0;
          r0 = *ri; ri += ivs;
          i0 = *ii; ii += ivs;
          *ro = r0; ro += ovs;
          *io = i0; io += ovs;
     }
}

static int applicable_vec(const problem_dft *p)
{
     return (p->vecsz.rnk == 1 && p->ro != p->ri);
}

static const rnk0adt adt_vec =
{
     apply_vec, applicable_vec, "dft-rank0-vec"
};

/*-----------------------------------------------------------------------*/
/* rank-0 dft, vl > 1, [io]vs == 1, using memcpy */
static void apply_io1(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     uint vl = ego->vl;
     memcpy(ro, ri, vl * sizeof(R));
     memcpy(io, ii, vl * sizeof(R));
}

static int applicable_io1(const problem_dft *p)
{
     return (1
             && applicable_vec(p)
             && p->vecsz.dims[0].is == 1
             && p->vecsz.dims[0].os == 1
	  );
}

static const rnk0adt adt_io1 =
{
     apply_io1, applicable_io1, "dft-rank0-io1-memcpy"
};

/*-----------------------------------------------------------------------*/
/* rank-0 dft, vl > 1, [io]vs == 2 (interleaved) using memcpy */
static void apply_io2(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     uint vl = ego->vl;
     UNUSED(ii);
     UNUSED(io);		/* i{in,out}put == r{in,out}put + 1 */
     memcpy(ro, ri, vl * sizeof(R) * 2);
}

static int applicable_io2(const problem_dft *p)
{
     return (1
             && applicable_vec(p)
             && p->vecsz.dims[0].is == 2
             && p->vecsz.dims[0].os == 2
             && p->ii == p->ri + 1 && p->io == p->ro + 1
	  );
}

static const rnk0adt adt_io2 =
{
     apply_io2, applicable_io2, "dft-rank0-io2-memcpy"
};

/*-----------------------------------------------------------------------*/
/* generic stuff: */
static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(%s%v)", ego->slv->adt->nam, ego->vl);
}

static int score(const solver *ego, const problem *p)
{
     return (applicable(ego, p)) ? GOOD : BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p;
     P *pln;
     uint vl;
     int is, os;

     static const plan_adt padt = {
	  X(dft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *) 0;

     p = (const problem_dft *) p_;
     if (p->vecsz.rnk == 0) {
          vl = 1U;
          is = os = 1;
     } else {
          vl = p->vecsz.dims[0].n;
          is = p->vecsz.dims[0].is;
          os = p->vecsz.dims[0].os;
     }

     pln = MKPLAN_DFT(P, &padt, ego->adt->apply);

     pln->vl = vl;
     pln->ivs = is;
     pln->ovs = os;
     pln->slv = ego;

     /* 2*vl loads, 2*vl stores */
     pln->super.super.ops = X(ops_other)(4 * vl);
     return &(pln->super.super);
}

static solver *mksolver(const rnk0adt *adt)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(dft_rank0_register)(planner *p)
{
     uint i;
     static const rnk0adt *adts[] = {
	  &adt_cpy1, &adt_vec, &adt_io1, &adt_io2
     };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
          REGISTER_SOLVER(p, mksolver(adts[i]));
}
