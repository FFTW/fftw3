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

/* $Id: conf.c,v 1.28 2006-01-05 03:04:27 stevenj Exp $ */

#include "rdft.h"

static const solvtab s =
{
     SOLVTAB(X(rdft_indirect_register)),
     SOLVTAB(X(rdft_rank0_register)),
     SOLVTAB(X(rdft_vrank3_transpose_register)),
     SOLVTAB(X(rdft_vrank_geq1_register)),

     SOLVTAB(X(rdft_nop_register)),
     SOLVTAB(X(rdft_buffered_register)),
     SOLVTAB(X(rdft_generic_register)),
     SOLVTAB(X(rdft_rank_geq2_register)),

     SOLVTAB(X(dft_r2hc_register)),

     SOLVTAB(X(rdft_dht_register)),
     SOLVTAB(X(dht_r2hc_register)),
     SOLVTAB(X(dht_rader_register)),

     SOLVTAB(X(rdft2_vrank_geq1_register)),
     SOLVTAB(X(rdft2_nop_register)),
     SOLVTAB(X(rdft2_rank0_register)),
     SOLVTAB(X(rdft2_buffered_register)),
     SOLVTAB(X(rdft2_rank_geq2_register)),
     SOLVTAB(X(rdft2_radix2_register)),

     SOLVTAB(X(hc2hc_generic_register)),

     SOLVTAB_END
};

void X(rdft_conf_standard)(planner *p)
{
     X(solvtab_exec)(s, p);
     X(solvtab_exec)(X(solvtab_rdft_r2hc), p);
     X(solvtab_exec)(X(solvtab_rdft_hc2r), p);
     X(solvtab_exec)(X(solvtab_rdft_r2r), p);
}
