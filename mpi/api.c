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

#include "api.h"
#include "fftw3_mpi.h"
#include "ifftw_mpi.h"
#include "mpi_transpose.h"

#define MPI_FLAGS(f) ((f) >> 27)

static int mpi_inited = 0;

static void init(void)
{
     if (!mpi_inited) {
          X(mpi_conf_standard)(X(the_planner)());
	  mpi_inited = 1;
     }
}

void X(mpi_cleanup)(void)
{
     X(cleanup)();
     mpi_inited = 0;
}

ptrdiff_t X(mpi_local_size_2d_transposed)(ptrdiff_t nx, ptrdiff_t ny,
					  ptrdiff_t xblock, ptrdiff_t yblock,
					  MPI_Comm comm,
					  ptrdiff_t *local_nx,
					  ptrdiff_t *local_x_start,
					  ptrdiff_t *local_ny, 
					  ptrdiff_t *local_y_start)
{
     int my_pe;
     if (!xblock) xblock = X(default_block)(nx, comm);
     if (!yblock) yblock = X(default_block)(ny, comm);
     *local_nx = X(current_block)(nx, xblock, comm);
     *local_ny = X(current_block)(ny, yblock, comm);
     MPI_Comm_rank(comm, &my_pe);
     *local_x_start = *local_nx == 0 ? 0 : xblock * my_pe;
     *local_y_start = *local_ny == 0 ? 0 : yblock * my_pe;
     return X(imax)(*local_nx * ny, *local_ny * nx);
}

ptrdiff_t X(mpi_local_size_2d)(ptrdiff_t nx, ptrdiff_t ny,
			       MPI_Comm comm,
			       ptrdiff_t *local_nx, ptrdiff_t *local_x_start)
{
     ptrdiff_t local_ny, local_y_start;
     return X(mpi_local_size_2d_transposed)(nx, ny,
					    FFTW_MPI_DEFAULT_BLOCK,
					    FFTW_MPI_DEFAULT_BLOCK,
					    comm,
					    local_nx, local_x_start,
					    &local_ny, &local_y_start);
}

X(plan) X(mpi_plan_transpose)(ptrdiff_t nx, ptrdiff_t ny, R *in, R *out, 
			      MPI_Comm comm, unsigned flags)
			      
{
     return X(mpi_plan_many_transpose)(nx, ny, 1,
				       FFTW_MPI_DEFAULT_BLOCK,
				       FFTW_MPI_DEFAULT_BLOCK,
				       in, out, comm, flags);
}

X(plan) X(mpi_plan_many_transpose)(ptrdiff_t nx, ptrdiff_t ny, 
				   ptrdiff_t howmany,
				   ptrdiff_t xblock, ptrdiff_t yblock,
				   R *in, R *out, 
				   MPI_Comm comm, unsigned flags)
{
     init();

     if (howmany < 0 || xblock < 0 || yblock < 0 ||
	 nx <= 0 || ny <= 0) return 0;

     if (!xblock) xblock = X(default_block)(nx, comm);
     if (!yblock) yblock = X(default_block)(ny, comm);

     return 
	  X(mkapiplan)(FFTW_FORWARD, flags,
		       X(mkproblem_mpi_transpose)(howmany, nx, ny,
						  in, out, xblock, yblock,
						  comm, MPI_FLAGS(flags)));
}
