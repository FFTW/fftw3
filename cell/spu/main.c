#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cbe_mfc.h>
#include <spu_mfcio.h>
#include "fftw-spu.h"
#include "../fftw-cell.h"

#define MAX_DMA_SIZE 16384

#if FFTW_SINGLE
typedef unsigned long long complex_hack_t;
#else
typedef vector float complex_hack_t;
#endif

static void complex_memcpy(R *dst, int dstride,
			   const R *src, int sstride,
			   int n)
{
     int i;

     for (i = 0; i < n - 3; i += 4, src += 4 * sstride, dst += 4 * dstride) {
	  complex_hack_t x0 = *((complex_hack_t *)(src + 0 * sstride));
	  complex_hack_t x1 = *((complex_hack_t *)(src + 1 * sstride));
	  complex_hack_t x2 = *((complex_hack_t *)(src + 2 * sstride));
	  complex_hack_t x3 = *((complex_hack_t *)(src + 3 * sstride));
	  *((complex_hack_t *)(dst + 0 * dstride)) = x0;
	  *((complex_hack_t *)(dst + 1 * dstride)) = x1;
	  *((complex_hack_t *)(dst + 2 * dstride)) = x2;
	  *((complex_hack_t *)(dst + 3 * dstride)) = x3;
     }
     for (; i < n; ++i, src += sstride, dst += dstride) {
	  complex_hack_t x0 = *((complex_hack_t *)(src + 0 * sstride));
	  *((complex_hack_t *)(dst + 0 * dstride)) = x0;
     }
}

/* add k to spu_addr, 0 < k <= ALIGNMENT, so that 
   (spu_addr + k) % ALIGNMENT == ppu_addr % ALIGNMENT */
static void *align_like(void *spu_addr_, unsigned long long ppu_addr)
{
     char *spu_addr = spu_addr_;
     return spu_addr + ((((unsigned)ppu_addr) - ((unsigned)spu_addr))
			& (ALIGNMENT - 1));
}


/* universal DMA transfer routine */
static void dma1d(void *spu_addr_, unsigned long long ppu_addr, size_t sz,
		  unsigned int cmd)
		     
{
     unsigned int tag_id = 0;
     char *spu_addr = spu_addr_;

     while (sz > 0) {
	  /* select chunk to align ppu_addr */
	  size_t chunk = ALIGNMENT - (ppu_addr & (ALIGNMENT - 1));

	  /* if already aligned, transfer the whole thing */
	  if (chunk == ALIGNMENT || chunk > sz) 
	       chunk = sz;
	  
	  /* ...up to MAX_DMA_SIZE */
	  if (chunk > MAX_DMA_SIZE) 
	       chunk = MAX_DMA_SIZE;

	  spu_mfcdma32(spu_addr, ppu_addr, chunk, tag_id, cmd);
	  sz -= chunk; spu_addr += chunk; ppu_addr += chunk;
	  spu_mfcstat(2);
     }
}

/* 2D dma transfer routine, works for 
   ppu_stride_bytes == 2 * sizeof(R) or ppu_vstride_bytes == 2 * sizeof(R) */
static void dma2d(R *A, unsigned long long ppu_addr, 
		  size_t n, /* int spu_stride = 2 , */ int ppu_stride_bytes,
		  size_t v, /* int spu_vstride = 2 * n, */
		  int ppu_vstride_bytes,
		  unsigned int cmd,
		  R *buf)
{
     int vv, i;

     if (ppu_stride_bytes == 2 * sizeof(R)) { 
	  /* contiguous array on the PPU side */
	  
	  /* if the input is a 1D contiguous array, collapse n into v */
	  if (ppu_vstride_bytes == ppu_stride_bytes * n) {
	       n *= v;
	       v = 1;
	  }
	  
	  for (vv = 0; vv < v; ++vv) {
	       dma1d(A, ppu_addr, n * 2 * sizeof(R), cmd);
	       A += 2 * n;
	       ppu_addr += ppu_vstride_bytes;
	  }
     } else {
	  /* ppu_vstride_bytes == 2 * sizeof(R) */
	  for (i = 0; i < n; ++i) {
	       R *bufalign = align_like(buf, ppu_addr);
	       if (cmd == MFC_PUT_CMD)
		    complex_memcpy(bufalign, 2, A, 2 * n, v);
	       dma1d(bufalign, ppu_addr, v * 2 * sizeof(R), cmd);
	       if (cmd == MFC_GET_CMD)
		    complex_memcpy(A, 2 * n, bufalign, 2, v);
	       A += 2;
	       ppu_addr += ppu_stride_bytes;
	  }
     }
}
		     
static void build_plan(struct spu_plan *p,
		       int n, R *W, const struct spu_radices *pp)
{
     int r;
     const signed char *q;

     /* t-codelets */
     for (q = pp->r; (r = *q) > 0; ++q, ++p) {
	  n /= r;
	  p->r = r;
	  p->m = n;
	  p->W = W;
	  W += 2 * (r - 1) * n;
     }

     /* final n-codelet */
     p->r = r;
}

static void wait(void)
{
     while (spu_stat_in_mbox () < 1);
     (void) spu_read_in_mbox();
}

static void ack(void)
{
     spu_write_out_mbox(0);
}

static void flip_ri(R *A, int ncomplex)
{
     int i;
     for (i = 0; i < ncomplex - 7 * VL; i += 8 * VL) {
	  V x0 = LD(A + 2 * i + 0 * VL, 0, 0);
	  V x1 = LD(A + 2 * i + 2 * VL, 0, 0);
	  V x2 = LD(A + 2 * i + 4 * VL, 0, 0);
	  V x3 = LD(A + 2 * i + 6 * VL, 0, 0);
	  V x4 = LD(A + 2 * i + 8 * VL, 0, 0);
	  V x5 = LD(A + 2 * i + 10 * VL, 0, 0);
	  V x6 = LD(A + 2 * i + 12 * VL, 0, 0);
	  V x7 = LD(A + 2 * i + 14 * VL, 0, 0);
	  x0 = FLIP_RI(x0);
	  x1 = FLIP_RI(x1);
	  x2 = FLIP_RI(x2);
	  x3 = FLIP_RI(x3);
	  x4 = FLIP_RI(x4);
	  x5 = FLIP_RI(x5);
	  x6 = FLIP_RI(x6);
	  x7 = FLIP_RI(x7);
	  ST(A + 2 * i + 0 * VL, x0, 0, 0);
	  ST(A + 2 * i + 2 * VL, x1, 0, 0);
	  ST(A + 2 * i + 4 * VL, x2, 0, 0);
	  ST(A + 2 * i + 6 * VL, x3, 0, 0);
	  ST(A + 2 * i + 8 * VL, x4, 0, 0);
	  ST(A + 2 * i + 10 * VL, x5, 0, 0);
	  ST(A + 2 * i + 12 * VL, x6, 0, 0);
	  ST(A + 2 * i + 14 * VL, x7, 0, 0);
     }
     for (; i < ncomplex; i += VL) {
	  V x0 = LD(A + 2 * i, 0, 0);
	  x0 = FLIP_RI(x0);
	  ST(A + 2 * i, x0, 0, 0);
     }
}


/* compute DFT of contiguous arrays (is = os = 2) */
static void do_dft(struct dft_context *dft)
{
     int v, vv;
     int chunk;
     struct spu_plan plan[MAX_PLAN_LEN];
     R *W, *A, *Aalign, *Balign, *buf;
     int n = dft->n;
     int twon = 2 * n;
     int nbytes = twon * sizeof(R);

     X(spu_alloc_reset)();

     /* obtain twiddle factors */
     W = X(spu_alloc)(dft->Wsz_bytes + ALIGNMENT);
     W = align_like(W, dft->W);
     dma1d(W, dft->W, dft->Wsz_bytes, MFC_GET_CMD);
     
     /* build plan */
     build_plan(plan, dft->n, W, &dft->r);

     /* allocation:
	A: (chunk + 1) * nbytes + 2 * ALIGNMENT
	buf: 2 * sizeof(R) * chunk + ALIGNMENT.

	Solving for chunk yields: 
     */
     if (VL > 1 &&
	 (dft->is_bytes != 2 * sizeof(R) || dft->os_bytes != 2 * sizeof(R))) {
	  /* either the input or the output operates in vector mode.  
	     Must guarantee chunk % VL == 0 */
	  chunk = VL * ((X(spu_alloc_avail)() - 3 * ALIGNMENT - nbytes) / 
			(VL * (nbytes + 2 * sizeof(R))));
     } else {
	  chunk = (X(spu_alloc_avail)() - 3 * ALIGNMENT - nbytes) / 
	       (nbytes + 2 * sizeof(R));
     }

     buf = X(spu_alloc)(2 * sizeof(R) * chunk + ALIGNMENT);
     A = X(spu_alloc)((chunk + 1) * nbytes + 2 * ALIGNMENT);

     /* for all vector elements we are responsible for: */
     for (v = dft->v0; v < dft->v1; v += chunk) {
	  if (chunk > dft->v1 - v)
	       chunk = dft->v1 - v;

	  Aalign = align_like(A + ALIGNMENT + twon, 
			      dft->xi + v * dft->ivs_bytes);
	  Balign = align_like(A, dft->xo + v * dft->ovs_bytes);

	  /* obtain data */
	  dma2d(Aalign, dft->xi + v * dft->ivs_bytes,
		n, /* 2, */ dft->is_bytes,
		chunk, /* twon, */ dft->ivs_bytes,
		MFC_GET_CMD, buf);

	  if (dft->sign == 1)
	       flip_ri(Aalign, n * chunk);

	  /* compute FFTs */
	  for (vv = 0; vv < chunk; ++vv) 
	       X(spu_execute_plan)(plan,
				   Aalign + vv * twon, 
				   Balign + vv * twon);

	  if (dft->sign == 1)
	       flip_ri(Balign, n * chunk);

	  /* write data back */
	  dma2d(Balign, dft->xo + v * dft->ovs_bytes,
		n, /* 2, */ dft->os_bytes,
		chunk, /* twon, */ dft->ovs_bytes,
		MFC_PUT_CMD, buf);
     }
}


int main(unsigned long long spu_id, unsigned long long parm)
{  
     static struct spu_context ctx __attribute__ ((aligned (ALIGNMENT)));
     UNUSED(spu_id);
     spu_writech(MFC_WrTagMask, -1);

     for (;;) {
	  wait();

	  /* obtain context */
	  dma1d(&ctx, parm, sizeof(ctx), MFC_GET_CMD);

	  switch (ctx.op) {
	      case SPE_DFT:
		   do_dft(&ctx.u.dft);
		   break;

	      case SPE_EXIT:
		   return 0;
	  }

	  ack();
     }
}
