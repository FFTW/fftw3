#include <spu_intrinsics.h>
#include <cbe_mfc.h>

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

typedef const R *(*t_codelet) (R *ri, R *ii, const R *W,
			       stride ios, INT m, INT dist);

extern const n_codelet X(n2fvtab)[33];
extern const t_codelet X(t1fvtab)[33];

void X(spu_execute_plan) (struct spu_plan * p, const R *in, R *out);

void X(spu_alloc_reset)(void);
void *X(spu_alloc)(size_t sz);
size_t X(spu_alloc_avail)(void);

void X(spu_complex_memcpy)(R *dst, int dstride,
			   const R *src, int sstride,
			   int n);

void X(spu_complex_transpose)(R *A_, int n);
void X(spu_complex_transpose_and_swap)(R *A_, R *B_, int ni, int nj);

void X(spu_dma1d)(void *spu_addr, long long ppu_addr, size_t sz,
		  unsigned int cmdl);

void X(spu_dma2d)(R *A, long long ppu_addr, 
		  int n, /* int spu_stride = 2 , */ int ppu_stride_bytes,
		  int v, /* int spu_vstride = 2 * n, */
		  int ppu_vstride_bytes,
		  unsigned int cmdl,
		  R *buf, int nbuf);

struct transpose_context;
struct dft_context;
void X(spu_do_transpose)(const struct transpose_context *t);
void X(spu_do_dft)(const struct dft_context *dft);

/* max # of DMA lists */
#define MAX_LIST_SZ 64

/* DMA preferred alignment */
#define ALIGNMENT 128

/* add k to spu_addr, 0 < k <= ALIGNMENT, so that 
   (spu_addr + k) % ALIGNMENT == ppu_addr % ALIGNMENT */
#define ALIGN_LIKE(spu_addr, ppu_addr)						\
  (void *)(((char *)(spu_addr)) +						\
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

const R *X(spu_t1fv_10) (R *ri, R *ii, const R *W, stride ios, INT m,
			 INT dist);
const R *X(spu_t1fv_12) (R *ri, R *ii, const R *W, stride ios, INT m,
			 INT dist);
const R *X(spu_t1fv_15) (R *ri, R *ii, const R *W, stride ios, INT m,
			 INT dist);
const R *X(spu_t1fv_16) (R *ri, R *ii, const R *W, stride ios, INT m,
			 INT dist);
const R *X(spu_t1fv_2) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
const R *X(spu_t1fv_32) (R *ri, R *ii, const R *W, stride ios, INT m,
			 INT dist);
const R *X(spu_t1fv_3) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
const R *X(spu_t1fv_4) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
const R *X(spu_t1fv_5) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
const R *X(spu_t1fv_6) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
const R *X(spu_t1fv_7) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
const R *X(spu_t1fv_8) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
const R *X(spu_t1fv_9) (R *ri, R *ii, const R *W, stride ios, INT m,
			INT dist);
