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

/* $Id: buffered.c,v 1.4 2002-06-09 15:01:41 athena Exp $ */

#include "dft.h"

typedef struct {
     uint nbuf;
     uint maxbufsz;
     uint skew_alignment;
     uint skew;
     const char *nam;
} bufadt;

typedef struct {
     solver super;
     const bufadt *adt;
} S;

typedef struct {
     plan_dft super;

     plan *cld, *cldcpy, *cld_rest;
     uint n, vl, nbuf, bufdist;
     int ivs, ovs;
     R *bufs;

     const S *slv;
} P;

/* transform a vector input with the help of bufs */
static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     plan_dft *cld = (plan_dft *) ego->cld;
     plan_dft *cldcpy = (plan_dft *) ego->cldcpy;
     plan_dft *cld_rest;
     uint i, i1, vl = ego->vl, nbuf = ego->nbuf;
     int ivs = ego->ivs, ovs = ego->ovs;
     R *bufs = ego->bufs;

     /* note unsigned i:  the obvious statement

	     for (i = 0; i <= vl - nbuf; i += nbuf) 

        is wrong */
     for (i = nbuf; i <= vl; i += nbuf) {
	  i1 = i - nbuf;
	  /* transform to bufs: */
	  cld->apply(ego->cld, ri + i1 * ivs, ii + i1 * ivs, bufs, bufs + 1);

	  /* copy back */
	  cldcpy->apply(ego->cldcpy, bufs, bufs + 1,
			ro + i1 * ovs, io + i1 * ovs);
     }

     /* Do the remaining transforms, if any: */
     cld_rest = (plan_dft *) ego->cld_rest;
     i1 = i - nbuf;
     cld_rest->apply(ego->cld_rest, ri + i1 * ivs, ii + i1 * ivs,
		     ro + i1 * ovs, io + i1 * ovs);
}


static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     ego->cld->adt->awake(ego->cld, flg);
     ego->cldcpy->adt->awake(ego->cldcpy, flg);
     ego->cld_rest->adt->awake(ego->cld_rest, flg);

     if (flg) {
	  if (!ego->bufs)
	       ego->bufs = 
		    (R *) fftw_malloc(sizeof(R) * ego->nbuf * ego->bufdist * 2,
				      BUFFERS);
     } else {
	  if (ego->bufs) fftw_free(ego->bufs);
	  ego->bufs = 0;
     }
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     if (ego->bufs) fftw_free(ego->bufs);
     fftw_plan_destroy(ego->cld_rest);
     fftw_plan_destroy(ego->cldcpy);
     fftw_plan_destroy(ego->cld);
     fftw_free(ego);
}

static void print(plan *ego_, plan_printf prntf)
{
     P *ego = (P *) ego_;
     prntf("(%s-%u-x%u/%u-%u ", 
	   ego->slv->adt->nam, ego->n, ego->nbuf, ego->vl, 
	   ego->bufdist % ego->n);
     ego->cld->adt->print(ego->cld, prntf);
     prntf(" ");
     ego->cldcpy->adt->print(ego->cldcpy, prntf);
     prntf(" ");
     ego->cld_rest->adt->print(ego->cld_rest, prntf);
     prntf(")");
}

static uint compute_nbuf(uint n, uint vl, const S *ego)
{
     uint i, nbuf = ego->adt->nbuf, maxbufsz = ego->adt->maxbufsz;

     if (nbuf * n > maxbufsz)
	  nbuf = fftw_uimax((uint)1, maxbufsz / n);

     /*
      * Look for a buffer number (not too big) that divides the
      * vector length, in order that we only need one child plan:
      */
     for (i = nbuf; i < vl && i < 2 * nbuf; ++i)
	  if (vl % i == 0)
	       return i;

     /* whatever... */
     nbuf = fftw_uimin(nbuf, vl);
     return nbuf;
}


static int toobig(uint n, const S *ego)
{
     return (n > ego->adt->maxbufsz);
}

static int applicable(const problem *p_, const S *ego)
{
     if (DFTP(p_)) {
	  const problem_dft *p = (const problem_dft *) p_;
	  iodim *d = p->sz.dims;

	  if (1 
	      && p->vecsz.rnk <= 1
	      && p->sz.rnk == 1
	      
	      /*  should decide whether to be BAD or UGLY when problem
		  is too big. */
	      && !toobig(d[0].n, ego)
	       ) {

	       /*
		 In principle, the buffered transforms might
		 be useful when working out of place.
		 However, in order to prevent infinite loops
		 in the planner, we require that the output
		 stride of the buffered transforms be less
		 than greater than 2.
	       */
	       if (p->ri != p->ro) 
		    return (d[0].os > 2);

	       /* We can always do a single transform in-place */
	       if (p->vecsz.rnk == 0)
		    return 1;

	       /*
		* If the problem is in place, the input/output strides must
		* be the same or the whole thing must fit in the buffer.
		*/
	       return ((fftw_tensor_inplace_strides(p->sz) &&
			fftw_tensor_inplace_strides(p->vecsz))
		       || (compute_nbuf(d[0].n, p->vecsz.dims[0].n, ego) 
			   == p->vecsz.dims[0].n));
	  }
     }
     return 0;
}

static int score(const solver *ego_, const problem *p_)
{
     const S *ego = (const S *) ego_;
     const problem_dft *p;
     if (!applicable(p_, ego))
	  return BAD;

     p = (const problem_dft *) p_;
     if (p->ri != p->ro)
	  return UGLY;

     return GOOD;
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const S *ego = (const S *) ego_;
     const bufadt *adt = ego->adt;
     P *pln;
     plan *cld = (plan *) 0;
     plan *cldcpy = (plan *) 0;
     plan *cld_rest = (plan *) 0;
     problem *cldp = 0;
     const problem_dft *p = (const problem_dft *) p_;
     R *bufs = (R *) 0;
     uint nbuf = 0, bufdist, n, vl;
     int ivs, ovs;

     static const plan_adt padt = { 
	  fftw_dft_solve, awake, print, destroy 
     };


     if (!applicable(p_, ego))
	  goto nada;

     n = fftw_tensor_sz(p->sz);
     vl = fftw_tensor_sz(p->vecsz);

     nbuf = compute_nbuf(n, vl, ego);
     A(nbuf > 0);

     /*  
      * Determine BUFDIST, the offset between successive array bufs.
      * bufdist = n + skew, where skew is chosen such that bufdist %
      * skew_alignment = skew.
      */
     if (vl == 1) {
	  bufdist = n;
	  ivs = ovs = 0;
     } else {
	  bufdist = 
	       n + ((adt->skew_alignment + adt->skew - n % adt->skew_alignment)
		    % adt->skew_alignment);
	  A(p->vecsz.rnk == 1);
	  ivs = p->vecsz.dims[0].is;
	  ovs = p->vecsz.dims[0].os;
     }

     /* initial allocation for the purpose of planning */
     bufs = (R *) fftw_malloc(sizeof(R) * nbuf * bufdist * 2, BUFFERS);

     cldp = 
	  fftw_mkproblem_dft_d(
	       fftw_mktensor_1d(n, p->sz.dims[0].is, 2),
	       fftw_mktensor_1d(nbuf, ivs, bufdist * 2),
	       p->ri, p->ii, bufs, bufs + 1);

     cld = plnr->adt->mkplan(plnr, cldp);
     fftw_problem_destroy(cldp);
     if (!cld) goto nada;

     /* copying back from the buffer is a rank-0 transform: */
     cldp = 
	  fftw_mkproblem_dft_d(
	       fftw_mktensor(0),
	       fftw_mktensor_2d(nbuf, bufdist * 2, ovs, 
				n, 2, p->sz.dims[0].os),
	       bufs, bufs + 1, p->ro, p->io);

     cldcpy = plnr->adt->mkplan(plnr, cldp);
     fftw_problem_destroy(cldp);
     if (!cldcpy) goto nada;

     /* deallocate buffers, let awake() allocate them for real */
     fftw_free(bufs); bufs = 0;

     /* plan the leftover transforms (cld_rest): */
     cldp = 
	  fftw_mkproblem_dft_d(
	       fftw_tensor_copy(p->sz),
	       fftw_mktensor_1d(vl % nbuf, ivs, ovs),
	       p->ri, p->ii, p->ro, p->io);
     cld_rest = plnr->adt->mkplan(plnr, cldp);
     fftw_problem_destroy(cldp);
     if (!cld_rest) goto nada;

     pln = MKPLAN_DFT(P, &padt, apply);
     pln->cld = cld;
     pln->cldcpy = cldcpy;
     pln->cld_rest = cld_rest;
     pln->slv = ego;
     pln->n = n;
     pln->vl = vl;
     pln->ivs = ivs;
     pln->ovs = ovs;

     pln->nbuf = nbuf;
     pln->bufdist = bufdist;
     pln->bufs = 0;		/* let awake() reallocate buffer space */

     pln->super.super.cost =
	 +(cldcpy->cost + cld->cost) * (vl / nbuf)
	 + (cld_rest ? cld_rest->cost : 0);
     pln->super.super.flops =
	 fftw_flops_add(fftw_flops_mul((vl / nbuf), cld->flops),
			(cld_rest ? cld_rest->flops :
			 /* the Sun compiler does not like the obvious
			    fftw_flops_zero and we have to use this contorsion
			  */
			 *(flopcnt *) &fftw_flops_zero));

     return &(pln->super.super);

   nada:
     if (bufs)
	  fftw_free(bufs);
     if (cld_rest)
	  fftw_plan_destroy(cld_rest);
     if (cldcpy)
	  fftw_plan_destroy(cldcpy);
     if (cld)
	  fftw_plan_destroy(cld);
     return (plan *) 0;
}

static solver *mksolver(const bufadt *adt)
{
     static const solver_adt sadt = { mkplan, score };
     S *slv = MKSOLVER(S, &sadt);
     slv->adt = adt;
     return &(slv->super);
}


void fftw_dft_buffered_register(planner *p)
{
     /* FIXME: what are good defaults? */
     static const bufadt adt = {
	  /* nbuf */           8,   
	  /* maxbufsz */       (16384 / sizeof(R)),
	  /* skew_alignment */ 8,
	  /* skew */           5,
	  /* nam */            "dft-buffered"
     };

     p->adt->register_solver(p, mksolver(&adt));
}
