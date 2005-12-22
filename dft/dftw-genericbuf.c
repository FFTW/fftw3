
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

/* express a twiddle problem in terms of dft + multiplication by
   twiddle factors */

#include "ct.h"

typedef struct {
     ct_solver super;
     INT batchsz;
} S;

typedef struct {
     plan_dftw super;

     INT r, m, s, mb, me;
     INT batchsz;
     plan *cld;

     int log2_twradix;
     R *W0, *W1;

     const S *slv;
} P;

/* approximate log2(sqrt(n)) */
static int choose_log2_twradix(INT n)
{
     int log2r = 0;
     while (n > 0) {
	  ++log2r;
	  n /= 4;
     }
     return log2r;
}

#define BATCHDIST(r) ((r) + 16)

/**************************************************************/
static void mktwiddle(P *ego, int flg)
{
     if (flg) {
	  INT n = ego->r * ego->m;
	  int log2_twradix = choose_log2_twradix(n);
	  INT twradix = ((INT)1) << log2_twradix;
	  INT n0 = twradix;
	  INT n1 = (n + twradix - 1) / twradix;
	  INT i;
	  R *W0, *W1;

	  ego->log2_twradix = log2_twradix;
	  ego->W0 = W0 = (R *)MALLOC(n0 * 2 * sizeof(R), TWIDDLES);
	  ego->W1 = W1 = (R *)MALLOC(n1 * 2 * sizeof(R), TWIDDLES);

	  for (i = 0; i < n0; ++i) 
	       X(cexp)(i, n, W0+2*i);

	  for (i = 0; i < n1; ++i) 
	       X(cexp)(i * twradix, n, W1+2*i);

     } else {
	  X(ifree0)(ego->W0); ego->W0 = 0;
	  X(ifree0)(ego->W1); ego->W1 = 0;
     }
}

static void bytwiddle(const P *ego, INT m0, INT m1, R *buf, R *rio, R *iio)
{
     INT j, k;
     INT r = ego->r, s = ego->s, m = ego->m;
     int twshft = ego->log2_twradix;
     INT twmsk = (((INT)1) << twshft) - 1;

     const R *W0 = ego->W0, *W1 = ego->W1;
     for (j = 0; j < r; ++j) {
	  for (k = m0; k < m1; ++k) {
	       INT jk = j * k;
	       INT jk0 = jk & twmsk;
	       INT jk1 = jk >> twshft;
	       E xr = rio[s * (j * m + k)];
	       E xi = iio[s * (j * m + k)];
	       E wr0 = W0[2 * jk0];
	       E wi0 = W0[2 * jk0 + 1];
	       E wr1 = W1[2 * jk1];
	       E wi1 = W1[2 * jk1 + 1];
	       E wr = wr1 * wr0 - wi1 * wi0;
	       E wi = wi1 * wr0 + wr1 * wi0;
	       buf[j * 2 + 2 * BATCHDIST(r) * (k - m0) + 0] = 
		    xr * wr + xi * wi;
	       buf[j * 2 + 2 * BATCHDIST(r) * (k - m0) + 1] = 
		    xi * wr - xr * wi;
	  }
     }
}

static int applicable0(const S *ego, INT r, INT m, INT vl, int dec,
		       const planner *plnr)
{
     return (1 
	     && vl == 1
	     && dec == DECDIT
	     && m >= ego->batchsz
	     && m % ego->batchsz == 0
	     && r >= 64
	     && m >= r
	  );
}

static int applicable(const S *ego, INT r, INT m, INT vl, int dec,
		      const planner *plnr)
{
     if (!applicable0(ego, r, m, vl, dec, plnr))
	  return 0;
     if (NO_UGLYP(plnr) && m * r < 65536)
	  return 0;

     return 1;
}

static void dobatch(const P *ego, INT m0, INT m1, R *buf, R *rio, R *iio)
{
     plan_dft *cld;

     bytwiddle(ego, m0, m1, buf, rio, iio);

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, buf, buf + 1, buf, buf + 1);
     X(cpy2d_pair_co)(buf,buf+1,rio + ego->s * m0, iio + ego->s * m0,
		      m1-m0, 2 * BATCHDIST(ego->r), ego->s,
		      ego->r, 2, ego->m * ego->s);
}

static void apply(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     R *buf = (R *) MALLOC(sizeof(R) * 2 * BATCHDIST(ego->r) * ego->batchsz,
			   BUFFERS);
     INT m;

     for (m = ego->mb; m < ego->me - ego->batchsz; m += ego->batchsz) 
	  dobatch(ego, m, m + ego->batchsz, buf, rio, iio);

     dobatch(ego, m, ego->me, buf, rio, iio);
     
     X(ifree)(buf);
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
     A(!ego->W0 && !ego->W1);
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(dftw-genericbuf/%D-%D-%D%(%p%))", 
	      ego->batchsz, ego->r, ego->m, ego->cld);
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
     R *buf;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     A(mstart >= 0 && mstart + mcount <= m);
     if (!applicable(ego, r, m, vl, dec, plnr))
          return (plan *)0;

     buf = (R *) MALLOC(sizeof(R) * 2 * BATCHDIST(r) * ego->batchsz, BUFFERS);
     cld = X(mkplan_d)(plnr, 
			X(mkproblem_dft_d)(
			     X(mktensor_1d)(r, 2, 2),
			     X(mktensor_1d)(ego->batchsz,
					    2 * BATCHDIST(r), 
					    2 * BATCHDIST(r)),
			     buf, buf + 1, buf, buf + 1
			     )
			);
     X(ifree)(buf);
     if (!cld) goto nada;

     pln = MKPLAN_DFTW(P, &padt, apply);
     pln->slv = ego;
     pln->cld = cld;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->batchsz = ego->batchsz;
     pln->mb = mstart;
     pln->me = mstart + mcount;
     pln->W0 = pln->W1 = 0;

     {
	  double n0 = (r - 1) * (mcount - 1);
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

static void regsolver(planner *plnr, INT r, INT batchsz)
{
     S *slv = (S *)X(mksolver_ct)(sizeof(S), r, DECDIT, mkcldw);
     slv->batchsz = batchsz;
     REGISTER_SOLVER(plnr, &(slv->super.super));
}

void X(ct_genericbuf_register)(planner *p)
{
     static const INT radices[] = { -1, -2, -4, -8, -16, -32, -64 };
     static const INT batchsizes[] = { 4, 8, 16, 32, 64 };
     unsigned i, j;

     for (i = 0; i < sizeof(radices) / sizeof(radices[0]); ++i) 
	  for (j = 0; j < sizeof(batchsizes) / sizeof(batchsizes[0]); ++j) 
	       regsolver(p, radices[i], batchsizes[j]);
}
