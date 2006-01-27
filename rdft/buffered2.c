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

/* $Id: buffered2.c,v 1.45 2006-01-27 02:10:50 athena Exp $ */

#include "rdft.h"

typedef struct {
     INT nbuf;
     INT maxbufsz;
     INT skew_alignment;
     INT skew;
     const char *nam;
} bufadt;

typedef struct {
     solver super;
     const bufadt *adt;
} S;

typedef struct {
     plan_rdft2 super;

     plan *cld, *cldrest;
     INT n, vl, nbuf, bufdist;
     INT os, ivs, ovs;

     const S *slv;
} P;

/***************************************************************************/

/* FIXME: have alternate copy functions that push a vector loop inside
   the n loops? */

/* copy halfcomplex array r (contiguous) to complex (strided) array rio/iio. */
static void hc2c(INT n, R *r, R *rio, R *iio, INT os)
{
     INT i;

     rio[0] = r[0];
     iio[0] = 0;

     for (i = 1; i + i < n; ++i) {
	  rio[i * os] = r[i];
	  iio[i * os] = r[n - i];
     }

     if (i + i == n) {	/* store the Nyquist frequency */
	  rio[i * os] = r[i];
	  iio[i * os] = K(0.0);
     }
}

/* reverse of hc2c */
static void c2hc(INT n, R *rio, R *iio, INT is, R *r)
{
     INT i;

     r[0] = rio[0];

     for (i = 1; i + i < n; ++i) {
	  r[i] = rio[i * is];
	  r[n - i] = iio[i * is];
     }

     if (i + i == n)		/* store the Nyquist frequency */
	  r[i] = rio[i * is];
}

/***************************************************************************/

static void apply_r2hc(const plan *ego_, R *r, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld = (plan_rdft *) ego->cld;
     INT i, j, vl = ego->vl, nbuf = ego->nbuf, bufdist = ego->bufdist;
     INT n = ego->n;
     INT ivs = ego->ivs, ovs = ego->ovs, os = ego->os;
     R *bufs;

     bufs = (R *)MALLOC(sizeof(R) * nbuf * bufdist, BUFFERS);

     for (i = nbuf; i <= vl; i += nbuf) {
          /* transform to bufs: */
          cld->apply((plan *) cld, r, bufs);
	  r += ivs;

          /* copy back */
	  for (j = 0; j < nbuf; ++j, rio += ovs, iio += ovs)
	       hc2c(n, bufs + j*bufdist, rio, iio, os);
     }

     /* Do the remaining transforms, if any: */
     {
	  plan_rdft *cldrest = (plan_rdft *) ego->cldrest;
	  R *b = bufs;
	  cldrest->apply((plan *) cldrest, r, bufs);
	  for (i -= nbuf; i < vl; ++i, rio += ovs, iio += ovs, b += bufdist)
	       hc2c(n, b, rio, iio, os);
     }

     X(ifree)(bufs);
}

static void apply_hc2r(const plan *ego_, R *r, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld = (plan_rdft *) ego->cld;
     INT i, j, vl = ego->vl, nbuf = ego->nbuf, bufdist = ego->bufdist;
     INT n = ego->n;
     INT ivs = ego->ivs, ovs = ego->ovs, is = ego->os;
     R *bufs;

     bufs = (R *)MALLOC(sizeof(R) * nbuf * bufdist, BUFFERS);

     for (i = nbuf; i <= vl; i += nbuf) {
          /* copy to bufs */
	  for (j = 0; j < nbuf; ++j, rio += ivs, iio += ivs)
	       c2hc(n, rio, iio, is, bufs + j*bufdist);

          /* transform back: */
          cld->apply((plan *) cld, bufs, r);
	  r += ovs;
     }

     /* Do the remaining transforms, if any: */
     {
	  plan_rdft *cldrest;
	  R *b = bufs;
	  for (i -= nbuf; i < vl; ++i, rio += ivs, iio += ivs, b += bufdist)
	       c2hc(n, rio, iio, is, b);
	  cldrest = (plan_rdft *) ego->cldrest;
	  cldrest->apply((plan *) cldrest, bufs, r);
     }

     X(ifree)(bufs);
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;

     X(plan_awake)(ego->cld, wakefulness);
     X(plan_awake)(ego->cldrest, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cldrest);
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(%s-%s-%D%v/%D-%D%(%p%)%(%p%))",
              ego->slv->adt->nam,
	      ego->super.apply == apply_r2hc ? "r2hc" : "hc2r",
              ego->n, ego->nbuf,
              ego->vl, ego->bufdist % ego->n,
              ego->cld, ego->cldrest);
}

static INT min_nbuf(const problem_rdft2 *p, INT n, INT vl)
{
     INT is, os, ivs, ovs;

     if (p->r != p->rio && p->r != p->iio)
	  return 1;
     if (X(rdft2_inplace_strides(p, RNK_MINFTY)))
	  return 1;
     A(p->vecsz->rnk == 1); /*  rank 0 and MINFTY are inplace */

     X(rdft2_strides)(p->kind, p->sz->dims, &is, &os);
     X(rdft2_strides)(p->kind, p->vecsz->dims, &ivs, &ovs);
     
     /* handle one potentially common case: "contiguous" real and
	complex arrays, which overlap because of the differing sizes. */
     if (n * X(iabs)(is) <= X(iabs)(ivs)
	 && (n/2 + 1) * X(iabs)(os) <= X(iabs)(ovs)
	 && ( ((p->rio - p->iio) <= X(iabs)(os)) || 
	      ((p->iio - p->rio) <= X(iabs)(os)) )
	 && ivs > 0 && ovs > 0) {
	  INT vsmin = X(imin)(ivs, ovs);
	  INT vsmax = X(imax)(ivs, ovs);
	  return(((vsmax - vsmin) * vl + vsmin - 1) / vsmin);
     }

     return vl; /* punt: just buffer the whole vector */
}

static INT compute_nbuf(INT n, INT vl, const S *ego)
{
     return X(compute_nbuf)(n, vl, ego->adt->nbuf, ego->adt->maxbufsz);
}

static int toobig(INT n, const S *ego)
{
     return (n > ego->adt->maxbufsz);
}

static int applicable0(const problem *p_, const S *ego, const planner *plnr)
{
     const problem_rdft2 *p = (const problem_rdft2 *) p_;
     UNUSED(ego);
     return(p->vecsz->rnk <= 1 && p->sz->rnk == 1
	    && !(toobig(p->sz->dims[0].n, ego) && CONSERVE_MEMORYP(plnr)));
}

static int applicable(const problem *p_, const S *ego, const planner *plnr)
{
     const problem_rdft2 *p;

     if (NO_BUFFERINGP(plnr)) return 0;

     if (!applicable0(p_, ego, plnr)) return 0;

     p = (const problem_rdft2 *) p_;
     if (NO_UGLYP(plnr)) {
	  if (p->r != p->rio && p->r != p->iio) return 0;
	  if (toobig(p->sz->dims[0].n, ego)) return 0;
     }
     return 1;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const bufadt *adt = ego->adt;
     P *pln;
     plan *cld = (plan *) 0;
     plan *cldrest = (plan *) 0;
     problem *cldp = 0;
     const problem_rdft2 *p = (const problem_rdft2 *) p_;
     R *bufs = (R *) 0;
     INT nbuf = 0, bufdist, n, vl;
     INT ivs, ovs;
     unsigned u_reset = 0;

     static const plan_adt padt = {
	  X(rdft2_solve), awake, print, destroy
     };


     if (!applicable(p_, ego, plnr))
          goto nada;

     n = p->sz->dims[0].n;
     X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs);

     nbuf = X(imax)(compute_nbuf(n, vl, ego), min_nbuf(p, n, vl));
     A(nbuf > 0);

     /*
      * Determine BUFDIST, the offset between successive array bufs.
      * bufdist = n + skew, where skew is chosen such that bufdist %
      * skew_alignment = skew.
      */
     if (vl == 1) {
          bufdist = n;
     } else {
          bufdist =
               n + ((adt->skew_alignment + adt->skew - n % adt->skew_alignment)
                    % adt->skew_alignment);
          A(p->vecsz->rnk == 1);
     }

     /* initial allocation for the purpose of planning */
     bufs = (R *) MALLOC(sizeof(R) * nbuf * bufdist, BUFFERS);

     if (p->r == p->rio || p->r == p->iio)
	  u_reset = NO_DESTROY_INPUT; /* ok to destroy input */

     if (p->kind == R2HC)
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, p->sz->dims[0].is, 1),
		    X(mktensor_1d)(nbuf, ivs, bufdist),
		    TAINT(p->r, ivs * nbuf), bufs, &p->kind);
     else {
	  A(p->kind == HC2R);
	  u_reset = NO_DESTROY_INPUT; /* always ok to destroy buf */
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, 1, p->sz->dims[0].os),
		    X(mktensor_1d)(nbuf, bufdist, ovs),
		    bufs, TAINT(p->r, ovs * nbuf), &p->kind);
     }
     if (!(cld = X(mkplan_f_d)(plnr, cldp, 0, 0, u_reset))) goto nada;

     /* plan the leftover transforms (cldrest): */
     if (p->kind == R2HC)
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, p->sz->dims[0].is, 1),
		    X(mktensor_1d)(vl % nbuf, ivs, bufdist),
		    p->r + ivs * (nbuf * (vl / nbuf)), bufs, &p->kind);
     else /* HC2R */
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, 1, p->sz->dims[0].os),
		    X(mktensor_1d)(vl % nbuf, bufdist, ovs),
			 bufs, p->r + ovs * (nbuf * (vl / nbuf)), &p->kind);
     if (!(cldrest = X(mkplan_d)(plnr, cldp))) goto nada;

     /* deallocate buffers, let apply() allocate them for real */
     X(ifree)(bufs);
     bufs = 0;

     pln = MKPLAN_RDFT2(P, &padt, p->kind == R2HC ? apply_r2hc : apply_hc2r);
     pln->cld = cld;
     pln->cldrest = cldrest;
     pln->slv = ego;
     pln->n = n;
     pln->vl = vl;
     if (p->kind == R2HC) {
	  pln->ivs = ivs * nbuf;
	  pln->ovs = ovs;
	  pln->os = p->sz->dims[0].os; /* stride of rio/iio  */
     }
     else { /* HC2R */
	  pln->ivs = ivs;
	  pln->ovs = ovs * nbuf;
	  pln->os = p->sz->dims[0].is; /* stride of rio/iio  */
     }


     pln->nbuf = nbuf;
     pln->bufdist = bufdist;

     X(ops_madd)(vl / nbuf, &cld->ops, &cldrest->ops,
		 &pln->super.super.ops);
     pln->super.super.ops.other += (p->kind == R2HC ? (n + 2) : n) * vl;

     return &(pln->super.super);

 nada:
     X(ifree0)(bufs);
     X(plan_destroy_internal)(cldrest);
     X(plan_destroy_internal)(cld);
     return (plan *) 0;
}

static solver *mksolver(const bufadt *adt)
{
     static const solver_adt sadt = { PROBLEM_RDFT2, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(rdft2_buffered_register)(planner *p)
{
     /* FIXME: what are good defaults? */
     static const bufadt adt = {
	  /* nbuf */           8,
	  /* maxbufsz */       (INT)(65536 / sizeof(R)),
	  /* skew_alignment */ 8,
	  /* skew */           5,
	  /* nam */            "rdft2-buffered"
     };

     REGISTER_SOLVER(p, mksolver(&adt));
}
