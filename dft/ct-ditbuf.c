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

/* $Id: ct-ditbuf.c,v 1.1 2002-06-10 20:30:37 athena Exp $ */

/* decimation in time Cooley-Tukey.  Codelet operates on
   contiguous buffer rather than directly on the output array.  */

/* FIXME: find a way to use rank-0 transforms for this stuff */

#include "dft.h"
#include "ct.h"

/*
   Copy A -> B, where A and B are n0 x n1 complex matrices
   such that the (i0, i1) element has index (i0 * s0 + i1 * s1). 
*/
static inline void cpy0(uint n0, uint n1, 
			const R *rA, const R *iA, int sa0, int sa1, 
			R *rB, R *iB, int sb0, int sb1)
{
     uint i0, i1;

     for (i0 = 0; i0 < n0; ++i0) {
	  for (i1 = 0; i1 < n1; ++i1) {
	       R xr, xi;
	       xr = rA[i0 * sa0 + i1 * sa1];
	       xi = iA[i0 * sa0 + i1 * sa1];
	       rB[i0 * sb0 + i1 * sb1] = xr;
	       iB[i0 * sb0 + i1 * sb1] = xi;
	  }
     }
}

static void cpy(uint n0, uint n1, 
		const R *rA, const R *iA, int sa0, int sa1, 
		R *rB, R *iB, int sb0, int sb1)
{
     if (n1 == 4) {
	  if (sa1 == 2 && sb0 == 2) {
	       cpy0(n0, 4, rA, iA, sa0, 2, rB, iB, 2, sb1);
	       return;
	  }
	  if (sa0 == 2 && sb1 == 2) {
	       cpy0(n0, 4, rA, iA, 2, sa1, rB, iB, sb0, 2);
	       return;
	  }
     }
     cpy0(n0, n1, rA, iA, sa0, sa1, rB, iB, sb0, sb1);
}

static const R *doit(kdft_dit k, R *rA, R *iA, const R *W, int ios, int dist, 
		     uint r, uint batchsz, R *buf, stride bufstride)
{
     cpy(r, batchsz, rA, iA, ios, dist, buf, buf + 1, 2, 2 * r);
     W = k(buf, buf + 1, W, bufstride, batchsz, 2 * r);
     cpy(r, batchsz, buf, buf + 1, 2, 2 * r, rA, iA, ios, dist);
     return W;
}

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     plan_ct *ego = (plan_ct *) ego_;
     plan *cld0 = ego->cld;
     plan_dft *cld = (plan_dft *) cld0;

     /* two-dimensional r x vl sub-transform: */
     cld->apply(cld0, ri, ii, ro, io);

     {
	  const uint batchsz = 4; /* FIXME: parametrize? */
          uint i, j, m = ego->m, vl = ego->vl, r = ego->r;
          int os = ego->os, ovs = ego->ovs, ios = ego->ios_for_buf;
	  R *buf = (R *) STACK_MALLOC(r * batchsz * 2 * sizeof(R));

          for (i = 0; i < vl; ++i) {
	       R *rA = ro + i * ovs, *iA = io + i * ovs;
	       const R *W = ego->W;

	       for (j = m; j >= batchsz; j -= batchsz) {
		    W = doit(ego->k.dit, rA, iA, W, ios, os, r, 
			     batchsz, buf, ego->vs);
		    rA += os * (int)batchsz;
		    iA += os * (int)batchsz;
	       }

	       /* do remaining j calls, if any */
	       if (j > 0)
		    doit(ego->k.dit, rA, iA, W, ios, os, r, j, buf, ego->vs);

	  }

	  STACK_FREE(buf);
     }
}

static int applicable(const solver_ct *ego, const problem *p_)
{
     if (X(dft_ct_applicable)(ego, p_)) {
          const ct_desc *e = ego->desc;
          return (1
                  /* stride is always 2 */
                  && (!e->is || e->is == 2)
	       );
     }
     return 0;
}

static void finish(plan_ct *ego)
{
     ego->ios_for_buf = ego->m * ego->os;
     ego->vs = X(mkstride)(ego->r, 2);
     ego->super.super.flops =
          X(flops_add)(ego->cld->flops,
                       X(flops_mul)(ego->vl * ego->m, ego->slv->desc->flops));
}

static problem *mkcld(const solver_ct *ego, const problem_dft *p)
{
     iodim *d = p->sz.dims;
     const ct_desc *e = ego->desc;
     uint m = d[0].n / e->radix;

     tensor radix = X(mktensor_1d)(e->radix, d[0].is, m * d[0].os);
     tensor cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);

     return X(mkproblem_dft_d)(X(mktensor_1d)(m, e->radix * d[0].is, d[0].os),
			       cld_vec, p->ri, p->ii, p->ro, p->io);
}

static int score(const solver *ego_, const problem *p_)
{
     const solver_ct *ego = (const solver_ct *) ego_;
     const problem_dft *p = (const problem_dft *) p_;
     uint n;

     if (!applicable(ego, p_))
          return BAD;

     n = p->sz.dims[0].n;

     if (0
	 || n <= 512 /* favor non-buffered version */
	 || n / ego->desc->radix <= 4
	  )
          return UGLY;

     return GOOD;
}


static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     static const ctadt adt = {
	  mkcld, finish, applicable, apply
     };
     return X(mkplan_dft_ct)((const solver_ct *) ego, p, plnr, &adt);
}


solver *X(mksolver_dft_ct_ditbuf)(kdft_dit codelet, const ct_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     static const char name[] = "dft-ditbuf";
     union kct k;
     k.dit = codelet;

     return X(mksolver_dft_ct)(k, desc, name, &sadt);
}
