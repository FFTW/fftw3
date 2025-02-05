/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
 *
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
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

#include "kernel/ifftw.h"

#if HAVE_LASX

#if defined(unix) || defined(linux)

#define LOONGARCH_CFG2 0x2
#define LOONGARCH_CFG2_LASX    (1 << 7)

int X(have_simd_lasx)(void)
{
    static int init = 0, res;
    int cfg2 = 0;

    if (!init){
        res  = 0;
        init = 1;

        /* Use cpucfg to get cpu features */
        __asm__ volatile(
            "cpucfg %0, %1 \n\t"
            : "+&r"(cfg2)
            : "r"(LOONGARCH_CFG2)
        );

        /* Check if support LASX */
        if (cfg2 & LOONGARCH_CFG2_LASX)
            res = 1;
    }
    return res;
}

#else
/* don't know how to autodetect LSX; assume it is present */
  int X(have_simd_lasx)(void)
  {
       return 1;
  }
#endif /* unix / linux */

#endif /* HAVE_LASX */
