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

/* $Id: conf.c,v 1.14 2002-06-30 18:37:55 athena Exp $ */

#include "dft.h"

static const solvtab s =
{
     X(dft_indirect_register),
     X(dft_rank0_register),
     X(dft_rank_geq2_register),
     X(dft_vrank_geq1_register),
     X(dft_vrank2_transpose_register),
     X(dft_vrank3_transpose_register),
     X(dft_buffered_register),
     X(dft_rader_register),
     X(dft_nop_register),
     0
};

void X(dft_conf_standard)(planner *p)
{
     X(solvtab_exec)(s, p);
     X(solvtab_exec)(X(solvtab_dft_standard), p);
     X(solvtab_exec)(X(solvtab_dft_inplace), p);
#if HAVE_K7
     X(solvtab_exec)(X(solvtab_dft_k7), p);
#endif
#if HAVE_SIMD
     X(solvtab_exec)(X(solvtab_dft_simd), p);
#endif
}
