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

#ifdef HAVE_ALTIVEC_H
#include <altivec.h>
#endif

#ifdef __VEC__

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

extern const vector unsigned int X(altivec_ld_selmsk);

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
     vector unsigned char ml = vec_lvsr(fivs + 8, (R *)aligned_like);
     vector unsigned char mh = vec_lvsl(0, (R *)aligned_like);
     vector unsigned char msk =
	  (vector unsigned char)vec_sel((V)mh, (V)ml, X(altivec_ld_selmsk));
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

extern const vector unsigned int X(altivec_flipri_perm);
static inline V FLIP_RI(V x)
{
     return vec_perm(x, x, (const vector unsigned char)X(altivec_flipri_perm));
}

extern const vector float X(altivec_chsr_msk);
extern const vector float X(altivec_chsr_sgn);

static inline V CHS_R(V x)
{
     return vec_xor(x, X(altivec_chsr_msk));
}

static inline V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

static inline V VFMAI(V b, V c)
{
     return VFMA(FLIP_RI(b), X(altivec_chsr_sgn), c);
}

static inline V VFNMSI(V b, V c)
{
     return VFNMS(FLIP_RI(b), X(altivec_chsr_sgn), c);
}

#define VTW(x) {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_SIN, 0, x}, {TW_SIN, 1, x}
#define TWVL (VL)

static inline V BYTW(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = VBYI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFMA(ti, si, VMUL(tr, sr));
}

static inline V BYTWJ(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = VBYI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFNMS(ti, si, VMUL(tr, sr));
}

#define RIGHT_CPU X(have_altivec)
extern int RIGHT_CPU(void);

#define SIMD_VSTRIDE_OKA(x) ((x) == 2)
#define BEGIN_SIMD()
#define END_SIMD()

#endif /* #ifdef __VEC__ */
