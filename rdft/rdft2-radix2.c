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

/* $Id: rdft2-radix2.c,v 1.5 2002-08-29 12:20:55 athena Exp $ */

/*
  Compute RDFT2 of even size via either a DFT or a vector RDFT of
  size n/2.

  This file is meant as a temporary hack until we do the right thing.

  The right thing is: 1) get rid of reduction to DFT, and 2) implement
  arbitrary even-radix reduction to RDFT.  We currently reduce to DFT
  so as to exploit the SIMD code.  We currently do radix-2 only in
  order to avoid generating yet another set of codelets.
*/

#include "rdft.h"
#include "dft.h"

typedef struct {
     int (*applicable) (const problem *p_, const planner *plnr);
     void (*apply) (plan *ego_, R *r, R *rio, R *iio);
     problem *(*mkcld) (const problem_rdft2 *p);
     opcnt ops;
     const char *nam;
} madt;

typedef struct {
     solver super;
     const madt *adt;
} S;

typedef struct {
     plan_dft super;
     plan *cld;
     twid *td;
     int is, os, ivs, ovs;
     uint n, vl;
     const S *slv;
} P;

/* common applicability function of forward problems */
static int applicable_f(const problem *p_, const planner *plnr)
{
     UNUSED(plnr);
     if (RDFT2P(p_)) {
          const problem_rdft2 *p = (const problem_rdft2 *) p_;
          return (1
                  && p->kind == R2HC
                  && p->vecsz.rnk <= 1
                  && p->sz.rnk == 1
		  && (p->sz.dims[0].n % 2) == 0
	       );
     }

     return 0;
}

/* common applicability function of backward problems */
static int applicable_b(const problem *p_, const planner *plnr)
{
     if (RDFT2P(p_)) {
          const problem_rdft2 *p = (const problem_rdft2 *) p_;
          return (1
                  && p->kind == HC2R
		  && (p->r == p->rio || (plnr->flags & DESTROY_INPUT))
                  && p->vecsz.rnk <= 1
                  && p->sz.rnk == 1
		  && (p->sz.dims[0].n % 2) == 0
	       );
     }

     return 0;
}


/*
 * forward rdft2 via dft
 */
static void k_f_dft(R *rio, R *iio, const R *W, uint n, int dist)
{
     uint i;
     R *riop, *iiop, *riom, *iiom;

     riop = rio; iiop = iio; riom = rio + n * dist; iiom = iio + n * dist;

     /* i = 0 and i = n */
     {
          E rop = *riop, iop = *iiop;
          *riop = rop + iop;      riop += dist;
          *riom = rop - iop;      riom -= dist;
          *iiop = 0.0;            iiop += dist;
          *iiom = 0.0;            iiom -= dist;
     }

     /* middle elements */
     for (W += 2, i = 2; i < n; i += 2, W += 2) {
          E rop = *riop, iop = *iiop, rom = *riom, iom = *iiom;
          E wr = W[0], wi = W[1];
          E re = rop + rom;
          E ie = iop - iom;
          E rd = rom - rop;
          E id = iop + iom;
          E tr = rd * wr - id * wi;
          E ti = id * wr + rd * wi;
          *riop = 0.5 * (re + ti);          riop += dist;
          *iiop = 0.5 * (ie + tr);          iiop += dist;
          *riom = 0.5 * (re - ti);          riom -= dist;
          *iiom = 0.5 * (tr - ie);          iiom -= dist;
     }

     /* i = n/2 when n is even */
     if (!(n & 1)) *iiop = -(*iiop);
}

static void apply_f_dft(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;

     {
          /* transform input as a vector of complex numbers */
          plan_dft *cld = (plan_dft *) ego->cld;
          cld->apply((plan *) cld, r, r + ego->is, rio, iio);
     }

     {
          uint i, vl = ego->vl, n2 = ego->n / 2;
          int ovs = ego->ovs, os = ego->os;
          const R *W = ego->td->W;
          for (i = 0; i < vl; ++i, rio += ovs, iio += ovs)
               k_f_dft(rio, iio, W, n2, os);
     }
}

static problem *mkcld_f_dft(const problem_rdft2 *p)
{
     const iodim *d = p->sz.dims;
     return X(mkproblem_dft_d) (
	  X(mktensor_1d)(d[0].n / 2, d[0].is * 2, d[0].os),
	  X(tensor_copy)(p->vecsz), 
	  p->r, p->r + d[0].is, p->rio, p->iio);
}

static const madt adt_f_dft = {
     applicable_f, apply_f_dft, mkcld_f_dft, {10, 8, 0, 0}, "r2hc2-dft"
};

/*
 * forward rdft2 via rdft
 */
static void k_f_rdft(R *rio, R *iio, const R *W, uint n, int dist)
{
     uint i;
     R *riop, *iiop, *riom, *iiom;

     riop = rio; iiop = iio; riom = rio + n * dist; iiom = iio + n * dist;

     /* i = 0 and i = n */
     {
          E rop = *riop, iop = *iiop;
          *riop = rop + iop;      riop += dist;
          *riom = rop - iop;      riom -= dist;
          *iiop = 0.0;            iiop += dist;
          *iiom = 0.0;            iiom -= dist;
     }

     /* middle elements */
     for (W += 2, i = 2; i < n; i += 2, W += 2) {
          E r0 = *riop, r1 = *iiop, i0 = *riom, i1 = *iiom;
          E wr = W[0], wi = W[1];
          E tr = r1 * wr + i1 * wi;
          E ti = i1 * wr - r1 * wi;
          *riop = (r0 + tr);          riop += dist;
          *iiop = (i0 + ti);          iiop += dist;
          *riom = (r0 - tr);          riom -= dist;
          *iiom = (ti - i0);          iiom -= dist;
     }

     /* i = n/2 when n is even */
     if (!(n & 1)) *iiop = -(*iiop);
}

static void apply_f_rdft(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;

     {
          plan_rdft *cld = (plan_rdft *) ego->cld;
          cld->apply((plan *) cld, r, rio);
     }

     {
          uint i, vl = ego->vl, n2 = ego->n / 2;
          int ovs = ego->ovs, os = ego->os;
          const R *W = ego->td->W;
          for (i = 0; i < vl; ++i, rio += ovs, iio += ovs)
               k_f_rdft(rio, iio, W, n2, os);
     }
}

static problem *mkcld_f_rdft(const problem_rdft2 *p)
{
     const iodim *d = p->sz.dims;

     tensor radix = X(mktensor_1d)(2, d[0].is, p->iio - p->rio);
     tensor cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);

     return X(mkproblem_rdft_1_d) (
	  X(mktensor_1d)(d[0].n / 2, 2 * d[0].is, d[0].os),
	  cld_vec, p->r, p->rio, R2HC);
}

static const madt adt_f_rdft = {
     applicable_f, apply_f_rdft, mkcld_f_rdft, {6, 4, 0, 0}, "r2hc2-rdft"
};


/*
 * backward rdft2 via dft
 */
static void k_b_dft(R *rio, R *iio, const R *W, uint n, int dist)
{
     uint i;
     R *riop, *iiop, *riom, *iiom;

     riop = rio; iiop = iio; riom = rio + n * dist; iiom = iio + n * dist;

     /* i = 0 and i = n */
     {
          E rop = *riop, iop = *riom;
          *riop = rop + iop;      riop += dist;
          *iiop = rop - iop;      riom -= dist;
                                  iiop += dist;
                                  iiom -= dist;
     }

     /* middle elements */
     for (W += 2, i = 2; i < n; i += 2, W += 2) {
          E a = *riop, b = *iiop, c = *riom, d = *iiom;
          E wr = W[0], wi = W[1];
	  E re = a + c, ti = a - c, ie = b - d, tr = b + d;
	  E rd = tr * wr + ti * wi;
	  E id = ti * wr - tr * wi;
	  *riop = re - rd;          riop += dist;
	  *iiop = ie + id;          iiop += dist;
	  *riom = re + rd;          riom -= dist;
	  *iiom = id - ie;          iiom -= dist;
     }

     /* i = n/2 when n is even */
     if (!(n & 1)) { *riop *= 2.0; *iiop *= -2.0; }
}

static void apply_b_dft(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;
     {
          uint i, vl = ego->vl, n2 = ego->n / 2;
          int ivs = ego->ivs, is = ego->is;
          const R *W = ego->td->W;
	  R *rio1 = rio, *iio1 = iio;
          for (i = 0; i < vl; ++i, rio1 += ivs, iio1 += ivs)
               k_b_dft(rio, iio, W, n2, is);
     }

     {
          plan_dft *cld = (plan_dft *) ego->cld;
	  /* swap r/i because of backward transform */
          cld->apply((plan *) cld, iio, rio, r + ego->os, r);
     }
}

static problem *mkcld_b_dft(const problem_rdft2 *p)
{
     const iodim *d = p->sz.dims;

     return X(mkproblem_dft_d) (
	  X(mktensor_1d)(d[0].n / 2, d[0].is, 2 * d[0].os),
	  X(tensor_copy)(p->vecsz), 
	  p->iio, p->rio, p->r + d[0].os, p->r);
}

static const madt adt_b_dft = {
     applicable_b, apply_b_dft, mkcld_b_dft, {10, 8, 0, 0}, "hc2r2-dft"
};

/*
 * backward rdft2 via backward rdft
 */
static void k_b_rdft(R *rio, R *iio, const R *W, uint n, int dist)
{
     uint i;
     R *riop, *iiop, *riom, *iiom;

     riop = rio; iiop = iio; riom = rio + n * dist; iiom = iio + n * dist;

     /* i = 0 and i = n */
     {
          E rop = *riop, iop = *riom;
          *riop = rop + iop;      riop += dist;
          *iiop = rop - iop;      riom -= dist;
                                  iiop += dist;
                                  iiom -= dist;
     }

     /* middle elements */
     for (W += 2, i = 2; i < n; i += 2, W += 2) {
          E a = *riop, b = *iiop, c = *riom, d = *iiom;
          E wr = W[0], wi = W[1];
	  E r0 = a + c, r1 = a - c, i0 = b - d, i1 = b + d;
	  *riop = r0;                 riop += dist;
	  *riom = i0;		      riom -= dist;
	  *iiop = r1 * wr - i1 * wi;  iiop += dist;
	  *iiom = i1 * wr + r1 * wi;  iiom -= dist;
     }

     /* i = n/2 when n is even */
     if (!(n & 1)) { *riop *= 2.0; *iiop *= -2.0; }
}

static void apply_b_rdft(plan *ego_, R *r, R *rio, R *iio)
{
     P *ego = (P *) ego_;

     {
          uint i, vl = ego->vl, n2 = ego->n / 2;
          int ivs = ego->ivs, is = ego->is;
          const R *W = ego->td->W;
	  R *rio1 = rio, *iio1 = iio;
          for (i = 0; i < vl; ++i, rio1 += ivs, iio1 += ivs)
               k_b_rdft(rio1, iio1, W, n2, is);
     }

     {
          plan_rdft *cld = (plan_rdft *) ego->cld;
          cld->apply((plan *) cld, rio, r);
     }
}

static problem *mkcld_b_rdft(const problem_rdft2 *p)
{
     const iodim *d = p->sz.dims;

     tensor radix = X(mktensor_1d)(2, p->iio - p->rio, d[0].os);
     tensor cld_vec = X(tensor_append)(radix, p->vecsz);
     X(tensor_destroy)(radix);

     return X(mkproblem_rdft_1_d) (
	  X(mktensor_1d)(d[0].n / 2, d[0].is, 2 * d[0].os),
	  cld_vec, p->rio, p->r, HC2R);
}

static const madt adt_b_rdft = {
     applicable_b, apply_b_rdft, mkcld_b_rdft, {6, 4, 0, 0}, "hc2r2-rdft"
};

/*
 * common stuff 
 */
static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     static const tw_instr twinstr[] = { {TW_FULL, 0, 2}, {TW_NEXT, 1, 0} };
     AWAKE(ego->cld, flg);
     X(twiddle_awake)(flg, &ego->td, twinstr, ego->n, 2, (ego->n / 2 + 1) / 2);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy) (ego->cld);
     X(free) (ego);
}

static void print(plan *ego_, printer * p)
{
     P *ego = (P *) ego_;
     p->print(p, "(%s-%u%v%(%p%))", ego->slv->adt->nam,
              ego->n, ego->vl, ego->cld);
}

static int score(const solver *ego_, const problem *p_,
                 const planner *plnr)
{
     const S *ego = (const S *) ego_;
     UNUSED(plnr);
     if (ego->adt->applicable(p_, plnr))
          return GOOD;
     return BAD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_rdft2 *p;
     problem *cldp;
     plan *cld;
     const iodim *d;

     static const plan_adt padt = {
          X(rdft2_solve), awake, print, destroy
     };

     if (!ego->adt->applicable(p_, plnr))
          return (plan *) 0;

     p = (const problem_rdft2 *) p_;

     cldp = ego->adt->mkcld(p);
     cld = MKPLAN(plnr, cldp);
     X(problem_destroy) (cldp);
     if (!cld)
          return (plan *) 0;

     pln = MKPLAN_RDFT2(P, &padt, ego->adt->apply);

     d = p->sz.dims;
     pln->n = d[0].n;
     pln->os = d[0].os;
     pln->is = d[0].is;
     X(tensor_tornk1) (&p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     pln->cld = cld;
     pln->td = 0;
     pln->slv = ego;

     pln->super.super.ops =      /* approximately */
	  X(ops_add)(cld->ops,
		     X(ops_mul)(pln->vl * ((pln->n/2 + 1) / 2),
				ego->adt->ops));

     return &(pln->super.super);
}

static solver *mksolver(const madt *adt)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(rdft2_radix2_register)(planner *p)
{
     uint i;
     static const madt *const adts[] = {
	  &adt_f_dft, &adt_f_rdft, 
	  &adt_b_dft, &adt_b_rdft
     };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
          REGISTER_SOLVER(p, mksolver(adts[i]));
}
