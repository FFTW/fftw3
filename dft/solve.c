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

/* $Id: solve.c,v 1.3 2003-02-28 23:28:58 stevenj Exp $ */

#include "dft.h"

/* use the apply() operation for DFT problems */
void X(dft_solve)(const plan *ego_, const problem *p_)
{
     const plan_dft *ego = (const plan_dft *) ego_;
     const problem_dft *p = (const problem_dft *) p_;
     ego->apply(ego_, p->ri, p->ii, p->ro, p->io);
}
