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

/* express a dftw problem in terms of dft + multiplication by
   twiddle factors */

#include "dft.h"

struct P_s;
typedef struct {
     void (*bytwiddle)(const struct P_s *ego, R *rio, R *iio);
     void (*mktwiddle)(struct P_s *ego, int flg);
     int (*applicable)(const problem *p_, const planner *plnr);
     const char *nam;
} wadt;

typedef struct {
     solver super;
     const wadt *adt;
} S;

typedef struct P_s {
     plan_dft super;

     const wadt *adt;
     int r, m, s, vl, vs;
     plan *cld;

     /* defined only for solver1: */
     twid *td;

     /* defined only for solver2 */
     int log2_twradix;
     R *W0, *W1;

     const S *slv;
     int dec;
} P;

/* approximate log2(sqrt(n)) */
static int choose_log2_twradix(int n)
{
     int log2r = 0;
     while (n > 0) {
	  ++log2r;
	  n /= 4;
     }
     return log2r;
}

/**************************************************************/
static void mktwiddle2(P *ego, int flg)
{
     if (flg) {
	  int n = ego->r * ego->m;
	  int log2_twradix = choose_log2_twradix(n);
	  int twradix = 1 << log2_twradix;
	  int n0 = twradix;
	  int n1 = (n + twradix - 1) / twradix;
	  int i;
	  R *W0, *W1;

	  ego->log2_twradix = log2_twradix;
	  ego->W0 = W0 = (R *)MALLOC(n0 * 2 * sizeof(R), TWIDDLES);
	  ego->W1 = W1 = (R *)MALLOC(n1 * 2 * sizeof(R), TWIDDLES);

	  for (i = 0; i < n0; ++i) {
	       W0[2 * i] = X(cos2pi)(i, n);
	       W0[2 * i + 1] = X(sin2pi)(i, n);
	  }
	  for (i = 0; i < n1; ++i) {
	       W1[2 * i] = X(cos2pi)(i * twradix, n);
	       W1[2 * i + 1] = X(sin2pi)(i * twradix, n);
	  }
     } else {
	  X(ifree0)(ego->W0); ego->W0 = 0;
	  X(ifree0)(ego->W1); ego->W1 = 0;
     }
}

static void bytwiddle2(const P *ego, R *rio, R *iio)
{
     int i, j, k;
     int r = ego->r, m = ego->m, s = ego->s, vl = ego->vl, vs = ego->vs;
     int twshft = ego->log2_twradix;
     int twmsk = (1 << twshft) - 1;

     const R *W0 = ego->W0, *W1 = ego->W1;
     for (i = 0; i < vl; ++i) {
	  for (j = 1; j < r; ++j) {
	       for (k = 1; k < m; ++k) {
		    unsigned jk = j * k;
		    int jk0 = jk & twmsk;
		    int jk1 = jk >> twshft;
		    E xr = rio[s * (j * m + k)];
		    E xi = iio[s * (j * m + k)];
		    E wr0 = W0[2 * jk0];
		    E wi0 = W0[2 * jk0 + 1];
		    E wr1 = W1[2 * jk1];
		    E wi1 = W1[2 * jk1 + 1];
		    E wr = wr1 * wr0 - wi1 * wi0;
		    E wi = wi1 * wr0 + wr1 * wi0;
		    rio[s * (j * m + k)] = xr * wr + xi * wi;
		    iio[s * (j * m + k)] = xi * wr - xr * wi;
	       }
	  }
	  rio += vs;
	  iio += vs;
     }
}

static int applicable2(const problem *p_, const planner *plnr)
{
     if (DFTWP(p_)) {
          const problem_dftw *p = (const problem_dftw *) p_;
          return (1 
		  && (!NO_UGLYP(plnr) || (p->m * p->r > 65536))
	       );
     }
     return 0;
}

/**************************************************************/
static void mktwiddle1(P *ego, int flg)
{
     static const tw_instr tw_template[] = { { TW_FULL, 0, 0 }, 
					     { TW_NEXT, 1, 0 } };

     tw_instr tw[2];

     /* note that R and M are swapped, to allow for sequential
	access both to data and twiddles */
     tw[0] = tw_template[0];
     tw[1] = tw_template[1];
     tw[0].i = ego->m;

     X(twiddle_awake)(flg, &ego->td, tw, ego->r * ego->m, ego->m, ego->r);
}

static void bytwiddle1(const P *ego, R *rio, R *iio)
{
     int i, j, k;
     int r = ego->r, m = ego->m, s = ego->s, vl = ego->vl, vs = ego->vs;
     ptrdiff_t ip = iio - rio;
     R *p;

     for (i = 0; i < vl; ++i) {
	  const R *W = ego->td->W;

	  /* loop invariant: p = rio + s * (j * m + k) + i * vs. */
	  p = rio + i * vs;

	  for (j = 1, p += s * m, W += 2 * (m - 1); j < r; ++j) {
	       for (k = 1, p += s; k < m; ++k, p += s) {
		    E xr = p[0];
		    E xi = p[ip];
		    E wr = W[0];
		    E wi = W[1];
		    p[0] = xr * wr + xi * wi;
		    p[ip] = xi * wr - xr * wi;
		    W += 2;
	       }
	  }
     }
}

static int applicable1(const problem *p_, const planner *plnr)
{
     if (DFTWP(p_)) {
          const problem_dftw *p = (const problem_dftw *) p_;
          return (1 
		  && !NO_UGLYP(plnr)
	       );
     }
     return 0;
}

/**************************************************************/

static void apply_dit(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;

     ego->adt->bytwiddle(ego, rio, iio);

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, rio, iio, rio, iio);
}

static void apply_dif(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, rio, iio, rio, iio);

     ego->adt->bytwiddle(ego, rio, iio);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld, flg);
     ego->adt->mktwiddle(ego, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     A(!ego->W0 && !ego->W1);
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(%s-%s-%d-%d%v%(%p%))", 
	      ego->adt->nam, ego->dec == DECDIT ? "dit" : "dif",
	      ego->r, ego->m, ego->vl, ego->cld);
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const problem_dftw *p;
     const S *ego = (const S *)ego_;
     P *pln;
     plan *cld = 0;

     static const plan_adt padt = {
	  X(dftw_solve), awake, print, destroy
     };

     if (!ego->adt->applicable(p_, plnr))
          return (plan *)0;

     p = (const problem_dftw *) p_;

     cld = X(mkplan_d)(plnr, 
			X(mkproblem_dft_d)(
			     X(mktensor_1d)(p->r, p->m * p->s, p->m * p->s),
			     X(mktensor_2d)(p->m, p->s, p->s,
					    p->vl, p->vs, p->vs),
			     p->rio, p->iio, p->rio, p->iio)
			);
     if (!cld) goto nada;

     pln = MKPLAN_DFTW(P, &padt, p->dec == DECDIT ? apply_dit : apply_dif);
     pln->slv = ego;
     pln->adt = ego->adt;
     pln->cld = cld;
     pln->r = p->r;
     pln->m = p->m;
     pln->s = p->s;
     pln->vl = p->vl;
     pln->vs = p->vs;
     pln->dec = p->dec;
     pln->W0 = pln->W1 = 0;
     pln->td = 0;

     {
	  double n0 = (p->r - 1) * (p->m - 1) * p->vl;
	  pln->super.super.ops = cld->ops;
	  pln->super.super.ops.mul += 8 * n0;
	  pln->super.super.ops.add += 4 * n0;
	  pln->super.super.ops.other += 8 * n0;
     }
     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld);
     return (plan *) 0;
}

static solver *mksolver(const wadt *adt)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}

void X(dftw_dft_register)(planner *p)
{
     static const wadt a[] = {
	  { bytwiddle1, mktwiddle1, applicable1, "dftw-dft1" },
	  { bytwiddle2, mktwiddle2, applicable2, "dftw-dft2" },
     };
     unsigned i;

     for (i = 0; i < sizeof(a) / sizeof(a[0]); ++i) 
	  REGISTER_SOLVER(p, mksolver(&a[i]));
}
