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

/* direct DFT solver via cell library */

#include "dft.h"

#if HAVE_CELL

#include "simd.h"
#include "fftw-cell.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_dft super;
     struct spu_radices radices;
     /* strides expressed in reals */
     INT n, is, os, v, ivs, ovs;
     int sign;
     int Wsz;
     R *W;
} P;


/* expressed in real numbers */
static int compute_twiddle_size(const struct spu_radices *p, int n)
{
     int sz = 0;
     int r;
     signed char *q;

     for (q = p->r; (r = *q) > 0; ++q) {
	  n /= r;
	  sz += 2 * (r - 1) * n;
     }

     return sz;
}

static void fill_twiddles(R *W, const signed char *q, int n)
{
     int r;
     R d[2];

     if ((r = *q) > 0) {
	  triggen *t = X(mktriggen)(AWAKE_SINCOS, n);
	  int i, j, v, m = n / r;

	  for (j = 0; j < m; j += VL) {
	       for (i = 1; i < r; ++i) {
		    for (v = 0; v < VL; ++v) {
			 t->cexp(t, i * (j + v), d);
			 *W++ = d[0];
		    }
		    for (v = 0; v < VL; ++v) {
			 t->cexp(t, i * (j + v), d);
			 *W++ = d[1];
		    }
	       }
	  }
	  X(triggen_destroy)(t);
	  fill_twiddles(W, q + 1, m);
     }
}

static R *make_twiddles(const struct spu_radices *p, int n, int *Wsz)
{
     *Wsz = compute_twiddle_size(p, n);
     R *W = X(cell_aligned_malloc)(*Wsz * sizeof(R));
     fill_twiddles(W, p->r, n);
     return W;
}


static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     R *xi, *xo;
     int i, v;
     int nspe = X(cell_nspe)();

     /* find pointer to beginning of data, depending on sign */
     if (ego->sign == -1) {
	  xi = ri; xo = ro;
     } else {
	  xi = ii; xo = io;
     }

     /* fill contexts */
     v = ego->v;
     for (i = 0; i < nspe; ++i) {
	  int chunk;
	  struct spu_context *ctx = X(cell_get_ctx)(i);
	  struct dft_context *dft = &ctx->u.dft;

	  ctx->op = SPE_DFT;

	  dft->r = ego->radices;
	  dft->n = ego->n;
	  dft->is_bytes = ego->is * sizeof(R);
	  dft->os_bytes = ego->os * sizeof(R);
	  dft->ivs_bytes = ego->ivs * sizeof(R);
	  dft->ovs_bytes = ego->ovs * sizeof(R);
	  dft->sign = ego->sign;
	  dft->Wsz_bytes = ego->Wsz * sizeof(R);
	  dft->W = (unsigned long long)ego->W;
	  dft->xi = (unsigned long long)xi;
	  dft->xo = (unsigned long long)xo;

	  /* partition v into pieces of equal size, subject to alignment
	     constraints */
	  if (VL > 1 && (ego->is != 2 || ego->os != 2)) {
	       /* either the input or the output operates in vector mode */
	       A(v % VL == 0); /* enforced in find_radices() */
	       chunk = VL * (v / (VL * (nspe - i)));
	  } else {
	       chunk = v / (nspe - i);
	  }

	  dft->v1 = v;
	  v -= chunk;
	  dft->v0 = v;
     }

     A(v == 0);

     /* activate spe's */
     X(cell_spe_awake_all)();

     /* wait for completion */
     X(cell_spe_wait_all)();
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(ifree0)(ego->W);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dft-direct-cell-%D%v)", ego->n, ego->v);
}

static 
const struct spu_radices *find_radices(R *ri, R *ii, R *ro, R *io,
				       int n, int is, int os,
				       int v, int ivs, int ovs,
				       int *sign)
{
     const struct spu_radices *p;
     struct cell_dft_plan *pln;
     R *xi, *xo;

     /* valid n? */
     if (n <= 0 || n > MAX_N || ((n % REQUIRE_N_MULTIPLE_OF) != 0))
	  return 0;
     
     /* see if we have a plan for this N */
     p = X(spu_radices) + n / REQUIRE_N_MULTIPLE_OF;
     if (!p->r[0])
	  return 0; 

     /* check whether the data format is supported */
     if (ii == ri + 1 && io == ro + 1) { /* R I R I ... format */
	  *sign = -1;
	  xi = ri; xo = ro;
     } else if (ri == ii + 1 && ro == io + 1) { /* I R I R ... format */
	  *sign = 1;
	  xi = ii; xo = io;
     } else
	  return 0; /* can't do it */
     
     if (!ALIGNEDA(xi) || !ALIGNEDA(xo))
	  return 0;

     /* either is or ivs must be 2 */
     if (is == 2) {
	  /* DFT of contiguous input.  Vector strides must be
	     aligned */
	  if (!SIMD_STRIDE_OKA(ivs))
	       return 0;
     } else if (ivs == 2) {
	  /* Vector DFT.  Strides must be aligned */
	  if (!SIMD_STRIDE_OKA(is) || ((v % VL) != 0))
	       return 0;
     } else
	  return 0; /* can't do it */

     /* same for output strides */
     if (os == 2) {
	  if (!SIMD_STRIDE_OKA(ovs))
	       return 0;
     } else if (ovs == 2) {
	  /* Vector DFT.  Strides must be aligned */
	  if (!SIMD_STRIDE_OKA(os) || ((v % VL) != 0))
	       return 0;
     } else
	  return 0; /* can't do it */

     return p;
}

static int applicable(const planner *plnr, const problem_dft *p)
{
     return (
	  1
	  && p->sz->rnk == 1
	  && p->vecsz->rnk <= 1
	  && !NO_SIMDP(plnr)

	  && (0
	      /* can operate out-of-place */
	      || p->ri != p->ro

	      /*
	       * can compute one transform in-place, no matter
	       * what the strides are.
	       */
	      || p->vecsz->rnk == 0

	      /* can operate in-place as long as strides are the same */
	      || (X(tensor_inplace_strides2)(p->sz, p->vecsz))
	       )
	  );
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_dft *p = (const problem_dft *) p_;
     iodim *d;
     INT v, ivs, ovs;
     int sign;
     const struct spu_radices *radices;

     static const plan_adt padt = {
	  X(dft_solve), X(null_awake), print, destroy
     };

     UNUSED(plnr);

     if (!applicable(plnr, p))
	  return (plan *)0;

     d = p->sz->dims;
     X(tensor_tornk1)(p->vecsz, &v, &ivs, &ovs);

     radices = find_radices(p->ri, p->ii, p->ro, p->io,
			    d[0].n, d[0].is, d[0].os,
			    v, ivs, ovs,
			    &sign);
     if (!radices)
	  return (plan *)0;
	 
     pln = MKPLAN_DFT(P, &padt, apply);

     pln->radices = *radices;
     pln->n = d[0].n;
     pln->is = d[0].is;
     pln->os = d[0].os;
     pln->v = v;
     pln->ivs = ivs;
     pln->ovs = ovs;
     pln->sign = sign;
     pln->W = make_twiddles(radices, pln->n, &pln->Wsz);

     X(ops_zero)(&pln->super.super.ops);
     pln->super.super.could_prune_now_p = 1;
     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_DFT, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(dft_direct_cell_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}

#endif /* HAVE_CELL */
