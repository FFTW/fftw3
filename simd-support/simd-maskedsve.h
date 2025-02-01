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

/* The goal is to limit to the required width by using masking.
 * However, some SVE instructions are limited to two-addresses
 * rather than three adresses when masked.
 * (i.e. they do X op= Y, not X = Z op X)
 * Loads will put zero in masked-out value.
 * For performance reason, we want to use non-masked for the instructions
 * with a two-addresses masked form: add & sub.
 * But ACLE doesn't have the non-masked form...
 * clang 11 & armclang 20.2 used masked form in assembly and lots of copies
 * gcc 10 uses the non-masked form (!) and no copies
 */
//#define USE_UNMASKED_ASSEMBLY
/* Define below to use masking instead of branching in STu
 */
//#define BRANCHLESS_STU

/* do we need to mask VLIT somehow ?*/
#define VLIT(re, im) DS(svdupq_n_f64(re,im),svdupq_n_f32(re,im,re,im))
#define VLIT1(val) TYPESUF(svdup_n,_z)(MASKA,val)
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
#define FLIP_RI(x) TYPE(svtrn1)(VDUPH(x),x)
#else
#define FLIP_RI(x) TYPE(svtrn1)(VDUPH(x),x)
#endif

/* there might be a better way */
#define VCONJ(x) TYPESUF(svmul,_x)(MASKA,x,VCONEMI)
#define VBYI(x)  TYPESUF(svcadd,_x)(MASKA,VZERO,x,90)

#define VNEG(a)   TYPESUF(svneg,_x)(MASKA,a)
#if !defined(USE_UNMASKED_ASSEMBLY)
#define VADD(a,b) TYPESUF(svadd,_x)(MASKA,a,b)
#define VSUB(a,b) TYPESUF(svsub,_x)(MASKA,a,b)
#define VMUL(a,b) TYPESUF(svmul,_x)(MASKA,a,b)
#else
static inline V VADD(const V a, const V b) {
	V r;
#ifdef FFTW_SINGLE
	asm("fadd %[r].s, %[a].s, %[b].s\n" : [r]"=w"(r) : [a]"w"(a), [b]"w"(b));
#else
	asm("fadd %[r].d, %[a].d, %[b].d\n" : [r]"=w"(r) : [a]"w"(a), [b]"w"(b));
#endif
	return r;
}
static inline V VSUB(const V a, const V b) {
	V r;
#ifdef FFTW_SINGLE
	asm("fsub %[r].s, %[a].s, %[b].s\n" : [r]"=w"(r) : [a]"w"(a), [b]"w"(b));
#else
	asm("fsub %[r].d, %[a].d, %[b].d\n" : [r]"=w"(r) : [a]"w"(a), [b]"w"(b));
#endif
	return r;
}
static inline V VMUL(const V a, const V b) {
	V r;
#ifdef FFTW_SINGLE
	asm("fmul %[r].s, %[a].s, %[b].s\n" : [r]"=w"(r) : [a]"w"(a), [b]"w"(b));
#else
	asm("fmul %[r].d, %[a].d, %[b].d\n" : [r]"=w"(r) : [a]"w"(a), [b]"w"(b));
#endif
	return r;
}
#endif
#define VFMA(a, b, c)  TYPESUF(svmad,_x)(MASKA,b,a,c)
#define VFMS(a, b, c)  TYPESUF(svnmsb,_x)(MASKA,b,a,c)
#define VFNMS(a, b, c) TYPESUF(svmsb,_x)(MASKA,b,a,c)
#define VFMAI(b, c)    TYPESUF(svcadd,_x)(MASKA,c,b,90)
#define VFNMSI(b, c)   TYPESUF(svcadd,_x)(MASKA,c,b,270)

static inline V VFMACONJ(V b, V c) {
	V m = TYPESUF(svcmla,_x)(MASKA,c,b,VRONE,0);
	return TYPESUF(svcmla,_x)(MASKA,m,b,VRONE,270);
}
#define VFMSCONJ(b,c)  VFMACONJ(b,VNEG(c))
#define VFNMSCONJ(b,c) VNEG(VFMSCONJ(b,c))

static inline V VZMUL(V a, V b) {
	V m = TYPESUF(svcmla,_x)(MASKA,VZERO,a,b,0);
	return TYPESUF(svcmla,_x)(MASKA,m,a,b,90);
}
static inline V VZMULJ(V a, V b) {
        V m = TYPESUF(svcmla,_x)(MASKA,VZERO,a,b,0);
        return TYPESUF(svcmla,_x)(MASKA,m,a,b,270);
}
/* there might be a better way */
static inline V VZMULI(V a, V b) {
	V m = VZMUL(a,b);
	return VFMAI(m, VZERO);
}
/* there might be a better way */
static inline V VZMULIJ(V a, V b) {
	V m = VZMULJ(a,b);
	return VFMAI(m, VZERO);
}

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
  svint64_t gvvl = svindex_s64(0, ivs/2);

  return svreinterpret_f32_f64(svld1_gather_s64index_f64(MASKA, (const double *)x, gvvl));
}

static inline void STu(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  const svint64_t gvvl = svindex_s64(0, ovs/2);
#if !defined(BRANCHLESS_STU)
  if (ovs==0) { // FIXME: hack for extra_iter hack support
    v = svreinterpret_f32_f64(svdup_lane_f64(svreinterpret_f64_f32(v),0));
  }
  svst1_scatter_s64index_f64(MASKA, (double *)x, gvvl, svreinterpret_f64_f32(v));
#else
  /* no-branch implementation of extra_iter hack support
   * if ovs is non-zero, keep the original MASKA;
   * if not, only store one 64 bits element (two 32 bits consecutive)
   */
  const svbool_t which = svdupq_n_b64(ovs != 0, ovs != 0);
  const svbool_t mask = svsel_b(which, MASKA, svptrue_pat_b64(SV_VL1));
  svst1_scatter_s64index_f64(mask, (double *)x, gvvl, svreinterpret_f64_f32(v));
#endif
}

#else /* !FFTW_SINGLE */

static inline V LDu(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)aligned_like; /* UNUSED */
  svint64_t  gvvl = svindex_s64(0, ivs);
  gvvl = svzip1_s64(gvvl, svadd_n_s64_x(MASKA, gvvl, 1));

  return svld1_gather_s64index_f64(MASKA, x, gvvl);
}

static inline void STu(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  svint64_t gvvl = svindex_s64(0, ovs);
  gvvl = svzip1_s64(gvvl, svadd_n_s64_x(MASKA, gvvl, 1));
#if !defined(BRANCHLESS_STU)
  if (ovs==0) { // FIXME: hack for extra_iter hack support
    v = svdupq_lane_f64(v,0);
  }
  svst1_scatter_s64index_f64(MASKA, x, gvvl, v);
#else
  /* no-branch implementation of extra_iter hack support
   * if ovs is non-zero, keep the original MASKA;
   * if not, only store two 64 bits elements
   */
  const svbool_t which = svdupq_n_b64(ovs != 0, ovs != 0);
  const svbool_t mask = svsel_b(which, MASKA, svptrue_pat_b64(SV_VL2));

  svst1_scatter_s64index_f64(mask, x, gvvl, v);
#endif
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
  svint32_t  gvvl = svindex_s32(0, ovs);

  svst1_scatter_s32index_f32(MASKA, x, gvvl, v);
}
#define STN4(x, v0, v1, v2, v3, ovs)  /* no-op */
#else /* !FFTW_SINGLE */
#define STM2(x, v, ovs, a) ST(x, v, ovs, a)
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)aligned_like; /* UNUSED */
  svint64_t  gvvl = svindex_s64(0, ovs);

  svst1_scatter_s64index_f64(MASKA, x, gvvl, v);
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
