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

/* express a hc2hc problem in terms of rdft + multiplication by
   twiddle factors */

#include "ct.h"

typedef ct_solver S;

typedef struct {
     plan_hc2hc super;

     int r, m, s, vl, vs;
     plan *cld;
     twid *td;
} P;


/**************************************************************/
static void mktwiddle(P *ego, int flg)
{
     static const tw_instr tw[] = { { TW_HALF, 0, 0 }, { TW_NEXT, 1, 0 } };

     /* note that R and M are swapped, to allow for sequential
	access both to data and twiddles */
     X(twiddle_awake)(flg, &ego->td, tw, ego->r * ego->m, ego->m, ego->r);
}

static void bytwiddle(const P *ego, R *IO)
{
     int i, j, k;
     int r = ego->r, m = ego->m, s = ego->s, vl = ego->vl, vs = ego->vs;
     int ms = m * s;

     for (i = 0; i < vl; ++i, IO += vs) {
	  const R *W = ego->td->W;

	  A(m % 2 == 1);
	  for (k = 1, W += (m - 1); k < r; ++k) {
	       /* pr := IO + j * s + k * ms */
	       R *pr = IO + s + k * ms;

	       /* pi := IO + (m - j) * s + k * ms */
	       R *pi = IO - s + (k + 1) * ms;

	       for (j = 1; j + j < m; ++j, pr += s, pi -= s) {
		    E xr = *pr;
		    E xi = *pi;
		    E wr = W[0];
		    E wi = W[1];
		    *pr = xr * wr + xi * wi;
		    *pi = xi * wr - xr * wi;
		    W += 2;
	       }
	  }
     }
}

static void reorder(const P *ego, R *IO)
{
     int i, k;
     int r = ego->r, m = ego->m, s = ego->s, vl = ego->vl, vs = ego->vs;
     int ms = m * s;

     for (i = 0; i < vl; ++i, IO += vs) {
	  for (k = 1; k + k < r; ++k) {
	       R *p0 = IO + k * ms;
	       R *p1 = IO + (r - k) * ms;
	       int j;

	       for (j = 1; j + j < m; ++j) {
		    E rp, ip, im, rm;
		    rp = p0[j * s];
		    ip = p0[ms - j * s];
		    rm = p1[j * s];
		    im = p1[ms - j * s];
		    p0[j * s] = rp - im;
		    p0[ms - j * s] = ip + rm;
		    p1[ms - j * s] = rp + im;
		    p1[j * s] = rm - ip;
	       }
	  }

	  for (k = 0; k + k < r; ++k) {
	       /* pr := IO + (m - j) * s + k * ms */
	       R *pr = IO + (k + 1) * ms - s;
	       /* pi := IO + (m - j) * s + (r - 1 - k) * ms */
	       R *pi = IO + (r - k) * ms - s;
 	       /* twoj := 2 * j */
	       int twoj;
	       for (twoj = 2; twoj < m; twoj += 2, pr -= s, pi -= s) {
		    R t = *pr;
		    *pr = *pi;
		    *pi = t;
	       }
	  }
     }
}

static int applicable(rdft_kind kind, int r, int m, const planner *plnr)
{
     return (1 
	     && (R2HC_KINDP(kind) || HC2R_KINDP(kind))
&& (R2HC_KINDP(kind)) /* FIXME: implement HC2R */
	     && (m % 2)
	     && (r % 2)
	     && !NO_UGLYP(plnr)
	  );
}

/**************************************************************/

static void apply_dit(const plan *ego_, R *IO)
{
     const P *ego = (const P *) ego_;
     plan_rdft *cld;

     bytwiddle(ego, IO);

     cld = (plan_rdft *) ego->cld;
     cld->apply(ego->cld, IO, IO);

     reorder(ego, IO);
}


static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld, flg);
     mktwiddle(ego, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(hc2hc-generic-%d-%d%v%(%p%))", 
	      ego->r, ego->m, ego->vl, ego->cld);
}

static plan *mkcldw(const ct_solver *ego_, 
		    rdft_kind kind, int r, int m, int s, int vl, int vs, 
		    R *IO, planner *plnr)
{
     P *pln;
     plan *cld = 0;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     UNUSED(ego_);

     if (!applicable(kind, r, m, plnr))
          return (plan *)0;

     cld = X(mkplan_d)(plnr, 
			X(mkproblem_rdft_1_d)(
			     X(mktensor_1d)(r, m * s, m * s),
			     X(mktensor_2d)(m, s, s, vl, vs, vs),
			     IO, IO, kind)
			);
     if (!cld) goto nada;

     pln = MKPLAN_HC2HC(P, &padt, apply_dit);
     pln->cld = cld;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->vl = vl;
     pln->vs = vs;
     pln->td = 0;

     {
	  /* FIXME */
	  double n0 = (r - 1) * (m - 1) / 2 * vl;
	  pln->super.super.ops = cld->ops;
	  pln->super.super.ops.mul += 4 * n0;
	  pln->super.super.ops.add += 4 * n0;
	  pln->super.super.ops.other += 8 * n0;
     }
     return &(pln->super.super);

 nada:
     X(plan_destroy_internal)(cld);
     return (plan *) 0;
}

static solver *mksolver(int r)
{
     S *slv = (S *)X(mksolver_rdft_ct)(sizeof(S), r, mkcldw);
     return &(slv->super);
}

void X(rdft_ct_generic_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver(0));
}
