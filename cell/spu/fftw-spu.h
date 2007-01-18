/*
 * Copyright (c) 2007 Massachusetts Institute of Technology
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

#include <spu_intrinsics.h>
#include <spu_mfcio.h>

#define FAKE_CRUDE_TIME		/* the spu does not have proper time headers */
#include <ifftw.h>

/* override the definition in <ifftw.h>, since this hack is not
   required for the spu */
#undef MAKE_VOLATILE_STRIDE
#define MAKE_VOLATILE_STRIDE(x) (void)0

#ifdef FFTW_SINGLE
#include "spu-single.h"
#else
#include "spu-double.h"
#endif

struct spu_plan {
     int r;   /* Radix.  R<0 means nontwiddle codelet and stop */
     int m;
     R *W;    /* twiddle factors */
};

typedef void (*n_codelet) (const R *ri, const R *ii, R *ro, R *io,
			   stride is, stride os, INT v, INT ivs, INT ovs);

typedef void (*t_codelet) (R *ri, R *ii, const R *W, 
			   stride rs, INT mb, INT me, INT ms);

extern const n_codelet X(n2fvtab)[33];
extern const t_codelet X(t1fvtab)[33];

void X(spu_execute_plan) (struct spu_plan * p, const R *in, R *out);

void X(spu_alloc_reset)(void);
void *X(spu_alloc)(size_t sz);
size_t X(spu_alloc_avail)(void);

void X(spu_dma1d)(void *spu_addr, unsigned long long ppu_addr, size_t sz,
		  unsigned int cmdl);

void X(spu_dma2d)(R *A, unsigned long long ppu_addr, 
		  int n, /* int spu_stride = 2 , */ int ppu_stride_bytes,
		  int v, /* int spu_vstride = 2 * n, */
		  int ppu_vstride_bytes,
		  unsigned int cmdl);

struct dft_context;
void X(spu_do_dft)(const struct dft_context *dft);

struct transpose_context;
void X(spu_do_transpose)(const struct transpose_context *t);

struct copy_context;
void X(spu_do_copy)(const struct copy_context *c);

/* DMA preferred alignment */
#define ALIGNMENT 128

/* add k to spu_addr, 0 < k <= ALIGNMENT, so that 
   (spu_addr + k) % ALIGNMENT == ppu_addr % ALIGNMENT */
#define ALIGN_LIKE(spu_addr, ppu_addr)					 \
(void *)(((char *)(spu_addr)) +						 \
  ((((unsigned)(ppu_addr)) - ((unsigned)(spu_addr))) & (ALIGNMENT - 1)))

/* list of codelets: */
void X(spu_n2fv_2) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_3) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_4) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_5) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_6) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_7) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_8) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_9) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_10) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_11) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_12) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_13) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_14) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_15) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_16) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_32) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);

void X(spu_t1fv_10) (R *ri, R *ii, const R *W, stride rs, 
		     INT mb, INT me, INT ms);
void X(spu_t1fv_12) (R *ri, R *ii, const R *W, stride rs,
		     INT mb, INT me, INT ms);
void X(spu_t1fv_15) (R *ri, R *ii, const R *W, stride rs,
		     INT mb, INT me, INT ms);
void X(spu_t1fv_16) (R *ri, R *ii, const R *W, stride rs,
		     INT mb, INT me, INT ms);
void X(spu_t1fv_2) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
void X(spu_t1fv_32) (R *ri, R *ii, const R *W, stride rs,
		     INT mb, INT me, INT ms);
void X(spu_t1fv_3) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
void X(spu_t1fv_4) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
void X(spu_t1fv_5) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
void X(spu_t1fv_6) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
void X(spu_t1fv_7) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
void X(spu_t1fv_8) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
void X(spu_t1fv_9) (R *ri, R *ii, const R *W, stride rs,
		    INT mb, INT me, INT ms);
