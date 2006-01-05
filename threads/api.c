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

#include "api.h"
#include "threads.h"

static int threads_inited = 0;

/* should be called before all other FFTW functions! */
int X(init_threads)(void)
{
     if (!threads_inited) {
	  planner *plnr;

          if (X(ithreads_init)())
               return 0;

	  /* this should be the first time the_planner is called,
	     and hence the time it is configured */
	  plnr = X(the_planner)();
	  X(threads_conf_standard)(plnr);
	       
          threads_inited = 1;
     }
     return 1;
}

void X(cleanup_threads)(void)
{
     X(cleanup)();
     if (threads_inited) {
	  X(threads_cleanup)();
	  threads_inited = 0;
     }
}

void X(plan_with_nthreads)(int nthreads)
{
     planner *plnr;

     if (!threads_inited) {
	  X(cleanup)();
	  X(init_threads)();
     }
     A(threads_inited);
     plnr = X(the_planner)();
     plnr->nthr = X(imax)(1, nthreads);
     X(threads_setmax)(plnr->nthr);
}
