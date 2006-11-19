#include <stdlib.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include "fftw-spu.h"
#include "../fftw-cell.h"

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
static void do_dft(const struct dft_context *dft)
{
     int v, vv;
     int chunk, nbuf;
     struct spu_plan plan[MAX_PLAN_LEN];
     R *W, *A, *Aalign, *Balign, *buf;
     int n = dft->n;
     int twon = 2 * n;
     int nbytes = twon * sizeof(R);

     X(spu_alloc_reset)();

     /* obtain twiddle factors */
     W = X(spu_alloc)(dft->Wsz_bytes + ALIGNMENT);
     W = ALIGN_LIKE(W, dft->W);
     X(spu_dma1d)(W, dft->W, dft->Wsz_bytes, MFC_GETL_CMD);
     
     /* build plan */
     build_plan(plan, dft->n, W, &dft->r);

     /* Compute values of chunk and nbuf, and allocate proper buffers.
	This code is the proof that one can write FORTRAN in any
	language. */
     {
	  size_t avail = X(spu_alloc_avail)() - 3 * ALIGNMENT - nbytes;
	  if (dft->is_bytes != 2 * sizeof(R) || 
	      dft->os_bytes != 2 * sizeof(R)) {
	       /* We use dma2d in vector mode, and threfore chunk must
		  be a multiple of VL.  Compute chunk/VL first, and 
		  multiply by VL at the end */
		  
	       const int chunkalign = (128 / (2 * sizeof(R))) / VL;
	       const int maxchunk = (dft->v1 - dft->v0) / VL;
	       avail /= VL;

	       /* initial value of nbuf (approx. 1/8 of memory for buffers) */
	       nbuf = 1 + n/8; 
	       if (nbuf > MAX_LIST_SZ) nbuf = MAX_LIST_SZ;

	       /* for given value of nbuf, try to increase chunk, subject
		  to available memory, maxchunk, and alignment constraints */
	       chunk = avail / (nbytes + 2 * sizeof(R) * nbuf);
	       if (chunk > maxchunk) chunk = maxchunk;
	       if (chunk > chunkalign) chunk &= ~(chunkalign-1);

	       /* for given value of chunk, make nbuf as large as possible */
	       if (chunk > 0)
		    nbuf = (avail - chunk * nbytes) / (2 * sizeof(R) * chunk);
	       if (nbuf > MAX_LIST_SZ) nbuf = MAX_LIST_SZ;

	       /* finally, compute the true value of chunk */
	       chunk *= VL;
	  } else {
	       /* easy case, we need no buffers */
	       const int chunkalign = (128 / (2 * sizeof(R)));
	       nbuf = 0;
	       chunk = avail / nbytes;
	       if (chunk > chunkalign) chunk &= ~(chunkalign-1);
	  }

	  buf = X(spu_alloc)(2 * sizeof(R) * nbuf * chunk + ALIGNMENT);
	  A = X(spu_alloc)((chunk + 1) * nbytes + 2 * ALIGNMENT);
     } /* QED */

     /* for all vector elements we are responsible for: */
     for (v = dft->v0; v < dft->v1; v += chunk) {
	  if (chunk > dft->v1 - v)
	       chunk = dft->v1 - v;

	  Aalign = ALIGN_LIKE(A + ALIGNMENT + twon, 
			      dft->xi + v * dft->ivs_bytes);
	  Balign = ALIGN_LIKE(A, dft->xo + v * dft->ovs_bytes);

	  /* obtain data */
	  X(spu_dma2d)(Aalign, dft->xi + v * dft->ivs_bytes,
		       n, /* 2, */ dft->is_bytes,
		       chunk, /* twon, */ dft->ivs_bytes,
		       MFC_GETL_CMD, buf, nbuf);

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
	  X(spu_dma2d)(Balign, dft->xo + v * dft->ovs_bytes,
		       n, /* 2, */ dft->os_bytes,
		       chunk, /* twon, */ dft->ovs_bytes,
		       MFC_PUTL_CMD, buf, nbuf);
     }
}



static void do_transpose(const struct transpose_context *t)
{
     int i, j, ni, nj;
     int n = t->n, nspe = t->nspe, my_id = t->my_id;
     int s1_bytes = t->s1_bytes;
     int s0_bytes = t->s0_bytes;
     int nblock, blocksz;
     size_t avail;
     R *A, *B, *Aalign, *Balign;

     X(spu_alloc_reset)();

     /* poor man's square root */
     avail = X(spu_alloc_avail)();
     blocksz = 32;
     while (2 * ((blocksz + 16) * (blocksz + 16) * 2 * sizeof(R) + ALIGNMENT) < 
	    avail)
	  blocksz += 16;

     A = X(spu_alloc)(blocksz * blocksz * 2 * sizeof(R) + ALIGNMENT);
     B = X(spu_alloc)(blocksz * blocksz * 2 * sizeof(R) + ALIGNMENT);

     nblock = 0;
     ni = blocksz;
     for (i = 0; i < n; i += ni) {
	  nj = blocksz;
	  for (j = 0; j < n; j += nj) {
	       if ((nblock++ % nspe) != my_id)
		    continue; /* block is not ours */

	       if (ni > n - i) ni = n - i;
	       if (nj > n - j) nj = n - j;

	       if (i == j) {
		    /* diagonal block */
		    Aalign = ALIGN_LIKE(A, t->A + (i * s1_bytes + j * s0_bytes));
		    X(spu_dma2d)(Aalign, t->A + (i * s1_bytes + j * s0_bytes),
				 nj, s0_bytes, ni, s1_bytes, MFC_GETL_CMD, 0, 0);
		    X(spu_complex_transpose)(Aalign, ni /* == nj */);
		    X(spu_dma2d)(Aalign, t->A + (i * s1_bytes + j * s0_bytes),
				 nj, s0_bytes, ni, s1_bytes, MFC_PUTL_CMD, 0, 0);
	       } else if (i > j) {
		    Aalign = ALIGN_LIKE(A, t->A + (i * s1_bytes + j * s0_bytes));
		    Balign = ALIGN_LIKE(B, t->A + (j * s1_bytes + i * s0_bytes));
		    X(spu_dma2d)(Aalign, t->A + (i * s1_bytes + j * s0_bytes),
				 nj, s0_bytes, ni, s1_bytes, MFC_GETL_CMD, 0, 0);
		    X(spu_dma2d)(Balign, t->A + (j * s1_bytes + i * s0_bytes),
				 ni, s0_bytes, nj, s1_bytes, MFC_GETL_CMD, 0, 0);
		    X(spu_complex_transpose_and_swap)(Aalign, Balign, ni, nj);
		    X(spu_dma2d)(Aalign, t->A + (i * s1_bytes + j * s0_bytes),
				 nj, s0_bytes, ni, s1_bytes, MFC_PUTL_CMD, 0, 0);
		    X(spu_dma2d)(Balign, t->A + (j * s1_bytes + i * s0_bytes),
				 ni, s0_bytes, nj, s1_bytes, MFC_PUTL_CMD, 0, 0);
	       } else {
		    /* nothing */
	       }
	  }
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
	  X(spu_dma1d)(&ctx, parm, sizeof(ctx), MFC_GETL_CMD);

	  switch (ctx.op) {
	      case SPE_DFT:
		   do_dft(&ctx.u.dft);
		   break;

	      case SPE_TRANSPOSE:
		   do_transpose(&ctx.u.transpose);
		   break;

	      case SPE_EXIT:
		   return 0;
	  }

	  ack();
     }
}
