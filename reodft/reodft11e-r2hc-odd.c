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

/* $Id: reodft11e-r2hc-odd.c,v 1.3 2003-02-26 01:56:01 stevenj Exp $ */

/* Do an R{E,O}DFT11 problem via an R2HC problem of the same *odd* size,
   with some permutations and post-processing, as described in:

     S. C. Chan and K. L. Ho, "Fast algorithms for computing the
     discrete Cosine transform," IEEE Trans. Circuits Systems II:
     Analog & Digital Sig. Proc. 39 (3), 185--190 (1992).

   (For even sizes, see reodft11e-radix2.c.)  

   This algorithm is related to the 8 x n prime-factor-algorithm (PFA)
   decomposition of the size 8n "logical" DFT corresponding to the
   R{EO}DFT11.

   Aside from very confusing notation (several symbols are redefined
   from one line to the next), be aware that this paper has some
   errors.  In particular, the signs are wrong in Eqs. (34-35).  Also
   Eq. (36-37) should be simply C(k) = C(2k + 1 mod N), and similarly
   for S (or, equivalently, the second cases should have N - 2*k - 1
   instead of N - k - 1).  Note also that in their definition of the
   DFT, similarly to FFTW's, the exponent's sign is -1, but they
   forgot to correspondingly multiply S (the sine terms) by -1.
*/

#include "reodft.h"

typedef struct {
     solver super;
} S;

typedef struct {
     plan_rdft super;
     plan *cld;
     int is, os;
     int n;
     int vl;
     int ivs, ovs;
     rdft_kind kind;
} P;

static DK(SQRT2, +1.4142135623730950488016887242096980785696718753769);

static void apply_re11(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     int i, n = ego->n;
     int iv, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;
     R *buf;

     buf = (R *) MALLOC(sizeof(R) * n, BUFFERS);

     for (iv = 0; iv < vl; ++iv, I += ivs, O += ovs) {
	  /* FIXME: split this loop to eliminate if-else statements and % */
	  for (i = 0; i < n; ++i) {
	       int m = (8 * i + n) % (8*n); /* n odd => m odd */
	       if (m < 2*n)
		    buf[i] = I[is * ((m - 1) / 2)];
	       else if (m < 4*n)
		    buf[i] = -I[is * ((4*n - m - 1) / 2)];
	       else if (m < 6*n)
		    buf[i] = -I[is * ((m - 4*n - 1) / 2)];
	       else
		    buf[i] = I[is * ((8*n - m - 1) / 2)];
	  }
	  
	  { /* child plan: R2HC of size n */
	       plan_rdft *cld = (plan_rdft *) ego->cld;
	       cld->apply((plan *) cld, buf, buf);
	  }
	  
	  /* FIXME: split/unroll loop to eliminate if-else's and %,
	     and also to touch each element of buf only once. */
	  for (i = 0; i < n; ++i) {
	       int k;
	       R c, s;

	       k = (2*i + 1) % n;
	       
	       if (k < n - k) {
		    c = buf[k];
		    s = k == 0 ? 0.0 : -buf[n - k];
	       }
	       else {
		    c = buf[n - k];
		    s = k == n ? 0.0 : buf[k];
	       }

	       O[os * i] = SQRT2 * (c * (1 - 2 * (((i+1)/2) % 2)) -
				    s * (1 - 2 * ((i/2) % 2)));
	  }
     }

     X(ifree)(buf);
}

/* like for rodft01, rodft11 is obtained from redft11 by
   reversing the input and flipping the sign of every other output. */
/* FIXME: clean up after addressing FIXME's in apply_re11 */
static void apply_ro11(plan *ego_, R *I, R *O)
{
     P *ego = (P *) ego_;
     int is = ego->is, os = ego->os;
     int i, n = ego->n;
     int iv, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;
     R *buf;

     buf = (R *) MALLOC(sizeof(R) * n, BUFFERS);

     for (iv = 0; iv < vl; ++iv, I += ivs, O += ovs) {
	  /* FIXME: split this loop to eliminate if-else statements and % */
	  for (i = 0; i < n; ++i) {
	       int m = (8 * i + n) % (8*n); /* n odd => m odd */
	       if (m < 2*n)
		    buf[i] = I[is * (n - 1 - (m - 1) / 2)];
	       else if (m < 4*n)
		    buf[i] = -I[is * (n - 1 - (4*n - m - 1) / 2)];
	       else if (m < 6*n)
		    buf[i] = -I[is * (n - 1 - (m - 4*n - 1) / 2)];
	       else
		    buf[i] = I[is * (n - 1 - (8*n - m - 1) / 2)];
	  }
	  
	  { /* child plan: R2HC of size n */
	       plan_rdft *cld = (plan_rdft *) ego->cld;
	       cld->apply((plan *) cld, buf, buf);
	  }
	  
	  /* FIXME: split/unroll loop to eliminate if-else's and % */
	  for (i = 0; i < n; ++i) {
	       int k;
	       R c, s;

	       k = (2*i + 1) % n;
	       
	       if (k < n - k) {
		    c = buf[k];
		    s = k == 0 ? 0.0 : -buf[n - k];
	       }
	       else {
		    c = buf[n - k];
		    s = k == n ? 0.0 : buf[k];
	       }

	       O[os * i] = SQRT2 * (c * (1 - 2 * (((i+1)/2) % 2)) -
				    s * (1 - 2 * ((i/2) % 2))) *
		    (1 - 2 * (i % 2));
	  }
     }

     X(ifree)(buf);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;
     AWAKE(ego->cld, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cld);
}

static void print(plan *ego_, printer *p)
{
     P *ego = (P *) ego_;
     p->print(p, "(%se-r2hc-odd-%d%v%(%p%))",
	      X(rdft_kind_str)(ego->kind), ego->n, ego->vl, ego->cld);
}

static int applicable0(const solver *ego_, const problem *p_)
{
     UNUSED(ego_);
     if (RDFTP(p_)) {
          const problem_rdft *p = (const problem_rdft *) p_;
          return (1
		  && p->sz->rnk == 1
		  && p->vecsz->rnk <= 1
		  && p->sz->dims[0].n % 2 == 1
		  && (p->kind[0] == REDFT11 || p->kind[0] == RODFT11)
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
     plan *cld;
     R *buf;
     int n;
     opcnt ops;

     static const plan_adt padt = {
	  X(rdft_solve), awake, print, destroy
     };

     if (!applicable(ego_, p_, plnr))
          return (plan *)0;

     p = (const problem_rdft *) p_;

     n = p->sz->dims[0].n;
     buf = (R *) MALLOC(sizeof(R) * n, BUFFERS);

     cld = X(mkplan_d)(plnr, X(mkproblem_rdft_1_d)(X(mktensor_1d)(n, 1, 1),
                                                   X(mktensor_0d)(),
                                                   buf, buf, R2HC));
     X(ifree)(buf);
     if (!cld)
          return (plan *)0;

     pln = MKPLAN_RDFT(P, &padt, p->kind[0]==REDFT11 ? apply_re11:apply_ro11);
     pln->n = n;
     pln->is = p->sz->dims[0].is;
     pln->os = p->sz->dims[0].os;
     pln->cld = cld;
     pln->kind = p->kind[0];
     
     X(tensor_tornk1)(p->vecsz, &pln->vl, &pln->ivs, &pln->ovs);
     
     X(ops_zero)(&ops);
     /* FIXME */

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(pln->vl, &ops, &pln->super.super.ops);
     X(ops_madd2)(pln->vl, &cld->ops, &pln->super.super.ops);

     return &(pln->super.super);
}

/* constructor */
static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan };
     S *slv = MKSOLVER(S, &sadt);
     return &(slv->super);
}

void X(reodft11e_r2hc_odd_register)(planner *p)
{
     REGISTER_SOLVER(p, mksolver());
}
