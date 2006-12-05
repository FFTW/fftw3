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
#include "ct.h"

#if HAVE_CELL

#include "simd.h"
#include "fftw-cell.h"

typedef struct {
     solver super;
     int cutdim;
} S;

typedef struct {
     plan_dft super;
     struct spu_radices radices;
     /* strides expressed in reals */
     INT n, is, os;
     struct cell_iodim v[2];
     int cutdim;
     int sign;
     int Wsz;
     R *W;

     /* optional twiddle factors for dftw: */
     INT rw, mw;  /* rw == 0 indicates no twiddle factors */
     twid *td;
} P;


/* op counts of SPE codelets */
static const opcnt n_ops[33] = {
     [2] = {2, 0, 0, 0},
     [3] = {3, 1, 3, 0},
     [4] = {6, 0, 2, 0}, 
     [5] = {7, 2, 9, 0}, 
     [6] = {12, 2, 6, 0},
     [7] = {9, 3, 21, 0},
     [8] = {16, 0, 10, 0}, 
     [9] = {12, 4, 34, 0}, 
     [10] = {24, 4, 18, 0},
     [11] = {15, 5, 55, 0},
     [12] = {30, 2, 18, 0},
     [13] = {31, 6, 57, 0},
     [14] = {32, 6, 42, 0},
     [15] = {36, 7, 42, 0},
     [16] = {38, 0, 34, 0},
     [32] = {88, 0, 98, 0},
};

static const opcnt t_ops[33] = {
     [2] = {3, 2, 0, 0},
     [3] = {5, 5, 3, 0},
     [4] = {9, 6, 2, 0},
     [5] = {11, 10, 9, 0},
     [6] = {17, 12, 6, 0},
     [7] = {15, 15, 21, 0},
     [8] = {23, 14, 10, 0},
     [9] = {20, 20, 34, 0},
     [10] =  {33, 22, 18, 0},
     [12] = {41, 24, 18, 0},
     [15] = {50, 35, 42, 0},
     [16] = {53, 30, 34, 0},
     [32] = {119, 62, 98, 0},
};

static void compute_opcnt(const struct spu_radices *p, 
			  int n, int v, opcnt *ops)
{
     int r;
     signed char *q;

     X(ops_zero)(ops);
     for (q = p->r; (r = *q) > 0; ++q, v *= r) 
	  X(ops_madd2)((v * (n / r)) / VL, &t_ops[r], ops);

     X(ops_madd2)(v / VL, &n_ops[-r], ops);
}

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

/* FIXME: find a way to use the normal FFTW twiddle mechanisms for this */
static void fill_twiddles(enum wakefulness wakefulness,
			  R *W, const signed char *q, int n)
{
     int r;

     for ( ; (r = *q) > 0; ++q) {
	  triggen *t = X(mktriggen)(wakefulness, n);
	  int i, j, v, m = n / r;

	  for (j = 0; j < m; j += VL) {
	       for (i = 1; i < r; ++i) {
		    for (v = 0; v < VL; ++v) {
			 t->cexp(t, i * (j + v), W);
			 W += 2;
		    }
	       }
	  }
	  X(triggen_destroy)(t);
	  n = m;
     }
}

static R *make_twiddles(enum wakefulness wakefulness,
			const struct spu_radices *p, int n, int *Wsz)
{
     *Wsz = compute_twiddle_size(p, n);
     R *W = X(cell_aligned_malloc)(*Wsz * sizeof(R));
     fill_twiddles(wakefulness, W, p->r, n);
     return W;
}

static int fits_in_local_store(INT n, INT v)
{
     /* the SPU has space for 4 * MAX_N complex numbers.  We need
	n*(v+1) for data plus n for twiddle factors. */
     return n * (v+2) <= 4 * MAX_N;
}

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     R *xi, *xo;
     int i, v;
     int nspe = X(cell_nspe)();
     int cutdim = ego->cutdim;

     /* find pointer to beginning of data, depending on sign */
     if (ego->sign == FFT_SIGN) {
	  xi = ri; xo = ro;
     } else {
	  xi = ii; xo = io;
     }

     /* fill contexts */
     v = ego->v[cutdim].n1;

     for (i = 0; i < nspe; ++i) {
	  int chunk;
	  struct spu_context *ctx = X(cell_get_ctx)(i);
	  struct dft_context *dft = &ctx->u.dft;

	  ctx->op = SPE_DFT;

	  dft->r = ego->radices;
	  dft->n = ego->n;
	  dft->is_bytes = ego->is * sizeof(R);
	  dft->os_bytes = ego->os * sizeof(R);
	  dft->v[0] = ego->v[0];
	  dft->v[1] = ego->v[1];
	  dft->sign = ego->sign;
	  dft->Wsz_bytes = ego->Wsz * sizeof(R);
	  dft->W = (INT)ego->W;
	  dft->xi = (INT)xi;
	  dft->xo = (INT)xo;

	  /* partition v into pieces of equal size, subject to alignment
	     constraints */
	  if ((ego->is == 2 && ego->os == 2) || cutdim > 0) {
	       chunk = (v - ego->v[cutdim].n0) / (nspe - i);
	  } else {
	       /* Either CUTDIM==0 or the input is not contiguous.
		  The SPE needs multiples of VL in this case */
	       chunk = VL * ((v - ego->v[cutdim].n0) / (VL * (nspe - i)));
	  }

	  dft->v[cutdim].n1 = v;
	  v -= chunk;
	  dft->v[cutdim].n0 = v;

	  /* optional dftw twiddles */
	  if (ego->rw) 
	       dft->Ww = (INT)ego->td->W;
     }

     A(v == ego->v[cutdim].n0);

     /* activate spe's */
     X(cell_spe_awake_all)();

     /* wait for completion */
     X(cell_spe_wait_all)();
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     int i;
     p->print(p, "(dft-direct-cell-%D/%d", ego->n, ego->cutdim);
     for (i = 0; i < 2; ++i)
	  p->print(p, "%v", (INT)ego->v[i].n1);
     p->print(p, ")");
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;

     /* awake the optional dftw twiddles */
     if (ego->rw) {
	  static const tw_instr tw[] = {
	       { TW_CEXP, 0, 0 },
	       { TW_FULL, 0, 0 }, 
	       { TW_NEXT, 1, 0 } 
	  };
	  X(twiddle_awake)(wakefulness, &ego->td, tw, 
			   ego->rw * ego->mw, ego->rw, ego->mw);
     }

     /* awake the twiddles for the dft part */
     switch (wakefulness) {
	 case SLEEPY:
	      X(ifree0)(ego->W);
	      ego->W = 0;
	      break;
	 default:
	      ego->W = make_twiddles(wakefulness, &ego->radices, 
				     ego->n, &ego->Wsz);
	      break;
     }
}

static int build_vdim(const iodim *d, const tensor *vecsz,
		      struct cell_iodim *v)
{
     int i;
     int contigdim = -1;
     iodim *t;

     /* allow vecrnk=0 if the strides are ok */
     if (vecsz->rnk == 0 && d->is == 2 && d->os == 2)
	  goto skip_contigdim;

     for (i = 0; i < vecsz->rnk; ++i) {
	  t = vecsz->dims + i;
	  if (t->n % VL) /* FIXME: too conservative, but hard to get right */
	       return 0; 
	  if (( 
		   /* contiguous input across D */
		   (d->is == 2 && SIMD_STRIDE_OKA(t->is))
		   ||
		   /* contiguous input across T */
		   (t->is == 2 && SIMD_STRIDE_OKA(d->is))
		   )
	      && (
		   /* contiguous output across D */
		   (d->os == 2 && SIMD_STRIDE_OKA(t->os))
		   ||
		   /* contiguous output across T */
		   (t->os == 2 && SIMD_STRIDE_OKA(d->os))
		   )) {
	       /* choose I as the contiguous dimension */
	       contigdim = i;
	  } else {
	       /* vector strides must be aligned */
	       if (!SIMD_STRIDE_OKA(t->is) || !SIMD_STRIDE_OKA(t->os))
		    return 0;
	  }
     }

     /* no contiguous dimension found, can't do it */
     if (contigdim < 0)
	  return 0;

     /* contigdim goes first */
     t = vecsz->dims + contigdim;
     v->n0 = 0; v->n1 = t->n; 
     v->is_bytes = t->is * sizeof(R); v->os_bytes = t->os * sizeof(R);
     v->dm = 0;
     ++v;

 skip_contigdim:
     /* other dimensions next */
     for (i = 0; i < vecsz->rnk; ++i) 
	  if (i != contigdim) {
	       t = vecsz->dims + i;
	       v->n0 = 0; 
	       v->n1 = t->n; 
	       v->is_bytes = t->is * sizeof(R); 
	       v->os_bytes = t->os * sizeof(R);
	       v->dm = 0;
	       ++v;
	  }
     
     /* complete tensor */
     for (; i < 2; ++i) {
	  v->n0 = 0; v->n1 = 1; v->is_bytes = 0; v->os_bytes = 0;
	  v->dm = 0; ++v;
     }

     return 1; 
}

static 
const struct spu_radices *find_radices(R *ri, R *ii, R *ro, R *io,
				       int n, int *sign)
{
     const struct spu_radices *p;
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
	  *sign = FFT_SIGN;
	  xi = ri; xo = ro;
     } else if (ri == ii + 1 && ro == io + 1) { /* I R I R ... format */
	  *sign = -FFT_SIGN;
	  xi = ii; xo = io;
     } else
	  return 0; /* can't do it */
     
     if (!ALIGNEDA(xi) || !ALIGNEDA(xo))
	  return 0;

     return p;
}

static const plan_adt padt = {
     X(dft_solve), awake, print, X(plan_null_destroy)
};

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const S *ego = (const S *)ego_;
     const problem_dft *p = (const problem_dft *) p_;
     iodim *d;
     int sign;
     const struct spu_radices *radices;
     struct cell_iodim vd[2];
     int cutdim;

     /* basic conditions */
     if (!(1
	   && X(cell_nspe)() > 0
	   && p->sz->rnk == 1
	   && p->vecsz->rnk <= 2
	   && !NO_SIMDP(plnr)
	      ))
	  return 0;

     /* see if SPE supports N */
     d = p->sz->dims;
     radices = find_radices(p->ri, p->ii, p->ro, p->io, d[0].n, &sign);
     if (!radices)
	  return 0;

     /* see if the SPE dma supports the strides that we have */
     if (!build_vdim(d, p->vecsz, vd))
	  return 0;

     cutdim = ego->cutdim;

     if (!(0
	   /* either N is contiguous... */ 
	   || (d[0].is == 2 && d[0].os == 2)

	   /* ... or vd[cutdim] must be a multiple of VL if cutdim==0
	      (see computation of CHUNK in apply()) */
	   || cutdim > 0
	   || (vd[cutdim].n1 % VL) == 0
	      ))
	  return 0;

     /* see if we can do it without overwriting the input with
	itself */
     if (!(0
	   /* can operate out-of-place */
	   || p->ri != p->ro

	   /* all strides are in-place */
	   || (1
	       && d[0].is == d[0].os
	       && vd[0].is_bytes == vd[0].os_bytes
	       && vd[1].is_bytes == vd[1].os_bytes)

	   /* we cut across in-place dimension 1 and dimension 0 fits
	      into local store */
	   || (1
	       && cutdim == 1
	       && vd[1].is_bytes == vd[1].os_bytes
	       && fits_in_local_store(d[0].n, vd[0].n1 - vd[0].n0))
	      ))
	  return 0;

     pln = MKPLAN_DFT(P, &padt, apply);

     pln->radices = *radices;
     pln->n = d[0].n;
     pln->is = d[0].is;
     pln->os = d[0].os;
     pln->sign = sign;
     pln->v[0] = vd[0];
     pln->v[1] = vd[1];
     pln->cutdim = cutdim;
     pln->W = 0;

     pln->rw = 0;

     compute_opcnt(radices, pln->n, 
		   vd[0].n1 * vd[1].n1, &pln->super.super.ops);

     /* penalize cuts across short dimensions */
     if (vd[cutdim].n1 < vd[1-cutdim].n1)
	  pln->super.super.ops.other += 3.14159;

//     pln->super.super.could_prune_now_p = 1;
     return &(pln->super.super);
}

static void solver_destroy(solver *ego)
{
     UNUSED(ego);
     X(cell_deactivate_spes)();
}

static solver *mksolver(int cutdim)
{
     static const solver_adt sadt = { PROBLEM_DFT, mkplan, solver_destroy };
     S *slv = MKSOLVER(S, &sadt);
     slv->cutdim = cutdim;
     X(cell_activate_spes)();
     return &(slv->super);
}

void X(dft_direct_cell_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver(0));
     REGISTER_SOLVER(p, mksolver(1));
}

/**************************************************************/
/* solvers with twiddle factors: */

typedef struct {
     plan_dftw super;
     plan *cld;
} Pw;

static void destroyw(plan *ego_)
{
     Pw *ego = (Pw *) ego_;
     X(plan_destroy_internal)(ego->cld);
}

static void printw(const plan *ego_, printer *p)
{
     const Pw *ego = (const Pw *) ego_;
     const P *cld = (const P *) ego->cld;
     p->print(p, "(dftw-direct-cell-%D-%D%v%(%p%))", 
	      cld->rw, cld->mw, cld->v[1].n1,
	      ego->cld);
}

static void awakew(plan *ego_, enum wakefulness wakefulness)
{
     Pw *ego = (Pw *) ego_;
     X(plan_awake)(ego->cld, wakefulness);
}

static void applyw(const plan *ego_, R *rio, R *iio)
{
     const Pw *ego = (const Pw *) ego_;
     dftapply cldapply = ((plan_dft *) ego->cld)->apply;
     cldapply(ego->cld, rio, iio, rio, iio);
}

static plan *mkcldw(const ct_solver *ego, 
		    INT r, INT irs, INT ors,
		    INT m, INT ms,
		    INT v, INT ivs, INT ovs,
		    INT mstart, INT mcount,
		    R *rio, R *iio,
		    planner *plnr)
{
     const struct spu_radices *radices;
     int sign;
     Pw *pln;
     P *cld;
     struct cell_iodim d[2];
     int cutdim, dm = 0;

     static const plan_adt padtw = {
	  0, awakew, printw, destroyw
     };

     /* use only if cell is enabled */
     if (NO_SIMDP(plnr) || X(cell_nspe)() <= 0)
	  return 0;

     /* don't bother for small N */
     if (r * m * v <= MAX_N / 8 /* ARBITRARY */)
	  return 0;

     /* check whether the R dimension is supported */
     radices = find_radices(rio, iio, rio, iio, r, &sign);

     if (!radices)
	  return 0;

     /* check the M dimension */
     if ((m % VL) || (mstart % VL) || (mcount % VL))
	  return 0;

     /* encode decimation in DM */
     switch (ego->dec) {
	 case DECDIT: 
	 case DECDIT+TRANSPOSE: 
	      dm = 1; 
	      break;
	 case DECDIF: 
	 case DECDIF+TRANSPOSE: 
	      dm = -1; 
	      break;
     }

     if (ms == 2) {
	  /* Case 1: contiguous I/O across M.  In SPU, let N=R, V0=M,
	     V1=V. Cut M */

	  /* other strides must be in-place and aligned */
	  if (!(1
		&& irs == ors 
		&& ivs == ovs 
		&& SIMD_STRIDE_OKA(ivs) 
		&& SIMD_STRIDE_OKA(irs)
		   ))
	       return 0;

	  d[0].n0 = mstart; d[0].n1 = mstart + mcount;
	  d[0].is_bytes = d[0].os_bytes = ms * sizeof(R);
	  d[0].dm = dm;

	  d[1].n0 = 0; d[1].n1 = v; 
	  d[1].is_bytes = ivs * sizeof(R); d[1].os_bytes = ovs * sizeof(R);
	  d[1].dm = 0;

	  cutdim = 0;
     } else if ((irs == 2 || ivs == 2) && (ors == 2 || ovs == 2)) {
	  /* Case 2: contiguous I/O across either V or R.  In
	     SPU, let N=R, V0=V, V1=M.  Cut M */
	  
	  /* either the strides are in-place or V*R fits into the SPU
	     local store */
	  if (!(0
		|| ((irs == ors) && (ivs == ovs))
		|| fits_in_local_store(r, v) ))
	       return 0;

	  /* check alignment */
	  if (!(1
		/* M stride must be aligned */
		&& SIMD_STRIDE_OKA(ms)

		/* other strides are either contiguous or aligned */
		&& (irs == 2 || SIMD_STRIDE_OKA(irs))
		&& (ors == 2 || SIMD_STRIDE_OKA(ors))
		&& (ivs == 2 || SIMD_STRIDE_OKA(ivs))
		&& (ovs == 2 || SIMD_STRIDE_OKA(ovs))
		   ))
	       return 0;
	  
	  d[0].n0 = 0; d[0].n1 = v; 
	  d[0].is_bytes = ivs * sizeof(R); d[0].os_bytes = ovs * sizeof(R);
	  d[0].dm = 0;

	  d[1].n0 = mstart; d[1].n1 = mstart + mcount;
	  d[1].is_bytes = d[1].os_bytes = ms * sizeof(R);
	  d[1].dm = dm;

	  cutdim = 1;
     } else
	  return 0; /* can't do it */

     cld = MKPLAN_DFT(P, &padt, apply);

     cld->radices = *radices;
     cld->n = r;
     cld->is = irs;
     cld->os = ors;
     cld->sign = sign;
     cld->W = 0;

     cld->rw = r; cld->mw = m; cld->td = 0;
     
     /* build vdim by hand */
     cld->v[0] = d[0];
     cld->v[1] = d[1];
     cld->cutdim = cutdim;

     pln = MKPLAN_DFTW(Pw, &padtw, applyw);
     pln->cld = &cld->super.super;
     compute_opcnt(radices, cld->n, mcount * v, &pln->super.super.ops);
     /* for twiddle factors: one mul and one fma per complex point */
     pln->super.super.ops.fma += (cld->n * mcount * v) / VL;
     pln->super.super.ops.mul += (cld->n * mcount * v) / VL;

     return &(pln->super.super);
}

static void regsolverw(planner *plnr, INT r, int dec)
{
     ct_solver *slv = X(mksolver_ct)(sizeof(ct_solver), r, dec, mkcldw);
     REGISTER_SOLVER(plnr, &(slv->super));
}

void X(ct_cell_direct_register)(planner *p)
{
     int n;

     for (n = 0; n <= MAX_N; n += REQUIRE_N_MULTIPLE_OF) {
	  const struct spu_radices *r = 
	       X(spu_radices) + n / REQUIRE_N_MULTIPLE_OF;
	  if (r->r[0]) {
	       regsolverw(p, n, DECDIT);
	       regsolverw(p, n, DECDIF+TRANSPOSE);
	  }
     }
}


#endif /* HAVE_CELL */


