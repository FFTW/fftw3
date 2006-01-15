/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Massachusetts Institute of Technology
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

#include "bench.h"

#define	ENOUGH_DURATION_TEN(one)	one one one one one one one one one one

/* the Intel compiler optimizes away this routine if we put it in
   timer.c.  */
char ***bench_do_useless_work(int n, char ***p)
{
     while (n-- > 0) {
	  int k;
	  for (k = 0; k < 10; ++k) {
	       ENOUGH_DURATION_TEN(p = (char ***) *p;);
	  }
     }
     return (p);
}
