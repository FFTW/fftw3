/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

/* $Id: problem.c,v 1.3 2002-06-03 12:09:05 athena Exp $ */

#include "dft.h"

static void destroy(problem *ego_)
{
     problem_dft *ego = (problem_dft *) ego_;
     fftw_tensor_destroy(ego->vecsz);
     fftw_tensor_destroy(ego->sz);
     fftw_free(ego_);
}

static unsigned int hash(const problem *ego_)
{
     const problem_dft *ego = (const problem_dft *) ego_;
     return (fftw_tensor_hash(ego->sz) * 31415 +
	     fftw_tensor_hash(ego->vecsz) * 27183);
}

static int equal(const problem *ego_, const problem *problem_)
{
     if (ego_->adt == problem_->adt) {
	  const problem_dft *e = (const problem_dft *) ego_;
	  const problem_dft *p = (const problem_dft *) problem_;

	  return (fftw_tensor_equal(p->sz, e->sz) &&
		  fftw_tensor_equal(p->vecsz, e->vecsz) &&
		  ((p->ri == p->ro) == (e->ri == e->ro)));
     }
     return 0;
}

static void zerotens(tensor sz, R *ri, R *ii)
{
     if (sz.rnk == 0)
	  ri[0] = ii[0] = 0;
     else if (sz.rnk == 1) {
	  /* this case is redundant */
	  int i, n = sz.dims[0].n, is = sz.dims[0].is;

	  for (i = 0; i < n; ++i)
	       ri[i * is] = ii[i * is] = 0.0;
     } else if (sz.rnk > 0) {
	  int i, n = sz.dims[0].n, is = sz.dims[0].is;

	  sz.dims++;
	  sz.rnk--;
	  for (i = 0; i < n; ++i)
	       zerotens(sz, ri + i * is, ii + i * is);
     }
}

static void zero(const problem *ego_)
{
     const problem_dft *ego = (const problem_dft *) ego_;
     tensor sz = fftw_tensor_append(ego->vecsz, ego->sz);
     zerotens(sz, ego->ri, ego->ii);
     fftw_tensor_destroy(sz);
}

static const problem_adt padt = {
     equal,
     hash,
     zero,
     destroy
};

int fftw_problem_dft_p(const problem *p)
{
     return (p->adt == &padt);
}

problem *fftw_mkproblem_dft(const tensor sz, const tensor vecsz,
			    R *ri, R *ii, R *ro, R *io)
{
     problem_dft *ego =
	  (problem_dft *)fftw_mkproblem(sizeof(problem_dft), &padt);

     /* both in place or both out of place */
     CK((ri == ro) == (ii == io));

     ego->sz = fftw_tensor_compress(sz);
     ego->vecsz = fftw_tensor_compress_contiguous(vecsz);
     ego->ri = ri;
     ego->ii = ii;
     ego->ro = ro;
     ego->io = io;

     return &(ego->super);
}

/* Same as fftw_mkproblem_dft, but also destroy input tensors. */
problem *fftw_mkproblem_dft_d(tensor sz, tensor vecsz,
			      R *ri, R *ii, R *ro, R *io)
{
     problem *p;
     p = fftw_mkproblem_dft(sz, vecsz, ri, ii, ro, io);
     fftw_tensor_destroy(vecsz);
     fftw_tensor_destroy(sz);
     return p;
}
