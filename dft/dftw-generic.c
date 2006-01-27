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

/* express a twiddle problem in terms of dft + multiplication by
   twiddle factors */

#include "ct.h"

typedef ct_solver S;

typedef struct {
     plan_dftw super;

     INT r, m, s, vl, vs, mstart, mcount;
     plan *cld;

     /* defined only for solver1: */
     twid *td;

     const S *slv;
     int dec;
} P;

static void mktwiddle(P *ego, enum wakefulness wakefulness)
{
     static const tw_instr tw[] = { { TW_FULL, 0, 0 }, { TW_NEXT, 1, 0 } };

     /* note that R and M are swapped, to allow for sequential
	access both to data and twiddles */
     X(twiddle_awake)(wakefulness, &ego->td, tw, 
		      ego->r * ego->m, ego->m, ego->r);
}

static void bytwiddle(const P *ego, R *rio, R *iio)
{
     INT i, j, k;
     INT r = ego->r, m = ego->m, s = ego->s, vl = ego->vl, vs = ego->vs;
     INT mcount = ego->mcount, mstart = ego->mstart;
     INT jstart = mstart == 0;
     INT jrem_W = 2 * ((m - 1) - (mcount - jstart));
     INT jrem_p = s * (m - mcount);
     INT ip = iio - rio;
     R *p;

     for (i = 0; i < vl; ++i) {
	  const R *W = ego->td->W + 2 * (mstart - 1 + jstart);

	  /* loop invariant: p = rio + s * (k * m + j) + i * vs. */
	  p = rio + i * vs;

	  for (k = 1, p += s * m, W += 2 * (m - 1); k < r; ++k) {
	       for (j = jstart, p += jstart*s; j < mcount; ++j, p += s) {
		    E xr = p[0];
		    E xi = p[ip];
		    E wr = W[0];
		    E wi = W[1];
		    p[0] = xr * wr + xi * wi;
		    p[ip] = xi * wr - xr * wi;
		    W += 2;
	       }
	       W += jrem_W;
	       p += jrem_p;
	  }
     }
}

static int applicable(INT r, INT m, const planner *plnr)
{
     UNUSED(r); UNUSED(m);
     return (1 
	     && !NO_SLOWP(plnr)
	  );
}

static void apply_dit(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;

     bytwiddle(ego, rio, iio);

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, rio, iio, rio, iio);
}

static void apply_dif(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, rio, iio, rio, iio);

     bytwiddle(ego, rio, iio);
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     X(plan_awake)(ego->cld, wakefulness);
     mktwiddle(ego, wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dftw-generic-%s-%D-%D%v%(%p%))", 
	      ego->dec == DECDIT ? "dit" : "dif",
	      ego->r, ego->m, ego->vl, ego->cld);
}

static plan *mkcldw(const ct_solver *ego_, 
		    int dec, INT r, INT m, INT s, INT vl, INT vs, 
		    INT mstart, INT mcount,
		    R *rio, R *iio,
		    planner *plnr)
{
     const S *ego = (const S *)ego_;
     P *pln;
     plan *cld = 0;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     A(mstart >= 0 && mstart + mcount <= m);
     if (!applicable(r, m, plnr))
          return (plan *)0;

     cld = X(mkplan_d)(plnr, 
			X(mkproblem_dft_d)(
			     X(mktensor_1d)(r, m * s, m * s),
			     X(mktensor_2d)(mcount, s, s, vl, vs, vs),
			     rio, iio, rio, iio)
			);
     if (!cld) goto nada;

     pln = MKPLAN_DFTW(P, &padt, dec == DECDIT ? apply_dit : apply_dif);
     pln->slv = ego;
     pln->cld = cld;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->vl = vl;
     pln->vs = vs;
     pln->mstart = mstart;
     pln->mcount = mcount;
     pln->dec = dec;
     pln->td = 0;

     {
	  double n0 = (r - 1) * (mcount - 1) * vl;
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

static void regsolver(planner *plnr, INT r, int dec)
{
     S *slv = (S *)X(mksolver_ct)(sizeof(S), r, dec, mkcldw);
     REGISTER_SOLVER(plnr, &(slv->super));
     if (X(mksolver_ct_hook)) {
	  slv = (S *)X(mksolver_ct_hook)(sizeof(S), r, dec, mkcldw);
	  REGISTER_SOLVER(plnr, &(slv->super));
     }
}

void X(ct_generic_register)(planner *p)
{
     regsolver(p, 0, DECDIT);
     regsolver(p, 0, DECDIF);
}
