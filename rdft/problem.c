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

/* $Id: problem.c,v 1.15 2002-08-05 18:17:58 stevenj Exp $ */

#include "rdft.h"
#include <stddef.h>

static void destroy(problem *ego_)
{
     problem_rdft *ego = (problem_rdft *) ego_;
     X(tensor_destroy)(ego->vecsz);
     X(tensor_destroy)(ego->sz);
     X(free)(ego_);
}

static unsigned int hash(const problem *p_)
{
     const problem_rdft *p = (const problem_rdft *) p_;
     return (0xDEADBEEF
	     ^ ((p->I == p->O) * 31)
	     ^ (p->kind * 37)
	     ^ (X(tensor_hash)(p->sz) * 10487)
             ^ (X(tensor_hash)(p->vecsz) * 27197)
	  );
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

		  && p->kind == e->kind

		  && X(tensor_equal)(p->sz, e->sz)
                  && X(tensor_equal)(p->vecsz, e->vecsz)
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

const char *X(rdft_kind_str)(rdft_kind kind)
{
     switch (kind) {
	 case R2HC00: return "r2hc";
	 case R2HC01: return "r2hc01";
	 case R2HC10: return "r2hc10";
	 case R2HC11: return "r2hc11";

	 case HC2R00: return "hc2r";
	 case HC2R01: return "hc2r01";
	 case HC2R10: return "hc2r10";
	 case HC2R11: return "hc2r11";

	 case DHT: return "dht";

	 case REDFT00: return "redft00";
	 case REDFT01: return "redft01";
	 case REDFT10: return "redft10";
	 case REDFT11: return "redft11";

	 case RODFT00: return "rodft00";
	 case RODFT01: return "rodft01";
	 case RODFT10: return "rodft10";
	 case RODFT11: return "rodft11";

	 default: A(0); return 0;
     }
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

tensor X(rdft_real_sz)(rdft_kind kind, const tensor sz)
{
     uint i;
     tensor sz_real;
     sz_real = X(tensor_copy)(sz);
     for (i = 0; i < sz.rnk; ++i)
	  sz_real.dims[i].n = X(rdft_real_n)(kind, sz_real.dims[i].n);
     return sz_real;
}

static void print(problem *ego_, printer *p)
{
     const problem_rdft *ego = (const problem_rdft *) ego_;
     p->print(p, "(rdft %u %td %d %T %T)", 
	      X(alignment_of)(ego->I),
	      ego->O - ego->I, 
	      (int)ego->kind,
	      &ego->sz,
	      &ego->vecsz);
}

static int scan(scanner *sc, problem **p)
{
     tensor sz = { 0, 0 }, vecsz = { 0, 0 };
     uint align;
     ptrdiff_t offio;
     int kind;
     R *I;
     
     if (!sc->scan(sc, "%u %td %d %T %T",
		   &align, &offio, &kind, &sz, &vecsz)) {
	  X(tensor_destroy)(sz);
	  X(tensor_destroy)(vecsz);
	  return 0;
     }
     I = (R *) ((char *) 0 + align);
     *p = X(mkproblem_rdft_d)(sz, vecsz, I, I + offio, kind);
     return 1;
}

static void zero(const problem *ego_)
{
     const problem_rdft *ego = (const problem_rdft *) ego_;
     tensor sz = X(tensor_append)(ego->vecsz, ego->sz);
     X(rdft_zerotens)(sz, ego->I);
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
     "rdft"
};

int X(problem_rdft_p)(const problem *p)
{
     return (p->adt == &padt);
}

problem *X(mkproblem_rdft)(const tensor sz, const tensor vecsz,
			   R *I, R *O, rdft_kind kind)
{
     problem_rdft *ego =
          (problem_rdft *)X(mkproblem)(sizeof(problem_rdft), &padt);

     uint i, total_n = 1;
     for (i = 0; i < sz.rnk; ++i)
	  total_n *= X(rdft_real_n)(kind, sz.dims[i].n);
     A(total_n > 0); /* or should we use vecsz RNK_MINFTY? */
     
     ego->sz = X(tensor_compress)(sz);
     ego->vecsz = X(tensor_compress_contiguous)(vecsz);
     ego->I = I;
     ego->O = O;
     ego->kind = kind;

     A(FINITE_RNK(ego->sz.rnk));
     return &(ego->super);
}

/* Same as X(mkproblem_rdft), but also destroy input tensors. */
problem *X(mkproblem_rdft_d)(tensor sz, tensor vecsz,
			     R *I, R *O, rdft_kind kind)
{
     problem *p;
     p = X(mkproblem_rdft)(sz, vecsz, I, O, kind);
     X(tensor_destroy)(vecsz);
     X(tensor_destroy)(sz);
     return p;
}

void X(problem_rdft_register)(planner *p)
{
     REGISTER_PROBLEM(p, &padt);
}

