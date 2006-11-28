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
} S;

typedef struct {
     plan_dft super;
     struct spu_radices radices;
     /* strides expressed in reals */
     INT n, is, os;
     struct cell_iotensor v;
     int cutdim;
     int sign;
     int Wsz;
     R *W;

     /* optional twiddle factors for dftw: */
     INT rw, mw;  /* rw == 0 indicates no twiddle factors */
     R *Ww;
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

static void fill_twiddles(enum wakefulness wakefulness,
			  R *W, const signed char *q, int n)
{
     int r;
     R d[2];

     if ((r = *q) > 0) {
	  triggen *t = X(mktriggen)(wakefulness, n);
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
	  fill_twiddles(wakefulness, W, q + 1, m);
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
     v = ego->v.dims[cutdim].n1;

     for (i = 0; i < nspe; ++i) {
	  int chunk;
	  struct spu_context *ctx = X(cell_get_ctx)(i);
	  struct dft_context *dft = &ctx->u.dft;

	  ctx->op = SPE_DFT;

	  dft->r = ego->radices;
	  dft->n = ego->n;
	  dft->is_bytes = ego->is * sizeof(R);
	  dft->os_bytes = ego->os * sizeof(R);
	  dft->v = ego->v;
	  dft->sign = ego->sign;
	  dft->Wsz_bytes = ego->Wsz * sizeof(R);
	  dft->W = (INT)ego->W;
	  dft->xi = (INT)xi;
	  dft->xo = (INT)xo;

	  /* partition v into pieces of equal size, subject to alignment
	     constraints */
	  chunk = VL * ((v - ego->v.dims[cutdim].n0) / (VL * (nspe - i)));

	  dft->v.dims[cutdim].n1 = v;
	  v -= chunk;
	  dft->v.dims[cutdim].n0 = v;

	  /* optional dftw twiddles */
	  if ((dft->rw = ego->rw)) 
	       dft->Ww = (INT)ego->Ww;
     }

     A(v == ego->v.dims[cutdim].n0);

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
     for (i = 0; i < CELL_DFT_MAXVRANK; ++i)
	  p->print(p, "%v", (INT)ego->v.dims[i].n1);
     p->print(p, ")");
}

static R *make_twiddlesw(enum wakefulness wakefulness, int r, int m)

{
     R *W0= X(cell_aligned_malloc)(r * m * 2 * sizeof(R));
     R *W = W0;
     R d[2];
     triggen *t = X(mktriggen)(wakefulness, r * m);
     int i, j, v;
     for (j = 0; j < m; j++) {
	  for (i = 0; i < r; i += VL) {
	       for (v = 0; v < VL; ++v) {
		    t->cexp(t, (i+v) * j, d);
		    *W++ = d[0];
	       }
	       for (v = 0; v < VL; ++v) {
		    t->cexp(t, (i+v) * j, d);
		    *W++ = d[1];
	       }
	  }
     }
     X(triggen_destroy)(t);

     return W0;
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;

     /* awake the optional dftw twiddles */
     if (ego->rw) {
	  switch (wakefulness) {
	      case SLEEPY:
		   X(ifree0)(ego->Ww);
		   ego->Ww = 0;
		   break;
	      default:
		   ego->Ww = make_twiddlesw(wakefulness, ego->rw, ego->mw);
		   break;
	  }
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

static int build_vtensor(const iodim *d, const tensor *vecsz,
			 struct cell_iotensor *tens)
{
     int i;
     int contigdim = -1;
     iodim *t;
     struct cell_iodim *v = tens->dims;

     /* allow vecrnk=0 if the strides are ok */
     if (vecsz->rnk == 0 && d->is == 2 && d->os == 2)
	  goto skip_contigdim;

     for (i = 0; i < vecsz->rnk; ++i) {
	  t = vecsz->dims + i;
	  if (t->n % VL) /* FIXME: too conservative, but hard to get it
			    right */
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

     /* build cell_iotensor: */
     
     /* contigdim goes first */
     t = vecsz->dims + contigdim;
     v->n0 = 0; v->n1 = t->n; 
     v->is_bytes = t->is * sizeof(R); v->os_bytes = t->os * sizeof(R);
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
	       ++v;
	  }
     
     /* complete cell tensor */
     for (; i < CELL_DFT_MAXVRANK; ++i) {
	  v->n0 = 0; v->n1 = 1; v->is_bytes = 0; v->os_bytes = 0; ++v;
     }

     return 1; 
}

/* choose a dimension that should be cut when parallelizing */
static int choose_cutdim(const struct cell_iotensor *tens)
{
     int cutdim = 0;
     int i;

     for (i = 1; i < CELL_DFT_MAXVRANK; ++i) {
	  const struct cell_iodim *cut = &tens->dims[cutdim];
	  const struct cell_iodim *t = &tens->dims[i];

	  if ((cut->is_bytes == 2 * sizeof(R) || 
	       cut->os_bytes == 2 * sizeof(R))
	      && t->n1 > 1) {
	       /* since cutdim is contiguous, prefer to cut I */
	       cutdim = i;
	  } else if (t->n1 > cut->n1) {
	       /* prefer to cut the longer dimension */
	       cutdim = i;
	  }
     }
     return cutdim;
}

static 
const struct spu_radices *find_radices(R *ri, R *ii, R *ro, R *io,
				       int n, int is, int os,
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

static int applicable(const planner *plnr, const problem_dft *p)
{
     return (
	  1
	  && X(cell_nspe)() > 0
	  && p->sz->rnk == 1
	  && p->vecsz->rnk <= CELL_DFT_MAXVRANK
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

static const plan_adt padt = {
     X(dft_solve), awake, print, X(plan_null_destroy)
};

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_dft *p = (const problem_dft *) p_;
     iodim *d;
     int sign;
     const struct spu_radices *radices;
     struct cell_iotensor v;

     UNUSED(plnr); UNUSED(ego);

     if (!applicable(plnr, p))
	  return (plan *)0;

     d = p->sz->dims;

     radices = find_radices(p->ri, p->ii, p->ro, p->io,
			    d[0].n, d[0].is, d[0].os,
			    &sign);
     if (!radices)
	  return (plan *)0;

     if (!build_vtensor(d, p->vecsz, &v))
	  return 0;
	 
     pln = MKPLAN_DFT(P, &padt, apply);

     pln->radices = *radices;
     pln->n = d[0].n;
     pln->is = d[0].is;
     pln->os = d[0].os;
     pln->sign = sign;
     pln->v = v;
     pln->cutdim = choose_cutdim(&v);
     pln->W = 0;

     pln->rw = 0;

     X(ops_zero)(&pln->super.super.ops);
     pln->super.super.ops.other += 100; /* FIXME */
     pln->super.super.could_prune_now_p = 1;
     return &(pln->super.super);
}

static void solver_destroy(solver *ego)
{
     UNUSED(ego);
     X(cell_deactivate_spes)();
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { PROBLEM_DFT, mkplan, solver_destroy };
     S *slv = MKSOLVER(S, &sadt);
     X(cell_activate_spes)();
     return &(slv->super);
}

void X(dft_direct_cell_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
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
     int i;
     p->print(p, "(dftw-direct-cell-%D-%D%v%(%p%))", 
	      cld->rw, cld->mw, cld->v.dims[1].n1,
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
		    int dec, INT r, INT m, INT s, INT vl, INT vs, 
		    INT mstart, INT mcount,
		    R *rio, R *iio,
		    planner *plnr)
{
     const struct spu_radices *radices;
     int sign;
     Pw *pln;
     P *cld;
     struct cell_iodim *v;

     static const plan_adt padtw = {
	  0, awakew, printw, destroyw
     };

     UNUSED(ego);

     /* use only if cell is enabled */
     if (NO_SIMDP(plnr) || X(cell_nspe)() <= 0)
	  return 0;

     /* don't bother for small N */
     if (r * m <= MAX_N)
	  return 0;

     /* only handle input contiguous in the M dimension */
     if (s != 2)
	  return 0;

     /* M dimension must be kosher */
     if ((m % VL) || (mstart % VL) || (mcount % VL))
	  return 0;

     /* vector dimension, if any, must be kosher */
     if (!SIMD_STRIDE_OKA(vs))
	  return 0;

     radices = find_radices(rio, iio, rio, iio,
			    r, m * s, m * s,
			    &sign);

     if (!radices)
	  return 0;

     cld = MKPLAN_DFT(P, &padt, apply);

     cld->radices = *radices;
     cld->n = r;
     cld->is = m * s;
     cld->os = m * s;
     cld->sign = sign;
     cld->W = 0;

     cld->rw = r; cld->mw = m; cld->Ww = 0;
     
     /* build vtensor by hand */
     v = cld->v.dims;
     v[0].n0 = mstart; v[0].n1 = mstart + mcount;
     v[0].is_bytes = v[0].os_bytes = s * sizeof(R);
     v[1].n0 = 0; v[1].n1 = vl; v[1].is_bytes = v[1].os_bytes = vs * sizeof(R);
     cld->cutdim = 0;

     pln = MKPLAN_DFTW(Pw, &padtw, applyw);
     pln->cld = &cld->super.super;
     X(ops_zero)(&pln->super.super.ops);
     pln->super.super.ops.other += 100; /* FIXME */
     return &(pln->super.super);
}

static void regsolverw(planner *plnr, INT r, int dec)
{
     ct_solver *slv = X(mksolver_ct)(sizeof(ct_solver), r, dec, mkcldw);
     REGISTER_SOLVER(plnr, &(slv->super));
}

void X(ct_cell_direct_register)(planner *p)
{
#if 0
     regsolverw(p, -1, DECDIT);
     regsolverw(p, 16, DECDIT);
     regsolverw(p, 64, DECDIT);
     regsolverw(p, 128, DECDIT);
     regsolverw(p, 256, DECDIT);
#endif
     int n;

     for (n = 32; n <= MAX_N; n += REQUIRE_N_MULTIPLE_OF) {
	  const struct spu_radices *r = 
	       X(spu_radices) + n / REQUIRE_N_MULTIPLE_OF;
	  if (r->r[0])
	       regsolverw(p, n, DECDIT);
     }

}


#endif /* HAVE_CELL */


