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

/* $Id: buffered2.c,v 1.25 2003-01-13 09:20:37 athena Exp $ */

#include "rdft.h"

typedef struct {
     uint nbuf;
     uint maxbufsz;
     uint skew_alignment;
     uint skew;
     const char *nam;
} bufadt;

typedef struct {
     solver super;
     const bufadt *adt;
} S;

typedef struct {
     plan_rdft2 super;

     plan *cld, *cldrest;
     uint n, vl, nbuf, bufdist;
     int os, ivs, ovs;

     const S *slv;
} P;

/***************************************************************************/

/* FIXME: have alternate copy functions that push a vector loop inside
   the n loops? */

/* copy halfcomplex array r (contiguous) to complex (strided) array rio/iio. */
static void hc2c(uint n, R *r, R *rio, R *iio, int os)
{
     uint n2 = (n + 1) / 2;
     uint i;

     rio[0] = r[0];
     iio[0] = 0;
     for (i = 1; i < ((n2 - 1) & 3) + 1; ++i) {
	  rio[i * os] = r[i];
	  iio[i * os] = r[n - i];
     }
     for (; i < n2; i += 4) {
	  R r0, r1, r2, r3;
	  R i0, i1, i2, i3;
	  r0 = r[i];
	  r1 = r[i + 1];
	  r2 = r[i + 2];
	  r3 = r[i + 3];
	  i3 = r[n - (i + 3)];
	  i2 = r[n - (i + 2)];
	  i1 = r[n - (i + 1)];
	  i0 = r[n - i];
	  rio[i * os] = r0;
	  iio[i * os] = i0;
	  rio[(i + 1) * os] = r1;
	  iio[(i + 1) * os] = i1;
	  rio[(i + 2) * os] = r2;
	  iio[(i + 2) * os] = i2;
	  rio[(i + 3) * os] = r3;
	  iio[(i + 3) * os] = i3;
     }
     if ((n & 1) == 0) {	/* store the Nyquist frequency */
	  rio[n2 * os] = r[n2];
	  iio[n2 * os] = 0.0;
     }
}

/* reverse of hc2c */
static void c2hc(uint n, R *rio, R *iio, int is, R *r)
{
     uint n2 = (n + 1) / 2;
     uint i;

     r[0] = rio[0];
     for (i = 1; i < ((n2 - 1) & 3) + 1; ++i) {
	  r[i] = rio[i * is];
	  r[n - i] = iio[i * is];
     }
     for (; i < n2; i += 4) {
	  R r0, r1, r2, r3;
	  R i0, i1, i2, i3;
	  r0 = rio[i * is];
	  i0 = iio[i * is];
	  r1 = rio[(i + 1) * is];
	  i1 = iio[(i + 1) * is];
	  r2 = rio[(i + 2) * is];
	  i2 = iio[(i + 2) * is];
	  r3 = rio[(i + 3) * is];
	  i3 = iio[(i + 3) * is];
	  r[i] = r0;
	  r[i + 1] = r1;
	  r[i + 2] = r2;
	  r[i + 3] = r3;
	  r[n - (i + 3)] = i3;
	  r[n - (i + 2)] = i2;
	  r[n - (i + 1)] = i1;
	  r[n - i] = i0;
     }
     if ((n & 1) == 0)		/* store the Nyquist frequency */
	  r[n2] = rio[n2 * is];
}

/***************************************************************************/

static void apply_r2hc(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;
     plan_rdft *cld = (plan_rdft *) ego->cld;
     uint i, j, vl = ego->vl, nbuf = ego->nbuf, bufdist = ego->bufdist;
     uint n = ego->n;
     int ivs = ego->ivs, ovs = ego->ovs, os = ego->os;
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

static void apply_hc2r(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;
     plan_rdft *cld = (plan_rdft *) ego->cld;
     uint i, j, vl = ego->vl, nbuf = ego->nbuf, bufdist = ego->bufdist;
     uint n = ego->n;
     int ivs = ego->ivs, ovs = ego->ovs, is = ego->os;
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

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld, flg);
     AWAKE(ego->cldrest, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cldrest);
     X(plan_destroy_internal)(ego->cld);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(%s-%s-%u%v/%u-%u%(%p%)%(%p%))",
              ego->slv->adt->nam,
	      ego->super.apply == apply_r2hc ? "r2hc" : "hc2r",
              ego->n, ego->nbuf,
              ego->vl, ego->bufdist % ego->n,
              ego->cld, ego->cldrest);
}

static uint min_nbuf(const problem_rdft2 *p, uint n, uint vl)
{
     int is, os, ivs, ovs;

     if (p->r != p->rio && p->r != p->iio)
	  return 1;
     if (X(rdft2_inplace_strides(p, RNK_MINFTY)))
	  return 1;
     A(p->vecsz->rnk == 1); /*  rank 0 and MINFTY are inplace */

     if (R2HC_KINDP(p->kind)) {
	  ivs =  p->vecsz->dims[0].is; /* real stride */
	  is = p->sz->dims[0].is;
	  ovs =  p->vecsz->dims[0].os; /* complex stride */
	  os = p->sz->dims[0].os;
     }
     else {
	  ivs =  p->vecsz->dims[0].os; /* real stride */
	  is = p->sz->dims[0].os;
	  ovs =  p->vecsz->dims[0].is; /* complex stride */
	  os = p->sz->dims[0].is;
     }
     
     /* handle one potentially common case: "contiguous" real and
	complex arrays, which overlap because of the differing sizes. */
     if (n * X(iabs)(is) <= X(iabs)(ivs)
	 && (n/2 + 1) * X(iabs)(os) <= X(iabs)(ovs)
	 && X(iabs)((int) (p->rio - p->iio)) <= X(iabs)(os)
	 && ivs > 0 && ovs > 0) {
	  uint vsmin = X(uimin)(ivs, ovs);
	  uint vsmax = X(uimax)(ivs, ovs);
	  return(((vsmax - vsmin) * vl + vsmin - 1) / vsmin);
     }

     return vl; /* punt: just buffer the whole vector */
}

static uint compute_nbuf(uint n, uint vl, const S *ego)
{
     return X(compute_nbuf)(n, vl, ego->adt->nbuf, ego->adt->maxbufsz);
}

static int toobig(uint n, const S *ego)
{
     return (n > ego->adt->maxbufsz);
}

static int applicable0(const problem *p_, const S *ego, const planner *plnr)
{
     UNUSED(ego);
     if (RDFT2P(p_)) {
          const problem_rdft2 *p = (const problem_rdft2 *) p_;
	  return(p->vecsz->rnk <= 1 && p->sz->rnk == 1
		 && !(toobig(p->sz->dims[0].n, ego) && CONSERVE_MEMORYP(plnr)));
     }
     return 0;
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
     uint nbuf = 0, bufdist, n, vl;
     int ivs, ovs;

     static const plan_adt padt = {
	  X(rdft2_solve), awake, print, destroy
     };


     if (!applicable(p_, ego, plnr))
          goto nada;

     n = p->sz->dims[0].n;
     X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs);

     nbuf = X(uimax)(compute_nbuf(n, vl, ego), min_nbuf(p, n, vl));
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

     if (R2HC_KINDP(p->kind))
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, p->sz->dims[0].is, 1),
		    X(mktensor_1d)(nbuf, ivs, bufdist),
		    p->r, bufs, &p->kind);
     else {
	  plnr->problem_flags |= DESTROY_INPUT; /* always ok to destroy buf */
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, 1, p->sz->dims[0].os),
		    X(mktensor_1d)(nbuf, bufdist, ovs),
		    bufs, p->r, &p->kind);
     }
     if (!(cld = X(mkplan_d)(plnr, cldp))) goto nada;

     /* plan the leftover transforms (cldrest): */
     if (R2HC_KINDP(p->kind))
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, p->sz->dims[0].is, 1),
		    X(mktensor_1d)(vl % nbuf, ivs, bufdist),
		    p->r, bufs, &p->kind);
     else
	  cldp =
	       X(mkproblem_rdft_d)(
		    X(mktensor_1d)(n, 1, p->sz->dims[0].os),
		    X(mktensor_1d)(vl % nbuf, bufdist, ovs),
		    bufs, p->r, &p->kind);
     if (!(cldrest = X(mkplan_d)(plnr, cldp))) goto nada;

     /* deallocate buffers, let apply() allocate them for real */
     X(ifree)(bufs);
     bufs = 0;

     pln = MKPLAN_RDFT2(P, &padt, R2HC_KINDP(p->kind) ? apply_r2hc:apply_hc2r);
     pln->cld = cld;
     pln->cldrest = cldrest;
     pln->slv = ego;
     pln->n = n;
     pln->vl = vl;
     if (R2HC_KINDP(p->kind)) {
	  pln->ivs = ivs * nbuf;
	  pln->ovs = ovs;
	  pln->os = p->sz->dims[0].os; /* stride of rio/iio  */
     }
     else {
	  pln->ivs = ivs;
	  pln->ovs = ovs * nbuf;
	  pln->os = p->sz->dims[0].is; /* stride of rio/iio  */
     }


     pln->nbuf = nbuf;
     pln->bufdist = bufdist;

     X(ops_madd)(vl / nbuf, &cld->ops, &cldrest->ops,
		 &pln->super.super.ops);
     pln->super.super.ops.other += (R2HC_KINDP(p->kind) ? (n + 2) : n) * vl;

     return &(pln->super.super);

 nada:
     X(ifree0)(bufs);
     X(plan_destroy_internal)(cldrest);
     X(plan_destroy_internal)(cld);
     return (plan *) 0;
}

static solver *mksolver(const bufadt *adt)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(rdft2_buffered_register)(planner *p)
{
     /* FIXME: what are good defaults? */
     static const bufadt adt = {
	  /* nbuf */           8,
	  /* maxbufsz */       (65536 / sizeof(R)),
	  /* skew_alignment */ 8,
	  /* skew */           5,
	  /* nam */            "rdft2-buffered"
     };

     REGISTER_SOLVER(p, mksolver(&adt));
}
