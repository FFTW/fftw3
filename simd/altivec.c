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

/* $Id: altivec.c,v 1.3 2003-03-15 20:29:43 stevenj Exp $ */

#include "ifftw.h"
#include "simd.h"

#if HAVE_ALTIVEC

const vector unsigned int X(altivec_ld_selmsk) = 
   (vector unsigned int)VLIT(0, 0, 0xFFFFFFFF, 0xFFFFFFFF);

const vector unsigned int X(altivec_flipri_perm) = 
   (vector unsigned int)VLIT(0x04050607, 0x00010203,
			     0x0c0d0e0f, 0x08090a0b);

const vector float X(altivec_chsr_sgn) = 
   (vector float)VLIT(-1.0, 1.0, -1.0, 1.0);
const vector float X(altivec_chsr_msk) = 
   (vector float)VLIT(-0.0, 0.0, -0.0, 0.0);

#endif
