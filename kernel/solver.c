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

/* $Id: solver.c,v 1.6 2006-01-05 03:04:27 stevenj Exp $ */

#include "ifftw.h"

solver *X(mksolver)(size_t size, const solver_adt *adt)
{
     solver *s = (solver *)MALLOC(size, SOLVERS);

     s->adt = adt;
     s->refcnt = 0;
     return s;
}

void X(solver_use)(solver *ego)
{
     ++ego->refcnt;
}

void X(solver_destroy)(solver *ego)
{
     if ((--ego->refcnt) == 0)
          X(ifree)(ego);
}

void X(solver_register)(planner *plnr, solver *s)
{
     plnr->adt->register_solver(plnr, s);
}
