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

/* $Id: rank0.c,v 1.7 2002-09-22 16:25:20 athena Exp $ */

/* plans for rank-0 RDFTs (copy operations) */

#include "rdft.h"

#ifdef HAVE_STRING_H
#include <string.h>		/* for memcpy() */
#endif


typedef struct {
     rdftapply apply;
     int (*applicable)(const problem_rdft *p);
     const char *nam;
} rnk0adt;

typedef struct {
     solver super;
     const rnk0adt *adt;
} S;

typedef struct {
     plan_rdft super;
     uint vl;
     int ivs, ovs;
     const S *slv;
} P;

/* generic applicability function */
static int applicable(const solver *ego_, const problem *p_)
{
     if (RDFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->I != p->O
                  && p->sz->rnk == 0
                  && ego->adt->applicable(p)
	       );
     }
     return 0;
}

/*-----------------------------------------------------------------------*/
/* rank-0 rdft, vl == 1: just a copy */
static void apply_1(plan *ego_, R *I, R *O)
{
     UNUSED(ego_);
     *O = *I;
}

static int applicable_1(const problem_rdft *p)
{
     return (p->vecsz->rnk == 0);
}

static const rnk0adt adt_cpy1 =
{
     apply_1, applicable_1, "rdft-rank0-cpy1"
};

/*-----------------------------------------------------------------------*/
/* rank-0 rdft, vl > 1: just a copy loop (unroll 4) */
static void apply_vec(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     uint i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;

     for (i = 4; i <= vl; i += 4) {
          R r0, r1, r2, r3;
          r0 = *I; I += ivs;
          r1 = *I; I += ivs;
          r2 = *I; I += ivs;
          r3 = *I; I += ivs;
          *O = r0; O += ovs;
          *O = r1; O += ovs;
          *O = r2; O += ovs;
	  *O = r3; O += ovs;
     }
     for (; i < vl + 4; ++i) {
          R r0;
          r0 = *I; I += ivs;
          *O = r0; O += ovs;
     }
}

static int applicable_vec(const problem_rdft *p)
{
     return (p->vecsz->rnk == 1 && p->O != p->I);
}

static const rnk0adt adt_vec =
{
     apply_vec, applicable_vec, "rdft-rank0-vec"
};

/*-----------------------------------------------------------------------*/
/* rank-0 rdft, vl > 1, [io]vs == 1, using memcpy */
static void apply_io1(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     uint vl = ego->vl;
     memcpy(O, I, vl * sizeof(R));
}

static int applicable_io1(const problem_rdft *p)
{
     return (1
             && applicable_vec(p)
             && p->vecsz->dims[0].is == 1
             && p->vecsz->dims[0].os == 1
	  );
}

static const rnk0adt adt_io1 =
{
     apply_io1, applicable_io1, "rdft-rank0-io1-memcpy"
};

/*-----------------------------------------------------------------------*/
/* generic stuff: */

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(%s%v)", ego->slv->adt->nam, ego->vl);
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_rdft *p;
     P *pln;
     uint vl;
     int is, os;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, X(plan_null_destroy)
     };

     UNUSED(plnr);

     if (!applicable(ego_, p_))
          return (plan *) 0;

     p = (const problem_rdft *) p_;
     if (p->vecsz->rnk == 0) {
          vl = 1U;
          is = os = 1;
     } else {
          vl = p->vecsz->dims[0].n;
          is = p->vecsz->dims[0].is;
          os = p->vecsz->dims[0].os;
     }

     pln = MKPLAN_RDFT(P, &padt, ego->adt->apply);

     pln->vl = vl;
     pln->ivs = is;
     pln->ovs = os;
     pln->slv = ego;

     /* vl loads, vl stores */
     pln->super.super.ops = X(ops_other)(2 * vl);
     return &(pln->super.super);
}

static solver *mksolver(const rnk0adt *adt)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(rdft_rank0_register)(planner *p)
{
     uint i;
     static const rnk0adt *const adts[] = {
	  &adt_cpy1, &adt_vec, &adt_io1
     };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
          REGISTER_SOLVER(p, mksolver(adts[i]));
}
