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

/* $Id: plan.c,v 1.16 2002-09-22 20:03:30 athena Exp $ */

#include "ifftw.h"

/* "Plan: To bother about the best method of accomplishing an
   accidental result."  (Ambrose Bierce, The Enlarged Devil's
   Dictionary). */

plan *X(mkplan)(size_t size, const plan_adt *adt)
{
     plan *p = (plan *)fftw_malloc(size, PLANS);

     A(adt->destroy);
     p->refcnt = p->awake_refcnt = 0;
     p->adt = adt;
     X(ops_zero)(&p->ops);
     p->pcost = 0.0;
     
     return p;
}

/*
 * use a plan
 */
void X(plan_use)(plan *ego)
{
     ++ego->refcnt;
}

/*
 * destroy a plan
 */
void X(plan_destroy)(plan *ego)
{
     if (ego && --ego->refcnt == 0) {
	  if (ego->awake_refcnt > 0)
	       ego->adt->awake(ego, 0);
          ego->adt->destroy(ego);
	  X(free)(ego);
     }
}

/* dummy destroy routine for plans with no local state */
void X(plan_null_destroy)(plan *ego)
{
     UNUSED(ego);
     /* nothing */
}

void X(plan_awake)(plan *ego, int flag)
{
     if (flag) {
	  if (!ego->awake_refcnt)
	       ego->adt->awake(ego, flag);
	  ++ego->awake_refcnt;
     } else {
	  --ego->awake_refcnt;
	  if (!ego->awake_refcnt)
	       ego->adt->awake(ego, flag);
     }
}

