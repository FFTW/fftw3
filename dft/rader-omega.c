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

#include "dft.h"

static rader_tl *omegas = 0;

R *X(dft_rader_mkomega)(plan *p_, int n, int ginv)
{
     plan_dft *p = (plan_dft *) p_;
     R *omega;
     int i, gpower;
     trigreal scale;

     if ((omega = X(rader_tl_find)(n, n, ginv, omegas)))
	  return omega;

     omega = (R *)MALLOC(sizeof(R) * (n - 1) * 2, TWIDDLES);

     scale = n - 1.0; /* normalization for convolution */

     for (i = 0, gpower = 1; i < n-1; ++i, gpower = MULMOD(gpower, ginv, n)) {
	  omega[2*i] = X(cos2pi)(gpower, n) / scale;
	  omega[2*i+1] = FFT_SIGN * X(sin2pi)(gpower, n) / scale;
     }
     A(gpower == 1);

     AWAKE(p_, 1);
     p->apply(p_, omega, omega + 1, omega, omega + 1);
     AWAKE(p_, 0);

     X(rader_tl_insert)(n, n, ginv, omega, &omegas);
     return omega;
}

void X(dft_rader_free_omega)(R **omega)
{
     X(rader_tl_delete)(*omega, &omegas);
     *omega = 0;
}
