/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

#ifndef FFTW_SINGLE
#error "ALTIVEC only works in single precision"
#endif

/* define these unconditionally, because they are used by
   taint.c which is compiled without altivec */
#define VL 2            /* SIMD complex vector length */
#define ALIGNMENT 8     /* alignment for LD/ST */
#define ALIGNMENTA 16   /* alignment for LDA/STA */
#define SIMD_VSTRIDE_OKA(x) ((x) == 2)
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#define RIGHT_CPU X(have_altivec)
extern int RIGHT_CPU(void);

#if defined(__VEC__) || defined(FAKE__VEC__)

#include <altivec.h>

typedef vector float V;
#define VLIT(x0, x1, x2, x3) {x0, x1, x2, x3}
#define LDK(x) x
#define DVK(var, val) const V var = VLIT(val, val, val, val)

static inline V VADD(V a, V b) { return vec_add(a, b); }
static inline V VSUB(V a, V b) { return vec_sub(a, b); }
static inline V VFMA(V a, V b, V c) { return vec_madd(a, b, c); }
static inline V VFNMS(V a, V b, V c) { return vec_nmsub(a, b, c); }

static inline V VMUL(V a, V b)
{
     DVK(zero, -0.0);
     return VFMA(a, b, zero);
}

static inline V VFMS(V a, V b, V c) { return VSUB(VMUL(a, b), c); }

static inline V LDA(const R *x, int ivs, const R *aligned_like) 
{
     UNUSED(ivs);
     UNUSED(aligned_like);
     return vec_ld(0, (R *)x);
}

static inline V LD(const R *x, int ivs, const R *aligned_like) 
{
     /* common subexpressions */
     int fivs = 4 * ivs;
       /* you are not expected to understand this: */
     const vector unsigned int perm = VLIT(0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
     vector unsigned char ml = vec_lvsr(fivs + 8, (R *)aligned_like);
     vector unsigned char mh = vec_lvsl(0, (R *)aligned_like);
     vector unsigned char msk = 
	  (vector unsigned char)vec_sel((V)mh, (V)ml, perm);
     /* end of common subexpressions */

     return vec_perm(vec_ld(0, (R *)x), vec_ld(fivs, (R *)x), msk);
}

/* store lower half */
static inline void STH(R *x, V v, const R *aligned_like)
{
     v = vec_perm(v, v, vec_lvsr(0, (R *)aligned_like));
     vec_ste(v, 0, x);
     vec_ste(v, 4, x);
}

static inline void STL(R *x, V v, int ovs, const R *aligned_like)
{
     int fovs = 4 * ovs;
     v = vec_perm(v, v, vec_lvsr(fovs + 8, (R *)aligned_like));
     vec_ste(v, fovs, x);
     vec_ste(v, 4 + fovs, x);
}

static inline void STA(R *x, V v, int ovs, const R *aligned_like) 
{
     vec_st(v, 0, x);
}

static inline void ST(R *x, V v, int ovs, const R *aligned_like) 
{
     STH(x, v, aligned_like);
     STL(x, v, ovs, aligned_like);
}

#define STPAIR1(x, v, ovs, aligned_like) /* no-op */

static inline void STPAIR2(R *x, V v0, V v1, int ovs)
{
     const vector unsigned int even = 
	  VLIT(0x00010203, 0x04050607, 0x10111213, 0x14151617);
     const vector unsigned int odd = 
	  VLIT(0x08090a0b, 0x0c0d0e0f, 0x18191a1b, 0x1c1d1e1f);
     STA(x, vec_perm(v0, v1, (vector unsigned char)even), ovs, 0);
     STA(x + ovs, vec_perm(v0, v1, (vector unsigned char)odd), ovs, 0);
}

static inline V FLIP_RI(V x)
{
     const vector unsigned int perm = 
	  VLIT(0x04050607, 0x00010203, 0x0c0d0e0f, 0x08090a0b);
     return vec_perm(x, x, (vector unsigned char)perm);
}

static inline V CHS_R(V x)
{
     const V pmpm = VLIT(-0.0, 0.0, -0.0, 0.0);
     return vec_xor(x, pmpm);
}

static inline V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

static inline V VFMAI(V b, V c)
{
     const V pmpm = VLIT(-1.0, 1.0, -1.0, 1.0);
     return VFMA(FLIP_RI(b), pmpm, c);
}

static inline V VFNMSI(V b, V c)
{
     const V pmpm = VLIT(-1.0, 1.0, -1.0, 1.0);
     return VFNMS(FLIP_RI(b), pmpm, c);
}

/* twiddle storage #1: compact, slower */
#define VTW1(x) {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_SIN, 0, x}, {TW_SIN, 1, x}
#define TWVL1 (VL)

static inline V BYTW1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = VBYI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFMA(ti, si, VMUL(tr, sr));
}

static inline V BYTWJ1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = VBYI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFNMS(ti, si, VMUL(tr, sr));
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define VTW2(x)								\
  {TW_COS, 0, x}, {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_COS, 1, x},	\
  {TW_SIN, 0, -x}, {TW_SIN, 0, x}, {TW_SIN, 1, -x}, {TW_SIN, 1, x}
#define TWVL2 (2 * VL)

static __inline__ V BYTW2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFMA(ti, si, VMUL(tr, sr));
}

static __inline__ V BYTWJ2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFNMS(ti, si, VMUL(tr, sr));
}

#define BEGIN_SIMD()
#define END_SIMD()

#endif /* #ifdef __VEC__ */
