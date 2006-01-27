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

/* $Id: ct.c,v 1.6 2006-01-27 02:10:50 athena Exp $ */

#include "threads.h"

typedef struct {
     plan_dft super;
     plan *cld;
     plan **cldws;
     int nthr;
     INT r, ts;
} P;

typedef struct {
     plan **cldws;
     R *r, *i;
     INT ts;
} PD;

static void *spawn_apply(spawn_data *d)
WITH_ALIGNED_STACK({
     PD *ego = (PD *) d->data;
     INT thr_num = d->thr_num;
     INT offset = thr_num * ego->ts;
     
     plan_dftw *cldw = (plan_dftw *) (ego->cldws[thr_num]);
     cldw->apply((plan *) cldw, ego->r + offset, ego->i + offset);
     return 0;
})

static void apply_dit(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, ri, ii, ro, io);

     {
	  PD d;
	  
	  d.r = ro; d.i = io;
	  d.cldws = ego->cldws;
	  d.ts = ego->ts;

	  X(spawn_loop)(ego->nthr, ego->nthr, spawn_apply, (void*)&d);
     }
}

static void apply_dif(const plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     const P *ego = (const P *) ego_;
     plan_dft *cld;

     {
	  PD d;
	  
	  d.r = ri; d.i = ii;
	  d.cldws = ego->cldws;
	  d.ts = ego->ts;

	  X(spawn_loop)(ego->nthr, ego->nthr, spawn_apply, (void*)&d);
     }

     cld = (plan_dft *) ego->cld;
     cld->apply(ego->cld, ri, ii, ro, io);
}

static void awake(plan *ego_, enum wakefulness wakefulness)
{
     P *ego = (P *) ego_;
     int i;
     X(plan_awake)(ego->cld, wakefulness);
     for (i = 0; i < ego->nthr; ++i)
	  X(plan_awake)(ego->cldws[i], wakefulness);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     int i;
     X(plan_destroy_internal)(ego->cld);
     for (i = 0; i < ego->nthr; ++i)
	  X(plan_destroy_internal)(ego->cldws[i]);
     X(ifree)(ego->cldws);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     int i;
     p->print(p, "(dft-thr-ct-%s-x%d/%D",
	      ego->super.apply == apply_dit ? "dit" : "dif",
	      ego->nthr, ego->r);
     for (i = 0; i < ego->nthr; ++i)
          if (i == 0 || (ego->cldws[i] != ego->cldws[i-1] &&
                         (i <= 1 || ego->cldws[i] != ego->cldws[i-2])))
               p->print(p, "%(%p%)", ego->cldws[i]);
     p->print(p, "%(%p%))", ego->cld);
}

static plan *mkplan(const solver *ego_, const problem *p_, planner *plnr)
{
     const ct_solver *ego = (const ct_solver *) ego_;
     const problem_dft *p;
     P *pln = 0;
     plan *cld = 0, **cldws = 0;
     INT n, r, m, vl, ivs, ovs;
     INT block_size, ts;
     int i, nthr, plnr_nthr_save;
     iodim *d;
     tensor *t1, *t2;

     static const plan_adt padt = {
	  X(dft_solve), awake, print, destroy
     };

     if (plnr->nthr <= 1 || !X(ct_applicable)(ego, p_, plnr))
          return (plan *) 0;

     p = (const problem_dft *) p_;
     d = p->sz->dims;
     n = d[0].n;
     r = X(choose_radix)(ego->r, n);
     m = n / r;

     X(tensor_tornk1)(p->vecsz, &vl, &ivs, &ovs);

     block_size = (m + plnr->nthr - 1) / plnr->nthr;
     nthr = (int)((m + block_size - 1) / block_size);
     plnr_nthr_save = plnr->nthr;
     plnr->nthr = (plnr->nthr + nthr - 1) / nthr;

     cldws = (plan **) MALLOC(sizeof(plan *) * nthr, PLANS);
     for (i = 0; i < nthr; ++i) cldws[i] = (plan *) 0;

     switch (ego->dec) {
	 case DECDIT:
	 {
	      ts = d[0].os * block_size;

	      for (i = 0; i < nthr; ++i) {
		   cldws[i] = ego->mkcldw(ego, 
					  DECDIT, r, m, d[0].os, vl, ovs,
					  i*block_size, 
					  (i == nthr - 1) ? 
					  (m - i*block_size) : block_size,
					  p->ro + ts*i, p->io + ts*i, plnr);
		   if (!cldws[i]) goto nada;
	      }

	      t1 = X(mktensor_1d)(r, d[0].is, m * d[0].os);
	      t2 = X(tensor_append)(t1, p->vecsz);
	      X(tensor_destroy)(t1);
	      
	      plnr->nthr = plnr_nthr_save;

	      cld = X(mkplan_d)(plnr, 
				X(mkproblem_dft_d)(
				     X(mktensor_1d)(m, r * d[0].is, d[0].os),
				     t2, p->ri, p->ii, p->ro, p->io)
		   );
	      if (!cld) goto nada;

	      pln = MKPLAN_DFT(P, &padt, apply_dit);
	      break;
	 }
	 case DECDIF:
	 {
	      ts = d[0].is * block_size;

	      for (i = 0; i < nthr; ++i) {
		   cldws[i] = ego->mkcldw(ego, 
					  DECDIF, r, m, d[0].is, vl, ivs,
					  i*block_size, 
					  (i == nthr - 1) ? 
					  (m - i*block_size) : block_size,
					  p->ri + ts*i, p->ii + ts*i, plnr);
		   if (!cldws[i]) goto nada;
	      }

	      t1 = X(mktensor_1d)(r, m * d[0].is, d[0].os);
	      t2 = X(tensor_append)(t1, p->vecsz);
	      X(tensor_destroy)(t1);

	      plnr->nthr = plnr_nthr_save;

	      cld = X(mkplan_d)(plnr, 
				X(mkproblem_dft_d)(
				     X(mktensor_1d)(m, d[0].is, r * d[0].os),
				     t2, p->ri, p->ii, p->ro, p->io)
		   );
	      if (!cld) goto nada;
	      
	      pln = MKPLAN_DFT(P, &padt, apply_dif);
	      break;
	 }

	 default: A(0);
	      
     }

     pln->cld = cld;
     pln->cldws = cldws;
     pln->nthr = nthr;
     pln->ts = ts;
     pln->r = r;
     X(ops_zero)(&pln->super.super.ops);
     for (i = 0; i < nthr; ++i)
          X(ops_add2)(&cldws[i]->ops, &pln->super.super.ops);
     X(ops_add2)(&cld->ops, &pln->super.super.ops);
     return &(pln->super.super);

 nada:
     if (cldws) {
	  for (i = 0; i < nthr; ++i)
	       X(plan_destroy_internal)(cldws[i]);
	  X(ifree)(cldws);
     }
     X(plan_destroy_internal)(cld);
     return (plan *) 0;
}

ct_solver *X(mksolver_ct_threads)(size_t size, INT r, int dec,
				  ct_mkinferior mkcldw)
{
     static const solver_adt sadt = { PROBLEM_DFT, mkplan };
     ct_solver *slv = (ct_solver *) X(mksolver)(size, &sadt);
     slv->r = r;
     slv->dec = dec;
     slv->mkcldw = mkcldw;
     return slv;
}
