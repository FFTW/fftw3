/*
 * Copyright (c) 2003, 2007-11 Matteo Frigo
 * Copyright (c) 2003, 2007-11 Massachusetts Institute of Technology
 * Copyright (c) 2012-2013 Romain Dolbeau
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

#include "ifftw.h"

#if HAVE_AVX512

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)

#include "amd64-cpuid.h"

int X(have_simd_avx512)(void)
{
    static int init = 0, res;

    if (!init)
    {
        if((xgetbv_eax(0) & 0x6) == 0x6)
        {
            int eax,ebc,ecx,edx;
            cpuid_all(7,0,&eax,&ebx,&ecx,&edx);
            /* Bit 16 of ebx for CPUID level 7 is AVX-512 foundation layer support (ErikL) */
            res = ( (ebx & (1 << 16))  != 0 );
        }
        init = 1;
    }
    return res;
}

#else /* 32-bit code */

#error "Avx512 is 64 bits only"

#endif

#endif
