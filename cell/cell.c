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

#define USE_PS_AREA 1

#ifdef HAVE_LIBSPE2
#  include <pthread.h>
#  include <libspe2.h>
#else /* !HAVE_LIBSPE2 */
#  include <libspe.h>
#endif /* !HAVE_LIBSPE2 */

#include "fftw-cell.h"
#include <stdlib.h> /* posix_memalign */

extern spe_program_handle_t X(spu_fftw);

void *X(cell_aligned_malloc)(size_t n)
{
     void *p;
     posix_memalign(&p, 128, n);
     return p;
}

static struct spu_context *ctx[MAX_NSPE];
#ifdef HAVE_LIBSPE2
static pthread_t spe_pthread[MAX_NSPE];
static spe_context_ptr_t spe_id[MAX_NSPE];
#  if USE_PS_AREA
static spe_spu_control_area_t *ps_area[MAX_NSPE];
#  endif
#else
static speid_t spe_id[MAX_NSPE];
#endif
static int refcnt = 0;
static int nspe = -1;

static void set_default_nspe(void)
{
     if (nspe < 0) {
	  /* set NSPE to the maximum of 8 and the number of physical
	     SPEs.  A two-processor Cell blade reports 16 SPEs, but we
	     only want to use one processor by default. */
#ifdef HAVE_LIBSPE2
	  int phys = spe_cpu_info_get(SPE_COUNT_PHYSICAL_SPES, -1);
#else
	  int phys = spe_count_physical_spes();
#endif
	  if (phys > 8)
	       phys = 8;
	  X(cell_set_nspe)(phys);
     }
}

#ifdef HAVE_LIBSPE2
static void *ppu_thread(void *arg)
{
     int rc, i = (int) ((uintptr_t) arg);
     do {
	  unsigned int entry = SPE_DEFAULT_ENTRY;
	  rc = spe_context_run(spe_id[i], &entry, 0, ctx[i], NULL, NULL);
     } while (rc > 0);
     pthread_exit(NULL);
}
#endif

void X(cell_activate_spes)(void)
{
     if (refcnt++ == 0) {
	  int i;

	  set_default_nspe();
	  for(i = 0; i < nspe; ++i) {
	       ctx[i] = X(cell_aligned_malloc)(sizeof(*ctx[i]));

#ifdef HAVE_LIBSPE2
#  if USE_PS_AREA
	       spe_id[i] = spe_context_create(SPE_MAP_PS, NULL);
	       if (spe_id[i])
		    ps_area[i] = (spe_spu_control_area_t *)
			 spe_ps_area_get(spe_id[i], SPE_CONTROL_AREA);
	       else { /* OS doesn't support SPE_MAP_PS */
		    spe_id[i] = spe_context_create(0, NULL);
		    ps_area[i] = NULL;
	       }
#  else
	       spe_id[i] = spe_context_create(0, NULL);
#  endif
	       spe_program_load(spe_id[i], &X(spu_fftw));
	       pthread_create(&spe_pthread[i], NULL, ppu_thread, 
			      (void *) ((uintptr_t) i));
#else
	       spe_id[i] = spe_create_thread(0, &X(spu_fftw), ctx[i],
					     NULL, -1, 0);
#endif
	  }
     }
}

void X(cell_deactivate_spes)(void)
{
     if (--refcnt == 0) {
	  int i;

	  for (i = 0; i < nspe; ++i)
	       ctx[i]->op = FFTW_SPE_EXIT;

	  X(cell_spe_awake_all)();

	  for (i = 0; i < nspe; ++i) {
#ifdef HAVE_LIBSPE2
	       pthread_join(spe_pthread[i], NULL);
	       spe_context_destroy(spe_id[i]);
#else
	       int status = 0;
	       spe_wait(spe_id[i], &status, 0);
#endif
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
#ifdef HAVE_LIBSPE2
     unsigned int zero = 0;
#endif

     for (i = 0; i < nspe; ++i) {
	  ctx[i]->done = 0;
     }
     /* FIXME: do we need this? */
     asm volatile ("sync");

     for (i = 0; i < nspe; ++i) {
#ifdef HAVE_LIBSPE2
#  if USE_PS_AREA
	  if (ps_area[i])
	       ps_area[i]->SPU_In_Mbox = 0;
	  else /* NULL if not supported by OS */
#  endif
	  spe_in_mbox_write(spe_id[i], &zero, 1, SPE_MBOX_ANY_NONBLOCKING);
#else
	  spe_write_in_mbox(spe_id[i], 0);
#endif
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
