/*
 * Copyright (c) 2007 Massachusetts Institute of Technology
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

#include "ifftw.h"

#if HAVE_CELL

#include "simd.h"
#include "fftw-cell.h"

int X(cell_transpose_applicable)(R *I, const iodim *d, INT vl)
{
     return (1
	     && X(cell_nspe)() > 0
	     && ALIGNEDA(I)
	     && (d->n % VL) == 0
	     && FITS_IN_INT(d->n)
	     && FITS_IN_INT(d->is * sizeof(R))
	     && FITS_IN_INT(d->os * sizeof(R))
	     && vl == 2
	     && ((d->is == vl && SIMD_STRIDE_OKA(d->os))
		 ||
		 (d->os == vl && SIMD_STRIDE_OKA(d->is)))
	  );
}

void X(cell_transpose)(R *I, INT n, INT s0, INT s1, INT vl)
{
     int i, nspe = X(cell_nspe)();

     for (i = 0; i < nspe; ++i) {
	  struct spu_context *ctx = X(cell_get_ctx)(i);
	  struct transpose_context *t = &ctx->u.transpose;

	  ctx->op = FFTW_SPE_TRANSPOSE;

	  if (s0 == vl) {
	       t->s0_bytes = s0 * sizeof(R);
	       t->s1_bytes = s1 * sizeof(R);
	  } else {
	       t->s0_bytes = s1 * sizeof(R);
	       t->s1_bytes = s0 * sizeof(R);
	  }
	  t->n = n;
	  t->A = (uintptr_t)I;
	  t->nspe = nspe;
	  t->my_id = i;
     }

     /* activate spe's */
     X(cell_spe_awake_all)();

     /* wait for completion */
     X(cell_spe_wait_all)();
}

#endif /* HAVE_CELL */
