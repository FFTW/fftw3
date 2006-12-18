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


/*
  Compute RDFT2 of even size via either a DFT or a vector RDFT of
  size n/2.

  This file is meant as a temporary hack until we do the right thing.

  The right thing is: 1) get rid of reduction to DFT, and 2) implement
  arbitrary even-radix reduction to RDFT.  We currently reduce to DFT
  so as to exploit the SIMD code.  We currently do only radix-2 in
  order to avoid generating yet another set of codelets.
*/

#include "rdft.h"
#include "dft.h"

typedef struct {
     int (*applicable) (const problem *p_, const planner *plnr);
     void (*apply) (const plan *ego_, R *r0, R *r1, R *cr, R *ci);
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
     INT is, os, ivs, ovs;
     INT n, vl;
     const S *slv;
} P;

/* common applicability function of forward problems */
static int applicable_f(const problem *p_, const planner *plnr)
{
     const problem_rdft2 *p = (const problem_rdft2 *) p_;
     UNUSED(plnr);
     return (1
	     && p->kind == R2HC
	     && p->vecsz->rnk <= 1
	     && p->sz->rnk == 1
	     && (p->sz->dims[0].n % 2) == 0
	  );
}

/* common applicability function of backward problems */
static int applicable_b(const problem *p_, const planner *plnr)
{
     const problem_rdft2 *p = (const problem_rdft2 *) p_;

     return (1
	     && p->kind == HC2R
	     && (p->r0 == p->cr || !NO_DESTROY_INPUTP(plnr))
	     && p->vecsz->rnk <= 1
	     && p->sz->rnk == 1
	     && (p->sz->dims[0].n % 2) == 0
	  );
}

/*
 * forward rdft2 via dft
 */
static void k_f_dft(R *cr, R *ci, const R *W, INT n, INT dist)
{
     INT i;
     R *pp = cr, *pm = cr + n * dist;
     INT im = ci - cr;

     /* i = 0 and i = n */
     {
          E rop = pp[0], iop = pp[im];
          pp[0] = rop + iop;
          pm[0] = rop - iop;
          pp[im] = K(0.0);
          pm[im] = K(0.0);
	  pp += dist; pm -= dist;
     }

     /* middle elements */
     for (W += 2, i = 2; i < n; i += 2, W += 2) {
          E rop = pp[0], iop = pp[im], rom = pm[0], iom = pm[im];
          E wr = W[0], wi = W[1];
          E re = rop + rom;
          E ie = iop - iom;
          E rd = rom - rop;
          E id = iop + iom;
          E tr = rd * wr - id * wi;
          E ti = id * wr + rd * wi;
          pp[0] = K(0.5) * (re + ti);
          pp[im] = K(0.5) * (ie + tr);
          pm[0] = K(0.5) * (re - ti);
          pm[im] = K(0.5) * (tr - ie);
	  pp += dist; pm -= dist;
     }

     /* i = n/2 when n is even */
     if (!(n & 1)) pp[im] = -pp[im];
}

static void apply_f_dft(const plan *ego_, R *r0, R *r1, R *cr, R *ci)
{
     const P *ego = (const P *) ego_;

     {
          /* transform input as a vector of complex numbers */
          plan_dft *cld = (plan_dft *) ego->cld;
          cld->apply((plan *) cld, r0, r1, cr, ci);
     }

     {
          INT i, vl = ego->vl, n2 = ego->n / 2;
          INT ovs = ego->ovs, os = ego->os;
          const R *W = ego->td->W;
          for (i = 0; i < vl; ++i, cr += ovs, ci += ovs)
               k_f_dft(cr, ci, W, n2, os);
     }
}

static problem *mkcld_f_dft(const problem_rdft2 *p)
{
     const iodim *d = p->sz->dims;
     return X(mkproblem_dft_d) (
	  X(mktensor_1d)(d[0].n / 2, d[0].is, d[0].os),
	  X(tensor_copy)(p->vecsz),
	  p->r0, p->r1, p->cr, p->ci);
}

static const madt adt_f_dft = {
     applicable_f, apply_f_dft, mkcld_f_dft, {10, 8, 0, 0}, "r2hc2-dft"
};

/*
 * backward rdft2 via dft
 */
static void k_b_dft(R *cr, R *ci, const R *W, INT n, INT dist)
{
     INT i;
     R *pp = cr, *pm = cr + n * dist;
     INT im = ci - cr;

     /* i = 0 and i = n */
     {
          E rop = pp[0], iop = pm[0];
          pp[0] = rop + iop;
          pp[im] = rop - iop;
	  pp += dist; pm -= dist;
     }

     /* middle elements */
     for (W += 2, i = 2; i < n; i += 2, W += 2) {
          E a = pp[0], b = pp[im], c = pm[0], d = pm[im];
          E wr = W[0], wi = W[1];
	  E re = a + c, ti = a - c, ie = b - d, tr = b + d;
	  E rd = tr * wr + ti * wi;
	  E id = ti * wr - tr * wi;
	  pp[0] = re - rd;
	  pp[im] = ie + id;
	  pm[0] = re + rd;
	  pm[im] = id - ie;
	  pp += dist; pm -= dist;
     }

     /* i = n/2 when n is even */
     if (!(n & 1)) { pp[0] *= K(2.0); pp[im] *= -K(2.0); }
}

static void apply_b_dft(const plan *ego_, R *r0, R *r1, R *cr, R *ci)
{
     const P *ego = (const P *) ego_;
     {
          INT i, vl = ego->vl, n2 = ego->n / 2;
          INT ivs = ego->ivs, is = ego->is;
          const R *W = ego->td->W;
	  R *cr1 = cr, *ci1 = ci;
          for (i = 0; i < vl; ++i, cr1 += ivs, ci1 += ivs)
               k_b_dft(cr1, ci1, W, n2, is);
     }

     {
          plan_dft *cld = (plan_dft *) ego->cld;
	  /* swap r/i because of backward transform */
          cld->apply((plan *) cld, ci, cr, r1, r0);
     }
}

static problem *mkcld_b_dft(const problem_rdft2 *p)
{
     const iodim *d = p->sz->dims;

     return X(mkproblem_dft_d) (
	  X(mktensor_1d)(d[0].n / 2, d[0].is, d[0].os),
	  X(tensor_copy)(p->vecsz),
	  p->ci, p->cr, p->r1, p->r0);
}

static const madt adt_b_dft = {
     applicable_b, apply_b_dft, mkcld_b_dft, {10, 8, 0, 0}, "hc2r2-dft"
};

/*
 * common stuff
 */
static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     static const tw_instr twinstr[] = { {TW_FULL, 0, 2}, {TW_NEXT, 1, 0} };
     X(plan_awake)(ego->cld, wakefulness);
     X(twiddle_awake)(wakefulness, &ego->td, twinstr, 
		      ego->n, 2, (ego->n / 2 + 1) / 2);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal) (ego->cld);
}

static void print(const plan *ego_, printer * p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(%s-%D%v%(%p%))", ego->slv->adt->nam,
              ego->n, ego->vl, ego->cld);
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const problem_rdft2 *p;
     plan *cld;
     const iodim *d;

     static const plan_adt padt = {
          X(rdft2_solve), awake, print, destroy
     };

     if (!ego->adt->applicable(p_, plnr))
          return (plan *) 0;

     p = (const problem_rdft2 *) p_;

     cld = X(mkplan_d)(plnr, ego->adt->mkcld(p));
     if (!cld) return (plan *) 0;

     pln = MKPLAN_RDFT2(P, &padt, ego->adt->apply);

     d = p->sz->dims;
     pln->n = d[0].n;
     pln->os = d[0].os;
     pln->is = d[0].is;
     X(tensor_tornk1) (p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     pln->cld = cld;
     pln->td = 0;
     pln->slv = ego;

     /* approximately */
     X(ops_madd)(pln->vl * ((pln->n/2 + 1) / 2), &ego->adt->ops,
		 &cld->ops, &pln->super.super.ops);

     return &(pln->super.super);
}

static solver *mksolver(const madt *adt)
{
     static const solver_adt sadt = { PROBLEM_RDFT2, mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(rdft2_radix2_register)(planner *p)
{
     unsigned i;
     static const madt *const adts[] = {
	  &adt_f_dft, &adt_b_dft,
     };

     for (i = 0; i < sizeof(adts) / sizeof(adts[0]); ++i)
          REGISTER_SOLVER(p, mksolver(adts[i]));
}
