/*
 * Copyright (c) 2003, 2007 Matteo Frigo
 * Copyright (c) 2003, 2007 Massachusetts Institute of Technology
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

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

/* MUTEXES: Mutual exclusion devices that are only released by the thread
   that acquired the lock.   Expected to be faster than the more general
   semaphores below. */
typedef pthread_mutex_t os_mutex_t;

static void os_mutex_init(os_mutex_t *s) 
{ 
     pthread_mutex_init(s, (pthread_mutexattr_t *)0);
}

static void os_mutex_destroy(os_mutex_t *s) { pthread_mutex_destroy(s); }
static void os_mutex_lock(os_mutex_t *s) { pthread_mutex_lock(s); }
static void os_mutex_unlock(os_mutex_t *s) { pthread_mutex_unlock(s); }


/* SEMAPHORES: Dijkstra-style semaphores where arbitrary threads can
   execute up() and down().  We only need binary semaphores. */
#if (defined(_POSIX_SEMAPHORES) && (_POSIX_SEMAPHORES >= 200112L))

   /* use optional POSIX semaphores. */
#  include <semaphore.h>
#  include <errno.h>

   typedef sem_t os_sem_t;

   static void os_sem_init(os_sem_t *s) { sem_init(s, 0, 0); }
   static void os_sem_destroy(os_sem_t *s) { sem_destroy(s); }

   static void os_sem_down(os_sem_t *s) 
   { 
	int err;
	do {
	     err = sem_wait(s);
	} while (err == -1 && errno == EINTR);
	CK(err == 0);
   }

   static void os_sem_up(os_sem_t *s) { sem_post(s); }

#else /* simulate semaphores with condition variables */

   typedef struct {
	pthread_mutex_t m;
	pthread_cond_t c;
	volatile int x;
   } os_sem_t; 

   static void os_sem_init(os_sem_t *s)
   {
	pthread_mutex_init(&s->m, (pthread_mutexattr_t *)0);
	pthread_cond_init(&s->c, (pthread_condattr_t *)0);

	/* wrap initialization in lock to exploit the release
	   semantics of pthread_mutex_unlock() */
	pthread_mutex_lock(&s->m);
	s->x = 0;
	pthread_mutex_unlock(&s->m);
   }

   static void os_sem_destroy(os_sem_t *s)
   {
	pthread_mutex_destroy(&s->m);
	pthread_cond_destroy(&s->c);
   }

   static void os_sem_down(os_sem_t *s)
   {
	pthread_mutex_lock(&s->m);
	while (s->x <= 0) 
	     pthread_cond_wait(&s->c, &s->m);
	--s->x;
	pthread_mutex_unlock(&s->m);
   }

   static void os_sem_up(os_sem_t *s)
   {
	pthread_mutex_lock(&s->m);
	++s->x;
	pthread_cond_signal(&s->c);
	pthread_mutex_unlock(&s->m);
   }

#endif

#define FFTW_WORKER void *

static void os_create_worker(FFTW_WORKER (*worker)(void *arg), 
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

#elif defined(__WIN32__) || defined(_WIN32) || defined(_WINDOWS)
/* hack: windef.h defines INT for its own purposes and this causes
   a conflict with our own INT in ifftw.h.  Divert the windows
   definition into another name unlikely to cause a conflict */
#define INT magnus_ab_INTegro_seclorum_nascitur_ordo
#include <windows.h>
#include <process.h>
#undef INT

typedef HANDLE os_mutex_t;

static void os_mutex_init(os_mutex_t *s) 
{ 
     *s = CreateMutex(NULL, FALSE, NULL);
}

static void os_mutex_destroy(os_mutex_t *s) 
{ 
     CloseHandle(*s);
}

static void os_mutex_lock(os_mutex_t *s)
{ 
     WaitForSingleObject(*s, INFINITE);
}

static void os_mutex_unlock(os_mutex_t *s) 
{ 
     ReleaseMutex(*s);
}

typedef HANDLE os_sem_t;

static void os_sem_init(os_sem_t *s) 
{
     *s = CreateSemaphore(NULL, 0, 0x7FFFFFFFL, NULL);
}

static void os_sem_destroy(os_sem_t *s) 
{ 
     CloseHandle(*s);
}

static void os_sem_down(os_sem_t *s) 
{ 
     WaitForSingleObject(*s, INFINITE);
}

static void os_sem_up(os_sem_t *s) 
{
     ReleaseSemaphore(*s, 1, NULL);
}

#define FFTW_WORKER unsigned __stdcall
typedef unsigned (__stdcall *winthread_start) (void *);

static void os_create_worker(winthread_start worker,
			     void *arg)
{
     _beginthreadex((void *)NULL,               /* security attrib */
		    0,				/* stack size */
		    worker,                     /* start address */
		    arg,			/* parameters */
		    0,				/* creation flags */
		    (unsigned *)NULL);		/* tid */
}

static void os_destroy_worker(void)
{
     _endthreadex(0);
}


#else
#error "No threading layer defined"
#endif

/************************************************************************/

/* Main code: */
struct worker {
     os_sem_t ready;
     struct work *volatile w;
     struct worker *volatile cdr;
};

struct work {
     spawn_function proc;
     spawn_data d;
     os_sem_t done;
};

static os_mutex_t queue_lock;
static struct worker *volatile worker_queue;
#define WITH_QUEUE_LOCK(what)			\
{						\
     os_mutex_lock(&queue_lock);		\
     what;					\
     os_mutex_unlock(&queue_lock);		\
}

static FFTW_WORKER worker(void *arg)
{
     struct worker ego;
     struct work *w = (struct work *)arg;
     os_sem_init(&ego.ready);

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
	  os_sem_up(&w->done);

	  /* wait until work becomes available */
	  os_sem_down(&ego.ready);
	  
	  w = ego.w;
     } while (w->proc);

     /* termination protocol */
     os_sem_up(&w->done);

     os_sem_destroy(&ego.ready);

     os_destroy_worker();
     /* UNREACHABLE */
     return 0;
}

static void kill_workforce(void)
{
     struct work w;

     w.proc = 0;
     os_sem_init(&w.done);

     WITH_QUEUE_LOCK({
	  /* tell all workers that they must terminate.  

	  Because workers enqueue themselves before signaling the
	  completion of the work, all workers belong to the worker queue
	  if we get here.  Also, all workers are waiting at
	  os_sem_down(&ego.ready), so we can hold the queue lock without
	  deadlocking */
	  while (worker_queue) {
	       struct worker *r = worker_queue;
	       worker_queue = r->cdr;
	       r->w = &w;
	       os_sem_up(&r->ready);
	       os_sem_down(&w.done);
	  }
     });

     os_sem_destroy(&w.done);
}

int X(ithreads_init)(void)
{
     os_mutex_init(&queue_lock);

     WITH_QUEUE_LOCK({
	  worker_queue = 0;
     })

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
	       os_sem_init(&w->done);

	       WITH_QUEUE_LOCK({
		    if (worker_queue) {
			 /* a worker is available.  Remove it from the
			    worker queue and wake it up */
			 struct worker *q = worker_queue;
			 worker_queue = q->cdr;
			 q->w = w;
			 os_sem_up(&q->ready);
		    } else {
			 /* no worker is available.  Create one */
			 os_create_worker(worker, w);
		    }
	       });
	  }
     }

     for (i = 0; i < nthr - 1; ++i) { 
	  struct work *w = &r[i];
	  os_sem_down(&w->done);
	  os_sem_destroy(&w->done);
     }

     STACK_FREE(r);
     THREAD_OFF; /* prevent debugging mode from failing under threads */
}

void X(threads_cleanup)(void)
{
     kill_workforce();
     os_mutex_destroy(&queue_lock);
}

#endif /* HAVE_THREADS */
