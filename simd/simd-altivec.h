/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

#define VL 2            /* SIMD complex vector length */
#define ALIGNMENT 16

typedef vector float V;
#define VADD(a, b) vec_add(a, b)
#define VSUB(a, b) vec_sub(a, b)
#define VFMA(a, b, c) vec_madd(a, b, c)
#define VFNMS(a, b, c) vec_nmsub(a, b, c) 
#define DVK(var, val) V var = (vector float)(val, val, val, val)
#define LDK(x) x

static inline V VMUL(V a, V b)
{
     DVK(zero, 0.0);
     return VFMA(a, b, zero);
}

static inline V VFMS(V a, V b, V c)
{
     return VSUB(VMUL(a, b), c);
}

/* load lower half */
static inline V LDL(const float *x) 
{
     V v = vec_ld(0, (float *)x);
     return vec_perm(v, v, vec_lvsl(0, (float *)x));
}

static inline V LDH(const float *x) 
{
     V v = vec_ld(0, (float *)x);
     return vec_perm(v, v, vec_lvsl(8, (float *)x));
}

static inline V LD(const float *x, int ivs) 
{
     const vector unsigned int msk = (vector unsigned int)(0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
     V l = LDL(x);
     V h = LDH(x + ivs);
     return vec_sel(l, h, msk);
}

/* store lower half */
static inline void STL(float *x, V v)
{
     v = vec_perm(v, v, vec_lvsr(0, x));
     vec_ste(v, 0, x);
     vec_ste(v, 4, x);
}

static inline void STH(float *x, V v)
{
     v = vec_perm(v, v, vec_lvsr(8, x));
     vec_ste(v, 0, x);
     vec_ste(v, 4, x);
}

static inline void ST(float *x, V v, int ovs) 
{						
     STL(x, v);
     STH(x + ovs, v);
}

static inline V FLIP_RI(V x)
{
     const vector unsigned char perm = 
	  (vector unsigned char)(4, 5, 6, 7, 0, 1, 2, 3, 
				 12, 13, 14, 15, 8, 9, 10, 11);
     return vec_perm(x, x, perm);
}

static inline V CHS_R(V x)
{
     const vector float msk = (vector float)(-1.0, 1.0, -1.0, 1.0);
     return VMUL(x, msk);
}

static inline V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

static inline V VFMAI(V b, V c)
{
     const vector float msk = (vector float)(-1.0, 1.0, -1.0, 1.0);
     return VFMA(FLIP_RI(b), msk, c);
}

static inline V VFNMSI(V b, V c)
{
     const vector float msk = (vector float)(-1.0, 1.0, -1.0, 1.0);
     return VFNMS(FLIP_RI(b), msk, c);
}

#define FAST_AND_BLOATED_TWIDDLES

#ifdef FAST_AND_BLOATED_TWIDDLES

/* avoid twiddle shuffling by storing twiddles twice in the
   proper format */

#define VTW(x) 								\
  {TW_COS, 0, x}, {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_COS, 1, x},	\
  {TW_SIN, 0, -x}, {TW_SIN, 0, x}, {TW_SIN, 1, -x}, {TW_SIN, 1, x}
#define TWVL (2 * VL)

static inline V BYTW(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFMA(ti, si, VMUL(tr, sr));
}

static inline V BYTWJ(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFNMS(ti, si, VMUL(tr, sr));
}

#else /* FAST_AND_BLOATED_TWIDDLES */

#define VTW(x) 								\
  {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_SIN, 0, x}, {TW_SIN, 1, x}
#define TWVL (VL)

static inline V BYTW(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFMA(CHS_R(ti), si, VMUL(tr, sr));
}

static inline V BYTWJ(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFNMS(CHS_R(ti), si, VMUL(tr, sr));
}
#endif

#define RIGHT_CPU() 1

