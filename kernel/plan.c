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

/* $Id: plan.c,v 1.2 2002-06-03 12:09:05 athena Exp $ */

#include "ifftw.h"

plan *fftw_mkplan(size_t size, const plan_adt *adt)
{
     plan *p = (plan *)fftw_malloc(size, PLANS);

     p->refcnt = 0;
     p->adt = adt;
     p->cost = 1.0e20;  /* default cost */
     return p;
}

/*
 * use a plan
 */
void fftw_plan_use(plan *ego)
{
     ++ego->refcnt;
}

/*
 * destroy a plan
 */
void fftw_plan_destroy(plan *ego)
{
     if ((--ego->refcnt) == 0) {
	  A(ego->adt->destroy);
	  ego->adt->destroy(ego);
     }
}
