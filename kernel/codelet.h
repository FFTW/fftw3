/*
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
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

/* $Id: codelet.h,v 1.1.1.1 2002-06-02 18:42:32 athena Exp $ */

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
#define DK(name, value) static const R name  = K(value)

/* FMA macros */

#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ RL FMA(RL a, RL b, RL c)
{
     return a * b + c;
}

static __inline__ RL FMS(RL a, RL b, RL c)
{
     return a * b - c;
}

static __inline__ RL FNMS(RL a, RL b, RL c)
{
     return -(a * b - c);
}

#else
#define FMA(a, b, c) (((a) * (b)) + (c))
#define FMS(a, b, c) (((a) * (b)) - (c))
#define FNMS(a, b, c) ((c) - ((a) * (b)))
#endif

#if defined(__GNUC__) && defined(__i386__)
/* 
   Inline loop in notw codelets.
   This is a good idea on Pentia and a bad idea everywhere else
*/

#define INLINE_NOTW __inline__
#else
#define INLINE_NOTW
#endif

/* bytecode to express twiddle factors computation */
enum { TW_COS, TW_SIN, TW_TAN, TW_NEXT };

typedef struct {
     unsigned int op:3;
     unsigned int v:4;
     int i:25;
} tw_instr;


/**************************************************************
 * types of codelets
 **************************************************************/

/* DFT codelets */

typedef struct {
     int sz;   /* size of transform computed */
     int is;   /* input stride, or 0 if any */
     int os;   /* output stride, or 0 if any */
     flopcnt flops;
} kdft_desc;

typedef void (*kdft) (const R *ri, const R *ii, R *ro, R *io,
		      stride is, stride os, int vl, int ivs, int ovs);
void fftw_kdft_register(planner *p, kdft codelet, const kdft_desc *desc);


typedef struct {
     int radix;
     const tw_instr *tw;
     int is;
     int vs;
     flopcnt flops;
} ct_desc;


typedef const R *(*kdft_dit) (R *rioarray, R *iioarray, const R *W,
			      stride ios, int m, int dist);
void fftw_kdft_dit_register(planner *p, kdft_dit codelet, const ct_desc *desc);


typedef const R *(*kdft_difsq) (R *rioarray, R *iioarray,
				const R *W, stride is, stride vs,
				int m, int dist);
void fftw_kdft_difsq_register(planner *p, kdft_difsq codelet, 
			      const ct_desc *desc);


typedef const R *(*kdft_dif) (R *rioarray, R *iioarray, const R *W,
			      stride ios, int m, int dist);
void fftw_kdft_dif_register(planner *p, kdft_dif codelet, const ct_desc *desc);

#endif				/* __CODELET_H__ */
