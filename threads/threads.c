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

#ifdef HAVE_THREADS
#if defined(USING_POSIX_THREADS)

#include <pthread.h>
#include <semaphore.h>

typedef sem_t os_sem_t; 

static void os_sem_init(os_sem_t *s)
{
     sem_init(s, 0, 0);
}

static void os_sem_destroy(os_sem_t *s)
{
     sem_destroy(s);
}

static void os_sem_down(os_sem_t *s)
{
     sem_wait(s);
}

static void os_sem_up(os_sem_t *s)
{
     sem_post(s);
}

static void os_create_worker(void *(*worker)(void *arg), 
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

static void os_destroy_worker(void)
{
     pthread_exit((void *)0);
}

#else
#error "No threading layer defined"
#endif

/************************************************************************/
int X(spinlock_mode_i_know_what_i_am_doing) = 0;
#define SPINLOCK_MODE X(spinlock_mode_i_know_what_i_am_doing)

/* two implementations of semaphores:

    If (!SPINLOCK_MODE), then X is unused and we use OS semaphores
    as is.

    If (SPINLOCK_MODE), then the OS semaphore S is used to implement
    an atomic swap on X, which we use to spin.  See comments in down()
    for why you must know what you are doing if you want spinlock mode.
*/
typedef struct {
     volatile int x;
     os_sem_t s;
} fftw_sem_t;

/* atomic exchange of S->X and X */
static int xchg(fftw_sem_t *s, int x)
{
     int r;
     os_sem_down(&s->s);
     r = s->x;
     s->x = x;
     os_sem_up(&s->s);
     return r;
}

static void fftw_sem_init(fftw_sem_t *s)
{
     os_sem_init(&s->s);

     if (SPINLOCK_MODE) {
	  s->x = 0;
	  os_sem_up(&s->s);
     }
}

static void fftw_sem_destroy(fftw_sem_t *s)
{
     os_sem_destroy(&s->s);
}

static void down(fftw_sem_t *s)
{
     if (SPINLOCK_MODE) {
	  if (xchg(s, 0) != 0) 
	       return; /* fast case */

	  do {
	       /* Spin on s->x, without yielding to the OS and without
		  clogging the bus with atomic operations. 

		  This implementation guarantees mutual exclusion, but
		  it may livelock because there is no guarantee that
		  the update xchg(s, 1) in up() becomes ever visible
		  to the current thread.  This property depends upon
		  the memory model, and can be portably guaranteed
		  only if we call os_sem_down(), which is precisely
		  what we are trying to avoid.
	       */
	       while (s->x == 0) ;
	  } while (xchg(s, 0) == 0);
     } else {
	  os_sem_down(&s->s);
     }
}

static void up(fftw_sem_t *s)
{
     if (SPINLOCK_MODE) {
	  xchg(s, 1);
     } else {
	  os_sem_up(&s->s);
     }
}

/* Main code: */
struct worker {
     fftw_sem_t ready;
     struct work *w;
     struct worker *cdr;
};

struct work {
     spawn_function proc;
     spawn_data d;
     fftw_sem_t done;
     struct work *cdr;
};

static fftw_sem_t queue_lock;
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
     fftw_sem_init(&ego.ready);

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

     fftw_sem_destroy(&ego.ready);

     os_destroy_worker();
     /* UNREACHABLE */
     return (void *)0;
}

static void kill_workforce(void)
{
     struct work w;

     w.proc = 0;
     fftw_sem_init(&w.done);

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

     fftw_sem_destroy(&w.done);
}

int X(ithreads_init)(void)
{
     fftw_sem_init(&queue_lock);
     up(&queue_lock);

     worker_queue = 0;

     return 0; /* no error */
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
	       fftw_sem_init(&w->done);

	       WITH_QUEUE_LOCK({
		    if (worker_queue) {
			 /* a worker is available.  Remove it from the
			    worker queue and wake it up */
			 struct worker *q = worker_queue;
			 worker_queue = q->cdr;
			 q->w = w;
			 up(&q->ready);
		    } else {
			 /* no worker is available.  Create one */
			 os_create_worker(worker, w);
		    }
	       });
	  }
     }

     for (i = 0; i < nthr - 1; ++i) { 
	  struct work *w = &r[i];
	  down(&w->done);
	  fftw_sem_destroy(&w->done);
     }

     STACK_FREE(r);
     THREAD_OFF; /* prevent debugging mode from failing under threads */
}

void X(threads_cleanup)(void)
{
     kill_workforce();
     fftw_sem_destroy(&queue_lock);
}

#endif /* HAVE_THREADS */
