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

/*
 * This header file must include every file or define every
 * type or macro which is required to compile a codelet.
 */

#ifndef __RDFT_CODELET_H__
#define __RDFT_CODELET_H__

#include "ifftw.h"

/**************************************************************
 * types of codelets
 **************************************************************/

/* FOOab, with a,b in {0,1}, denotes the FOO transform
   where a/b say whether the input/output are shifted by
   half a sample/slot. */

typedef enum {
     R2HC00, R2HC01, R2HC10, R2HC11,
     HC2R00, HC2R01, HC2R10, HC2R11,
     DHT, 
     REDFT00, REDFT01, REDFT10, REDFT11, /* real-even == DCT's */
     RODFT00, RODFT01, RODFT10, RODFT11  /*  real-odd == DST's */
} rdft_kind;

/* standard R2HC/HC2R transforms are unshifted */
#define R2HC R2HC00
#define HC2R HC2R00

#define R2HCII R2HC01
#define HC2RIII HC2R10

/* (k) >= R2HC00 produces a warning under gcc because checking x >= 0
   is superfluous for unsigned values...but it is needed because other
   compilers (e.g. icc) may define the enum to be a signed int...grrr. */
#define R2HC_KINDP(k) ((k) >= R2HC00 && (k) <= R2HC11) /* uses kr2hc_genus */
#define HC2R_KINDP(k) ((k) >= HC2R00 && (k) <= HC2R11) /* uses khc2r_genus */

#define R2R_KINDP(k) ((k) >= DHT) /* uses kr2r_genus */

#define REDFT_KINDP(k) ((k) >= REDFT00 && (k) <= REDFT11)
#define RODFT_KINDP(k) ((k) >= RODFT00 && (k) <= RODFT11)
#define REODFT_KINDP(k) ((k) >= REDFT00 && (k) <= RODFT11)

/* real-input DFT codelets */
typedef struct kr2hc_desc_s kr2hc_desc;

typedef struct {
     int (*okp)(
	  const kr2hc_desc *desc,
	  const R *I, const R *ro, const R *io,
	  INT is, INT ios, INT ros, INT vl, INT ivs, INT ovs);
     rdft_kind kind;
     INT vl;
} kr2hc_genus;

struct kr2hc_desc_s {
     INT sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const kr2hc_genus *genus;
     INT is;
     INT ros, ios;
     INT ivs;
     INT ovs;
};

typedef void (*kr2hc) (const R *I, R *ro, R *io, stride is,
		       stride ros, stride ios, INT vl, INT ivs, INT ovs);
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
	  const R *rio, const R *iio, INT ios, INT vs, INT m, INT dist);
     rdft_kind kind;
     int vl;
} hc2hc_genus;

struct hc2hc_desc_s {
     INT radix;
     const char *nam;
     const tw_instr *tw;
     const hc2hc_genus *genus;
     opcnt ops;
     INT s1;
     INT s2;
     INT dist;
};

typedef const R *(*khc2hc) (R *rioarray, R *iioarray, const R *W,
			    stride ios, INT m, INT dist);
void X(khc2hc_register)(planner *p, khc2hc codelet, const hc2hc_desc *desc);

extern const solvtab X(solvtab_rdft_r2hc);

/* real-output DFT codelets */
typedef struct khc2r_desc_s khc2r_desc;

typedef struct {
     int (*okp)(
	  const khc2r_desc *desc,
	  const R *ri, const R *ii, const R *O,
	  INT ris, INT iis, INT os, INT vl, INT ivs, INT ovs);
     rdft_kind kind;
     INT vl;
} khc2r_genus;

struct khc2r_desc_s {
     INT sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const khc2r_genus *genus;
     INT ris, iis;
     INT os;
     INT ivs;
     INT ovs;
};

typedef void (*khc2r) (const R *ri, const R *ii, R *O, stride ris,
		       stride iis, stride os, INT vl, INT ivs, INT ovs);
void X(khc2r_register)(planner *p, khc2r codelet, const khc2r_desc *desc);

/* real-output DFT codelets, type III (middle case of hc2hc DIF) */
typedef khc2r_desc khc2rIII_desc;
typedef khc2r_genus khc2rIII_genus;
typedef khc2r khc2rIII;
#define khc2rIII_register khc2r_register

extern const solvtab X(solvtab_rdft_hc2r);

/* real-input & output DFT-like codelets (DHT, etc.) */
typedef struct kr2r_desc_s kr2r_desc;

typedef struct {
     int (*okp)(
	  const kr2r_desc *desc,
	  const R *I, const R *O,
	  INT is, INT os, INT vl, INT ivs, INT ovs);
     INT vl;
} kr2r_genus;

struct kr2r_desc_s {
     INT sz;    /* size of transform computed */
     const char *nam;
     opcnt ops;
     const kr2r_genus *genus;
     rdft_kind kind;
     INT is, os;
     INT ivs;
     INT ovs;
};

typedef void (*kr2r) (const R *I, R *O, stride is, stride os,
		      INT vl, INT ivs, INT ovs);
void X(kr2r_register)(planner *p, kr2r codelet, const kr2r_desc *desc);

extern const solvtab X(solvtab_rdft_r2r);

#endif				/* __RDFT_CODELET_H__ */
