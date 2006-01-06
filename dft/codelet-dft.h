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

/* $Id: codelet-dft.h,v 1.10 2006-01-06 03:19:09 athena Exp $ */

/*
 * This header file must include every file or define every
 * type or macro which is required to compile a codelet.
 */

#ifndef __DFT_CODELET_H__
#define __DFT_CODELET_H__

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
	  INT is, INT os, INT vl, INT ivs, INT ovs,
	  const planner *plnr);
     INT vl;
} kdft_genus;

struct kdft_desc_s {
     INT sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const kdft_genus *genus;
     INT is;
     INT os;
     INT ivs;
     INT ovs;
};

typedef void (*kdft) (const R *ri, const R *ii, R *ro, R *io,
                      stride is, stride os, INT vl, INT ivs, INT ovs);
void X(kdft_register)(planner *p, kdft codelet, const kdft_desc *desc);


typedef struct ct_desc_s ct_desc;

typedef struct {
     int (*okp)(
	  const struct ct_desc_s *desc,
	  const R *rio, const R *iio, 
	  INT ios, INT vs, INT m, INT mb, INT me, INT dist,
	  const planner *plnr);
     INT vl;
} ct_genus;

struct ct_desc_s {
     INT radix;
     const char *nam;
     const tw_instr *tw;
     const ct_genus *genus;
     opcnt ops;
     INT s1;
     INT s2;
     INT dist;
};

typedef const R *(*kdftw) (R *rioarray, R *iioarray, const R *W,
			   stride ios, INT m, INT dist);
void X(kdft_dit_register)(planner *p, kdftw codelet, const ct_desc *desc);
void X(kdft_dif_register)(planner *p, kdftw codelet, const ct_desc *desc);


typedef const R *(*kdftwsq) (R *rioarray, R *iioarray,
			     const R *W, stride is, stride vs,
			     INT m, INT dist);
void X(kdft_difsq_register)(planner *p, kdftwsq codelet, const ct_desc *desc);


extern const solvtab X(solvtab_dft_standard);

#if HAVE_K7
extern const solvtab X(solvtab_dft_k7);
#endif

#if HAVE_SIMD
extern const solvtab X(solvtab_dft_simd);
#endif

#endif				/* __DFT_CODELET_H__ */
