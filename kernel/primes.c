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

/* $Id: primes.c,v 1.9 2003-03-15 20:29:43 stevenj Exp $ */

#include "ifftw.h"

/***************************************************************************/

/* Rader's algorithm requires lots of modular arithmetic, and if we
   aren't careful we can have errors due to integer overflows. */

#ifdef SAFE_MULMOD

#  include <limits.h>

/* compute (x * y) mod p, but watch out for integer overflows; we must
   have x, y >= 0, p > 0.  This routine is slow. */
int X(safe_mulmod)(int x, int y, int p)
{
     if (y == 0 || x <= INT_MAX / y)
	  return((x * y) % p);
     else {
	  int y2 = y/2;
	  return((fftw_safe_mulmod(x, y2, p) +
		  fftw_safe_mulmod(x, y - y2, p)) % p);
     }
}
#endif /* safe_mulmod ('long long' unavailable) */

/***************************************************************************/

/* Compute n^m mod p, where m >= 0 and p > 0.  If we really cared, we
   could make this tail-recursive. */
int X(power_mod)(int n, int m, int p)
{
     A(p > 0);
     if (m == 0)
	  return 1;
     else if (m % 2 == 0) {
	  int x = X(power_mod)(n, m / 2, p);
	  return MULMOD(x, x, p);
     }
     else
	  return MULMOD(n, X(power_mod)(n, m - 1, p), p);
}

/* the following two routines were contributed by Greg Dionne. */
static int get_prime_factors(int n, int *primef)
{
     int i;
     int size = 0;

     primef[size++] = 2;
     do
	  n >>= 1;
     while ((n & 1) == 0);

     if (n == 1)
	  return size;

     for (i = 3; i * i <= n; i += 2)
	  if (!(n % i)) {
	       primef[size++] = i;
	       do
		    n /= i;
	       while (!(n % i));
	  }
     if (n == 1)
	  return size;
     primef[size++] = n;
     return size;
}

int X(find_generator)(int p)
{
    int n, i, size;
    int primef[16];     /* smallest number = 32589158477190044730 > 2^64 */
    int pm1 = p - 1;

    if (p == 2)
	 return 1;

    size = get_prime_factors(pm1, primef);
    n = 2;
    for (i = 0; i < size; i++)
        if (X(power_mod)(n, pm1 / primef[i], p) == 1) {
            i = -1;
            n++;
        }
    return n;
}

/* Return first prime divisor of n  (It would be at best slightly faster to
   search a static table of primes; there are 6542 primes < 2^16.)  */
int X(first_divisor)(int n)
{
     int i;
     if (n <= 1)
	  return n;
     if (n % 2 == 0)
	  return 2;
     for (i = 3; i*i <= n; i += 2)
	  if (n % i == 0)
	       return i;
     return n;
}

int X(is_prime)(int n)
{
     return(n > 1 && X(first_divisor)(n) == n);
}

int X(next_prime)(int n)
{
     while (!X(is_prime)(n)) ++n;
     return n;
}
