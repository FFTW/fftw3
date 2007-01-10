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
