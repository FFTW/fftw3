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

/* $Id: dft.h,v 1.37 2003-05-16 17:02:06 athena Exp $ */

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

/* problemw.c: */
typedef struct {
     problem super;

     int dec;
#    define DECDIF 0
#    define DECDIT 1

     int r, m;
     int s;  /* stride on the side without twiddles */
     int ws; /* stride on the side with twiddles */
     int vl, vs, wvs;
     R *rio, *iio;
} problem_dftw;

int X(problem_dftw_p)(const problem *p);
#define DFTWP X(problem_dftw_p)  /* shorthand */

problem *X(mkproblem_dftw)(int dec, int r, int m, 
			   int s, int ws, int vl, int vs, int wvs,
			   R *rio, R *iio);

/* solve.c: */
void X(dft_solve)(const plan *ego_, const problem *p_);
void X(dftw_solve)(const plan *ego_, const problem *p_);

/* plan.c: */
typedef void (*dftapply) (const plan *ego, R *ri, R *ii, R *ro, R *io);
typedef void (*dftwapply) (const plan *ego, R *rio, R *iio);

typedef struct {
     plan super;
     dftapply apply;
} plan_dft;

typedef struct {
     plan super;
     dftwapply apply;
} plan_dftw;

plan *X(mkplan_dft)(size_t size, const plan_adt *adt, dftapply apply);
plan *X(mkplan_dftw)(size_t size, const plan_adt *adt, dftwapply apply);

#define MKPLAN_DFT(type, adt, apply) \
  (type *)X(mkplan_dft)(sizeof(type), adt, apply)

#define MKPLAN_DFTW(type, adt, apply) \
  (type *)X(mkplan_dftw)(sizeof(type), adt, apply)

/* various solvers */
solver *X(mksolver_dft_direct)(kdft k, const kdft_desc *desc);
solver *X(mksolver_dft_directw)(kdftw codelet, const ct_desc *desc, int dec);
solver *X(mksolver_dft_directwbuf)(kdftw codelet, 
				   const ct_desc *desc, int dec);
solver *X(mksolver_dft_directwsq)(kdftwsq codelet,
				  const ct_desc *desc, int dec);


extern void (*X(kdft_dit_register_hook))(planner *, kdftw, const ct_desc *);
extern void (*X(kdft_dif_register_hook))(planner *, kdftw, const ct_desc *);

void X(dft_ct_register)(planner *p);
void X(dft_ctsq_register)(planner *p);
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
void X(dftw_dft_register)(planner *p);

/* rader-omega.c: auxiliary stuff for rader */
R *X(dft_rader_mkomega)(plan *p_, int n, int ginv);
void X(dft_rader_free_omega)(R **omega);

/* configurations */
void X(dft_conf_standard)(planner *p);

#endif /* __DFT_H__ */
