/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* $Id: solve.c,v 1.3 2003-03-15 20:29:43 stevenj Exp $ */

#include "rdft.h"

/* use the apply() operation for RDFT problems */
void X(rdft_solve)(const plan *ego_, const problem *p_)
{
     const plan_rdft *ego = (const plan_rdft *) ego_;
     const problem_rdft *p = (const problem_rdft *) p_;
     ego->apply(ego_, p->I, p->O);
}
