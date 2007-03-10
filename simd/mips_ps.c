/*
 * Copyright (c) 2007 Codesourcery
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

#include "ifftw.h"
#include "simd.h"

#if HAVE_MIPS_PS

#define PS_SUPPORTED   (1<<18)
#define S_SUPPORTED    (1<<16)

/* In theory, MIPS processors supporting paired-single instructions
   should set the PS_SUPPORTED bit.  However, the Broadcom 1480 SB-1
   does not do this. 

   Instead, check that at least single-precision is supported.
*/
#define MASK           S_SUPPORTED

int RIGHT_CPU()
{
     unsigned long fir;  /* Floating point implementation reg */

     __asm__("cfc1 %0,$0" : "=r"(fir) );

     return (fir & MASK);
}

#endif
