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

/* $Id: dft.h,v 1.39 2003-07-05 17:05:51 athena Exp $ */

#ifndef __DFT_H__
#define __DFT_H__

#include "ifftw.h"
#include "codelet-dft.h"

/* problem.c: */
typedef struct {
     problem super;
     tensor *sz, *vecsz;
     R *ri, *ii, *ro, *io;
} problem_dft;

int X(problem_dft_p)(const problem *p);
#define DFTP X(problem_dft_p)  /* shorthand */

void X(dft_zerotens)(tensor *sz, R *ri, R *ii);
problem *X(mkproblem_dft)(const tensor *sz, const tensor *vecsz,
                          R *ri, R *ii, R *ro, R *io);
problem *X(mkproblem_dft_d)(tensor *sz, tensor *vecsz,
                            R *ri, R *ii, R *ro, R *io);

/* solve.c: */
void X(dft_solve)(const plan *ego_, const problem *p_);

/* plan.c: */
typedef void (*dftapply) (const plan *ego, R *ri, R *ii, R *ro, R *io);

typedef struct {
     plan super;
     dftapply apply;
} plan_dft;

plan *X(mkplan_dft)(size_t size, const plan_adt *adt, dftapply apply);

#define MKPLAN_DFT(type, adt, apply) \
  (type *)X(mkplan_dft)(sizeof(type), adt, apply)

/* various solvers */
solver *X(mksolver_dft_direct)(kdft k, const kdft_desc *desc);

extern void (*X(kdft_dit_register_hook))(planner *, kdftw, const ct_desc *);
extern void (*X(kdft_dif_register_hook))(planner *, kdftw, const ct_desc *);

void X(dft_rank0_register)(planner *p);
void X(dft_rank_geq2_register)(planner *p);
void X(dft_indirect_register)(planner *p);
void X(dft_vrank_geq1_register)(planner *p);
void X(dft_vrank2_transpose_register)(planner *p);
void X(dft_vrank3_transpose_register)(planner *p);
void X(dft_buffered_register)(planner *p);
void X(dft_generic_register)(planner *p);
void X(dft_rader_register)(planner *p);
void X(dft_bluestein_register)(planner *p);
void X(dft_nop_register)(planner *p);
void X(dft_ct_generic_register)(planner *p);

/* rader-omega.c: auxiliary stuff for rader */
R *X(dft_rader_mkomega)(plan *p_, int n, int ginv);
void X(dft_rader_free_omega)(R **omega);

/* configurations */
void X(dft_conf_standard)(planner *p);

#endif /* __DFT_H__ */
