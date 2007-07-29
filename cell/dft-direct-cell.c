/*
 * Copyright (c) 2007 Massachusetts Institute of Technology
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


/* op counts of SPU codelets */
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
			  INT n, INT v, opcnt *ops)
{
     INT r;
     signed char *q;

     X(ops_zero)(ops);

     for (q = p->r; (r = *q) > 0; ++q) 
	  X(ops_madd2)(v * (n / r) / VL, &t_ops[r], ops);

     X(ops_madd2)(v * (n / (-r)) / VL, &n_ops[-r], ops);
}

static INT extent(struct cell_iodim *d)
{
     return d->n1 - d->n0;
}

/* FIXME: this is totally broken */
static void cost_model(const P *pln, opcnt *ops)
{
     INT r = pln->n;
     INT v0 = extent(pln->v + 0);
     INT v1 = extent(pln->v + 1);

     compute_opcnt(&pln->radices, r, v0 * v1, ops);

     /* penalize cuts across short dimensions */
     if (extent(pln->v + pln->cutdim) < extent(pln->v + 1 - pln->cutdim))
	  ops->other += 3.14159;
}

/* expressed in real numbers */
static INT compute_twiddle_size(const struct spu_radices *p, INT n)
{
     INT sz = 0;
     INT r;
     signed char *q;

     for (q = p->r; (r = *q) > 0; ++q) {
	  n /= r;
	  sz += 2 * (r - 1) * n;
     }

     return sz;
}

/* FIXME: find a way to use the normal FFTW twiddle mechanisms for this */
static void fill_twiddles(enum wakefulness wakefulness,
			  R *W, const signed char *q, INT n)
{
     INT r;

     for ( ; (r = *q) > 0; ++q) {
	  triggen *t = X(mktriggen)(wakefulness, n);
	  INT i, j, v, m = n / r;

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
			const struct spu_radices *p, INT n, int *Wsz)
{
     INT sz = compute_twiddle_size(p, n);
     R *W = X(cell_aligned_malloc)(sz * sizeof(R));
     A(FITS_IN_INT(sz));
     *Wsz = sz;
     fill_twiddles(wakefulness, W, p->r, n);
     return W;
}

static int fits_in_local_store(INT n, INT v)
{
     /* the SPU has space for 3 * MAX_N complex numbers.  We need
	n*(v+1) for data plus n for twiddle factors. */
     return n * (v+2) <= 3 * MAX_N;
}

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     R *xi, *xo;
     int i, v;
     int nspe = X(cell_nspe)();
     int cutdim = ego->cutdim;
     int contiguous_r = ((ego->is == 2) && (ego->os == 2));

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

	  ctx->op = FFTW_SPE_DFT;

	  dft->r = ego->radices;
	  dft->n = ego->n;
	  dft->is_bytes = ego->is * sizeof(R);
	  dft->os_bytes = ego->os * sizeof(R);
	  dft->v[0] = ego->v[0];
	  dft->v[1] = ego->v[1];
	  dft->sign = ego->sign;
	  A(FITS_IN_INT(ego->Wsz * sizeof(R)));
	  dft->Wsz_bytes = ego->Wsz * sizeof(R);
	  dft->W = (uintptr_t)ego->W;
	  dft->xi = (uintptr_t)xi;
	  dft->xo = (uintptr_t)xo;

	  /* partition v into pieces of equal size, subject to alignment
	     constraints */
	  if (cutdim == 0 && !contiguous_r) {
	       /* CUTDIM = 0 and the SPU uses transposed DMA.
		  We must preserve the alignment of the dimension 0 in the
		  cut */
	       chunk = VL * ((v - ego->v[cutdim].n0) / (VL * (nspe - i)));
	  } else {
	       chunk = (v - ego->v[cutdim].n0) / (nspe - i);
	  }

	  dft->v[cutdim].n1 = v;
	  v -= chunk;
	  dft->v[cutdim].n0 = v;

	  /* optional dftw twiddles */
	  if (ego->rw) 
	       dft->Ww = (uintptr_t)ego->td->W;
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
	      free(ego->W);
	      ego->W = 0;
	      break;
	 default:
	      ego->W = make_twiddles(wakefulness, &ego->radices, 
				     ego->n, &ego->Wsz);
	      break;
     }
}

static int contiguous_or_aligned_p(INT s_bytes)
{
     return (s_bytes == 2 * sizeof(R)) || ((s_bytes % ALIGNMENTA) == 0);
}

static int build_vdim(int inplacep,
		      INT r, INT irs, INT ors,
		      INT m, INT ims, INT oms, int dm,
		      INT v, INT ivs, INT ovs,
		      struct cell_iodim vd[2], int cutdim)
{
     int vm, vv;
     int contiguous_r = ((irs == 2) && (ors == 2));

     /* 32-bit overflow? */
     if (!(1
	   && FITS_IN_INT(r)
	   && FITS_IN_INT(irs * sizeof(R))
	   && FITS_IN_INT(ors * sizeof(R))
	   && FITS_IN_INT(m)
	   && FITS_IN_INT(ims * sizeof(R))
	   && FITS_IN_INT(oms * sizeof(R))
	   && FITS_IN_INT(v)
	   && FITS_IN_INT(ivs * sizeof(R))
	   && FITS_IN_INT(ovs * sizeof(R))))
	  return 0;

     /* R dimension must be aligned in all cases */
     if (!(1
	   && r % VL == 0 /* REDUNDANT */
	   && contiguous_or_aligned_p(irs * sizeof(R))
	   && contiguous_or_aligned_p(ors * sizeof(R))))
	  return 0;
	   
     if ((irs == 2 || ims == 2) && (ors == 2 || oms == 2)) {
	  /* Case 1: in SPU, let N=R, V0=M, V1=V */
	  vm = 0; 
	  vv = 1;
     } else if ((irs == 2 || ivs == 2) && (ors == 2 || ovs == 2)) {
	  /* Case 2: in SPU, let N=R, V0=V, V1=M */
	  vm = 1; 
	  vv = 0;
     } else { 
	  /* can't do it */
	  return 0;
     }

     vd[vm].n0 = 0; vd[vm].n1 = m;
     vd[vm].is_bytes = ims * sizeof(R); vd[vm].os_bytes = oms * sizeof(R);
     vd[vm].dm = dm;

     vd[vv].n0 = 0; vd[vv].n1 = v; 
     vd[vv].is_bytes = ivs * sizeof(R); vd[vv].os_bytes = ovs * sizeof(R);
     vd[vv].dm = 0;

     /* Restrictions on the size of the SPU local store: */
     if (!(0
	   /* for contiguous I/O, one array of size R must fit into
	      local store.  (The fits_in_local_store() check is
	      redundant because R <= MAX_N holds, but we check anyway
	      for clarity */
	   || (contiguous_r && fits_in_local_store(r, 1))

	   /* for noncontiguous I/O, VL arrays of size R must fit into
	      local store because of transposed DMA */
	   || fits_in_local_store(r, VL)))
	  return 0;

     /* SPU DMA restrictions: */
     if (!(1
	   /* If R is noncontiguous, then the SPU uses transposed DMA
	      and therefore dimension 0 must be aligned */ 
	   && (contiguous_r || vd[0].n1 % VL == 0)

	   /* dimension 1 is arbitrary */

	   /* dimension-0 strides must be either contiguous or aligned */
	   && contiguous_or_aligned_p((INT)vd[0].is_bytes)
	   && contiguous_or_aligned_p((INT)vd[0].os_bytes)

	   /* dimension-1 strides must be aligned */
	   && ((vd[1].is_bytes % ALIGNMENTA) == 0)
	   && ((vd[1].os_bytes % ALIGNMENTA) == 0)
	      ))
	  return 0;

     /* see if we can do it without overwriting the input with itself */
     if (!(0
	   /* can operate out-of-place */
	   || !inplacep

	   /* all strides are in-place */
	   || (1
	       && irs == ors
	       && ims == oms
	       && ivs == ovs)

	   /* we cut across in-place dimension 1, and dimension 0 fits
	      into local store */
	   || (1
	       && cutdim == 1
	       && vd[cutdim].is_bytes == vd[cutdim].os_bytes
	       && fits_in_local_store(r, extent(vd + 0)))
	      ))
	  return 0;

     return 1;
}

static 
const struct spu_radices *find_radices(R *ri, R *ii, R *ro, R *io,
				       INT n, int *sign)
{
     const struct spu_radices *p;
     R *xi, *xo;

     /* 32-bit overflow? */
     if (!FITS_IN_INT(n))
	  return 0;

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
     int sign;
     const struct spu_radices *radices;
     struct cell_iodim vd[2];
     INT m, ims, oms, v, ivs, ovs;

     /* basic conditions */
     if (!(1
	   && X(cell_nspe)() > 0
	   && p->sz->rnk == 1
	   && p->vecsz->rnk <= 2
	   && !NO_SIMDP(plnr)
	      ))
	  return 0;

     /* see if SPU supports N */
     {
	  iodim *d = p->sz->dims;
	  radices = find_radices(p->ri, p->ii, p->ro, p->io, d[0].n, &sign);
	  if (!radices)
	       return 0;
     }

     /* canonicalize to vrank 2 */
     if (p->vecsz->rnk >= 1) {
	  iodim *d = p->vecsz->dims + 0;
	  m = d->n; ims = d->is; oms = d->os;
     } else {
	  m = 1; ims = oms = 0;
     }

     if (p->vecsz->rnk >= 2) {
	  iodim *d = p->vecsz->dims + 1;
	  v = d->n; ivs = d->is; ovs = d->os;
     } else {
	  v = 1; ivs = ovs = 0;
     }

     /* see if strides are supported by the SPU DMA routine  */
     {
	  iodim *d = p->sz->dims + 0;
	  if (!build_vdim(p->ri == p->ro,
			  d->n, d->is, d->os,
			  m, ims, oms, 0,
			  v, ivs, ovs,
			  vd, ego->cutdim))
	       return 0;
     }

     pln = MKPLAN_DFT(P, &padt, apply);

     pln->radices = *radices;
     {
	  iodim *d = p->sz->dims + 0;
	  pln->n = d[0].n;
	  pln->is = d[0].is;
	  pln->os = d[0].os;
     }
     pln->sign = sign;
     pln->v[0] = vd[0];
     pln->v[1] = vd[1];
     pln->cutdim = ego->cutdim;
     pln->W = 0;

     pln->rw = 0;

     cost_model(pln, &pln->super.super.ops);

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

typedef struct {
     ct_solver super;
     int cutdim;
} Sw;

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

static plan *mkcldw(const ct_solver *ego_, 
		    INT r, INT irs, INT ors,
		    INT m, INT ms,
		    INT v, INT ivs, INT ovs,
		    INT mstart, INT mcount,
		    R *rio, R *iio,
		    planner *plnr)
{
     const Sw *ego = (const Sw *)ego_;
     const struct spu_radices *radices;
     int sign;
     Pw *pln;
     P *cld;
     struct cell_iodim vd[2];
     int dm = 0;

     static const plan_adt padtw = {
	  0, awakew, printw, destroyw
     };

     /* use only if cell is enabled */
     if (NO_SIMDP(plnr) || X(cell_nspe)() <= 0)
	  return 0;

     /* no way in hell this SPU stuff is going to work with pthreads */
     if (mstart != 0 || mcount != m)
	  return 0;

     /* don't bother for small N */
     if (r * m * v <= MAX_N / 16 /* ARBITRARY */)
	  return 0;

     /* check whether the R dimension is supported */
     radices = find_radices(rio, iio, rio, iio, r, &sign);

     if (!radices)
	  return 0;

     /* encode decimation in DM */
     switch (ego->super.dec) {
	 case DECDIT: 
	 case DECDIT+TRANSPOSE: 
	      dm = 1; 
	      break;
	 case DECDIF: 
	 case DECDIF+TRANSPOSE: 
	      dm = -1; 
	      break;
     }

     if (!build_vdim(1,
		     r, irs, ors,
		     m, ms, ms, dm,
		     v, ivs, ovs,
		     vd, ego->cutdim))
	  return 0;

     cld = MKPLAN_DFT(P, &padt, apply);

     cld->radices = *radices;
     cld->n = r;
     cld->is = irs;
     cld->os = ors;
     cld->sign = sign;
     cld->W = 0;

     cld->rw = r; cld->mw = m; cld->td = 0;
     
     cld->v[0] = vd[0];
     cld->v[1] = vd[1];
     cld->cutdim = ego->cutdim;

     pln = MKPLAN_DFTW(Pw, &padtw, applyw);
     pln->cld = &cld->super.super;

     cost_model(cld, &pln->super.super.ops);

     /* for twiddle factors: one mul and one fma per complex point */
     pln->super.super.ops.fma += (r * m * v) / VL;
     pln->super.super.ops.mul += (r * m * v) / VL;

     /* FIXME: heuristics */
     /* pay penalty for large radices: */
     if (r > MAX_N / 16)
	  pln->super.super.ops.other += ((r - (MAX_N / 16)) * m * v);

     return &(pln->super.super);
}

/* heuristic to enable vector recursion */
static int force_vrecur(const ct_solver *ego, const problem_dft *p)
{
     iodim *d;
     INT n, r, m;
     INT cutoff = 128;

     A(p->vecsz->rnk == 1);
     A(p->sz->rnk == 1);

     n = p->sz->dims[0].n;
     r = X(choose_radix)(ego->r, n);
     m = n / r;

     d = p->vecsz->dims + 0;
     return (1
	     /* some vector dimension is contiguous */
	     && (d->is == 2 || d->os == 2)
	     
	     /* vector is sufficiently long */
	     && d->n >= cutoff

	     /* transform is sufficiently long */
	     && m >= cutoff
	     && r >= cutoff);
}

static void regsolverw(planner *plnr, INT r, int dec, int cutdim)
{
     Sw *slv = (Sw *)X(mksolver_ct)(sizeof(Sw), r, dec, mkcldw, force_vrecur);
     slv->cutdim = cutdim;
     REGISTER_SOLVER(plnr, &(slv->super.super));
}

void X(ct_cell_direct_register)(planner *p)
{
     INT n;

     for (n = 0; n <= MAX_N; n += REQUIRE_N_MULTIPLE_OF) {
	  const struct spu_radices *r = 
	       X(spu_radices) + n / REQUIRE_N_MULTIPLE_OF;
	  if (r->r[0]) {
	       regsolverw(p, n, DECDIT, 0);
	       regsolverw(p, n, DECDIT, 1);
	       regsolverw(p, n, DECDIF+TRANSPOSE, 0);
	       regsolverw(p, n, DECDIF+TRANSPOSE, 1);
	  }
     }
}


#endif /* HAVE_CELL */


