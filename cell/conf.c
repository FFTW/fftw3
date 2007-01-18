/*
 * Copyright (c) 2007 Massachusetts Institute of Technology
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

#include "dft.h"

#if HAVE_CELL

#include "fftw-cell.h"

static const solvtab s =
{
     SOLVTAB(X(dft_direct_cell_register)),
     SOLVTAB(X(ct_cell_direct_register)),
     SOLVTAB_END
};

void X(dft_conf_cell)(planner *p)
{
     X(solvtab_exec)(s, p);
}

#endif /* HAVE_CELL */
