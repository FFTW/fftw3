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

#include "mpi_transpose.h"

static void destroy(problem *ego_)
{
     problem_mpi_transpose *ego = (problem_mpi_transpose *) ego_;
     MPI_Comm_free(&ego->comm);
     X(ifree)(ego_);
}

static void hash(const problem *p_, md5 *m)
{
     const problem_mpi_transpose *p = (const problem_mpi_transpose *) p_;
     int i;
     X(md5puts)(m, "mpi-transpose");
     X(md5int)(m, p->I == p->O);
     X(md5int)(m, X(alignment_of)(p->I));
     X(md5int)(m, X(alignment_of)(p->O));
     X(md5INT)(m, p->vn);
     X(md5INT)(m, p->nx);
     X(md5INT)(m, p->ny);
     X(md5INT)(m, p->block);
     X(md5INT)(m, p->tblock);
     MPI_Comm_rank(p->comm, &i); X(md5int)(m, i);
     MPI_Comm_size(p->comm, &i); X(md5int)(m, i);
}

static void print(problem *ego_, printer *p)
{
     const problem_mpi_transpose *ego = (const problem_mpi_transpose *) ego_;
     int i;
     p->print(p, "(mpi-transpose %d %d %d %D %D %D %D %D", 
	      ego->I == ego->O,
	      X(alignment_of)(ego->I),
	      X(alignment_of)(ego->O),
	      ego->vn,
	      ego->nx, ego->ny,
	      ego->block, ego->tblock);
     MPI_Comm_rank(ego->comm, &i); p->print(p, " %d", i);
     MPI_Comm_size(ego->comm, &i); p->print(p, " %d)", i);
}

static void zero(const problem *ego_)
{
     const problem_mpi_transpose *ego = (const problem_mpi_transpose *) ego_;
     R *I = ego->I;
     INT i, N = ego->vn * ego->ny;

     N *= X(current_block)(ego->nx, ego->block, ego->comm);

     for (i = 0; i < N; ++i) I[i] = K(0.0);
}

static const problem_adt padt =
{
     PROBLEM_MPI_TRANSPOSE,
     hash,
     zero,
     print,
     destroy
};

problem *X(mkproblem_mpi_transpose)(INT vn, INT nx, INT ny,
				    R *I, R *O,
				    INT block, INT tblock,
				    MPI_Comm comm,
				    int flags)
{
     problem_mpi_transpose *ego =
          (problem_mpi_transpose *)X(mkproblem)(sizeof(problem_mpi_transpose), &padt);

     A(rnk > 0);
     A(nx > 0 && ny > 0);
     A(vn > 0);
     A(block > 0 && block <= nx && tblock > 0 && tblock <= ny);
     A(!(flags & TRANSPOSED)); /* flag not permitted */

     /* enforce pointer equality if untainted pointers are equal */
     if (UNTAINT(I) == UNTAINT(O))
	  I = O = JOIN_TAINT(I, O);

     ego->nx = nx;
     ego->ny = ny;
     ego->vn = vn;
     ego->I = I;
     ego->O = O;
     ego->flags = flags;
     ego->block = block;
     ego->tblock = tblock;

     MPI_Comm_dup(comm, &ego->comm);

     return &(ego->super);
}
