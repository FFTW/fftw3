/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

#ifndef __RDFT_H__
#define __RDFT_H__

#include "ifftw.h"
#include "codelet-rdft.h"

/* problem.c: */
typedef struct {
     problem super;
     tensor *sz, *vecsz;
     R *I, *O;
#if defined(STRUCT_HACK_KR)
     rdft_kind kind[1];
#elif defined(STRUCT_HACK_C99)
     rdft_kind kind[];
#else
     rdft_kind *kind;
#endif
} problem_rdft;

void X(rdft_zerotens)(tensor *sz, R *I);
problem *X(mkproblem_rdft)(const tensor *sz, const tensor *vecsz,
			   R *I, R *O, const rdft_kind *kind);
problem *X(mkproblem_rdft_d)(tensor *sz, tensor *vecsz,
			     R *I, R *O, const rdft_kind *kind);
problem *X(mkproblem_rdft_0_d)(tensor *vecsz, R *I, R *O);
problem *X(mkproblem_rdft_1)(const tensor *sz, const tensor *vecsz,
			     R *I, R *O, rdft_kind kind);
problem *X(mkproblem_rdft_1_d)(tensor *sz, tensor *vecsz,
			       R *I, R *O, rdft_kind kind);

const char *X(rdft_kind_str)(rdft_kind kind);

/* solve.c: */
void X(rdft_solve)(const plan *ego_, const problem *p_);

/* plan.c: */
typedef void (*rdftapply) (const plan *ego, R *I, R *O);

typedef struct {
     plan super;
     rdftapply apply;
} plan_rdft;

plan *X(mkplan_rdft)(size_t size, const plan_adt *adt, rdftapply apply);

#define MKPLAN_RDFT(type, adt, apply) \
  (type *)X(mkplan_rdft)(sizeof(type), adt, apply)

/* various solvers */

solver *X(mksolver_rdft_r2hc_direct)(kr2hc k, const kr2hc_desc *desc);
solver *X(mksolver_rdft_hc2r_direct)(khc2r k, const khc2r_desc *desc);
solver *X(mksolver_rdft_r2r_direct)(kr2r k, const kr2r_desc *desc);

void X(rdft_rank0_register)(planner *p);
void X(rdft_vrank3_transpose_register)(planner *p);
void X(rdft_rank_geq2_register)(planner *p);
void X(rdft_indirect_register)(planner *p);
void X(rdft_vrank_geq1_register)(planner *p);
void X(rdft_buffered_register)(planner *p);
void X(rdft_generic_register)(planner *p);
void X(rdft_rader_hc2hc_register)(planner *p);
void X(rdft_dht_register)(planner *p);
void X(dht_r2hc_register)(planner *p);
void X(dht_rader_register)(planner *p);
void X(dft_r2hc_register)(planner *p);
void X(rdft_nop_register)(planner *p);
void X(hc2hc_generic_register)(planner *p);

/****************************************************************************/
/* problem2.c: */
/* an RDFT2 problem transforms a 1d real array r[n] with stride is/os
   to/from an "unpacked" complex array {rio,iio}[n/2 + 1] with stride
   os/is.  Multidimensional transforms use complex DFTs for the
   noncontiguous dimensions.  vecsz has the usual interpretation.  */
typedef struct {
     problem super;
     tensor *sz;
     tensor *vecsz;
     R *r, *rio, *iio;
     rdft_kind kind; /* R2HC or HC2R */
} problem_rdft2;

problem *X(mkproblem_rdft2)(const tensor *sz, const tensor *vecsz,
			    R *r, R *rio, R *iio, rdft_kind kind);
problem *X(mkproblem_rdft2_d)(tensor *sz, tensor *vecsz,
			      R *r, R *rio, R *iio, rdft_kind kind);
int X(rdft2_inplace_strides)(const problem_rdft2 *p, int vdim);
INT X(rdft2_tensor_max_index)(const tensor *sz, rdft_kind k);
void X(rdft2_strides)(rdft_kind kind, const iodim *d, INT *is, INT *os);

/* verify.c: */
void X(rdft2_verify)(plan *pln, const problem_rdft2 *p, int rounds);

/* solve.c: */
void X(rdft2_solve)(const plan *ego_, const problem *p_);

/* plan.c: */
typedef void (*rdft2apply) (const plan *ego, R *r, R *rio, R *iio);

typedef struct {
     plan super;
     rdft2apply apply;
} plan_rdft2;

plan *X(mkplan_rdft2)(size_t size, const plan_adt *adt, rdft2apply apply);

#define MKPLAN_RDFT2(type, adt, apply) \
  (type *)X(mkplan_rdft2)(sizeof(type), adt, apply)

/* various solvers */

solver *X(mksolver_rdft2_r2hc_direct)(kr2hc k, const kr2hc_desc *desc);
solver *X(mksolver_rdft2_hc2r_direct)(khc2r k, const khc2r_desc *desc);

void X(rdft2_vrank_geq1_register)(planner *p);
void X(rdft2_buffered_register)(planner *p);
void X(rdft2_nop_register)(planner *p);
void X(rdft2_rank0_register)(planner *p);
void X(rdft2_rank_geq2_register)(planner *p);
void X(rdft2_radix2_register)(planner *p);

/****************************************************************************/

/* configurations */
void X(rdft_conf_standard)(planner *p);

#endif /* __RDFT_H__ */
