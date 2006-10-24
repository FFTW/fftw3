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

/* threads.c: Portable thread spawning for loops, via the X(spawn_loop)
   function.  The first portion of this file is a set of macros to
   spawn and join threads on various systems. */

#include "threads.h"
#include <pthread.h>
#include <semaphore.h>

/* PTHREADS GLUE: */
/* binary semaphore, implemented with general semaphores: */
typedef sem_t bsem_t; 

static void bsem_init(bsem_t *s)
{
     sem_init(s, 0, 0);
}

static void up(bsem_t *s)
{
     sem_post(s);
}

static void down(bsem_t *s)
{
     sem_wait(s);
}

static void bsem_destroy(bsem_t *s)
{
     sem_destroy(s);
}

static void create_worker(void *(*worker)(void *arg), 
			  void *arg)
{
     pthread_attr_t attr;
     pthread_t tid;

     pthread_attr_init(&attr);
     pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); 
     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

     pthread_create(&tid, &attr, worker, (void *)arg);
     pthread_attr_destroy(&attr);
}
/* end PTHREADS GLUE: */

/************************************************************************/
struct worker {
     bsem_t ready;
     struct work *w;
     struct worker *cdr;
};

struct work {
     spawn_function proc;
     spawn_data d;
     bsem_t done;
     struct work *cdr;
};

static bsem_t queue_lock;
static struct worker *worker_queue;
#define WITH_QUEUE_LOCK(what)			\
{						\
     down(&queue_lock);				\
     what;					\
     up(&queue_lock);				\
}

static void *worker(void *arg)
{
     struct worker ego;
     struct work *w = (struct work *)arg;
     bsem_init(&ego.ready);

     do {
	  A(w->proc);

	  /* do the work */
          w->proc(&w->d);

	  /* First enqueue the worker, then signal that work is
	     completed.  The opposite order would allow the parent to
	     continue, and possibly create another worker before we
	     get a chance to add ourselves to the work queue. */

	  /* enqueue worker into worker queue: */
	  WITH_QUEUE_LOCK({
	       ego.cdr = worker_queue;
	       worker_queue = &ego;
	  });

	  /* signal that work is done */
	  up(&w->done);

	  /* wait until work becomes available */
	  down(&ego.ready);
	  
	  w = ego.w;
     } while (w->proc);

     /* termination protocol */
     up(&w->done);

     bsem_destroy(&ego.ready);

     return (void *)0;
}

static void kill_workforce(void)
{
     struct work w;

     w.proc = 0;
     bsem_init(&w.done);

     WITH_QUEUE_LOCK({
	  /* tell all workers that they must terminate.  

	  Because workers enqueue themselves before signaling the
	  completion of the work, all workers belong to the worker queue
	  if we get here.  Also, all workers are waiting at
	  down(&ego.ready), so we can hold the queue lock without
	  deadlocking */
	  while (worker_queue) {
	       struct worker *r = worker_queue;
	       worker_queue = r->cdr;
	       r->w = &w;
	       up(&r->ready);
	       down(&w.done);
	  }
     });

     bsem_destroy(&w.done);
}

int X(ithreads_init)(void)
{
     bsem_init(&queue_lock);
     up(&queue_lock);

     worker_queue = 0;

     X(mksolver_ct_hook) = X(mksolver_ct_threads);
     X(mksolver_hc2hc_hook) = X(mksolver_hc2hc_threads);

     return 0; /* no error */
}

void X(threads_setmax)(int n)
{
     UNUSED(n);
}

/* Distribute a loop from 0 to loopmax-1 over nthreads threads.
   proc(d) is called to execute a block of iterations from d->min
   to d->max-1.  d->thr_num indicate the number of the thread
   that is executing proc (from 0 to nthreads-1), and d->data is
   the same as the data parameter passed to X(spawn_loop).

   This function returns only after all the threads have completed. */
void X(spawn_loop)(int loopmax, int nthr, spawn_function proc, void *data)
{
     int block_size;
     struct work *r;
     int i;

     A(loopmax >= 0);
     A(nthr > 0);
     A(proc);

     if (!loopmax) return;

     /* Choose the block size and number of threads in order to (1)
        minimize the critical path and (2) use the fewest threads that
        achieve the same critical path (to minimize overhead).
        e.g. if loopmax is 5 and nthr is 4, we should use only 3
        threads with block sizes of 2, 2, and 1. */
     block_size = (loopmax + nthr - 1) / nthr;
     nthr = (loopmax + block_size - 1) / block_size;

     THREAD_ON; /* prevent debugging mode from failing under threads */
     STACK_MALLOC(struct work *, r, sizeof(struct work) * nthr);
	  
     for (i = 0; i < nthr; ++i) {
	  struct work *w = &r[i];
	  spawn_data *d = &w->d;
	  d->max = (d->min = i * block_size) + block_size;
	  if (d->max > loopmax)
	       d->max = loopmax;
	  d->thr_num = i;
	  d->data = data;
	  w->proc = proc;
	   
	  if (i == nthr - 1) {
	       /* we do it ourselves */
	       proc(d);
	  } else {
	       /* dispatch work W to some worker */
	       bsem_init(&w->done);

	       WITH_QUEUE_LOCK({
		    if (worker_queue) {
			 /* a worker is available.  Remove it from the
			    worker queue and wake it up */
			 struct worker *r = worker_queue;
			 worker_queue = r->cdr;
			 r->w = w;
			 up(&r->ready);
		    } else {
			 /* no worker is available.  Create one */
			 create_worker(worker, w);
		    }
	       });
	  }
     }

     for (i = 0; i < nthr - 1; ++i) { 
	  struct work *w = &r[i];
	  down(&w->done);
	  bsem_destroy(&w->done);
     }

     STACK_FREE(r);
     THREAD_OFF; /* prevent debugging mode from failing under threads */
}

void X(threads_cleanup)(void)
{
     kill_workforce();
     bsem_destroy(&queue_lock);
     X(mksolver_ct_hook) = 0;
     X(mksolver_hc2hc_hook) = 0;
}
