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

int X(cell_copy_applicable)(R *I, R *O, const iodim *n, const iodim *v)
{
     return (1
	     && X(cell_nspe)() > 0
	     && UNTAINT(I) != UNTAINT(O)
	     && ALIGNEDA(I)
	     && ALIGNEDA(O)
	     && (n->n % VL) == 0
	     && ((v->n % VL) == 0 || (n->is == 2 && n->os == 2))
	     && ((n->is == 2 && SIMD_STRIDE_OKA(v->is)) 
		 ||
		 (v->is == 2 && SIMD_STRIDE_OKA(n->is)))
	     && ((n->os == 2 && SIMD_STRIDE_OKA(v->os))
		 ||
		 (v->os == 2 && SIMD_STRIDE_OKA(n->os)))
	     && FITS_IN_INT(n->n)
	     && FITS_IN_INT(n->is * sizeof(R))
	     && FITS_IN_INT(n->os * sizeof(R))
	     && FITS_IN_INT(v->n)
	     && FITS_IN_INT(v->is * sizeof(R))
	     && FITS_IN_INT(v->os * sizeof(R))
	  );
}

void X(cell_copy)(R *I, R *O, const iodim *n, const iodim *v)
{
     int i, nspe = X(cell_nspe)();

     for (i = 0; i < nspe; ++i) {
	  struct spu_context *ctx = X(cell_get_ctx)(i);
	  struct copy_context *c = &ctx->u.copy;

	  ctx->op = FFTW_SPE_COPY;

	  c->I = (uintptr_t)I;
	  c->O = (uintptr_t)O;
	  c->n = n->n;
	  c->v = v->n;
	  c->is_bytes = n->is * sizeof(R);
	  c->os_bytes = n->os * sizeof(R);
	  c->ivs_bytes = v->is * sizeof(R);
	  c->ovs_bytes = v->os * sizeof(R);
	  c->nspe = nspe;
	  c->my_id = i;
     }

     /* activate spe's */
     X(cell_spe_awake_all)();

     /* wait for completion */
     X(cell_spe_wait_all)();
}

#endif /* HAVE_CELL */
