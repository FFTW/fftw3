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

/* $Id: problem.c,v 1.24 2002-09-22 19:00:59 athena Exp $ */

#include "dft.h"
#include <stddef.h>

static void destroy(problem *ego_)
{
     problem_dft *ego = (problem_dft *) ego_;
     X(tensor_destroy2)(ego->vecsz, ego->sz);
     X(free)(ego_);
}

static void hash(const problem *p_, md5 *m)
{
     const problem_dft *p = (const problem_dft *) p_;
     X(md5puts)(m, "dft");
     X(md5int)(m, p->ri == p->ro);
     X(md5ptrdiff)(m, p->ii - p->ri);
     X(md5ptrdiff)(m, p->io - p->ro);
     X(md5uint)(m, X(alignment_of)(p->ri));
     X(md5uint)(m, X(alignment_of)(p->ro));
     X(tensor_md5)(m, p->sz);
     X(tensor_md5)(m, p->vecsz);
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

static void zero(const problem *ego_)
{
     const problem_dft *ego = (const problem_dft *) ego_;
     tensor *sz = X(tensor_append)(ego->vecsz, ego->sz);
     X(dft_zerotens)(sz, ego->ri, ego->ii);
     X(tensor_destroy)(sz);
}

static const problem_adt padt =
{
     hash,
     zero,
     print,
     destroy
};

int X(problem_dft_p)(const problem *p)
{
     return (p->adt == &padt);
}

problem *X(mkproblem_dft)(const tensor *sz, const tensor *vecsz,
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

     A(FINITE_RNK(ego->sz->rnk));
     return &(ego->super);
}

/* Same as X(mkproblem_dft), but also destroy input tensors. */
problem *X(mkproblem_dft_d)(tensor *sz, tensor *vecsz,
                            R *ri, R *ii, R *ro, R *io)
{
     problem *p;
     p = X(mkproblem_dft)(sz, vecsz, ri, ii, ro, io);
     X(tensor_destroy2)(vecsz, sz);
     return p;
}
