/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

/* $Id: primes.c,v 1.3 2002-09-14 01:54:50 athena Exp $ */

#include "ifftw.h"

/***************************************************************************/

/* Rader's algorithm requires lots of modular arithmetic, and if we
   aren't careful we can have errors due to integer overflows. */

#ifdef SAFE_MULMOD

#  include <limits.h>

/* compute (x * y) mod p, but watch out for integer overflows; we must
   have x, y >= 0, p > 0.  This routine is slow. */
uint X(safe_mulmod)(uint x, uint y, uint p)
{
     if (y == 0 || x <= UINT_MAX / y)
	  return((x * y) % p);
     else {
	  uint y2 = y/2;
	  return((fftw_safe_mulmod(x, y2, p) +
		  fftw_safe_mulmod(x, y - y2, p)) % p);
     }
}
#endif /* safe_mulmod ('long long' unavailable) */

/***************************************************************************/

/* Compute n^m mod p, where m >= 0 and p > 0.  If we really cared, we
   could make this tail-recursive. */
uint X(power_mod)(uint n, uint m, uint p)
{
     A(p > 0);
     if (m == 0)
	  return 1;
     else if (m % 2 == 0) {
	  uint x = X(power_mod)(n, m / 2, p);
	  return MULMOD(x, x, p);
     }
     else
	  return MULMOD(n, X(power_mod)(n, m - 1, p), p);
}

/* Find the period of n in the multiplicative group mod p (p prime).
   That is, return the smallest m such that n^m == 1 mod p. */
static uint period(uint n, uint p)
{
     uint prod = n, period = 1;

     while (prod != 1) {
	  prod = MULMOD(prod, n, p);
	  ++period;
	  A(prod != 0);
     }
     return period;
}

/* Find a generator for the multiplicative group mod p, where p is
   prime.  The generators are dense enough that this takes O(p)
   time, not O(p^2) as you might naively expect.   (There are even
   faster ways to find a generator; c.f. Knuth.) */
uint X(find_generator)(uint p)
{
     uint g;

     for (g = 1; g < p; ++g)
	  if (period(g, p) == p - 1)
	       break;
     A(g != p);
     return g;
}

/* Return first prime divisor of n  (It would be only slightly faster to
   search a static table of primes; there are 6542 primes < 2^16.)  */
uint X(first_divisor)(uint n)
{
     uint i;
     if (n <= 1)
	  return n;
     if (n % 2 == 0)
	  return 2;
     for (i = 3; i*i <= n; i += 2)
	  if (n % i == 0)
	       return i;
     return n;
}

int X(is_prime)(uint n)
{
     return(n > 1 && X(first_divisor)(n) == n);
}

uint X(next_prime)(uint n)
{
     while (!X(is_prime)(n)) ++n;
     return n;
}
