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

/* $Id: rank0.c,v 1.1 2002-06-07 11:07:46 athena Exp $ */

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

static const rnk0adt adt_1 = { apply_1, applicable_1, "DFT-RANK0-1" };

/*-----------------------------------------------------------------------*/
/* rank-0 in-place dft: no-op */
static void apply_nop(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     UNUSED(ego_);
     UNUSED(ri);
     UNUSED(ii);
     UNUSED(ro);
     UNUSED(io);
}

static int applicable_nop(const problem_dft *p)
{
     return (p->ro == p->ri && fftw_tensor_inplace_strides(p->vecsz));
}

static const rnk0adt adt_nop = { apply_nop, applicable_nop, "DFT-RANK0-NOP" };

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

	  r0 = ri[(i - 4) * ivs];
	  i0 = ii[(i - 4) * ivs];
	  r1 = ri[(i - 3) * ivs];
	  i1 = ii[(i - 3) * ivs];
	  r2 = ri[(i - 2) * ivs];
	  i2 = ii[(i - 2) * ivs];
	  r3 = ri[(i - 1) * ivs];
	  i3 = ii[(i - 1) * ivs];
	  ro[(i - 4) * ovs] = r0;
	  io[(i - 4) * ovs] = i0;
	  ro[(i - 3) * ovs] = r1;
	  io[(i - 3) * ovs] = i1;
	  ro[(i - 2) * ovs] = r2;
	  io[(i - 2) * ovs] = i2;
	  ro[(i - 1) * ovs] = r3;
	  io[(i - 1) * ovs] = i3;
     }
     for (i -= 4; i < vl; ++i) {
	  R r0, i0;
	  r0 = ri[i * ivs];
	  i0 = ii[i * ivs];
	  ro[i * ovs] = r0;
	  io[i * ovs] = i0;
     }
}

static int applicable_vec(const problem_dft *p)
{
     return (p->vecsz.rnk == 1 && p->ro != p->ri);
}

static const rnk0adt adt_vec = { apply_vec, applicable_vec, "DFT-RANK0-VEC" };

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
     return (p->vecsz.dims[0].is == 1 && p->vecsz.dims[0].os == 1);
}

static const rnk0adt adt_io1 = {
     apply_io1, applicable_io1, "DFT-RANK0-IO1-MEMCPY"
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
	     && p->vecsz.dims[0].is == 2
	     && p->vecsz.dims[0].os == 2 
	     && p->ii == p->ri + 1 && p->io == p->ro + 1
	  );
}

static const rnk0adt adt_io2 = { 
     apply_io2, applicable_io2, "DFT-RANK0-IO1-MEMCPY" 
};

/*-----------------------------------------------------------------------*/
/* generic stuff: */
static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     fftw_free(ego);
}

static void print(plan *ego_, plan_printf prntf)
{
     P *ego = (P *) ego_;
     prntf("(%s", ego->slv->adt->nam);
     if (ego->vl > 1)
	  prntf("-x%d", ego->vl);
     prntf(")");
}

static enum score score(const solver *ego, const problem *p)
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
	  fftw_dft_solve, fftw_null_awake, print, destroy 
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
     pln->super.super.cost = 1.0;	/* FIXME? */
     pln->super.super.flops = fftw_flops_zero;
     return &(pln->super.super);
}

static solver *mksolver(const rnk0adt *adt)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void fftw_dft_rank0_register(planner *p)
{
     uint i;
     static const rnk0adt *adts[] = {
	  &adt_1, &adt_nop, &adt_vec, &adt_io1, &adt_io2
     };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
	  p->adt->register_solver(p, mksolver(adts[i]));
}
