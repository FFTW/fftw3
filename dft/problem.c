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

/* $Id: problem.c,v 1.15 2002-08-01 18:56:02 stevenj Exp $ */

#include "dft.h"
#include <stddef.h>

static void destroy(problem *ego_)
{
     problem_dft *ego = (problem_dft *) ego_;
     X(tensor_destroy)(ego->vecsz);
     X(tensor_destroy)(ego->sz);
     X(free)(ego_);
}

static unsigned int hash(const problem *p_)
{
     const problem_dft *p = (const problem_dft *) p_;
     return (0
	     ^ ((p->ri == p->ro) * 17)
	     ^ ((p->ii - p->ri) * 19)
	     ^ ((p->io - p->ro) * 23)
	     ^ (X(tensor_hash)(p->sz) * 10477)
             ^ (X(tensor_hash)(p->vecsz) * 27191)
	  );
}

static int equal(const problem *ego_, const problem *problem_)
{
     if (ego_->adt == problem_->adt) {
          const problem_dft *e = (const problem_dft *) ego_;
          const problem_dft *p = (const problem_dft *) problem_;

          return (1

		  /* both in-place or both out-of-place */
                  && ((p->ri == p->ro) == (e->ri == e->ro))

		  /* distance between real and imag must be the same (1) */
		  && p->ii - p->ri == e->ii - e->ri

		  /* idem for output */
		  && p->io - p->ro == e->io - e->ro

		  /* aligments must match */
		  && X(alignment_of)(p->ri) == X(alignment_of)(e->ri)
		  && X(alignment_of)(p->ro) == X(alignment_of)(e->ro)
		  /* alignments for imag are implied by (1) */

		  && X(tensor_equal)(p->sz, e->sz)
                  && X(tensor_equal)(p->vecsz, e->vecsz)
	       );
     }
     return 0;
}

void X(dft_zerotens)(tensor sz, R *ri, R *ii)
{
     if (sz.rnk == RNK_MINFTY)
          return;
     else if (sz.rnk == 0)
          ri[0] = ii[0] = 0.0;
     else if (sz.rnk == 1) {
          /* this case is redundant but faster */
          uint i, n = sz.dims[0].n;
          int is = sz.dims[0].is;

          for (i = 0; i < n; ++i)
               ri[i * is] = ii[i * is] = 0.0;
     } else if (sz.rnk > 0) {
          uint i, n = sz.dims[0].n;
          int is = sz.dims[0].is;

          sz.dims++;
          sz.rnk--;
          for (i = 0; i < n; ++i)
               X(dft_zerotens)(sz, ri + i * is, ii + i * is);
     }
}

static void print(problem *ego_, printer *p)
{
     const problem_dft *ego = (const problem_dft *) ego_;
     p->print(p, "(dft %u %td %td %td %T %T)", 
	      X(alignment_of)(ego->ri),
	      ego->ro - ego->ri, 
	      ego->ii - ego->ri, 
	      ego->io - ego->ro,
	      &ego->sz,
	      &ego->vecsz);
}

static int scan(scanner *sc, problem **p)
{
     tensor sz = { 0, 0 }, vecsz = { 0, 0 };
     uint align;
     ptrdiff_t offio, offi, offo;
     R *ri, *ro;

     if (!sc->scan(sc, "%u %td %td %td %T %T",
		   &align, &offio, &offi, &offo, &sz, &vecsz)) {
	  X(tensor_destroy)(sz);
	  X(tensor_destroy)(vecsz);
	  return 0;
     }
     ri = (R *) ((char *) 0 + align);
     ro = ri + offio;
     *p = X(mkproblem_dft_d)(sz, vecsz, ri, ri + offi, ro, ro + offo);
     return 1;
}

static void zero(const problem *ego_)
{
     const problem_dft *ego = (const problem_dft *) ego_;
     tensor sz = X(tensor_append)(ego->vecsz, ego->sz);
     X(dft_zerotens)(sz, ego->ri, ego->ii);
     X(tensor_destroy)(sz);
}

static const problem_adt padt =
{
     equal,
     hash,
     zero,
     print,
     destroy,
     scan,
     "dft"
};

int X(problem_dft_p)(const problem *p)
{
     return (p->adt == &padt);
}

problem *X(mkproblem_dft)(const tensor sz, const tensor vecsz,
                          R *ri, R *ii, R *ro, R *io)
{
     problem_dft *ego =
          (problem_dft *)X(mkproblem)(sizeof(problem_dft), &padt);

     /* both in place or both out of place */
     CK((ri == ro) == (ii == io));

     ego->sz = X(tensor_compress)(sz);
     ego->vecsz = X(tensor_compress_contiguous)(vecsz);
     ego->ri = ri;
     ego->ii = ii;
     ego->ro = ro;
     ego->io = io;

     A(FINITE_RNK(ego->sz.rnk));
     return &(ego->super);
}

/* Same as X(mkproblem_dft), but also destroy input tensors. */
problem *X(mkproblem_dft_d)(tensor sz, tensor vecsz,
                            R *ri, R *ii, R *ro, R *io)
{
     problem *p;
     p = X(mkproblem_dft)(sz, vecsz, ri, ii, ro, io);
     X(tensor_destroy)(vecsz);
     X(tensor_destroy)(sz);
     return p;
}

void X(problem_dft_register)(planner *p)
{
     REGISTER_PROBLEM(p, &padt);
}
