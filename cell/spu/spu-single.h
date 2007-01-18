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

typedef vector float V;

#define VLIT(x0, x1, x2, x3) {x0, x1, x2, x3}
#define LDK(x) x
#define DVK(var, val) const V var = VLIT(val, val, val, val)
#define TWVL 2
#define VL 2

static inline V VADD(V a, V b)
{
     return spu_add(a, b);
}

static inline V VSUB(V a, V b)
{
     return spu_sub(a, b);
}

static inline V VFMA(V a, V b, V c)
{
     return spu_madd(a, b, c);
}

static inline V VFNMS(V a, V b, V c)
{
     return spu_nmsub(a, b, c);
}

static inline V VMUL(V a, V b)
{
     return spu_mul(a, b);
}

static inline V VFMS(V a, V b, V c)
{
     return VSUB(VMUL(a, b), c);
}

static inline void ST(R *x, V v, INT ovs, R *aligned_like)
{
     UNUSED(ovs);
     UNUSED(aligned_like);
     *((V *) x) = v;
}

static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
     UNUSED(ivs);
     UNUSED(aligned_like);
     return (*((V *) x));
}

static inline V FLIP_RI(V x)
{
     const vector unsigned char c =
	  { 0x04, 0x05, 0x06, 0x07,
	    0x00, 0x01, 0x02, 0x03,
	    0x0C, 0x0D, 0x0E, 0x0F,
	    0x08, 0x09, 0x0A, 0x0B };
     return spu_shuffle(x, x, c);
}

static inline V CHS_R(V x)
{
     const V pmpm = VLIT(-1.0, 1.0, -1.0, 1.0);
     return spu_mul(x, pmpm);
}

static inline V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

#define STM2(x, v, ovs, aligned_like)

static inline void STN2(R *x, V v0, V v1, INT ovs)
{
     const vector unsigned char c1 =
	  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12,
	    0x13, 0x14, 0x15, 0x16, 0x17 };
     const vector unsigned char c2 =
	  { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x18, 0x19, 0x1A,
	    0x1B, 0x1C, 0x1D, 0x1E, 0x1F };

     *((V *) x) = spu_shuffle(v0, v1, c1);
     *((V *) (x + ovs)) = spu_shuffle(v0, v1, c2);
}

static inline V VFNMSI(V b, V c)
{
     const V pmpm = VLIT(-1.0, 1.0, -1.0, 1.0);
     return VFNMS(FLIP_RI(b), pmpm, c);
}


static inline V VFMAI(V b, V c)
{
     const V pmpm = VLIT(-1.0, 1.0, -1.0, 1.0);
     return VFMA(FLIP_RI(b), pmpm, c);
}

static inline V BYTWJ(const R *t, V sr)
{
     const vector unsigned char c1 =
	  { 0x00, 0x01, 0x02, 0x03,
	    0x10, 0x11, 0x12, 0x13,
	    0x08, 0x09, 0x0A, 0x0B,
	    0x18, 0x19, 0x1A, 0x1B };
     const vector unsigned char c2 =
	  { 0x04, 0x05, 0x06, 0x07,
	    0x14, 0x15, 0x16, 0x17,
	    0x0C, 0x0D, 0x0E, 0x0F,
	    0x1C, 0x1D, 0x1E, 0x1F };

     V tx = *(const V *) t;
     V si = VBYI(sr);
     V tr = spu_shuffle(tx, tx, c1);
     V ti = spu_shuffle(tx, tx, c2);
     return VFNMS(si, ti, VMUL(sr, tr));
}
