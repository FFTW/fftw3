/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

#include "dft.h"

/*
 * Compute transforms of prime sizes using Rader's trick: turn them
 * into convolutions of size n - 1, which you then perform via a pair
 * of FFTs.   This file contains both nontwiddle (direct) and 
 * twiddle (DIT Cooley-Tukey) solvers.
 */

typedef struct {
     solver super;
} S;

typedef struct {
     plan_dft super;

     plan *cld1, *cld2;
     R *omega;
     int n, g, ginv;
     int is, os;
     plan *cld_omega;
} P;

typedef struct {
     P super;
     plan *cld;
     R *W;
     int os;
     int m;
} P_dit;


static rader_tl *twiddles = 0;

/***************************************************************************/

/* Below, we extensively use the identity that fft(x*)* = ifft(x) in
   order to share data between forward and backward transforms and to
   obviate the necessity of having separate forward and backward
   plans.  (Although we often compute separate plans these days anyway
   due to the differing strides, etcetera.)

   Of course, since the new FFTW gives us separate pointers to
   the real and imaginary parts, we could have instead used the
   fft(r,i) = ifft(i,r) form of this identity, but it was easier to
   reuse the code from our old version. */

static void apply_aux(int r, int ginv, plan *cld1,plan *cld2, const R *omega,
		      R *buf, R r0, R i0, R *ro, R *io, int os)
{
     int gpower, k;

     /* compute DFT of buf, storing in output (except DC): */
     {
	    plan_dft *cld = (plan_dft *) cld1;
	    cld->apply(cld1, buf, buf+1, ro+os, io+os);
     }

     /* set output DC component: */
     ro[0] = r0 + ro[os];
     io[0] = i0 + io[os];

     /* now, multiply by omega: */
     for (k = 0; k < r - 1; ++k) {
	  E rB, iB, rW, iW;
	  rW = omega[2*k];
	  iW = omega[2*k+1];
	  rB = ro[(k+1)*os];
	  iB = io[(k+1)*os];
	  ro[(k+1)*os] = rW * rB - iW * iB;
	  io[(k+1)*os] = -(rW * iB + iW * rB);
     }
     
     /* this will add input[0] to all of the outputs after the ifft */
     ro[os] += r0;
     io[os] -= i0;

     /* inverse FFT: */
     {
	    plan_dft *cld = (plan_dft *) cld2;
	    cld->apply(cld2, ro+os, io+os, buf, buf+1);
     }
     
     /* finally, do inverse permutation to unshuffle the output: */
     gpower = 1;
     for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, ginv, r)) {
	  ro[gpower * os] = buf[2*k];
	  io[gpower * os] = -buf[2*k+1];
     }
     A(gpower == 1);
}

static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int is;
     int k, gpower, g, r;
     R *buf;

     r = ego->n; is = ego->is; g = ego->g; 
     buf = (R *) MALLOC(sizeof(R) * (r - 1) * 2, BUFFERS);

     /* First, permute the input, storing in buf: */
     for (gpower = 1, k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	  R rA, iA;
	  rA = ri[gpower * is];
	  iA = ii[gpower * is];
	  buf[2*k] = rA; buf[2*k + 1] = iA;
     }
     /* gpower == g^(r-1) mod r == 1 */;

     apply_aux(r, ego->ginv, ego->cld1, ego->cld2, ego->omega, 
	       buf, ri[0], ii[0], ro, io, ego->os);

     X(ifree)(buf);
}

static void apply_dit(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P_dit *ego_dit = (const P_dit *) ego_;
     const P *ego;
     plan *cld1, *cld2;
     int os, osm;
     int j, k, gpower, g, ginv, r, m;
     R *buf;
     const R *omega, *W;

     {
	   plan *cld0 = ego_dit->cld;
	   plan_dft *cld = (plan_dft *) cld0;
	   cld->apply(cld0, ri, ii, ro, io);
     }

     ego = (const P *) ego_;
     cld1 = ego->cld1;
     cld2 = ego->cld2;
     r = ego->n;
     m = ego_dit->m;
     g = ego->g; 
     ginv = ego->ginv;
     omega = ego->omega;
     W = ego_dit->W;
     os = ego_dit->os;
     osm = ego->os;
     gpower = 1;

     buf = (R *) MALLOC(sizeof(R) * (r - 1) * 2, BUFFERS);

     for (j = 0; j < m; ++j, ro += os, io += os, W += 2*(r - 1)) {
	  /* First, permute the input and multiply by W, storing in buf: */
	  A(gpower == 1);
	  for (k = 0; k < r - 1; ++k, gpower = MULMOD(gpower, g, r)) {
	       E rA, iA, rW, iW;
	       rA = ro[gpower * osm];
	       iA = io[gpower * osm];
	       rW = W[2*k];
	       iW = W[2*k+1];
	       buf[2*k] = rW * rA - iW * iA;
	       buf[2*k + 1] = rW * iA + iW * rA;
	  }
	  /* gpower == g^(r-1) mod r == 1 */;
	  
	  apply_aux(r, ginv, cld1, cld2, omega, 
		    buf, ro[0], io[0], ro, io, osm);
     }

     X(ifree)(buf);
}

static R *mktwiddle(int m, int r, int g)
{
     int i, j, gpower;
     int n = r * m;
     R *W;

     if ((W = X(rader_tl_find)(m, r, g, twiddles)))
	  return W;

     W = (R *)MALLOC(sizeof(R) * (r - 1) * m * 2, TWIDDLES);
     for (i = 0; i < m; ++i) {
	  for (gpower = 1, j = 0; j < r - 1;
	       ++j, gpower = MULMOD(gpower, g, r)) {
	       int k = i * (r - 1) + j;
	       W[2*k] = X(cos2pi)(i * gpower, n);
	       W[2*k+1] = FFT_SIGN * X(sin2pi)(i * gpower, n);
	  }
	  A(gpower == 1);
     }

     X(rader_tl_insert)(m, r, g, W, &twiddles);
     return W;
}

static void free_twiddle(R *twiddle)
{
     X(rader_tl_delete)(twiddle, &twiddles);
}

/***************************************************************************/

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld1, flg);
     AWAKE(ego->cld2, flg);

     if (flg) {
	  if (!ego->omega) 
	       ego->omega = 
		    X(dft_rader_mkomega)(ego->cld_omega, ego->n, ego->ginv);
     } else {
	  X(dft_rader_free_omega)(&ego->omega);
     }
}

static void awake_dit(plan *ego_, int flg)
{
     P_dit *ego = (P_dit *) ego_;

     AWAKE(ego->cld, flg);
     if (flg)
	  ego->W = mktwiddle(ego->m, ego->super.n, ego->super.g);
     else {
	  free_twiddle(ego->W);
	  ego->W = 0;
     }

     awake(ego_, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld_omega);
     X(plan_destroy_internal)(ego->cld2);
     X(plan_destroy_internal)(ego->cld1);
}

static void destroy_dit(plan *ego_)
{
     P_dit *ego = (P_dit *) ego_;
     X(plan_destroy_internal)(ego->cld);
     destroy(ego_);
}

static void print_aux(const char *name, const P *ego, printer *p)
{
     p->print(p, "(%s-%d%ois=%oos=%(%p%)",
              name, ego->n, ego->is, ego->os, ego->cld1);
     if (ego->cld2 != ego->cld1)
          p->print(p, "%(%p%)", ego->cld2);
     if (ego->cld_omega != ego->cld1 && ego->cld_omega != ego->cld2)
          p->print(p, "%(%p%)", ego->cld_omega);
}

static void print(const plan *ego_, printer *p)
{
     print_aux("dft-rader", (const P *) ego_, p);
     p->putchr(p, ')');
}

static void print_dit(const plan *ego_, printer *p)
{
     const P_dit *ego_dit = (const P_dit *) ego_;

     print_aux("dft-rader-dit", (const P *) ego_, p);
     p->print(p, "%(%p%))", ego_dit->cld);
}

static int applicable0(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
	       && p->sz->rnk == 1
	       && p->vecsz->rnk == 0
	       && X(is_prime)(p->sz->dims[0].n)
	       );
     }

     return 0;
}

static int applicable0_dit(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          return (1
	       && p->sz->rnk == 1
	       && p->vecsz->rnk == 0
	       && p->sz->dims[0].n > 1
	       );
     }

     return 0;
}

static int applicable(const solver *ego_, const problem *p_,
		      const planner *plnr)
{
     return (!NO_UGLYP(plnr) && applicable0(ego_, p_));
}

static int applicable_dit(const solver *ego_, const problem *p_, 
			  const planner *plnr)
{
     return (!NO_UGLYP(plnr) && applicable0_dit(ego_, p_));
}

static int mkP(P *pln, int n, int is, int os, R *ro, R *io,
	       planner *plnr)
{
     plan *cld1 = (plan *) 0;
     plan *cld2 = (plan *) 0;
     plan *cld_omega = (plan *) 0;
     R *buf = (R *) 0;

     /* initial allocation for the purpose of planning */
     buf = (R *) MALLOC(sizeof(R) * (n - 1) * 2, BUFFERS);

     cld1 = X(mkplan_d)(plnr, 
			X(mkproblem_dft_d)(X(mktensor_1d)(n - 1, 2, os),
					   X(mktensor_1d)(1, 0, 0),
  					   buf, buf + 1, ro + os, io + os));
     if (!cld1) goto nada;

     cld2 = X(mkplan_d)(plnr, 
			X(mkproblem_dft_d)(X(mktensor_1d)(n - 1, os, 2),
					   X(mktensor_1d)(1, 0, 0),
  					   ro + os, io + os, buf, buf + 1));

     if (!cld2) goto nada;

     /* plan for omega array */
     plnr->planner_flags |= ESTIMATE;
     cld_omega = X(mkplan_d)(plnr, 
			     X(mkproblem_dft_d)(X(mktensor_1d)(n - 1, 2, 2),
						X(mktensor_1d)(1, 0, 0),
						buf, buf + 1, buf, buf + 1));
     if (!cld_omega) goto nada;

     /* deallocate buffers; let awake() or apply() allocate them for real */
     X(ifree)(buf);
     buf = 0;

     pln->cld1 = cld1;
     pln->cld2 = cld2;
     pln->cld_omega = cld_omega;
     pln->omega = 0;
     pln->n = n;
     pln->is = is;
     pln->os = os;
     pln->g = X(find_generator)(n);
     pln->ginv = X(power_mod)(pln->g, n - 2, n);
     A(MULMOD(pln->g, pln->ginv, n) == 1);

     X(ops_add)(&cld1->ops, &cld2->ops, &pln->super.super.ops);
     pln->super.super.ops.other += (n - 1) * (4 * 2 + 6) + 6;
     pln->super.super.ops.add += (n - 1) * 2 + 4;
     pln->super.super.ops.mul += (n - 1) * 4;

     return 1;

 nada:
     X(ifree0)(buf);
     X(plan_destroy_internal)(cld_omega);
     X(plan_destroy_internal)(cld2);
     X(plan_destroy_internal)(cld1);
     return 0;
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     P *pln;
     int n;
     int is, os;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(ego, p_, plnr))
	  return (plan *) 0;

     n = p->sz->dims[0].n;
     is = p->sz->dims[0].is;
     os = p->sz->dims[0].os;

     pln = MKPLAN_DFT(P, &padt, apply);
     if (!mkP(pln, n, is, os, p->ro, p->io, plnr)) {
	  X(ifree)(pln);
	  return (plan *) 0;
     }
     return &(pln->super.super);
}

static plan *mkplan_dit(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p = (const problem_dft *) p_;
     P_dit *pln = 0;
     int n, r, m;
     int is, os;
     plan *cld = (plan *) 0;

     static const plan_adt padt = {
	  X(dft_solve), awake_dit, print_dit, destroy_dit
     };

     if (!applicable_dit(ego, p_, plnr))
          goto nada;

     n = p->sz->dims[0].n;
     is = p->sz->dims[0].is;
     os = p->sz->dims[0].os;

     r = X(first_divisor)(n);
     m = n / r;

     cld = X(mkplan_d)(plnr, 
		       X(mkproblem_dft_d)(X(mktensor_1d)(m, r * is, os),
					  X(mktensor_1d)(r, is, m * os),
					  p->ri, p->ii, p->ro, p->io));
     if (!cld) goto nada;

     pln = MKPLAN_DFT(P_dit, &padt, apply_dit);
     if (!mkP(&pln->super, r, os*m, os*m, p->ro, p->io, plnr))
	  goto nada;

     pln->os = os;
     pln->m = m;
     pln->cld = cld;
     pln->W = 0;

     pln->super.super.super.ops.add += 2 * (r-1);
     pln->super.super.super.ops.mul += 4 * (r-1);
     X(ops_madd)(m, &pln->super.super.super.ops, &cld->ops,
		 &pln->super.super.super.ops);

     return &(pln->super.super.super);

 nada:
     X(plan_destroy_internal)(cld);
     X(ifree0)(pln);
     return (plan *) 0;
}

/* constructors */

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

static solver *mksolver_dit(void)
{
     static const solver_adt sadt = { mkplan_dit };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(dft_rader_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
     REGISTER_SOLVER(p, mksolver_dit());
}
