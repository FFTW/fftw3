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

/* FFTW-MPI internal header file */
#ifndef __IFFTW_MPI_H__
#define __IFFTW_MPI_H__

#include "ifftw.h"
#include "rdft.h"

#include <mpi.h>

/* mpi problem flags: problem-dependent meaning, but in general
   SCRAMBLED means some reordering *within* the dimensions, while
   TRANSPOSED means some reordering *of* the dimensions */
#define SCRAMBLED_IN (1 << 0)
#define SCRAMBLED_OUT (1 << 1)
#define TRANSPOSED_IN (1 << 2)
#define TRANSPOSED_OUT (1 << 3)

#define SCRAMBLEDP(flags) ((flags) & (SCRAMBLED_IN | SCRAMBLED_OUT))
#define TRANSPOSEDP(flags) ((flags) & (TRANSPOSED_IN | TRANSPOSED_OUT))

#if defined(FFTW_SINGLE)
#  define FFTW_MPI_TYPE MPI_FLOAT
#elif defined(FFTW_LDOUBLE)
#  define FFTW_MPI_TYPE MPI_LONG_DOUBLE
#else
#  define FFTW_MPI_TYPE MPI_DOUBLE
#endif

/* all fftw-mpi identifiers start with fftw_mpi (or fftwf_mpi etc.) */
#define XM(name) X(CONCAT(mpi_, name))

/***********************************************************************/
/* block distributions */

/* a distributed dimension of length n with input and output block
   sizes ib and ob, respectively. */
typedef enum { IB = 0, OB } block_kind;
typedef struct {
     INT n;
     INT b[2]; /* b[IB], b[OB] */
} ddim;

/* unlike tensors in the serial FFTW, the ordering of the dtensor
   dimensions matters - both the array and the block layout are
   row-major order. */
typedef struct {
     int rnk;
#if defined(STRUCT_HACK_KR)
     ddim dims[1];
#elif defined(STRUCT_HACK_C99)
     ddim dims[];
#else
     ddim *dims;
#endif
} dtensor;


/* dtensor.c: */
dtensor *XM(mkdtensor)(int rnk);
void XM(dtensor_destroy)(dtensor *sz);
dtensor *XM(dtensor_copy)(const dtensor *sz);
dtensor *XM(dtensor_canonical)(const dtensor *sz);
int XM(dtensor_validp)(const dtensor *sz);
void XM(dtensor_md5)(md5 *p, const dtensor *t);
void XM(dtensor_print)(const dtensor *t, printer *p);

/* block.c: */

/* for a single distributed dimension: */
INT XM(num_blocks)(INT n, INT block);
int XM(num_blocks_ok)(INT n, INT block, MPI_Comm comm);
INT XM(default_block)(INT n, INT n_pes);
INT XM(block)(INT n, INT block, INT which_block);

/* for multiple distributed dimensions: */
INT XM(num_blocks_total)(const dtensor *sz, block_kind k);
int XM(idle_process)(const dtensor *sz, block_kind k, INT which_pe);
void XM(block_coords)(const dtensor *sz, block_kind k, INT which_pe, 
		     INT *coords);
INT XM(total_block)(const dtensor *sz, block_kind k, INT which_pe);
int XM(is_local_after)(int dim, const dtensor *sz, block_kind k);
int XM(is_local)(const dtensor *sz, block_kind k);
int XM(is_block1d)(const dtensor *sz, block_kind k);

/***********************************************************************/
/* any_true.c */
int XM(any_true)(int condition, MPI_Comm comm);

/* conf.c */
void XM(conf_standard)(planner *p);

#endif /* __IFFTW_MPI_H__ */
