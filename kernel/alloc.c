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

/* $Id: alloc.c,v 1.1.1.1 2002-06-02 18:42:31 athena Exp $ */

#include "ifftw.h"

/**********************************************************
 *   DEBUGGING CODE
 **********************************************************/
#ifdef FFTW_DEBUG

/*
 * debugging malloc/free.  Initialize every malloced and freed area to
 * random values, just to make sure we are not using uninitialized
 * pointers.  Also check for writes past the ends of allocated blocks,
 * and a couple of other things.
 *
 * This code is a quick and dirty hack -- use at your own risk.
 */

struct mstat {
     int siz;
     int maxsiz;
     int cnt;
     int maxcnt;
};

static struct mstat mstat[MALLOC_WHAT_LAST];

#define MAGIC ((size_t)0xABadCafe)
#define PAD_FACTOR 2
#define THREE_SIZE_T (3 * sizeof(size_t))

struct minfo {
     const char *file;
     int line;
     size_t n;
     void *p;
     struct minfo *next;
};

static struct minfo *minfo = 0;

void *fftw_malloc_debug(size_t n, enum fftw_malloc_what what,
			const char *file, int line)
{
     char *p;
     size_t i;
     struct minfo *info;
     struct mstat *stat = mstat + what;

     if (n == 0)
	  n = 1;

     stat->siz += n;
     if (stat->siz > stat->maxsiz)
	  stat->maxsiz = stat->siz;

     p = (char *) malloc(PAD_FACTOR * n + THREE_SIZE_T);
     A(p);

     /* store the sz in a known position */
     ((size_t *) p)[0] = n;
     ((size_t *) p)[1] = MAGIC;
     ((size_t *) p)[2] = what;

     /* fill with junk */
     for (i = 0; i < PAD_FACTOR * n; i++)
	  p[i + THREE_SIZE_T] = (char) (i ^ 0xDEADBEEF);

     ++stat->cnt;

     if (stat->cnt > stat->maxcnt)
	  stat->maxcnt = stat->cnt;


     /* skip the info we stored previously */
     p = p + THREE_SIZE_T;

     /* record allocation in allocation list */
     info = (struct minfo *) malloc(sizeof(struct minfo));
     info->n = n;
     info->file = file;
     info->line = line;
     info->p = p;
     info->next = minfo;
     minfo = info;

     return (void *) p;
}

void fftw_free(void *p)
{
     char *q;

     A(p);

     q = ((char *) p) - THREE_SIZE_T;
     A(q);

     {
	  size_t n = ((size_t *) q)[0];
	  size_t magic = ((size_t *) q)[1];
	  int what = ((size_t *) q)[2];
	  size_t i;
	  struct mstat *stat = mstat + what;

	  /* set to zero to detect duplicate free's */
	  ((size_t *) q)[0] = 0;

	  A(magic == MAGIC);
	  ((size_t *) q)[1] = ~MAGIC;

	  stat->siz -= n;
	  A(stat->siz >= 0);

	  /* check for writing past end of array: */
	  for (i = n; i < PAD_FACTOR * n; ++i)
	       if (q[i + THREE_SIZE_T] != (char) (i ^ 0xDEADBEEF)) {
		    A(0 /* array bounds overwritten */ );
	       }
	  for (i = 0; i < PAD_FACTOR * n; ++i)
	       q[i + THREE_SIZE_T] = (char) (i ^ 0xBEEFDEAD);

	  --stat->cnt;

	  A(stat->cnt >= 0);
	  A((stat->cnt == 0 && stat->siz == 0) ||
	    (stat->cnt > 0 && stat->siz > 0));
	  free(q);
     }

     {
	  /* delete minfo entry */
	  struct minfo **i;

	  for (i = &minfo; *i; i = &((*i)->next)) {
	       if ((*i)->p == p) {
		    struct minfo *i0 = (*i)->next;
		    free(*i);
		    *i = i0;
		    return;
	       }
	  }

	  A(0 /* no entry in minfo list */ );
     }
}

void fftw_malloc_print_minfo(void)
{
     struct minfo *info;
     int what;
     static const char *names[MALLOC_WHAT_LAST] = {
	  "PLANS", "SOLVERS", "PROBLEMS",
	  "BUFFERS", "HASHT", "TENSORS", "PLANNERS", "TWIDDLES",
	  "OTHER"
     };

     printf("%10s %8s %8s %10s %10s\n",
	    "what", "cnt", "maxcnt", "siz", "maxsiz");

     for (what = 0; what < MALLOC_WHAT_LAST; ++what) {
	  struct mstat *stat = mstat + what;
	  printf("%10s %8d %8d %10d %10d\n",
		 names[what], stat->cnt, stat->maxcnt, 
		 stat->siz, stat->maxsiz);
     }

     if (minfo)
	  printf("\nUnfreed allocations:\n");

     for (info = minfo; info; info = info->next) {
	  printf("%s:%d:  %u bytes at %p\n",
		 info->file, info->line, info->n, info->p);
     }
}
#else
/**********************************************************
 *   NON DEBUGGING CODE
 **********************************************************/
/* production version, no hacks */

void *fftw_malloc_plain(size_t n)
{
     void *p;
     if (n == 0)
	  n = 1;
     p = malloc(n);
     A(p);
     return p;
}

void fftw_free(void *p)
{
     free(p);
}

#endif
