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

/* $Id: reodft010e-r2hc.c,v 1.12 2002-09-21 22:04:05 athena Exp $ */

/* Do an R{E,O}DFT{01,10} problem via an R2HC problem, with some
   pre/post-processing ala FFTPACK. */

#include "reodft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_rdft super;
     plan *cld;
     twid *td;
     int is, os;
     uint n;
     rdft_kind kind;
} P;

/* A real-even-01 DFT operates logically on a size-4N array:
                   I 0 -r(I*) -I 0 r(I*),
   where r denotes reversal and * denotes deletion of the 0th element.
   To compute the transform of this, we imagine performing a radix-4
   (real-input) DIF step, which turns the size-4N DFT into 4 size-N
   (contiguous) DFTs, two of which are zero and two of which are
   conjugates.  The non-redundant size-N DFT has halfcomplex input, so
   we can do it with a size-N hc2r transform.  (In order to share
   plans with the re10 (inverse) transform, however, we use the DHT
   trick to re-express the hc2r problem as r2hc.  This has little cost
   since we are already pre- and post-processing the data in {i,n-i}
   order.)  Finally, we have to write out the data in the correct
   order...the two size-N redundant (conjugate) hc2r DFTs correspond
   to the even and odd outputs in O (i.e. the usual interleaved output
   of DIF transforms); since this data has even symmetry, we only
   write the first half of it.

   The real-even-10 DFT is just the reverse of these steps, i.e. a
   radix-4 DIT transform.  There, however, we just use the r2hc
   transform naturally without resorting to the DHT trick.

   A real-odd-01 DFT is very similar, except that the input is
   0 I (rI)* 0 -I -(rI)*.  This format, however, can be transformed
   into precisely the real-even-01 format above by sending I -> rI
   and shifting the array by N.  The former swap is just another
   transformation on the input during preprocessing; the latter
   multiplies the even/odd outputs by i/-i, which combines with
   the factor of -i (to take the imaginary part) to simply flip
   the sign of the odd outputs.  Vice-versa for real-odd-10.

   The FFTPACK source code was very helpful in working this out.
   (They do unnecessary passes over the array, though.)
*/

static void apply_re01(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     uint i, n = ego->n;
     R *W = ego->td->W;
     R *buf;

     buf = (R *) fftw_malloc(sizeof(R) * n, BUFFERS);

     buf[0] = I[0];
     for (i = 1; i < n - i; ++i) {
	  E a, b, apb, amb, wa, wb;
	  a = I[is * i];
	  b = I[is * (n - i)];
	  apb = a + b;
	  amb = a - b;
	  wa = W[2*i];
	  wb = W[2*i + 1];
	  buf[i] = wa * amb + wb * apb; 
	  buf[n - i] = wa * apb - wb * amb; 
     }
     if (i == n - i) {
	  buf[i] = 2.0 * I[is * i] * W[2*i];
     }

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, buf, buf);
     }

     O[0] = buf[0];
     for (i = 1; i < n - i; ++i) {
	  E a, b;
	  uint k;
	  a = buf[i];
	  b = buf[n - i];
	  k = i + i;
	  O[os * (k - 1)] = a - b;
	  O[os * k] = a + b;
     }
     if (i == n - i) {
	  O[os * (n - 1)] = buf[i];
     }

     X(free)(buf);
}

/* ro01 is same as re01, but with i <-> n - 1 - i in the input and
   the sign of the odd output elements flipped. */
static void apply_ro01(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     uint i, n = ego->n;
     R *W = ego->td->W;
     R *buf;

     buf = (R *) fftw_malloc(sizeof(R) * n, BUFFERS);

     buf[0] = I[is * (n - 1)];
     for (i = 1; i < n - i; ++i) {
	  E a, b, apb, amb, wa, wb;
	  a = I[is * (n - 1 - i)];
	  b = I[is * (i - 1)];
	  apb = a + b;
	  amb = a - b;
	  wa = W[2*i];
	  wb = W[2*i+1];
	  buf[i] = wa * amb + wb * apb; 
	  buf[n - i] = wa * apb - wb * amb; 
     }
     if (i == n - i) {
	  buf[i] = 2.0 * I[is * (i - 1)] * W[2*i];
     }

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, buf, buf);
     }

     O[0] = buf[0];
     for (i = 1; i < n - i; ++i) {
	  E a, b;
	  uint k;
	  a = buf[i];
	  b = buf[n - i];
	  k = i + i;
	  O[os * (k - 1)] = b - a;
	  O[os * k] = a + b;
     }
     if (i == n - i) {
	  O[os * (n - 1)] = -buf[i];
     }

     X(free)(buf);
}

static void apply_re10(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     uint i, n = ego->n;
     R *W = ego->td->W;
     R *buf;

     buf = (R *) fftw_malloc(sizeof(R) * n, BUFFERS);

     buf[0] = I[0];
     for (i = 1; i < n - i; ++i) {
	  E u, v;
	  uint k = i + i;
	  u = I[is * (k - 1)];
	  v = I[is * k];
	  buf[n - i] = u;
	  buf[i] = v;
     }
     if (i == n - i) {
	  buf[i] = I[is * (n - 1)];
     }

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, buf, buf);
     }

     O[0] = 2.0 * buf[0];
     for (i = 1; i < n - i; ++i) {
	  E a, b, wa, wb;
	  a = 2.0 * buf[i];
	  b = 2.0 * buf[n - i];
	  wa = W[2*i];
	  wb = W[2*i + 1];
	  O[os * i] = wa * a + wb * b;
	  O[os * (n - i)] = wb * a - wa * b;
     }
     if (i == n - i) {
	  O[os * i] = 2.0 * buf[i] * W[2*i];
     }

     X(free)(buf);
}

/* ro10 is same as re10, but with i <-> n - 1 - i in the output and
   the sign of the odd input elements flipped. */
static void apply_ro10(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     uint i, n = ego->n;
     R *W = ego->td->W;
     R *buf;

     buf = (R *) fftw_malloc(sizeof(R) * n, BUFFERS);

     buf[0] = I[0];
     for (i = 1; i < n - i; ++i) {
	  E u, v;
	  uint k = i + i;
	  u = -I[is * (k - 1)];
	  v = I[is * k];
	  buf[n - i] = u;
	  buf[i] = v;
     }
     if (i == n - i) {
	  buf[i] = -I[is * (n - 1)];
     }

     {
	  plan_rdft *cld = (plan_rdft *) ego->cld;
	  cld->apply((plan *) cld, buf, buf);
     }

     O[n - 1] = 2.0 * buf[0];
     for (i = 1; i < n - i; ++i) {
	  E a, b, wa, wb;
	  a = 2.0 * buf[i];
	  b = 2.0 * buf[n - i];
	  wa = W[2*i];
	  wb = W[2*i + 1];
	  O[os * (n - 1 - i)] = wa * a + wb * b;
	  O[os * (i - 1)] = wb * a - wa * b;
     }
     if (i == n - i) {
	  O[os * (i - 1)] = 2.0 * buf[i] * W[2*i];
     }

     X(free)(buf);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     static const tw_instr reodft010e_tw[] = {
          { TW_COS, 0, 1 },
          { TW_SIN, 0, 1 },
          { TW_NEXT, 1, 0 }
     };

     AWAKE(ego->cld, flg);

     X(twiddle_awake)(flg, &ego->td, reodft010e_tw, 4*ego->n, 1, ego->n/2+1);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy)(ego->cld);
     X(free)(ego);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(%se-r2hc-%u%(%p%))",
	      X(rdft_kind_str)(ego->kind), ego->n * 2, ego->cld);
}

static int applicable0(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz.rnk == 1
		  && p->vecsz.rnk == 0
		  && (p->kind[0] == REDFT01 || p->kind[0] == REDFT10
		      || p->kind[0] == RODFT01 || p->kind[0] == RODFT10)
		  && p->sz.dims[0].n % 2 == 0
	       );
     }

     return 0;
}

static int applicable(const solver *ego, const problem *p, const planner *plnr)
{
     return (!NO_UGLYP(plnr) && applicable0(ego, p));
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     P *pln;
     const problem_rdft *p;
     problem *cldp;
     plan *cld;
     R *buf;
     uint n;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     n = p->sz.dims[0].n / 2;
     buf = (R *) fftw_malloc(sizeof(R) * n, BUFFERS);

     {
	  tensor sz = X(mktensor_1d)(n, 1, 1);
	  cldp = X(mkproblem_rdft_1)(&sz, &p->vecsz, buf, buf, R2HC);
	  X(tensor_destroy)(&sz);
     }

     cld = MKPLAN(plnr, cldp);
     X(problem_destroy)(cldp);
     X(free)(buf);
     if (!cld)
          return (plan *)0;

     switch (p->kind[0]) {
	 case REDFT01: pln = MKPLAN_RDFT(P, &padt, apply_re01); break;
	 case REDFT10: pln = MKPLAN_RDFT(P, &padt, apply_re10); break;
	 case RODFT01: pln = MKPLAN_RDFT(P, &padt, apply_ro01); break;
	 case RODFT10: pln = MKPLAN_RDFT(P, &padt, apply_ro10); break;
	 default: A(0); return (plan*)0;
     }

     pln->n = n;
     pln->is = p->sz.dims[0].is;
     pln->os = p->sz.dims[0].os;
     pln->cld = cld;
     pln->td = 0;
     pln->kind = p->kind[0];
     
     pln->super.super.ops = cld->ops;
     /* FIXME */

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(reodft010e_r2hc_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
