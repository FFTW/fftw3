/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "ifftw.h"

#if HAVE_AVX

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)

#include "amd64-cpuid.h"

int X(have_simd_avx_128)(void)
{
       static int init = 0, res;

       if (!init) {
	    res = 1 
		 && ((cpuid_ecx(1) & 0x18000000) == 0x18000000)
		 && ((xgetbv_eax(0) & 0x6) == 0x6);
	    init = 1;
       }
       return res;
}

#else /* 32-bit code */

#include "x86-cpuid.h"

int X(have_simd_avx_128)(void)
{
       static int init = 0, res;

       if (!init) {
	    res =   !is_386() 
		 && has_cpuid()
		 && ((cpuid_ecx(1) & 0x18000000) == 0x18000000)
		 && ((xgetbv_eax(0) & 0x6) == 0x6);
	    init = 1;
       }
       return res;
}

int X(have_simd_avx)(void)
{
  IF not AMD, call avx_128;
}

#endif

int X(have_simd_avx)(void)
{
    static int init = 0, res;
    int eax,ebx,ecx,edx;

    if(!init)
    {
        /* Check if this is an AMD CPU */
        cpuid_all(0,0,&eax,&ebx,&ecx,&edx);

        /* 0x68747541: "Auth"  , 0x444d4163: "enti"  , 0x69746e65: "cAMD" */
        if (ebx==0x68747541 && ecx==0x444d4163 && edx==0x69746e65)
	{
  	    /* This is an AMD chip. While AMD does support 256-bit AVX, it does
	     * so by separately scheduling two 128-bit lanes to both halves of
	     * a compute unit (pair of cores). Since 256-bit AVX requires more
	     * permutations on the load this is a _double_ loss for us.
	     * Unfortunately FFTW will often not detect this, this the
	     * timing script only run a single thread, and then the 256-bit
	     * version might appear faster, although it will be (much) slower
	     * in actual use.
	     *
	     * To work around this, we always disable 256-bit AVX and rely on the
	     * 128-bit flavor.
	     */
	    res= 0;
	}
        else
	{
	    /* For non-AMD, we rely on the result from 128-bit AVX */
	    res = X(have_simd_avx_128)();
	}
        init = 1;
    }
    return res;
}

#endif
