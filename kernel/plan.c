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

/* $Id: plan.c,v 1.8 2002-07-14 21:12:14 stevenj Exp $ */

#include "ifftw.h"

plan *X(mkplan)(size_t size, const plan_adt *adt)
{
     plan *p = (plan *)fftw_malloc(size, PLANS);

     A(adt->destroy);
     p->refcnt = p->awake_refcnt = 0;
     p->adt = adt;
     p->ops = X(ops_zero);
     p->pcost = 0.0;
     p->blessed = 0;
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
     if ((--ego->refcnt) == 0) 
          DESTROY(ego);
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

static void bless(plan *p, void *closure)
{
     UNUSED(closure);
     p->blessed = 1;
}

/* can a blessing be revoked? */
void X(plan_bless)(plan *ego)
{
     X(traverse_plan)(ego, bless, (void *) 0);
}
