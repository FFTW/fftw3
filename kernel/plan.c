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

/* $Id: plan.c,v 1.19 2003-01-26 16:51:16 athena Exp $ */

#include "ifftw.h"

/* "Plan: To bother about the best method of accomplishing an
   accidental result."  (Ambrose Bierce, The Enlarged Devil's
   Dictionary). */

plan *X(mkplan)(size_t size, const plan_adt *adt)
{
     plan *p = (plan *)MALLOC(size, PLANS);

     A(adt->destroy);
     p->awake_refcnt = 0;
     p->adt = adt;
     X(ops_zero)(&p->ops);
     p->pcost = 0.0;
     
     return p;
}

/*
 * destroy a plan
 */
void X(plan_destroy_internal)(plan *ego)
{
     if (ego) {
	  if (ego->awake_refcnt > 0)
	       ego->adt->awake(ego, 0);
          ego->adt->destroy(ego);
	  X(ifree)(ego);
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

