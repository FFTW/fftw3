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

/* $Id: codelet.h,v 1.8 2002-08-03 23:38:17 stevenj Exp $ */

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

typedef enum {
     R2HC = FFT_SIGN, HC2R = -FFT_SIGN, 
     R2HCII, HC2RIII,
     DHT, DST00, DST01, DST10, DST11, DCT00, DCT01, DCT10, DCT11
} rdft_kind;
#define R2HC_KINDP(k) ((k) == R2HC || (k) == R2HCII) /* uses kr2hc_genus */
#define HC2R_KINDP(k) ((k) == HC2R || (k) == HC2RIII) /* uses khc2r_genus */
#define R2R_KINDP(k) ((k) >= DHT)

/* real-input DFT codelets */
typedef struct kr2hc_desc_s kr2hc_desc;

typedef struct {
     int (*okp)(
	  const kr2hc_desc *desc,
	  const R *I, const R *ro, const R *io,
	  int is, int ios, int ros, uint vl, int ivs, int ovs);
     rdft_kind kind;
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
#define kr2hcII_register kr2hc_register

/* half-complex to half-complex DIT/DIF codelets: */
typedef struct hc2hc_desc_s hc2hc_desc;

typedef struct {
     int (*okp)(
	  const struct hc2hc_desc_s *desc,
	  const R *rio, const R *iio, int ios, int vs, uint m, int dist);
     rdft_kind kind;
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
     rdft_kind kind;
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
#define khc2rIII_register khc2r_register

void X(khc2hc_dif_register)(planner *p, khc2hc codelet, const hc2hc_desc *desc);

extern const solvtab X(solvtab_rdft_hc2r);

/* real-input & output DFT-like codelets (DHT, etc.) */
typedef struct kr2r_desc_s kr2r_desc;

typedef struct {
     int (*okp)(
	  const kr2r_desc *desc,
	  const R *I, const R *O,
	  int is, int os, uint vl, int ivs, int ovs);
     rdft_kind kind;
     uint vl;
} kr2r_genus;

struct kr2r_desc_s {
     uint sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const kr2r_genus *genus;
     int is, os;
     int ivs;
     int ovs;
};

typedef void (*kr2r) (const R *I, R *O, stride is, stride os,
		      uint vl, int ivs, int ovs);
void X(kr2r_register)(planner *p, kr2r codelet, const kr2r_desc *desc);

extern const solvtab X(solvtab_rdft_r2r);

#endif				/* __RDFT_CODELET_H__ */
