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

/* $Id: codelet.h,v 1.2 2002-07-21 04:22:14 stevenj Exp $ */

/*
 * This header file must include every file or define every
 * type or macro which is required to compile a codelet.
 */

#ifndef __RDFT_CODELET_H__
#define __RDFT_CODELET_H__

#include <math.h>
#include "ifftw.h"

/**************************************************************
 * types of codelets
 **************************************************************/

/* real-input DFT codelets */
typedef struct kr2hc_desc_s kr2hc_desc;

typedef struct {
     int (*okp)(
	  const kr2hc_desc *desc,
	  const R *I, const R *ro, const R *io,
	  int is, int ios, int ros, uint vl, int ivs, int ovs);
     uint vl;
} kr2hc_genus;

struct kr2hc_desc_s {
     uint sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const kr2hc_genus *genus;
     int is;
     int ros, ios;
     int ivs;
     int ovs;
};

typedef void (*kr2hc) (const R *I, R *ro, R *io, stride is,
		       stride ros, stride ios, uint vl, int ivs, int ovs);
void X(kr2hc_register)(planner *p, kr2hc codelet, const kr2hc_desc *desc);

/* real-input DFT codelets, type II (middle case of hc2hc DIT) */
typedef kr2hc_desc kr2hcII_desc;
typedef kr2hc_genus kr2hcII_genus;
typedef kr2hc kr2hcII;
void X(kr2hcII_register)(planner *p, kr2hcII codelet, const kr2hcII_desc *desc);

/* half-complex to half-complex DIT/DIF codelets: */
typedef struct hc2hc_desc_s hc2hc_desc;

typedef struct {
     int (*okp)(
	  const struct hc2hc_desc_s *desc,
	  const R *rio, const R *iio, int ios, int vs, uint m, int dist);
     uint vl;
} hc2hc_genus;

struct hc2hc_desc_s {
     uint radix;
     const char *nam;
     const tw_instr *tw;
     opcnt ops;
     const hc2hc_genus *genus;
     int s1;
     int s2;
     int dist;
};

typedef const R *(*khc2hc) (R *rioarray, R *iioarray, const R *W,
                              stride ios, uint m, int dist);
void X(khc2hc_dit_register)(planner *p, khc2hc codelet, const hc2hc_desc *desc);

extern const solvtab X(solvtab_rdft_r2hc);

/* real-output DFT codelets */
typedef struct khc2r_desc_s khc2r_desc;

typedef struct {
     int (*okp)(
	  const khc2r_desc *desc,
	  const R *ri, const R *ii, const R *O,
	  int ris, int iis, int os, uint vl, int ivs, int ovs);
     uint vl;
} khc2r_genus;

struct khc2r_desc_s {
     uint sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const khc2r_genus *genus;
     int ris, iis;
     int os;
     int ivs;
     int ovs;
};

typedef void (*khc2r) (const R *ri, const R *ii, R *O, stride ris,
		       stride iis, stride os, uint vl, int ivs, int ovs);
void X(khc2r_register)(planner *p, khc2r codelet, const khc2r_desc *desc);

/* real-output DFT codelets, type III (middle case of hc2hc DIF) */
typedef khc2r_desc khc2rIII_desc;
typedef khc2r_genus khc2rIII_genus;
typedef khc2r khc2rIII;
void X(khc2rIII_register)(planner *p, khc2rIII codelet, const khc2rIII_desc *desc);

void X(khc2hc_dif_register)(planner *p, khc2hc codelet, const hc2hc_desc *desc);

extern const solvtab X(solvtab_rdft_hc2r);

#endif				/* __RDFT_CODELET_H__ */
