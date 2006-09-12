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

#include "ifftw_mpi.h"

/* tproblem.c: */
typedef struct {
     problem super;
     INT vn; /* vector length (vector stride 1) */
     INT nx, ny; /* nx x ny transposed to ny x nx */
     R *I, *O; /* contiguous real arrays (both same size!) */

     int flags; /* problem flags: 

		     SCRAMBLED_IN = the input slab is already transposed 
                                    locally, and is stored in O, not I. 
		     SCRAMBLED_OUT = output slab is transposed locally
                     TRANSPOSED = not allowed */

     INT block, tblock; /* block size, slab decomposition;
			   tblock is for transposed blocks on output */

     MPI_Comm comm;
} problem_mpi_transpose;

problem *X(mkproblem_mpi_transpose)(INT vn, INT nx, INT ny,
				    R *I, R *O,
				    INT block, INT tblock,
				    MPI_Comm comm,
				    int flags);

/* tsolve.c: */
void X(mpi_transpose_solve)(const plan *ego_, const problem *p_);

/* plans have same operands as rdft plans, so just re-use */
typedef plan_rdft plan_mpi_transpose;
#define MKPLAN_MPI_TRANSPOSE(type, adt, apply) \
  (type *)X(mkplan_rdft)(sizeof(type), adt, apply)

/* various solvers */
void X(mpi_transpose_inplace_register)(planner *p);
void X(mpi_transpose_alltoall_register)(planner *p);
