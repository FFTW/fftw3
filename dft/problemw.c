/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* $Id: problemw.c,v 1.1 2003-05-15 23:09:07 athena Exp $ */

#include "dft.h"
#include <stddef.h>

static void destroy(problem *ego)
{
     X(ifree)(ego);
}

static void hash(const problem *p_, md5 *m)
{
     const problem_dftw *p = (const problem_dftw *) p_;
     X(md5puts)(m, "dftw");
     X(md5ptrdiff)(m, p->iio - p->rio);
     X(md5int)(m, X(alignment_of)(p->rio));
     X(md5int)(m, X(alignment_of)(p->iio));
     X(md5int)(m, p->dec);
     X(md5int)(m, p->r);
     X(md5int)(m, p->m);
     X(md5int)(m, p->s);
     X(md5int)(m, p->ws);
     X(md5int)(m, p->vl);
     X(md5int)(m, p->vs);
     X(md5int)(m, p->wvs);
}

static void print(problem *ego_, printer *p)
{
     const problem_dftw *ego = (const problem_dftw *) ego_;
     p->print(p, "(dftw %s %d %td (%d %d %d %d) (%d %d %d))", 
	      ego->dec == DECDIF ? "dif" : "dit",
	      X(alignment_of)(ego->rio),
	      ego->iio - ego->rio, 
	      ego->r, ego->m, ego->s, ego->ws,
	      ego->vl, ego->vs, ego->wvs);
}

static void zero(const problem *ego_)
{
     const problem_dftw *ego = (const problem_dftw *) ego_;
     tensor *sz0 = X(mktensor_1d)(ego->m * ego->r, ego->s, ego->ws);
     tensor *sz1 = X(mktensor_1d)(ego->vl, ego->vs, ego->wvs);
     tensor *sz = X(tensor_append)(sz1, sz0);
     X(dft_zerotens)(sz, UNTAINT(ego->rio), UNTAINT(ego->iio));
     X(tensor_destroy2)(sz0, sz1);
     X(tensor_destroy)(sz);
}

static const problem_adt padt =
{
     hash,
     zero,
     print,
     destroy
};

int X(problem_dftw_p)(const problem *p)
{
     return (p->adt == &padt);
}

problem *X(mkproblem_dftw)(int dec, int r, int m, 
			   int s, int ws,
			   int vl, int vs, int wvs,
			   R *rio, R *iio)
{
     problem_dftw *ego =
          (problem_dftw *)X(mkproblem)(sizeof(problem_dftw), &padt);

     A(TAINTOF(rio) == TAINTOF(iio));
     A(m > 1 && r > 1);  /* FIXME: should we allow this case,
			    canonicalizing to problem_dft? */

     ego->dec = dec;
     ego->r = r; ego->m = m; ego->s = s; ego->ws = ws;
     ego->vl = vl; ego->vs = vs; ego->wvs = wvs;

     ego->rio = rio;
     ego->iio = iio;

     return &(ego->super);
}
