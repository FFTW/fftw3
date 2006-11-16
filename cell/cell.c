/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

#include "fftw-cell.h"
#include <libspe.h>
#include <stdlib.h> /* posix_memalign */

extern spe_program_handle_t spu_fftw;

void *X(cell_aligned_malloc)(size_t n)
{
     void *p;
     posix_memalign(&p, 128, n);
     return p;
}

static struct spu_context *ctx[SPU_NUM_THREADS];
static speid_t spe_id[SPU_NUM_THREADS];
static int inited = 0;
static int nspe;
int X(init_cell)(void)
{
     if (!inited) {
	  int i;

	  nspe = SPU_NUM_THREADS;
	  for(i = 0; i < nspe; ++i) {
	       ctx[i] = X(cell_aligned_malloc)(sizeof(*ctx[i]));

	       spe_id[i] = spe_create_thread(0, &spu_fftw, ctx[i],
					     NULL, -1, 0);
	  }

	  inited = 1;
     }
}

int X(cell_nspe)(void)
{
     return nspe;
}

struct spu_context *X(cell_get_ctx)(int spe)
{
     return ctx[spe];
}

void X(cell_spe_awake_all)(void)
{
     int i;
     for (i = 0; i < nspe; ++i)
	  spe_write_in_mbox(spe_id[i], 0);
}

void X(cell_spe_wait_all)(void)
{
     int i;
     for (i = 0; i < nspe; ++i) {
	  while (spe_stat_out_mbox(spe_id[i]) <= 0)
	       ; 
	  (void) spe_read_out_mbox(spe_id[i]);
     }
}

#endif /* HAVE_CELL */
