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

#include "api.h"
#include "rdft.h"

rdft_kind *X(map_r2r_kind)(uint rank, const X(r2r_kind) *kind)
{
     uint i;
     rdft_kind *k;

     A(FINITE_RNK(rank));
     k = (rdft_kind *) fftw_malloc(rank * sizeof(rdft_kind), PROBLEMS);
     for (i = 0; i < rank; ++i) 
	  switch (kind[i]) {
	      case FFTW_R2HC: k[i] = R2HC; break;
	      case FFTW_HC2R: k[i] = HC2R; break;
	      case FFTW_DHT: k[i] = DHT; break;
	      case FFTW_REDFT00: k[i] = REDFT00; break;
	      case FFTW_REDFT01: k[i] = REDFT01; break;
	      case FFTW_REDFT10: k[i] = REDFT10; break;
	      case FFTW_REDFT11: k[i] = REDFT11; break;
	      case FFTW_RODFT00: k[i] = RODFT00; break;
	      case FFTW_RODFT01: k[i] = RODFT01; break;
	      case FFTW_RODFT10: k[i] = RODFT10; break;
	      case FFTW_RODFT11: k[i] = RODFT11; break;
	      default: A(0);
	  }
     return k;
}
