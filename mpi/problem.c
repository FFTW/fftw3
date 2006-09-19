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

#include "mpi-dft.h"

static void destroy(problem *ego_)
{
     problem_mpi_dft *ego = (problem_mpi_dft *) ego_;
     X(ifree0)(ego->n);
     MPI_Comm_free(&ego->comm);
     X(ifree)(ego_);
}

static void hash(const problem *p_, md5 *m)
{
     const problem_mpi_dft *p = (const problem_mpi_dft *) p_;
     int i;
     X(md5puts)(m, "mpi-dft");
     X(md5int)(m, p->I == p->O);
     X(md5int)(m, X(alignment_of)(p->I));
     X(md5int)(m, X(alignment_of)(p->O));
     X(md5INT)(m, p->vn);
     X(md5int)(m, p->rnk);
     for (i = 0; i < p->rnk; ++i) X(md5INT)(m, p->n[i]);
     X(md5int)(m, p->sign);
     X(md5int)(m, p->flags);
     X(md5INT)(m, p->block);
     X(md5INT)(m, p->tblock);
     MPI_Comm_rank(p->comm, &i); X(md5int)(m, i);
     MPI_Comm_size(p->comm, &i); X(md5int)(m, i);
}

static void print(problem *ego_, printer *p)
{
     const problem_mpi_dft *ego = (const problem_mpi_dft *) ego_;
     int i;
     p->print(p, "(mpi-dft %d %d %d %D %d", 
	      ego->I == ego->O,
	      X(alignment_of)(ego->I),
	      X(alignment_of)(ego->O),
	      ego->vn,
	      ego->rnk);
     for (i = 0; i < ego->rnk; ++i) p->print(p, " %D", ego->n[i]);
     p->print(p, " %d %d %D %D",
	      ego->sign, ego->flags, ego->block, ego->tblock);
     MPI_Comm_rank(ego->comm, &i); p->print(p, " %d", i);
     MPI_Comm_size(ego->comm, &i); p->print(p, " %d)", i);
}

static void zero(const problem *ego_)
{
     const problem_mpi_dft *ego = (const problem_mpi_dft *) ego_;
     R *I = ego->I;
     INT i, N = ego->vn * 2;
     int irnk;

     N *= X(current_block)(ego->n[0], ego->block, ego->comm);
     for (irnk = 1; irnk < ego->rnk; ++irnk) N *= ego->n[irnk];

     for (i = 0; i < N; ++i) I[i] = K(0.0);
}

static const problem_adt padt =
{
     PROBLEM_MPI_DFT,
     hash,
     zero,
     print,
     destroy
};

problem *X(mkproblem_mpi_dft)(INT vn, int rnk, const INT *n,
                              R *I, R *O,
                              INT block, INT tblock,
                              MPI_Comm comm,
			      int sign,
                              int flags)
{
     problem_mpi_dft *ego =
          (problem_mpi_dft *)X(mkproblem)(sizeof(problem_mpi_dft), &padt);
     int i;
     int use_tblock;

     A(rnk > 0);
     A(n);
     for (i = 0; i < rnk; ++i) A(n[i] > 0);
     A(vn > 0);
     A(block > 0 && block <= n[0]);
     A(sign == -1 || sign == 1);
     A(!(TRANSPOSED & flags) || rnk >= 2);
     use_tblock = (TRANSPOSED & flags) && rnk >= 2;
     A((use_tblock && tblock > 0 && tblock <= n[1]) 
       || (!use_tblock && block == tblock));

     /* enforce pointer equality if untainted pointers are equal */
     if (UNTAINT(I) == UNTAINT(O))
	  I = O = JOIN_TAINT(I, O);

     /* Note that we *don't* compress the rank by removing n=1 dimensions,
	because the output format depends on the dimensionality.  Rank
        compression will occur where it matters, in the serial FFTW. */

     ego->rnk = rnk;
     ego->n = (INT *) MALLOC(rnk * sizeof(INT), TENSORS);
     for (i = 0; i < rnk; ++i) ego->n[i] = n[i];

     ego->vn = vn;
     ego->I = I;
     ego->O = O;
     ego->sign = sign;
     ego->flags = flags;
     ego->block = block;
     ego->tblock = tblock;

     MPI_Comm_dup(comm, &ego->comm);

     return &(ego->super);
}
