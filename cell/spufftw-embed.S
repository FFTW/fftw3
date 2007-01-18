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

#include <config.h>
#if defined(FFTW_SINGLE)
#define XSPU_FFTW fftwf_spu_fftw
#else
#define XSPU_FFTW fftw_spu_fftw
#endif
	
	.section .data.spetoe,"aw",@progbits
	.p2align 7
unused:

	.section .rodata.speelf,"a",@progbits
	.p2align 7
fftw_spu_elf:
	.incbin "spu/spufftw"

	.section .data,"aw",@progbits
	.globl XSPU_FFTW
XSPU_FFTW:

#ifdef _LP64
	.int 24
	.int 0
	.quad fftw_spu_elf
	.quad unused
#else
	.int 12
	.int fftw_spu_elf
	.int unused
#endif
