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

/* $Id: rank0-vrank2.c,v 1.4 2005-01-10 23:57:30 athena Exp $ */

/* plans for rank-0 RDFTs (copy operations), with vector rank 2;
   these plans are not strictly necessary, but are an optimization
   ...especially for the common case (via dft plans) where one of
   the vector loops has length 2. */

#include "rdft.h"

#ifdef HAVE_STRING_H
#include <string.h>		/* for memcpy() */
#endif

typedef struct S_s S;

typedef struct {
     rdftapply apply;
     int (*applicable)(const S *s, const problem_rdft *p, planner *plnr);
     const char *nam;
} rnk0adt;

struct S_s {
     solver super;
     const rnk0adt *adt;
     int reverse; /* 0 or 1, indicating whether to swap loops */
};

typedef struct {
     plan_rdft super;
     int vl;
     int ivs, ovs;
     int vl2;
     int ivs2, ovs2;
     const S *slv;
} P;

/* generic applicability function */
static int applicable(const solver *ego_, const problem *p_, planner *plnr)
{
     if (RDFTP(p_)) {
          const S *ego = (const S *) ego_;
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->I != p->O
                  && p->sz->rnk == 0
		  && p->vecsz->rnk == 2

		  /* reversing the loops is ugly unless at least one of the
		     strides is backwards (i.e. dim[0] has a lower stride) */
		  && (!NO_UGLYP(plnr) || !ego->reverse
		      || (X(iabs)(p->vecsz->dims[0].is) 
			  < X(iabs)(p->vecsz->dims[1].is))
		      || (X(iabs)(p->vecsz->dims[0].os) 
			  < X(iabs)(p->vecsz->dims[1].os)))

                  && ego->adt->applicable(ego, p, plnr)
	       );
     }
     return 0;
}

/*-----------------------------------------------------------------------*/
/* rank-0 rdft, vl > 1, vl2 = 2, ivs2 = ovs2 = 1: copy consecutive pairs */
static void apply_consecpairs(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;

#if FFTW_2R_IS_DOUBLE
     if (DOUBLE_ALIGNED(I) && DOUBLE_ALIGNED(O)) {
	  /* copy floats two at a time, copying as double */
	  for (i = 4; i <= vl; i += 4) {
	       double r0, r1, r2, r3;
	       r0 = *((double *) I); I += ivs;
	       r1 = *((double *) I); I += ivs;
	       r2 = *((double *) I); I += ivs;
	       r3 = *((double *) I); I += ivs;
	       *((double *) O) = r0; O += ovs;
	       *((double *) O) = r1; O += ovs;
	       *((double *) O) = r2; O += ovs;
	       *((double *) O) = r3; O += ovs;
	  }
	  for (; i < vl + 4; ++i) {
	       double r0;
	       r0 = *((double *) I); I += ivs;
	       *((double *) O) = r0; O += ovs;
	  }
	  /* TODO: add SSE2 version for 2 doubles at a time? */
     }
     else {
#endif
     for (i = 4; i <= vl; i += 4) {
          R r0, r1, r2, r3;
          R i0, i1, i2, i3;
          r0 = I[0]; i0 = I[1]; I += ivs;
          r1 = I[0]; i1 = I[1]; I += ivs;
          r2 = I[0]; i2 = I[1]; I += ivs;
          r3 = I[0]; i3 = I[1]; I += ivs;
          O[0] = r0; O[1] = i0; O += ovs;
          O[0] = r1; O[1] = i1; O += ovs;
          O[0] = r2; O[1] = i2; O += ovs;
	  O[0] = r3; O[1] = i3; O += ovs;
     }
     for (; i < vl + 4; ++i) {
          R r0, i0;
          r0 = I[0]; i0 = I[1]; I += ivs;
          O[0] = r0; O[1] = i0; O += ovs;
     }
#if FFTW_2R_IS_DOUBLE
     }
#endif
}

static int applicable_consecpairs(const S *s, const problem_rdft *p, planner *plnr)
{
     UNUSED(plnr);
     return (p->vecsz->dims[1].n == 2
	     && p->vecsz->dims[1].is == 1
	     && p->vecsz->dims[1].os == 1
#if FFTW_2R_IS_DOUBLE
	     && p->vecsz->dims[0].is % 2 == 0
	     && p->vecsz->dims[0].os % 2 == 0
#endif
	     && !s->reverse);
}

static const rnk0adt adt_consecpairs =
{
     apply_consecpairs, applicable_consecpairs, "rdft-rank0-consecpairs"
};

/*-----------------------------------------------------------------------*/
/* rank-0 rdft, vl > 1, vl2 = 2: copy pairs */
static void apply_pairs(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;
     int ivs2 = ego->ivs2, ovs2 = ego->ovs2;

     for (i = 4; i <= vl; i += 4) {
          R r0, r1, r2, r3;
          R i0, i1, i2, i3;
          r0 = I[0]; i0 = I[ivs2]; I += ivs;
          r1 = I[0]; i1 = I[ivs2]; I += ivs;
          r2 = I[0]; i2 = I[ivs2]; I += ivs;
          r3 = I[0]; i3 = I[ivs2]; I += ivs;
          O[0] = r0; O[ovs2] = i0; O += ovs;
          O[0] = r1; O[ovs2] = i1; O += ovs;
          O[0] = r2; O[ovs2] = i2; O += ovs;
	  O[0] = r3; O[ovs2] = i3; O += ovs;
     }
     for (; i < vl + 4; ++i) {
          R r0, i0;
          r0 = I[0]; i0 = I[ivs2]; I += ivs;
          O[0] = r0; O[ovs2] = i0; O += ovs;
     }
}

static int applicable_pairs(const S *s, const problem_rdft *p, planner *plnr)
{
     return (p->vecsz->dims[(1+s->reverse)%2].n == 2 &&
	     (!NO_UGLYP(plnr) || !applicable_consecpairs(s, p, plnr)));
}

static const rnk0adt adt_pairs =
{
     apply_pairs, applicable_pairs, "rdft-rank0-pairs"
};

/*-----------------------------------------------------------------------*/
/* generic vrank-2, rank-0 rdft: vl > 1, vl2 > 1, unrolling vl2 loop */
static void apply_vrank2(const plan *ego_, R *I, R *O)
{
     const P *ego = (const P *) ego_;
     int i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;
     int j, vl2 = ego->vl2;
     int ivs2 = ego->ivs2, ovs2 = ego->ovs2;


     for (i = 0; i < vl; ++i) {
	  R *Ii = I;
	  R *Oi = O;
	  for (j = 4; j <= vl2; j += 4) {
	       R r0, r1, r2, r3;
	       r0 = *Ii; Ii += ivs2;
	       r1 = *Ii; Ii += ivs2;
	       r2 = *Ii; Ii += ivs2;
	       r3 = *Ii; Ii += ivs2;
	       *Oi = r0; Oi += ovs2;
	       *Oi = r1; Oi += ovs2;
	       *Oi = r2; Oi += ovs2;
	       *Oi = r3; Oi += ovs2;
	  }
	  for (; j < vl2 + 4; ++j) {
	       R r0;
	       r0 = *Ii; Ii += ivs2;
	       *Oi = r0; Oi += ovs2;
	  }
	  I += ivs;
	  O += ovs;
     }
}

static int applicable_vrank2(const S *s, const problem_rdft *p, planner *plnr)
{
     return (!NO_UGLYP(plnr) || !applicable_pairs(s, p, plnr));
}

static const rnk0adt adt_vrank2 =
{
     apply_vrank2, applicable_vrank2, "rdft-rank0-vrank2"
};

/*-----------------------------------------------------------------------*/
/* generic stuff: */

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(%s%s%v%v)", ego->slv->adt->nam,
	      ego->slv->reverse ? "-rev" : "", ego->vl, ego->vl2);
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const problem_rdft *p;
     P *pln;

     static const plan_adt padt = {
	  X(rdft_solve), X(null_awake), print, X(plan_null_destroy)
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *) 0;

     p = (const problem_rdft *) p_;
     pln = MKPLAN_RDFT(P, &padt, ego->adt->apply);

     pln->vl = p->vecsz->dims[ego->reverse].n;
     pln->ivs = p->vecsz->dims[ego->reverse].is;
     pln->ovs = p->vecsz->dims[ego->reverse].os;
     pln->vl2 = p->vecsz->dims[(1+ego->reverse)%2].n;
     pln->ivs2 = p->vecsz->dims[(1+ego->reverse)%2].is;
     pln->ovs2 = p->vecsz->dims[(1+ego->reverse)%2].os;
     pln->slv = ego;

     /* vl loads, vl stores */
     X(ops_other)(2 * pln->vl * pln->vl2, &pln->super.super.ops);
     return &(pln->super.super);
}

static solver *mksolver(const rnk0adt *adt, int reverse)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     slv->reverse = reverse;
     return &(slv->super);
}

void X(rdft_rank0_vrank2_register)(planner *p)
{
     unsigned i;
     int reverse[] = { 0, 0, 1, 0, 1 };
     static const rnk0adt *const adts[] = {
	  &adt_consecpairs, &adt_pairs, &adt_pairs, &adt_vrank2, &adt_vrank2
     };

     A(sizeof(adts) / sizeof(adts[0]) == sizeof(reverse) / sizeof(reverse[0]));
     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
          REGISTER_SOLVER(p, mksolver(adts[i], reverse[i]));
}
