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

#define ALIGNMENT 128

/* list of codelets: */
void X(spu_n2fv_10) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_12) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_14) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_16) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_2) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_32) (const R *ri, const R *ii, R *ro, R *io, stride is,
		     stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_4) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_6) (const R *ri, const R *ii, R *ro, R *io, stride is,
		    stride os, INT v, INT ivs, INT ovs);
void X(spu_n2fv_8) (const R *ri, const R *ii, R *ro, R *io, stride is,
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
