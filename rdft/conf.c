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

/* $Id: conf.c,v 1.5 2002-07-23 06:39:11 stevenj Exp $ */

#include "rdft.h"

static const solvtab s =
{
     X(rdft_indirect_register),
     X(rdft_rank0_register),
     X(rdft_vrank_geq1_register),
     X(rdft_vrank2_transpose_register),
     X(rdft_vrank3_transpose_register),
     X(rdft_nop_register),
     X(rdft_buffered_register),
     X(rdft_rader_dht_register),
     X(rdft_r2hc_hc2r_register),
/*
     X(rdft_rank_geq2_register),
     X(rdft_generic_register),
     X(rdft_rader_register),
*/
     0
};

void X(rdft_conf_standard)(planner *p)
{
     X(solvtab_exec)(s, p);
     X(solvtab_exec)(X(solvtab_rdft_r2hc), p);
     X(solvtab_exec)(X(solvtab_rdft_hc2r), p);
}
