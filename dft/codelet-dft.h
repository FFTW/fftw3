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

/* $Id: codelet-dft.h,v 1.3 2003-03-15 20:29:42 stevenj Exp $ */

/*
 * This header file must include every file or define every
 * type or macro which is required to compile a codelet.
 */

#ifndef __CODELET_H__
#define __CODELET_H__

#include "ifftw.h"

/**************************************************************
 * types of codelets
 **************************************************************/

/* DFT codelets */
typedef struct kdft_desc_s kdft_desc;

typedef struct {
     int (*okp)(
	  const kdft_desc *desc,
	  const R *ri, const R *ii, const R *ro, const R *io,
	  int is, int os, int vl, int ivs, int ovs,
	  const planner *plnr);
     int vl;
} kdft_genus;

struct kdft_desc_s {
     int sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const kdft_genus *genus;
     int is;
     int os;
     int ivs;
     int ovs;
};

typedef void (*kdft) (const R *ri, const R *ii, R *ro, R *io,
                      stride is, stride os, int vl, int ivs, int ovs);
void X(kdft_register)(planner *p, kdft codelet, const kdft_desc *desc);


typedef struct ct_desc_s ct_desc;

typedef struct {
     int (*okp)(
	  const struct ct_desc_s *desc,
	  const R *rio, const R *iio, int ios, int vs, int m, int dist,
	  const planner *plnr);
     int vl;
} ct_genus;

struct ct_desc_s {
     int radix;
     const char *nam;
     const tw_instr *tw;
     opcnt ops;
     const ct_genus *genus;
     int s1;
     int s2;
     int dist;
};

typedef const R *(*kdft_dit) (R *rioarray, R *iioarray, const R *W,
                              stride ios, int m, int dist);
void X(kdft_dit_register)(planner *p, kdft_dit codelet, const ct_desc *desc);


typedef const R *(*kdft_difsq) (R *rioarray, R *iioarray,
                                const R *W, stride is, stride vs,
                                int m, int dist);
void X(kdft_difsq_register)(planner *p, kdft_difsq codelet,
                            const ct_desc *desc);


typedef const R *(*kdft_dif) (R *rioarray, R *iioarray, const R *W,
                              stride ios, int m, int dist);
void X(kdft_dif_register)(planner *p, kdft_dif codelet, const ct_desc *desc);

extern const solvtab X(solvtab_dft_standard);
extern const solvtab X(solvtab_dft_inplace);

#if HAVE_K7
extern const solvtab X(solvtab_dft_k7);
#endif

#if HAVE_SIMD
extern const solvtab X(solvtab_dft_simd);
#endif

#endif				/* __CODELET_H__ */
