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

/* $Id: buffered.c,v 1.45 2003-02-28 23:28:58 stevenj Exp $ */

#include "dft.h"

typedef struct {
     int nbuf;
     int maxbufsz;
     int skew_alignment;
     int skew;
     const char *nam;
} bufadt;

typedef struct {
     solver super;
     const bufadt *adt;
} S;

typedef struct {
     plan_dft super;

     plan *cld, *cldcpy, *cldrest;
     int n, vl, nbuf, bufdist;
     int ivs, ovs;
     int roffset, ioffset;

     const S *slv;
} P;

/* transform a vector input with the help of bufs */
static void apply(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     int nbuf = ego->nbuf;
     R *bufs = (R *)MALLOC(sizeof(R) * nbuf * ego->bufdist * 2, BUFFERS);

     plan_dft *cld = (plan_dft *) ego->cld;
     plan_dft *cldcpy = (plan_dft *) ego->cldcpy;
     plan_dft *cldrest;
     int i, vl = ego->vl;
     int ivs = ego->ivs, ovs = ego->ovs;
     int roffset = ego->roffset, ioffset = ego->ioffset;

     /* note unsigned i:  the obvious statement

          for (i = 0; i <= vl - nbuf; i += nbuf) 

	is wrong */
     for (i = nbuf; i <= vl; i += nbuf) {
          /* transform to bufs: */
          cld->apply((plan *) cld, ri, ii, bufs + roffset, bufs + ioffset);
	  ri += ivs; ii += ivs;

          /* copy back */
          cldcpy->apply((plan *) cldcpy, bufs+roffset, bufs+ioffset, ro, io);
	  ro += ovs; io += ovs;
     }

     /* Do the remaining transforms, if any: */
     cldrest = (plan_dft *) ego->cldrest;
     cldrest->apply((plan *) cldrest, ri, ii, ro, io);

     X(ifree)(bufs);
}


static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     AWAKE(ego->cld, flg);
     AWAKE(ego->cldcpy, flg);
     AWAKE(ego->cldrest, flg);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(plan_destroy_internal)(ego->cldrest);
     X(plan_destroy_internal)(ego->cldcpy);
     X(plan_destroy_internal)(ego->cld);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     p->print(p, "(%s-%d%v/%d-%d%(%p%)%(%p%)%(%p%))",
              ego->slv->adt->nam,
              ego->n, ego->nbuf,
              ego->vl, ego->bufdist % ego->n,
              ego->cld, ego->cldcpy, ego->cldrest);
}

static int compute_nbuf(int n, int vl, const S *ego)
{
     return X(compute_nbuf)(n, vl, ego->adt->nbuf, ego->adt->maxbufsz);
}

static int toobig(int n, const S *ego)
{
     return (n > ego->adt->maxbufsz);
}

static int applicable0(const problem *p_, const S *ego, const planner *plnr)
{
     if (DFTP(p_)) {
          const problem_dft *p = (const problem_dft *) p_;
          const iodim *d = p->sz->dims;

          if (1
	      && p->vecsz->rnk <= 1
	      && p->sz->rnk == 1
	       ) {

	       if (toobig(p->sz->dims[0].n, ego) && CONSERVE_MEMORYP(plnr))
                    return 0;

               /*
		 In principle, the buffered transforms might be useful
		 when working out of place.  However, in order to
		 prevent infinite loops in the planner, we require
		 that the output stride of the buffered transforms be
		 greater than 2.
               */
               if (p->ri != p->ro)
                    return (d[0].os > 2);

               /* We can always do a single transform in-place */
               if (p->vecsz->rnk == 0)
                    return 1;

               /*
		* If the problem is in place, the input/output strides must
		* be the same or the whole thing must fit in the buffer.
		*/
               return ((X(tensor_inplace_strides2)(p->sz, p->vecsz))
                       || (compute_nbuf(d[0].n, p->vecsz->dims[0].n, ego)
                           == p->vecsz->dims[0].n));
          }
     }
     return 0;
}

static int applicable(const problem *p_, const S *ego, const planner *plnr)
{
     if (NO_BUFFERINGP(plnr)) return 0;
     if (!applicable0(p_, ego, plnr)) return 0;

     if (NO_UGLYP(plnr)) {
	  const problem_dft *p = (const problem_dft *) p_;
	  if (p->ri != p->ro) return 0;
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
     plan *cldcpy = (plan *) 0;
     plan *cldrest = (plan *) 0;
     const problem_dft *p = (const problem_dft *) p_;
     R *bufs = (R *) 0;
     int nbuf = 0, bufdist, n, vl;
     int ivs, ovs, roffset, ioffset;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (!applicable(p_, ego, plnr))
          goto nada;

     n = X(tensor_sz)(p->sz);

     X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs);

     nbuf = compute_nbuf(n, vl, ego);
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

     /* attempt to keep real and imaginary part in the same order,
	so as to allow optimizations in the the copy plan */
     roffset = (p->ri - p->ii > 0) ? 1 : 0;
     ioffset = 1 - roffset;

     /* initial allocation for the purpose of planning */
     bufs = (R *) MALLOC(sizeof(R) * nbuf * bufdist * 2, BUFFERS);

     cld = X(mkplan_d)(plnr,
		       X(mkproblem_dft_d)(
			    X(mktensor_1d)(n, p->sz->dims[0].is, 2),
			    X(mktensor_1d)(nbuf, ivs, bufdist * 2),
			    p->ri, p->ii, bufs + roffset, bufs + ioffset));
     if (!cld)
          goto nada;

     /* copying back from the buffer is a rank-0 transform: */
     cldcpy = X(mkplan_d)(plnr,
			  X(mkproblem_dft_d)(
			       X(mktensor_0d)(),
			       X(mktensor_2d)(nbuf, bufdist * 2, ovs,
					      n, 2, p->sz->dims[0].os),
			       bufs + roffset, bufs + ioffset, p->ro, p->io));
     if (!cldcpy)
          goto nada;

     /* deallocate buffers, let apply() allocate them for real */
     X(ifree)(bufs);
     bufs = 0;

     /* plan the leftover transforms (cldrest): */
     cldrest = X(mkplan_d)(plnr, 
			   X(mkproblem_dft_d)(
				X(tensor_copy)(p->sz),
				X(mktensor_1d)(vl % nbuf, ivs, ovs),
				p->ri, p->ii, p->ro, p->io));
     if (!cldrest)
          goto nada;

     pln = MKPLAN_DFT(P, &padt, apply);
     pln->cld = cld;
     pln->cldcpy = cldcpy;
     pln->cldrest = cldrest;
     pln->slv = ego;
     pln->n = n;
     pln->vl = vl;
     pln->ivs = ivs * nbuf;
     pln->ovs = ovs * nbuf;
     pln->roffset = roffset;
     pln->ioffset = ioffset;

     pln->nbuf = nbuf;
     pln->bufdist = bufdist;

     {
	  opcnt t;
	  X(ops_add)(&cld->ops, &cldcpy->ops, &t);
	  X(ops_madd)(vl / nbuf, &t, &cldrest->ops, &pln->super.super.ops);
     }

     return &(pln->super.super);

 nada:
     X(ifree0)(bufs);
     X(plan_destroy_internal)(cldrest);
     X(plan_destroy_internal)(cldcpy);
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


void X(dft_buffered_register)(planner *p)
{
     /* FIXME: what are good defaults? */
     static const bufadt adt = {
	  /* nbuf */           8,
	  /* maxbufsz */       (65536 / sizeof(R)),
	  /* skew_alignment */ 8,
#if HAVE_SIMD  /* 5 is odd and screws up the alignment. */
	  /* skew */           6,
#else
	  /* skew */           5,
#endif
	  /* nam */            "dft-buffered"
     };

     REGISTER_SOLVER(p, mksolver(&adt));
}
