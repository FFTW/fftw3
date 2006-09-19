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

/* mpi problem flags:

   meaning for rnk > 1:
      TRANSPOSED: output is globally transposed 
      SCRAMBLED_IN: input is *locally* transposed
      SCRAMBLED_OUT: output is *locally* transposed.

   meaning for rnk == 1:
      TRANSPOSED: not applicable
      SCRAMBLED_IN/SCRAMBLED_OUT: to be determined (some mixture of
                                  global and local transpositions)
*/
#define TRANSPOSED (1 << 0) /* only for rank >= 2 */
#define SCRAMBLED_IN (1 << 1) /* only for rank 1 */
#define SCRAMBLED_OUT (1 << 2) /* only for rank 1 */


#if defined(FFTW_SINGLE)
#  define FFTW_MPI_TYPE MPI_FLOAT
#elif defined(FFTW_LDOUBLE)
#  define FFTW_MPI_TYPE MPI_LONG_DOUBLE
#else
#  define FFTW_MPI_TYPE MPI_DOUBLE
#endif

/* block.c: */
int X(is_block_cyclic)(INT n, INT block, MPI_Comm comm);
INT X(default_block)(INT n, MPI_Comm comm);
INT X(current_block)(INT n, INT block, MPI_Comm comm);
INT X(some_block)(INT n, INT block, int which_pe, int n_pes);

/* any_true.c */
int X(any_true)(int condition, MPI_Comm comm);

/* conf.c */
void X(mpi_conf_standard)(planner *p);

#endif /* __IFFTW_MPI_H__ */
