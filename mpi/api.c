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
#include "fftw3-mpi.h"
#include "ifftw-mpi.h"
#include "mpi-transpose.h"
#include "mpi-dft.h"

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

/*************************************************************************/

ptrdiff_t X(mpi_local_size_many_transposed)(int rnk, const ptrdiff_t *n,
					    ptrdiff_t howmany,
					    ptrdiff_t xblock, ptrdiff_t yblock,
					    MPI_Comm comm,
					    ptrdiff_t *local_nx,
					    ptrdiff_t *local_x_start,
					    ptrdiff_t *local_ny, 
					    ptrdiff_t *local_y_start)
{
     int my_pe;
     MPI_Comm_rank(comm, &my_pe);
     if (rnk > 1) {
	  INT nx = n[0], ny = n[1];
	  int i;
	  for (i = 2; i < rnk; ++i) howmany *= n[i];
	  if (!xblock) xblock = X(default_block)(nx, comm);
	  if (!yblock) yblock = X(default_block)(ny, comm);
	  *local_nx = X(current_block)(nx, xblock, comm);
	  *local_ny = X(current_block)(ny, yblock, comm);
	  *local_x_start = *local_nx == 0 ? 0 : xblock * my_pe;
	  *local_y_start = *local_ny == 0 ? 0 : yblock * my_pe;
	  return X(imax)(*local_nx * ny, *local_ny * nx) * howmany;
     }
     else {
	  A(0); /* unimplemented */
	  return 0;
     }
}

ptrdiff_t X(mpi_local_size_many)(int rnk, const ptrdiff_t *n,
				 ptrdiff_t howmany,
				 ptrdiff_t xblock, ptrdiff_t yblock,
				 MPI_Comm comm,
				 ptrdiff_t *local_nx,
				 ptrdiff_t *local_x_start)
{
     ptrdiff_t local_ny, local_y_start;
     return X(mpi_local_size_many_transposed)(rnk, n, howmany,
					      xblock, yblock, comm,
					      local_nx, local_x_start,
					      &local_ny, &local_y_start);
}

ptrdiff_t X(mpi_local_size_transposed)(int rnk, const ptrdiff_t *n,
				       MPI_Comm comm,
				       ptrdiff_t *local_nx,
				       ptrdiff_t *local_x_start,
				       ptrdiff_t *local_ny,
				       ptrdiff_t *local_y_start)
{
     return X(mpi_local_size_many_transposed)(rnk, n, 1,
					      FFTW_MPI_DEFAULT_BLOCK,
					      FFTW_MPI_DEFAULT_BLOCK,
					      comm, local_nx, local_x_start,
					      local_ny, local_y_start);
}

ptrdiff_t X(mpi_local_size)(int rnk, const ptrdiff_t *n,
			    MPI_Comm comm,
			    ptrdiff_t *local_nx,
			    ptrdiff_t *local_x_start)
{
     ptrdiff_t local_ny, local_y_start;
     return X(mpi_local_size_transposed)(rnk, n, comm,
					 local_nx, local_x_start,
					 &local_ny, &local_y_start);
}

ptrdiff_t X(mpi_local_size_1d)(ptrdiff_t nx, MPI_Comm comm,
			       ptrdiff_t *local_nx, ptrdiff_t *local_x_start)
{
     return X(mpi_local_size)(1, &nx, comm, local_nx, local_x_start);
}

ptrdiff_t X(mpi_local_size_2d_transposed)(ptrdiff_t nx, ptrdiff_t ny,
					  MPI_Comm comm,
					  ptrdiff_t *local_nx,
					  ptrdiff_t *local_x_start,
					  ptrdiff_t *local_ny, 
					  ptrdiff_t *local_y_start)
{
     ptrdiff_t n[2];
     n[0] = nx; n[1] = ny;
     return X(mpi_local_size_transposed)(2, n, comm,
					 local_nx, local_x_start,
					 local_ny, local_y_start);
}

ptrdiff_t X(mpi_local_size_2d)(ptrdiff_t nx, ptrdiff_t ny, MPI_Comm comm,
			       ptrdiff_t *local_nx, ptrdiff_t *local_x_start)
{
     ptrdiff_t n[2];
     n[0] = nx; n[1] = ny;
     return X(mpi_local_size)(2, n, comm, local_nx, local_x_start);
}

ptrdiff_t X(mpi_local_size_3d_transposed)(ptrdiff_t nx, ptrdiff_t ny,
					  ptrdiff_t nz,
					  MPI_Comm comm,
					  ptrdiff_t *local_nx,
					  ptrdiff_t *local_x_start,
					  ptrdiff_t *local_ny, 
					  ptrdiff_t *local_y_start)
{
     ptrdiff_t n[3];
     n[0] = nx; n[1] = ny; n[2] = nz;
     return X(mpi_local_size_transposed)(3, n, comm,
					 local_nx, local_x_start,
					 local_ny, local_y_start);
}

ptrdiff_t X(mpi_local_size_3d)(ptrdiff_t nx, ptrdiff_t ny, ptrdiff_t nz,
			       MPI_Comm comm,
			       ptrdiff_t *local_nx, ptrdiff_t *local_x_start)
{
     ptrdiff_t n[3];
     n[0] = nx; n[1] = ny; n[2] = nz;
     return X(mpi_local_size)(3, n, comm, local_nx, local_x_start);
}

/*************************************************************************/
/* Transpose API */

X(plan) X(mpi_plan_many_transpose)(ptrdiff_t nx, ptrdiff_t ny, 
				   ptrdiff_t howmany,
				   ptrdiff_t xblock, ptrdiff_t yblock,
				   R *in, R *out, 
				   MPI_Comm comm, unsigned flags)
{
     init();

     if (howmany < 0 || xblock < 0 || yblock < 0 ||
	 nx <= 0 || ny <= 0 || flags & FFTW_MPI_TRANSPOSED) return 0;

     if (!xblock) xblock = X(default_block)(nx, comm);
     if (!yblock) yblock = X(default_block)(ny, comm);

     return 
	  X(mkapiplan)(FFTW_FORWARD, flags,
		       X(mkproblem_mpi_transpose)(howmany, nx, ny,
						  in, out, xblock, yblock,
						  comm, MPI_FLAGS(flags)));
}

X(plan) X(mpi_plan_transpose)(ptrdiff_t nx, ptrdiff_t ny, R *in, R *out, 
			      MPI_Comm comm, unsigned flags)
			      
{
     return X(mpi_plan_many_transpose)(nx, ny, 1,
				       FFTW_MPI_DEFAULT_BLOCK,
				       FFTW_MPI_DEFAULT_BLOCK,
				       in, out, comm, flags);
}

/*************************************************************************/
/* Complex DFT API */

X(plan) X(mpi_plan_many_dft)(int rnk, const ptrdiff_t *n,
			     ptrdiff_t howmany,
			     ptrdiff_t xblock, ptrdiff_t yblock,
			     C *in, C *out,
			     MPI_Comm comm, int sign, unsigned flags)
{
     int i;

     init();

     if (howmany < 0 || xblock < 0 || yblock < 0 || rnk < 1) return 0;
     for (i = 0; i < rnk; ++i) if (n[i] < 1) return 0;
     if (rnk == 1 && (flags & FFTW_MPI_TRANSPOSED)) return 0;

     if (!xblock) xblock = X(default_block)(n[0], comm);
     if (!yblock && rnk > 1) yblock = X(default_block)(n[1], comm);

     return
          X(mkapiplan)(sign, flags,
                       X(mkproblem_mpi_dft)(howmany, rnk, n,
					    (R *) in, (R *) out,
					    xblock, yblock,
					    comm, sign, MPI_FLAGS(flags)));
}

X(plan) X(mpi_plan_dft)(int rnk, const ptrdiff_t *n, C *in, C *out,
			MPI_Comm comm, int sign, unsigned flags)
{
     return X(mpi_plan_many_dft)(rnk, n, 1,
				 FFTW_MPI_DEFAULT_BLOCK,
				 FFTW_MPI_DEFAULT_BLOCK,
				 in, out, comm, sign, flags);
}

X(plan) X(mpi_plan_dft_1d)(ptrdiff_t nx, C *in, C *out,
			   MPI_Comm comm, int sign, unsigned flags)
{
     return X(mpi_plan_dft)(1, &nx, in, out, comm, sign, flags);
}

X(plan) X(mpi_plan_dft_2d)(ptrdiff_t nx, ptrdiff_t ny, C *in, C *out,
			   MPI_Comm comm, int sign, unsigned flags)
{
     ptrdiff_t n[2];
     n[0] = nx; n[1] = ny;
     return X(mpi_plan_dft)(2, n, in, out, comm, sign, flags);
}

X(plan) X(mpi_plan_dft_3d)(ptrdiff_t nx, ptrdiff_t ny, ptrdiff_t nz,
			   C *in, C *out,
			   MPI_Comm comm, int sign, unsigned flags)
{
     ptrdiff_t n[3];
     n[0] = nx; n[1] = ny; n[2] = nz;
     return X(mpi_plan_dft)(3, n, in, out, comm, sign, flags);
}
