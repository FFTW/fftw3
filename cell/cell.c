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

#include "fftw-cell.h"
#include <libspe.h>
#include <stdlib.h> /* posix_memalign */

extern spe_program_handle_t X(spu_fftw);

void *X(cell_aligned_malloc)(size_t n)
{
     void *p;
     posix_memalign(&p, 128, n);
     return p;
}

static struct spu_context *ctx[MAX_NSPE];
static speid_t spe_id[MAX_NSPE];
static int refcnt = 0;
static int nspe = -1;

static void set_default_nspe(void)
{
     if (nspe < 0) {
	  /* set NSPE to the maximum of 8 and the number of physical
	     SPEs.  A two-processor Cell blade reports 16 SPEs, but we
	     only want to use one processor by default. */
	  int phys = spe_count_physical_spes();
	  if (phys > 8)
	       phys = 8;
	  X(cell_set_nspe)(phys);
     }
}

void X(cell_activate_spes)(void)
{
     if (refcnt++ == 0) {
	  int i;

	  set_default_nspe();
	  for(i = 0; i < nspe; ++i) {
	       ctx[i] = X(cell_aligned_malloc)(sizeof(*ctx[i]));

	       spe_id[i] = spe_create_thread(0, &X(spu_fftw), ctx[i],
					     NULL, -1, 0);
	  }
     }
}

void X(cell_deactivate_spes)(void)
{
     if (--refcnt == 0) {
	  int i;
	  int status = 0;

	  for (i = 0; i < nspe; ++i)
	       ctx[i]->op = SPE_EXIT;

	  X(cell_spe_awake_all)();

	  for (i = 0; i < nspe; ++i) {
	       spe_wait(spe_id[i], &status, 0);
	       free(ctx[i]);
	       ctx[i] = 0;
	  }
     }
}

int X(cell_nspe)(void)
{
     return nspe;
}

void X(cell_set_nspe)(int n)
{
     if (n > MAX_NSPE)
	  n = MAX_NSPE;
     if (n < 0)
	  n = 0;
     nspe = n;
}

struct spu_context *X(cell_get_ctx)(int spe)
{
     return ctx[spe];
}

void X(cell_spe_awake_all)(void)
{
     int i;

     for (i = 0; i < nspe; ++i) {
	  ctx[i]->done = 0;
     }
     /* FIXME: do we need this? */
     asm volatile ("sync");

     for (i = 0; i < nspe; ++i) {
	  spe_write_in_mbox(spe_id[i], 0);
     }
}

void X(cell_spe_wait_all)(void)
{
     int i;
     for (i = 0; i < nspe; ++i) {
	  while (!ctx[i]->done)
	       ;
     }
     
     /* BE Programming Handbook, 19.6.4 */
     asm volatile ("lwsync");
}

#endif /* HAVE_CELL */
