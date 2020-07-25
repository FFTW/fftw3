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


#include "kernel/ifftw.h"

#if HAVE_SVE
#if defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
#endif

  static int sve_getwidth(void) {
#if defined(__GNUC__) && !defined(__ARM_FEATURE_SVE)
#warning "SVE not supported"
    return -1;
#else
    return svcntb()*8;
#endif
  }

  int X(have_simd_sve)(int minwidth)
  {
       static int init = 0, res;

       if (!init) {
	    init = sve_getwidth();
       }
       return ((init > 0) ? (minwidth <= init ? 1 : 0) : 0);
  }


#endif
