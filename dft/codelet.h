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

/* $Id: codelet.h,v 1.15 2002-06-30 18:37:55 athena Exp $ */

/*
 * This header file must include every file or define every
 * type or macro which is required to compile a codelet.
 */

#ifndef __CODELET_H__
#define __CODELET_H__

#include <math.h>
#include "ifftw.h"

/* macros used in codelets to reduce source code size */
#define K(x) ((R) x)
#define DK(name, value) const R name = K(value)

/* FMA macros */

#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ R FMA(R a, R b, R c)
{
     return a * b + c;
}

static __inline__ R FMS(R a, R b, R c)
{
     return a * b - c;
}

static __inline__ R FNMS(R a, R b, R c)
{
     return -(a * b - c);
}

#else
#define FMA(a, b, c) (((a) * (b)) + (c))
#define FMS(a, b, c) (((a) * (b)) - (c))
#define FNMS(a, b, c) ((c) - ((a) * (b)))
#endif

/**************************************************************
 * types of codelets
 **************************************************************/

/* DFT codelets */

typedef struct kdft_desc_s {
     uint sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     int (*okp)(
	  const struct kdft_desc_s *desc,
	  const R *ri, const R *ii, const R *ro, const R *io,
	  int is, int os, uint vl, int ivs, int ovs);
     int is;
     int os;
} kdft_desc;

typedef void (*kdft) (const R *ri, const R *ii, R *ro, R *io,
                      stride is, stride os, uint vl, int ivs, int ovs);
void X(kdft_register)(planner *p, kdft codelet, const kdft_desc *desc);


typedef struct ct_desc_s {
     uint radix;
     const char *nam;
     const tw_instr *tw;
     opcnt ops;
     int (*okp)(
	  const struct ct_desc_s *desc,
	  const R *rio, const R *iio, int ios, int vs, uint m, int dist);
     int s1;
     int s2;
} ct_desc;

typedef const R *(*kdft_dit) (R *rioarray, R *iioarray, const R *W,
                              stride ios, uint m, int dist);
void X(kdft_dit_register)(planner *p, kdft_dit codelet, const ct_desc *desc);


typedef const R *(*kdft_difsq) (R *rioarray, R *iioarray,
                                const R *W, stride is, stride vs,
                                uint m, int dist);
void X(kdft_difsq_register)(planner *p, kdft_difsq codelet,
                            const ct_desc *desc);


typedef const R *(*kdft_dif) (R *rioarray, R *iioarray, const R *W,
                              stride ios, uint m, int dist);
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
