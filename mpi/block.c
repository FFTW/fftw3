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

/* If the block size is small enough that n/block > n_pes, then
   we will interpret it as a block-cyclic distribution.  However,
   we don't have transpose solvers for block-cyclic yet. */
int X(is_block_cyclic)(INT n, INT block, MPI_Comm comm)
{
     int n_pes;
     MPI_Comm_size(comm, &n_pes);
     return ((n + block - 1) / block > n_pes);
}

/* Pick a default block size for dividing a problem of size n among
   the processors of the given communicator.   Divide as equally
   as possible, while minimizing the maximum block size among the
   processes as well as the number of processes with nonzero blocks. */
INT X(default_block)(INT n, MPI_Comm comm)
{
     int n_pes;
     MPI_Comm_size(comm, &n_pes);
     return ((n + n_pes - 1) / n_pes);
}

/* For a given block size and dimension n, compute the block size b
   and the starting offset s on the given process.   For block-cyclic
   format, this is the *total* size of the cyclic blocks on which_pe. */
INT X(some_block)(INT n, INT block, int which_pe, int n_pes)
{
     int n_blocks = (n + block - 1) / block;
     if (n_blocks > n_pes) { /* block-cyclic */
	  return (block * ((n_pes / n_blocks)
			   + (n_pes % n_blocks >= which_pe + 1))
		  + (((n_pes % n_blocks + n_pes - 1) % n_pes == which_pe)
		     * (n % block - block)));
     }
     else { /* ordinary block distribution */
	  n_pes = n_blocks;
	  if (which_pe >= n_pes)
	       return 0;
	  else
	       return ((which_pe == n_pes - 1)
		       ? (n - which_pe * block) : block);
     }
}

INT X(current_block)(INT n, INT block, MPI_Comm comm)
{
     int my_pe, n_pes;
     MPI_Comm_rank(comm, &my_pe);
     MPI_Comm_size(comm, &n_pes);
     return X(some_block)(n, block, my_pe, n_pes);
}
