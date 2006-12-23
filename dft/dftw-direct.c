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


#include "ct.h"

typedef struct {
     ct_solver super;
     const ct_desc *desc;
     int bufferedp;
     kdftw k;
} S;

typedef struct {
     plan_dftw super;
     kdftw k;
     INT r;
     stride rs;
     INT m, ms, v, vs, mb, me, extra_iter;
     stride brs;
     twid *td;
     const S *slv;
} P;


/*************************************************************
  Nonbuffered code
 *************************************************************/
static void apply(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     INT i, v = ego->v, vs = ego->vs;
     ASSERT_ALIGNED_DOUBLE;
     for (i = 0; i < v; ++i)
	  ego->k(rio + i * vs, iio + i * vs, ego->td->W, ego->rs, 
		 ego->mb, ego->me, ego->ms);
}

static void apply_extra_iter(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     INT i, v = ego->v, vs = ego->vs, mm = ego->me - 1, ms = ego->ms;
     ASSERT_ALIGNED_DOUBLE;
     for (i = 0; i < v; ++i, rio += vs, iio += vs) {
	  ego->k(rio, iio, ego->td->W, ego->rs, ego->mb, mm, ms);
	  ego->k(rio + mm*ms, iio + mm*ms, ego->td->W, 
		 ego->rs, mm, mm+2, 0);
     }
}

/*************************************************************
  Buffered code
 *************************************************************/
static void dobatch(kdftw k, R *rA, R *iA, const R *W, 
		    INT r, INT rs,
		    INT mb, INT me, INT ms,
		    R *buf, stride brs)
{
     X(cpy2d_pair_ci)(rA + mb * ms, iA + mb * ms, buf, buf + 1,
		      r, rs, WS(brs, 1),
		      me - mb, ms, 2);
     k(buf, buf + 1, W, brs, mb, me, 2);
     X(cpy2d_pair_co)(buf, buf + 1, rA + mb * ms, iA + mb * ms,
		      r, WS(brs, 1), rs,
		      me - mb, 2, ms);
}

/* must be even for SIMD alignment; should not be 2^k to avoid
   associativity conflicts */
static INT compute_batchsize(INT radix)
{
     /* round up to multiple of 4 */
     radix += 3;
     radix &= -4;

     return (radix + 2);
}

static void apply_buf(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     INT i, j, v = ego->v, r = ego->r;
     INT batchsz = compute_batchsize(r);
     R *buf;
     INT mb = ego->mb, me = ego->me;

     STACK_MALLOC(R *, buf, r * batchsz * 2 * sizeof(R));

     for (i = 0; i < v; ++i, rio += ego->vs, iio += ego->vs) {
	  for (j = mb; j < me - batchsz; j += batchsz) 
	       dobatch(ego->k, rio, iio, ego->td->W,
		       ego->r, WS(ego->rs, 1), j, j + batchsz, ego->ms,
		       buf, ego->brs);

	  dobatch(ego->k, rio, iio, ego->td->W,
		  ego->r, WS(ego->rs, 1), j, me, ego->ms,
		  buf, ego->brs);
     }

     STACK_FREE(buf);
}

/*************************************************************
  common code
 *************************************************************/
static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;

     X(twiddle_awake)(wakefulness, &ego->td, ego->slv->desc->tw,
		      ego->r * ego->m, ego->r, ego->m + ego->extra_iter);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->brs);
     X(stride_destroy)(ego->rs);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const ct_desc *e = slv->desc;

     if (slv->bufferedp)
	  p->print(p, "(dftw-directbuf/%D-%D/%D%v \"%s\")",
		   compute_batchsize(ego->r), ego->r,
		   X(twiddle_length)(ego->r, e->tw), ego->v, e->nam);
     else
	  p->print(p, "(dftw-direct-%D/%D%v \"%s\")",
		   ego->r, X(twiddle_length)(ego->r, e->tw), ego->v, e->nam);
}

static int applicable0(const S *ego,
		       INT r, INT irs, INT ors,
		       INT m, INT ms,
		       INT v, INT ivs, INT ovs,
		       INT mb, INT me,
		       R *rio, R *iio,
		       const planner *plnr, INT *extra_iter)
{
     const ct_desc *e = ego->desc;
     UNUSED(v);

     return (
	  1
	  && r == e->radix
	  && irs == ors /* in-place along R */
	  && ivs == ovs /* in-place along V */

	  /* check for alignment/vector length restrictions */
	  && ((*extra_iter = 0,
	       e->genus->okp(e, rio, iio, ivs, 0, m, mb, me, ms, plnr))
	      ||
	      (*extra_iter = 1,
	       (e->genus->okp(e, rio, iio, ivs, 0, m, 
			      mb, me - 1, ms, plnr)
		&&
		e->genus->okp(e, rio, iio, ivs, 0, m, 
			      me - 1, me + 1, ms, plnr))))

	  && (e->genus->okp(e, rio + ivs, iio + ivs, ivs, 0, m, 
			    mb, me - *extra_iter, ms, plnr))

	  );
}

static int applicable0_buf(const S *ego,
			   INT r, INT irs, INT ors,
			   INT m, INT ms,
			   INT v, INT ivs, INT ovs,
			   INT mb, INT me,
			   R *rio, R *iio,
			   const planner *plnr)
{
     const ct_desc *e = ego->desc;
     INT batchsz;
     UNUSED(v); UNUSED(ms); UNUSED(rio); UNUSED(iio);

     return (
	  1
	  && r == e->radix
	  && irs == ors /* in-place along R */
	  && ivs == ovs /* in-place along V */

	  /* check for alignment/vector length restrictions, both for
	     batchsize and for the remainder */
	  && (batchsz = compute_batchsize(r), 1)
	  && (e->genus->okp(e, 0, ((const R *)0) + 1, 2 * batchsz, 0,
			    m, mb, mb + batchsz, 2, plnr))
	  && (e->genus->okp(e, 0, ((const R *)0) + 1, 2 * batchsz, 0,
			    m, mb, me, 2, plnr))
	  );
}

static int applicable(const S *ego,
		      INT r, INT irs, INT ors,
		      INT m, INT ms,
		      INT v, INT ivs, INT ovs,
		      INT mb, INT me,
		      R *rio, R *iio,
		      const planner *plnr, INT *extra_iter)
{
     if (ego->bufferedp) {
	  *extra_iter = 0;
	  if (!applicable0_buf(ego,
			       r, irs, ors, m, ms, v, ivs, ovs, mb, me,
			       rio, iio, plnr))
	       return 0;
     } else {
	  if (!applicable0(ego,
			   r, irs, ors, m, ms, v, ivs, ovs, mb, me,
			   rio, iio, plnr, extra_iter))
	       return 0;
     }

     if (NO_UGLYP(plnr) && X(ct_uglyp)((ego->bufferedp? (INT)512 : (INT)16),
				       m * r, r))
	  return 0;

     if (m * r > 262144 && NO_FIXED_RADIX_LARGE_NP(plnr))
	  return 0;

     return 1;
}

static plan *mkcldw(const ct_solver *ego_,
		    INT r, INT irs, INT ors,
		    INT m, INT ms,
		    INT v, INT ivs, INT ovs,
		    INT mstart, INT mcount,
		    R *rio, R *iio,
		    planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const ct_desc *e = ego->desc;
     INT extra_iter;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     A(mstart >= 0 && mstart + mcount <= m);
     if (!applicable(ego,
		     r, irs, ors, m, ms, v, ivs, ovs, mstart, mstart + mcount,
		     rio, iio, plnr, &extra_iter))
          return (plan *)0;

     if (ego->bufferedp) {
	  pln = MKPLAN_DFTW(P, &padt, apply_buf);
     } else {
	  pln = MKPLAN_DFTW(P, &padt, extra_iter ? apply_extra_iter : apply);
     }

     pln->k = ego->k;
     pln->rs = X(mkstride)(r, irs);
     pln->td = 0;
     pln->r = r;
     pln->m = m;
     pln->ms = ms;
     pln->v = v;
     pln->vs = ivs;
     pln->mb = mstart;
     pln->me = mstart + mcount;
     pln->slv = ego;
     pln->brs = X(mkstride)(r, 2 * compute_batchsize(r));
     pln->extra_iter = extra_iter;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(v * (mcount/e->genus->vl), &e->ops, &pln->super.super.ops);

     if (ego->bufferedp) {
	  /* 8 load/stores * N * V */
	  pln->super.super.ops.other += 8 * r * mcount * v;
     }

     pln->super.super.could_prune_now_p =
	  (!ego->bufferedp && r >= 5 && r < 64 && m >= r);
     return &(pln->super.super);
}

static void regone(planner *plnr, kdftw codelet,
		   const ct_desc *desc, int dec, int bufferedp)
{
     S *slv = (S *)X(mksolver_ct)(sizeof(S), desc->radix, dec, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     slv->bufferedp = bufferedp;
     REGISTER_SOLVER(plnr, &(slv->super.super));
     if (X(mksolver_ct_hook)) {
	  slv = (S *)X(mksolver_ct_hook)(sizeof(S), desc->radix,
					 dec, mkcldw);
	  slv->k = codelet;
	  slv->desc = desc;
	  slv->bufferedp = bufferedp;
	  REGISTER_SOLVER(plnr, &(slv->super.super));
     }
}

void X(regsolver_ct_directw)(planner *plnr, kdftw codelet,
			     const ct_desc *desc, int dec)
{
     regone(plnr, codelet, desc, dec, /* bufferedp */ 0);
     regone(plnr, codelet, desc, dec, /* bufferedp */ 1);
}
