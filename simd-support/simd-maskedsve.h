/*
 * Copyright (c) 2003, 2007-11 Matteo Frigo
 * Copyright (c) 2003, 2007-11 Massachusetts Institute of Technology
 *
 * ARM SVE support implemented by Romain Dolbeau. (c) 2017 Romain Dolbeau
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#if defined(FFTW_LDOUBLE) || defined(FFTW_QUAD)
#error "SVE vector instructions only works in single or double precision"
#endif

#ifdef FFTW_SINGLE
#  define DS(d,s) s /* single-precision option */
#  define TYPE(name) name ## _f32
#  define TYPESUF(name,suf) name ## _f32 ## suf
#  define ALLA	svptrue_b32()
#else /* !FFTW_SINGLE */
#  define DS(d,s) d /* double-precision option */
#  define TYPE(name) name ## _f64
#  define TYPESUF(name,suf) name ## _f64 ## suf
#  define ALLA  svptrue_b64()
#endif /* FFTW_SINGLE */

//#define SIMD_SUFFIX  _sve  /* for renaming */
#if SVE_SIZE == 2048
#define VL DS(16, 32)        /* SIMD complex vector length */
#define MASKA DS(svptrue_pat_b64(SV_VL32),svptrue_pat_b32(SV_VL64))
#elif SVE_SIZE == 1024
#define VL DS(8, 16)        /* SIMD complex vector length */
#define MASKA DS(svptrue_pat_b64(SV_VL16),svptrue_pat_b32(SV_VL32))
#elif SVE_SIZE == 512
#define VL DS(4, 8)        /* SIMD complex vector length */
#define MASKA DS(svptrue_pat_b64(SV_VL8),svptrue_pat_b32(SV_VL16))
#elif SVE_SIZE == 256
#define VL DS(2, 4)        /* SIMD complex vector length */
#define MASKA DS(svptrue_pat_b64(SV_VL4),svptrue_pat_b32(SV_VL8))
#elif SVE_SIZE == 128
#define VL DS(1, 2)        /* SIMD complex vector length */
#define MASKA DS(svptrue_pat_b64(SV_VL2),svptrue_pat_b32(SV_VL4))
#else /* SVE_SIZE */
#error "SVE_SIZE must be 128, 256, 512, 1024, 2048 bits"
#endif /* SVE_SIZE */
#define SIMD_VSTRIDE_OKA(x) ((x) == 2) 
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#if defined(__GNUC__) && !defined(__ARM_FEATURE_SVE) /* sanity check */
#error "compiling simd-sve.h without SVE support"
#endif

#include <arm_sve.h>

typedef DS(svfloat64_t, svfloat32_t) V;

#define VLIT(re, im) DS(svdupq_n_f64(re,im),svdupq_n_f32(re,im,re,im))
#define VLIT1(val) DS(svdup_n_f64(val), svdup_n_f32(val))
#define LDK(x) x
#define DVK(var, val) V var = VLIT1(val)
#define VZERO VLIT1(DS(0.,0.f))
#define VRONE VLIT(DS(1.,1.f),DS(0.,0.f))
#define VCI VLIT(DS(0.,0.f),DS(1.,1.f))
#define VCONEMI VLIT(DS(1.,1.f),DS(-1.,-1.f))
#define VONE  VLIT1(DS(1.,1.f))
#define VMINUSONE VLIT1(DS(-1.,-1.f))

#define VDUPL(x) TYPE(svtrn1)(x,x)
#define VDUPH(x) TYPE(svtrn2)(x,x)

#ifdef FFTW_SINGLE
//#define FLIP_RI(x) svreinterpret_f32_u64(svrevw_u64_x(MASKA,svreinterpret_u64_f32(x)))
#define FLIP_RI(x) TYPE(svtrn1)(VDUPH(x),x)
#else
#define FLIP_RI(x) TYPE(svtrn1)(VDUPH(x),x)
#endif

/* FXIME: there is a better way, surely */
/* #define VCONJ(x)  TYPESUF(svcmla,_x)(MASKA,TYPESUF(svcmla,_x)(MASKA,VZERO,x,VRONE,0),x,VRONE,270) */
#define VCONJ(x) TYPESUF(svmul,_x)(MASKA,x,VCONEMI)
#define VBYI(x)  TYPESUF(svcmla,_x)(MASKA,TYPESUF(svcmla,_x)(MASKA,VZERO,x,VCI,0),x,VCI,90)

#define VNEG(a)   TYPESUF(svneg,_x)(MASKA,a)
#define VADD(a,b) TYPESUF(svadd,_x)(MASKA,a,b)
#define VSUB(a,b) TYPESUF(svsub,_x)(MASKA,a,b)
#define VMUL(a,b) TYPESUF(svmul,_x)(MASKA,a,b)
#define VFMA(a, b, c)  TYPESUF(svmad,_x)(MASKA,b,a,c)
#define VFMS(a, b, c)  TYPESUF(svnmsb,_x)(MASKA,b,a,c)
#define VFNMS(a, b, c) TYPESUF(svmsb,_x)(MASKA,b,a,c)
#define VFMAI(b, c)    TYPESUF(svcadd,_x)(MASKA,c,b,90)
#define VFNMSI(b, c)   TYPESUF(svcadd,_x)(MASKA,c,b,270)
/* FIXME: next 3 overkill ? */
#if 0
#define VFMACONJ(b,c)  TYPESUF(svcmla,_x)(MASKA,TYPESUF(svcmla,_x)(MASKA,c,b,VRONE,0),b,VRONE,270)
#else
/* Use inline functions instead of macros to avoid replicating inputs */
static inline V VFMACONJ(V b, V c) {
	V m = TYPESUF(svcmla,_x)(MASKA,c,b,VRONE,0);
	return TYPESUF(svcmla,_x)(MASKA,m,b,VRONE,270);
}
#endif
#define VFMSCONJ(b,c)  VFMACONJ(b,VNEG(c))
#define VFNMSCONJ(b,c) VNEG(VFMSCONJ(b,c))

#if 0
#define VZMUL(a,b)    TYPESUF(svcmla,_x)(MASKA,TYPESUF(svcmla,_x)(MASKA,VZERO,a,b,0),a,b,90)
#define VZMULJ(a,b)   TYPESUF(svcmla,_x)(MASKA,TYPESUF(svcmla,_x)(MASKA,VZERO,a,b,0),a,b,270)
#define VZMULI(a,b)   VZMUL(VCI,VZMUL(a,b))
#define VZMULIJ(a,b)   VZMUL(VCI,VZMULJ(a,b))
#else
/* Use inline functions instead of macros to avoid replicating inputs */
static inline V VZMUL(V a, V b) {
	V m = TYPESUF(svcmla,_x)(MASKA,VZERO,a,b,0);
	return TYPESUF(svcmla,_x)(MASKA,m,a,b,90);
}
static inline V VZMULJ(V a, V b) {
        V m = TYPESUF(svcmla,_x)(MASKA,VZERO,a,b,0);
        return TYPESUF(svcmla,_x)(MASKA,m,a,b,270);
}
/* FIXME: there's probably a better way */
static inline V VZMULI(V a, V b) {
	V m = VZMUL(a,b);
	return VZMUL(VCI,m);
}
/* FIXME: there's probably a better way */
static inline V VZMULIJ(V a, V b) {
	V m = VZMULJ(a,b);
	return VZMUL(VCI,m);
}
#endif

static inline V LDA(const R *x, INT ivs, const R *aligned_like) {
  (void)aligned_like; /* UNUSED */
  (void)ivs; /* UNUSED */
  return TYPE(svld1)(MASKA,x);
}
static inline void STA(R *x, V v, INT ovs, const R *aligned_like) {
  (void)aligned_like; /* UNUSED */
  (void)ovs; /* UNUSED */
  TYPE(svst1)(MASKA,x,v);
}

#if FFTW_SINGLE

static inline V LDu(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  svuint32_t  gvvl = svindex_u32(0, 1);
  gvvl = svmul_n_u32_x(svptrue_b32(), gvvl, sizeof(R)*ivs);
  gvvl = svzip1_u32(gvvl, gvvl);
  gvvl = svadd_u32_x(svptrue_b32(), gvvl, svdupq_n_u32(0,sizeof(R),0,sizeof(R)));
  
  return svld1_gather_u32offset_f32(MASKA, x, gvvl);
}

static inline void STu(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  if (ovs==0) { // FIXME: hack for extra_iter hack support
    v = svreinterpret_f32_f64(svdup_lane_f64(svreinterpret_f64_f32(v),0));
  }
  svuint32_t  gvvl = svindex_u32(0, 1);
  gvvl = svmul_n_u32_x(svptrue_b32(), gvvl, sizeof(R)*ovs);
  gvvl = svzip1_u32(gvvl, gvvl);
  gvvl = svadd_u32_x(svptrue_b32(), gvvl, svdupq_n_u32(0,sizeof(R),0,sizeof(R)));

  svst1_scatter_u32offset_f32(MASKA, x, gvvl, v);
}

#else /* !FFTW_SINGLE */

static inline V LDu(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)aligned_like; /* UNUSED */
  svuint64_t  gvvl = svindex_u64(0, 1);
  gvvl = svmul_n_u64_x(svptrue_b64(), gvvl, sizeof(R)*ivs);
  gvvl = svzip1_u64(gvvl, gvvl);
  gvvl = svadd_u64_x(svptrue_b64(), gvvl, svdupq_n_u64(0,sizeof(R)));

  return svld1_gather_u64offset_f64(MASKA, x, gvvl);
}

static inline void STu(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  if (ovs==0) { // FIXME: hack for extra_iter hack support
    v = svdupq_lane_f64(v,0);
  }
  svuint64_t  gvvl = svindex_u64(0, 1);
  gvvl = svmul_n_u64_x(svptrue_b64(), gvvl, sizeof(R)*ovs);
  gvvl = svzip1_u64(gvvl, gvvl);
  gvvl = svadd_u64_x(svptrue_b64(), gvvl, svdupq_n_u64(0,sizeof(R)));

  svst1_scatter_u64offset_f64(MASKA, x, gvvl, v);
}

#endif /* FFTW_SINGLE */

#define LD LDu
#define ST STu

#ifdef FFTW_SINGLE
#define STM2(x, v, ovs, a) ST(x, v, ovs, a)
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)aligned_like; /* UNUSED */
  svuint32_t  gvvl = svindex_u32(0, 1);
  gvvl = svmul_n_u32_x(svptrue_b32(), gvvl, sizeof(R)*ovs);

  svst1_scatter_u32offset_f32(MASKA, x, gvvl, v);
}
#define STN4(x, v0, v1, v2, v3, ovs)  /* no-op */
#else /* !FFTW_SINGLE */
#define STM2(x, v, ovs, a) ST(x, v, ovs, a)
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)aligned_like; /* UNUSED */
  svuint64_t  gvvl = svindex_u64(0, 1);
  gvvl = svmul_n_u64_x(svptrue_b64(), gvvl, sizeof(R)*ovs);

  svst1_scatter_u64offset_f64(MASKA, x, gvvl, v);
}
#define STN4(x, v0, v1, v2, v3, ovs)  /* no-op */
#endif /* FFTW_SINGLE */

/* twiddle storage #1: compact, slower */
#define REQ_VTW1
#define VTW_SIZE VL
#include "vtw.h"
#define TWVL1 (VL)
#undef VTW_SIZE
#undef REQ_VTW1

static inline V BYTW1(const R *t, V sr)
{
     return VZMUL(LDA(t, 2, t), sr);
}

static inline V BYTWJ1(const R *t, V sr)
{
     return VZMULJ(LDA(t, 2, t), sr);
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define REQ_VTW2
#define VTW_SIZE (2*VL)
#include "vtw.h"
#define TWVL2 (2*VL)
#undef VTW_SIZE
#undef REQ_VTW2

static inline V BYTW2(const R *t, V sr)
{
     V si = FLIP_RI(sr);
     V ti = LDA(t + 2*VL, 2, t + 4*VL);
     V tr = LDA(t, 2, t);
     return VFMA(tr, sr, VMUL(ti, si));
}

static inline V BYTWJ2(const R *t, V sr)
{
     V si = FLIP_RI(sr);
     V ti = LDA(t + 2*VL, 2, t + 4*VL);
     V tr = LDA(t, 2, t);
     return VFNMS(ti, si, VMUL(tr, sr));
}

/* twiddle storage #3 */
#define VTW3(v,x) VTW1(v,x)
#define TWVL3 TWVL1

/* twiddle storage for split arrays */
#define REQ_VTWS
#define VTW_SIZE (2*VL)
#include "vtw.h"
#define TWVLS (2*VL)
#undef VTW_SIZE
#undef REQ_VTWS

#define VLEAVE() /* nothing */

#include "simd-common.h"
