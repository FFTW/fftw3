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

/* $Id: ct-dit.c,v 1.1 2002-08-29 05:44:33 stevenj Exp $ */

/* decimation in time Cooley-Tukey, with codelet divided among threads */
#include "threads.h"
#include "ct.h"

typedef struct {
     plan_ct super;
     uint nthr;
     uint mloop;
     int sW;
} P;

typedef struct {
     R *ro, *io;
     kdft_dit k;
     R *W;
     int sW;
     stride ios;
     int os;
} PD;

static void *spawn_apply(spawn_data *d)
{
     PD *ego = (PD *) d->data;
     uint min = d->min, max = d->max;
     int os = ego->os;

     ego->k(ego->ro + min * os, ego->io + min * os,
	    ego->W + min * ego->sW, ego->ios, max - min, os);
     return 0;
}

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     plan_ct *ego = (plan_ct *) ego_;
     plan *cld0 = ego->cld;
     plan_dft *cld = (plan_dft *) cld0;

     /* two-dimensional r x vl sub-transform: */
     cld->apply(cld0, ri, ii, ro, io);

     {
	  P *ego_thr = (P *) ego_;
	  PD d;
	  
	  d.ro = ro; d.io = io;
	  d.k = ego->k.dit;
	  d.W = ego->td->W;
	  d.sW = ego_thr->sW;
	  d.ios = ego->ios;
	  d.os = ego->os;

	  X(spawn_loop)(ego_thr->mloop, ego_thr->nthr, spawn_apply, (void*)&d);
     }
}

static int applicable(const solver_ct *ego, const problem *p_,
		      const planner *plnr)
{
     if (plnr->nthr > 1 && X(dft_ct_applicable)(ego, p_)) {
          const ct_desc *e = ego->desc;
          const problem_dft *p = (const problem_dft *) p_;
          iodim *d = p->sz.dims;
	  uint m = d[0].n / e->radix;
          return (1
		  && p->vecsz.rnk == 0
		  && (e->genus->okp(e, p->ro, p->io, 
				    (int)m * d[0].os, 0, m, d[0].os, plnr))
	       );
     }
     return 0;
}

static void finish(plan_ct *ego)
{
     const ct_desc *d = ego->slv->desc;
     ego->ios = X(mkstride)(ego->r, ego->m * ego->os);
     ego->super.super.ops =
          X(ops_add)(ego->cld->ops,
		     X(ops_mul)(ego->vl * ego->m / d->genus->vl, d->ops));
}

static int score(const solver *ego_, const problem *p_, const planner *plnr)
{
     const solver_ct *ego = (const solver_ct *) ego_;
     const problem_dft *p;
     uint n;

     if (!applicable(ego, p_, plnr))
          return BAD;

     p = (const problem_dft *) p_;

     n = p->sz.dims[0].n;
     if (0
	 || n <= 16 
	 || n / ego->desc->radix <= 4
	  )
          return UGLY;

     return GOOD;
}

static plan *mkplan(const solver *ego, const problem *p, planner *plnr)
{
     plan *pln;
     static const ctadt adt = {
	  sizeof(P), X(dft_mkcld_dit), finish, applicable, apply
     };
     pln = X(mkplan_dft_ct)((const solver_ct *) ego, p, plnr, &adt);
     if (pln) {
	  P *pln_thr = (P *) pln;
	  pln_thr->nthr = plnr->nthr;
	  pln_thr->mloop = 
	       pln_thr->super.m / pln_thr->super.slv->desc->genus->vl;
	  pln_thr->sW = X(twiddle_length)(pln_thr->super.r, 
					  pln_thr->super.slv->desc->tw);
     }
     return pln;
}

solver *X(mksolver_dft_ct_dit_thr)(kdft_dit codelet, const ct_desc *desc)
{
     static const solver_adt sadt = { mkplan, score };
     static const char name[] = "dft-dit-thr";
     union kct k;
     k.dit = codelet;

     return X(mksolver_dft_ct)(k, desc, name, &sadt);
}
