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

/* $Id: problem.c,v 1.23 2002-09-01 23:22:54 athena Exp $ */

#include "rdft.h"
#include <stddef.h>

static void destroy(problem *ego_)
{
     problem_rdft *ego = (problem_rdft *) ego_;
     X(tensor_destroy)(ego->vecsz);
     X(tensor_destroy)(ego->sz);
     X(free)(ego_);
}

static void kind_hash(md5 *m, const rdft_kind *kind, uint rnk)
{
     uint i;
     for (i = 0; i < rnk; ++i)
	  X(md5int)(m, kind[i]);
}

static void hash(const problem *p_, md5 *m)
{
     const problem_rdft *p = (const problem_rdft *) p_;
     X(md5int)(m, p->I == p->O);
     kind_hash(m, p->kind, p->sz.rnk);
     X(md5uint)(m, X(alignment_of)(p->I));
     X(md5uint)(m, X(alignment_of)(p->O));
     X(tensor_md5)(m, p->sz);
     X(tensor_md5)(m, p->vecsz);
}

static int kind_equal(const rdft_kind *k1, const rdft_kind *k2, uint rnk)
{
     uint i;
     for (i = 0; i < rnk; ++i)
	  if (k1[i] != k2[i])
	       return 0;
     return 1;
}

static int equal(const problem *ego_, const problem *problem_)
{
     if (ego_->adt == problem_->adt) {
          const problem_rdft *e = (const problem_rdft *) ego_;
          const problem_rdft *p = (const problem_rdft *) problem_;

          return (1

		  /* both in-place or both out-of-place */
                  && ((p->I == p->O) == (e->I == e->O))

		  /* aligments must match */
		  && X(alignment_of)(p->I) == X(alignment_of)(e->I)
		  && X(alignment_of)(p->O) == X(alignment_of)(e->O)

		  && X(tensor_equal)(p->sz, e->sz)
                  && X(tensor_equal)(p->vecsz, e->vecsz)

		  && kind_equal(p->kind, e->kind, p->sz.rnk)
	       );
     }
     return 0;
}

void X(rdft_zerotens)(tensor sz, R *I)
{
     if (sz.rnk == RNK_MINFTY)
          return;
     else if (sz.rnk == 0)
          I[0] = 0.0;
     else if (sz.rnk == 1) {
          /* this case is redundant but faster */
          uint i, n = sz.dims[0].n;
          int is = sz.dims[0].is;

          for (i = 0; i < n; ++i)
               I[i * is] = 0.0;
     } else if (sz.rnk > 0) {
          uint i, n = sz.dims[0].n;
          int is = sz.dims[0].is;

          sz.dims++;
          sz.rnk--;
          for (i = 0; i < n; ++i)
               X(rdft_zerotens)(sz, I + i * is);
     }
}

#define KSTR_LEN 8

const char *X(rdft_kind_str)(rdft_kind kind)
{
     static const char kstr[][KSTR_LEN] = {
	  "r2hc", "r2hc01", "r2hc10", "r2hc11",
	  "hc2r", "hc2r01", "hc2r10", "hc2r11",
	  "dht",
	  "redft00", "redft01", "redft10", "redft11",
	  "rodft00", "rodft01", "rodft10", "rodft11"
     };
     A(kind >= 0 && kind < sizeof(kstr) / KSTR_LEN);
     return kstr[kind];
}

/* for a given transform order n, return the actual number of
   real values in the data input/output arrays.  This is
   less than n for real-{even,odd} transforms. */
uint X(rdft_real_n)(rdft_kind kind, uint n)
{
     switch (kind) {
	 case R2HC00: case R2HC01: case R2HC10: case R2HC11:
	 case HC2R00: case HC2R01: case HC2R10: case HC2R11:
	 case DHT:
	      return n;

	 case REDFT00: return n/2 + 1;
	 case RODFT00: return (n + 1)/2 - 1;

	 case REDFT01: case REDFT10: case REDFT11: return (n + 1)/2;
	 case RODFT01: case RODFT10: case RODFT11: return n/2;

	 default: A(0); return 0;
     }
}

tensor X(rdft_real_sz)(const rdft_kind *kind, const tensor sz)
{
     uint i;
     tensor sz_real;
     sz_real = X(tensor_copy)(sz);
     for (i = 0; i < sz.rnk; ++i)
	  sz_real.dims[i].n = X(rdft_real_n)(kind[i], sz_real.dims[i].n);
     return sz_real;
}

static void print(problem *ego_, printer *p)
{
     const problem_rdft *ego = (const problem_rdft *) ego_;
     uint i;
     p->print(p, "(rdft %u %td %T %T", 
	      X(alignment_of)(ego->I),
	      ego->O - ego->I, 
	      &ego->sz,
	      &ego->vecsz);
     for (i = 0; i < ego->sz.rnk; ++i)
	  p->print(p, " %d", (int)ego->kind[i]);
     p->print(p, ")");
}

static int scan(scanner *sc, problem **p)
{
     tensor sz = { 0, 0 }, vecsz = { 0, 0 };
     uint align, i;
     ptrdiff_t offio;
     rdft_kind *kind = (rdft_kind *) 0;
     R *I;
     
     if (!sc->scan(sc, "%u %td %T %T",
		   &align, &offio, &sz, &vecsz))
	  goto nada;
     kind = (rdft_kind *) fftw_malloc(sizeof(rdft_kind) * sz.rnk, OTHER);
     for (i = 0; i < sz.rnk; ++i) {
	  int k;
	  if (!sc->scan(sc, " %d", &k))
	       goto nada;
	  kind[i] = (rdft_kind) k; /* FIXME: check range? */
     }
     if (!sc->scan(sc,")"))
	  goto nada;
     
     I = X(ptr_with_alignment)(align);
     *p = X(mkproblem_rdft_d)(sz, vecsz, I, I + offio, kind);
     X(free)(kind);
     return 1;

 nada:
     if (kind)
	  X(free)(kind);
     X(tensor_destroy)(sz);
     X(tensor_destroy)(vecsz);
     return 0;
}

static void zero(const problem *ego_)
{
     const problem_rdft *ego = (const problem_rdft *) ego_;
     tensor rsz = X(rdft_real_sz)(ego->kind, ego->sz);
     tensor sz = X(tensor_append)(ego->vecsz, rsz);
     X(rdft_zerotens)(sz, ego->I);
     X(tensor_destroy)(sz);
     X(tensor_destroy)(rsz);
}

static const problem_adt padt =
{
     equal,
     hash,
     zero,
     print,
     destroy,
     scan,
     "rdft"
};

int X(problem_rdft_p)(const problem *p)
{
     return (p->adt == &padt);
}

problem *X(mkproblem_rdft)(const tensor sz, const tensor vecsz,
			   R *I, R *O, const rdft_kind *kind)
{
     problem_rdft *ego =
          (problem_rdft *)X(mkproblem)(sizeof(problem_rdft)
				       + sizeof(rdft_kind)
				       * (X(uimax)(1, sz.rnk) - 1), 
				       &padt);

     uint i, total_n = 1;
     for (i = 0; i < sz.rnk; ++i)
	  total_n *= X(rdft_real_n)(kind[i], sz.dims[i].n);
     A(total_n > 0); /* or should we use vecsz RNK_MINFTY? */

     /* FIXME: how are shifted transforms compressed? */
     ego->sz = X(tensor_compress)(sz);
     ego->vecsz = X(tensor_compress_contiguous)(vecsz);
     ego->I = I;
     ego->O = O;
     for (i = 0; i < sz.rnk; ++i)
	  ego->kind[i] = kind[i];

     A(FINITE_RNK(ego->sz.rnk));
     return &(ego->super);
}

/* Same as X(mkproblem_rdft), but also destroy input tensors. */
problem *X(mkproblem_rdft_d)(tensor sz, tensor vecsz,
			     R *I, R *O, const rdft_kind *kind)
{
     problem *p;
     p = X(mkproblem_rdft)(sz, vecsz, I, O, kind);
     X(tensor_destroy)(vecsz);
     X(tensor_destroy)(sz);
     return p;
}

/* As above, but for rnk == 1 only and takes a scalar kind parameter */
problem *X(mkproblem_rdft_1)(const tensor sz, const tensor vecsz,
			     R *I, R *O, rdft_kind kind)
{
     A(sz.rnk <= 1);
     return X(mkproblem_rdft)(sz, vecsz, I, O, &kind);
}

problem *X(mkproblem_rdft_1_d)(tensor sz, tensor vecsz,
			       R *I, R *O, rdft_kind kind)
{
     A(sz.rnk <= 1);
     return X(mkproblem_rdft_d)(sz, vecsz, I, O, &kind);
}

void X(problem_rdft_register)(planner *p)
{
     REGISTER_PROBLEM(p, &padt);
}

