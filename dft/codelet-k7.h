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

/* $Id: codelet-k7.h,v 1.4 2002-06-17 01:30:42 athena Exp $ */

/* K7 codelet stuff */

extern solvtab X(solvtab_dft_k7);

typedef struct {
     uint sz;    /* size of transform computed */
     int sign;
     opcnt ops;
} kdft_k7_desc;

typedef void (*kdft_k7)(const R *ri, R *ro, int is, int os,
			uint vl, int ivs, int ovs);
void X(kdft_k7_register)(planner *p, kdft_k7 codelet, 
			 const kdft_k7_desc *desc);
solver *X(mksolver_dft_direct_k7)(kdft_k7 k, const kdft_k7_desc *desc);

typedef const R *(*kdft_dit_k7)(R *io, const R *W, int ios, uint m, int dist);
void X(kdft_dit_register)(planner *p, kdft_dit codelet, const ct_desc *desc);
solver *X(mksolver_dft_ct_dit_k7)(kdft_dit_k7 codelet, const ct_desc *desc);

typedef const R *(*kdft_dif_k7)(R *io, const R *W, int ios, uint m, int dist);
void X(kdft_dif_register)(planner *p, kdft_dif codelet, const ct_desc *desc);
solver *X(mksolver_dft_ct_dif_k7)(kdft_dif_k7 codelet, const ct_desc *desc);



