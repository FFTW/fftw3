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

/************************* Thread Glue *************************/

/* Adding support for a new shared memory thread API should be easy.  You
   simply do the following things (look at the POSIX and Solaris
   threads code for examples):

   * Invent a symbol of the form USING_FOO_THREADS to denote
     the use of your thread API, and add an 
              #elif defined(USING_FOO_THREADS)
      before the #else clause below.  This is where you will put
      your thread definitions.  In this #elif, insert the following:

      -- #include any header files needed to use the thread API.

      -- Typedef fftw_thr_function to be a function pointer
         of the type used as a argument to fftw_thr_spawn
	 (i.e. the entry function for a thread).

      -- Define fftw_thr_id, via a typedef, to be the type
         that is used for thread identifiers.

      -- #define fftw_thr_spawn(tid_ptr, proc, data) to
         call whatever function to spawn a new thread.  The
         new thread should call proc(data) as its starting point,
         and tid_ptr is a pointer to a fftw_thr_id that
         is set to an identifier for the thread.  You can also
         define this as a subroutine (put it in threads.c)
	 if it is too complicated for a macro.  The prototype should
	 be:

	 void fftw_thr_spawn(fftw_thr_id *tid_ptr,
	                        fftw_thr_function proc,
				void *data);

      -- #define fftw_thr_wait(tid) to block until the thread
         whose identifier is tid has terminated.  You can also
         define this as a subroutine (put it in threads.c) if
	 it is too complicated for a macro.  The prototype should be:
	
	 void fftw_thr_wait(fftw_thr_id tid);

   * If semaphores are supported (which allows FFTW to pre-spawn the
     threads), then you should #define HAVE_SEMAPHORES and:
  
      -- typedef fftw_sem_id to the type for a semaphore id

      -- #define fftw_sem_init(&id) to initialize the semaphore
         id to zero (or equivalent)

      -- #define fftw_sem_destroy(&id) to destroy the id

      -- #define fftw_sem_wait(&id) to the equivalent of
         the SYSV sem_wait

      -- #define fftw_sem_post(&id) the equivalent of SYSV sem_post

     THIS IS CURRENTLY EXPERIMENTAL ONLY.

   * If you need to perform any initialization before using threads,
     put your initialization code in the X(ithreads_init)() function
     in threads.c, bracketed by the appropriate #ifdef of course.

   * Also, of course, you should modify config.h to #define
     USING_FOO_THREADS, or better yet modify and configure.ac so that
     autoconf can automatically detect your threads library.

   * Finally, if you do implement support for a new threads API, be
     sure to let us know at fftw@fftw.org so that we can distribute
     your code to others!

*/

/************************** MP directive Threads ****************************/

#if defined(USING_OPENMP_THREADS) || defined(USING_SGIMP_THREADS)

/* Use MP compiler directives to induce parallelism, in which case
   we don't need any of the thread spawning/waiting macros: */

typedef void * (*fftw_thr_function) (void *);

typedef char fftw_thr_id;  /* dummy */

#define fftw_thr_spawn(tid_ptr, proc, data) ((proc)(data))
#define fftw_thr_wait(tid) (0) /* do nothing */

#define USING_COMPILER_THREADS 1

/************************** Solaris Threads ****************************/
      
#elif defined(USING_SOLARIS_THREADS)

/* Solaris threads glue.  Tested. */

/* link with -lthread */

#include <thread.h>

/* Thread entry point: */
typedef void * (*fftw_thr_function) (void *);

typedef thread_t fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) \
     thr_create(0,0,proc,data,THR_BOUND,tid_ptr)

#define fftw_thr_wait(tid) thr_join(tid,0,0)

/************************** BeOS Threads ****************************/
      
#elif defined(USING_BEOS_THREADS)

/* BeOS threads glue.  Tested for DR8.2. */

#include <OS.h>

/* Thread entry point: */
typedef thread_entry fftw_thr_function;

typedef thread_id fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) { \
     *(tid_ptr) = spawn_thread(proc,"FFTW",B_NORMAL_PRIORITY,data); \
     resume_thread(*(tid_ptr)); \
}

/* wait_for_thread requires that we pass a valid pointer as the
   second argument, even if we're not interested in the result. */
#define fftw_thr_wait(tid) {long exit_val;wait_for_thread(tid, &exit_val);}

/************************** MacOS Threads ****************************/

#elif defined(USING_MACOS_THREADS)

/* MacOS (old! old!) MP threads glue. Experimental, untested! I do not
   have an MP MacOS system available to me...I just read the
   documentation.  There is actually a good chance that this will work
   (since the code below is so short), but I make no guarantees.
   Consider it to be a starting point for your own implementation.

   I also had to insert some code in threads.c. 

   MacOS X has real SMP support, thank goodness; I'm leaving this
   code here mainly for historical purposes. */

/* Using this code in the MacOS: (See the README file for general
   documenation on the FFTW threads code.)  To use this code, you have
   to do two things.  First of all, you have to #define the symbol
   USING_MACOS_THREADS. This can be done at the top of this file
   or perhaps in your compiler options.  Second, you have to weak-link
   your project to the MP library.

   In your code, you should check at run-time with MPLibraryIsLoaded()
   to see if the MP library is available. If it is not, it is
   still safe to call the fftw threads routines...in this case,
   however, you must always pass 1 for the nthreads parameter!
   (Otherwise, you will probably want to pass the value of
   MPProcessors() for the nthreads parameter.) */

#include <MP.h>

typedef TaskProc fftw_thr_function;

typedef MPQueueID fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) { \
     MPTaskID task; \
     MPCreateQueue(tid_ptr); \
     MPCreateTask(proc,data,kMPUseDefaultStackSize,*(tid_ptr),0,0, \
		  kMPNormalTaskOptions,&task); \
}

#define fftw_thr_wait(tid) { \
     void *param1,*param2,*param3; \
     MPWaitOnQueue(tid,&param1,&param2,&param3,kDurationForever); \
     MPDeleteQueue(tid); \
}

/************************** Win32 Threads ****************************/

#elif defined(__WIN32__) || defined(_WIN32) || defined(_WINDOWS)

/* Win32 threads glue.  We have not tested this code!  (I just implemented
   it by looking at a Win32 threads manual.)  Users have reported that this
   code works under NT using Microsoft compilers.
   
   This code should be automatically used on Windows, assuming that
   one of the above macros is defined by your compiler.  You must also
   link to the thread-safe version of the C runtime library. */

#include <windows.h>
#include <process.h>

typedef LPTHREAD_START_ROUTINE fftw_thr_function;
typedef HANDLE fftw_thr_id;

/* The following macros are based on a recommendation in the
   July 1999 Microsoft Systems Journal (online), to substitute
   a call to _beginthreadex for CreateThread.  The former is
   needed in order to make the C runtime library thread-safe
   (in particular, our threads may call malloc/free). */
typedef unsigned (__stdcall *PTHREAD_START) (void *);
#define chBEGINTHREADEX(psa, cbStack, pfnStartAddr, \
    pvParam, fdwCreate, pdwThreadID)                \
      ((HANDLE) _beginthreadex(                     \
         (void *) (psa),                            \
         (unsigned) (cbStack),                      \
         (PTHREAD_START) (pfnStartAddr),            \
         (void *) (pvParam),                        \
         (unsigned) (fdwCreate),                    \
         (unsigned *) (pdwThreadID))) 

#define fftw_thr_spawn(tid_ptr, proc, data) { \
     DWORD thrid; \
     *(tid_ptr) = chBEGINTHREADEX((LPSECURITY_ATTRIBUTES) NULL, 0, \
			          (fftw_thr_function) proc, (LPVOID) data, \
			          0, &thrid); \
}

#define fftw_thr_wait(tid) { \
     WaitForSingleObject(tid, INFINITE); \
     CloseHandle(tid); \
}

/************************** Mach cthreads ****************************/

#elif defined(USING_MACH_THREADS)

#ifdef HAVE_MACH_CTHREADS_H
#include <mach/cthreads.h>
#elif defined(HAVE_CTHREADS_H)
#include <cthreads.h>
#elif defined(HAVE_CTHREAD_H)
#include <cthread.h>
#endif

typedef cthread_fn_t fftw_thr_function;

typedef cthread_t fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) \
     *(tid_ptr) = cthread_fork(proc, (any_t) (data))

#define fftw_thr_wait(tid) cthread_join(tid)

/************************** POSIX Threads ****************************/

#elif defined(USING_POSIX_THREADS) /* use the default, POSIX threads: */

/* POSIX threads glue.  Tested. */

/* link with -lpthread, or better yet use ACX_PTHREAD in autoconf */

#include <pthread.h>

/* Thread entry point: */
typedef void * (*fftw_thr_function) (void *);

static pthread_attr_t fftw_pthread_attributes; /* attrs for POSIX threads */
static pthread_attr_t *fftw_pthread_attributes_p = 0;

typedef pthread_t fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data)  \
     CK(!pthread_create(tid_ptr,fftw_pthread_attributes_p,proc,data))

#define fftw_thr_wait(tid) CK(!pthread_join(tid,0))

/* POSIX semaphores are disabled for now because, at least on my Linux
   machine, they don't seem to offer much performance advantage. */
#if 0
#define HAVE_SEMAPHORES 1

#include <semaphore.h>

typedef sem_t fftw_sem_id;
#define fftw_sem_init(pid) CK(!sem_init(pid, 0, 0))
#define fftw_sem_destroy(pid)  CK(!sem_destroy(pid))
#define fftw_sem_wait(pid) CK(!sem_wait(pid))
#define fftw_sem_post(pid) CK(!sem_post(pid))

#endif /* 0 */

#elif defined(HAVE_THREADS)
#  error HAVE_THREADS is defined without any USING_*_THREADS
#endif


#if 0 /* 1 for experimental pre-spawned threads via Linux spinlocks */
#ifndef HAVE_SEMAPHORES
#define HAVE_SEMAPHORES 1

/* from x86 linux/kernel.h */
/* Optimization barrier */
/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")

#include <asm/spinlock.h>

typedef spinlock_t fftw_sem_id;
#define fftw_sem_init(pid) { spin_lock_init(pid); spin_lock(pid); }
#define fftw_sem_destroy(pid) (void) (pid)
#define fftw_sem_wait(pid) { spin_unlock_wait(pid); spin_lock(pid); }
#define fftw_sem_post(pid) spin_unlock(pid)

#endif /* !HAVE_SEMAPHORES */
#endif /* 0 */

/***********************************************************************/

#ifdef HAVE_THREADS

#ifdef HAVE_SEMAPHORES

typedef struct worker_data_s {
     fftw_thr_id tid;
     fftw_sem_id sid_ready;
     fftw_sem_id sid_done;
     spawn_function proc;
     spawn_data d;
     struct worker_data_s *next;
} worker_data;

static void *do_work(worker_data *w)
{
     while (1) {
	  fftw_sem_wait(&w->sid_ready);
	  if (!w->proc) break;
	  w->proc(&w->d);
	  fftw_sem_post(&w->sid_done);
     }
     return 0;
}

worker_data *workers = (worker_data *) 0;

/* make sure at least nworkers exist */
static void minimum_workforce(int nworkers)
{
     worker_data *w = workers;

     while (w) {
	  --nworkers;
	  w = w->next;
     }
     while (nworkers-- > 0) {
	  w = (worker_data *) MALLOC(sizeof(worker_data), OTHER);
	  w->next = workers;
	  fftw_sem_init(&w->sid_ready);
	  fftw_sem_init(&w->sid_done);
	  fftw_thr_spawn(&w->tid, (fftw_thr_function) do_work, (void *) w);
	  workers = w;
     }
}

static void kill_workforce(void)
{
     while (workers) {
	  worker_data *w = workers;
	  workers = w->next;
	  w->proc = (spawn_function) 0;
	  fftw_sem_post(&w->sid_ready);
	  fftw_thr_wait(w->tid);
	  fftw_sem_destroy(&w->sid_ready);
	  fftw_sem_destroy(&w->sid_done);
	  X(ifree)(w);
     }
}

#endif /* HAVE_SEMAPHORES */

/* Distribute a loop from 0 to loopmax-1 over nthreads threads.
   proc(d) is called to execute a block of iterations from d->min
   to d->max-1.  d->thr_num indicate the number of the thread
   that is executing proc (from 0 to nthreads-1), and d->data is
   the same as the data parameter passed to X(spawn_loop).

   This function returns only after all the threads have completed. */
void X(spawn_loop)(int loopmax, int nthr,
		   spawn_function proc, void *data)
{
     int block_size;

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

     if (nthr <= 1) {
	  spawn_data d;
	  d.min = 0; d.max = loopmax;
	  d.thr_num = 0;
	  d.data = data;
	  proc(&d);
     }
     else {
#if defined(USING_COMPILER_THREADS)
	  spawn_data d;
#elif defined(HAVE_SEMAPHORES)
	  spawn_data d;
	  worker_data *w;
#else
	  spawn_data *d;
	  fftw_thr_id *tid;
#endif
	  int i;
	  
	  THREAD_ON; /* prevent debugging mode from failing under threads */

#if defined(USING_COMPILER_THREADS)
	  
#if defined(USING_SGIMP_THREADS)
#pragma parallel local(d,i)
	  {
#pragma pfor iterate(i=0; nthr; 1)
#elif defined(USING_OPENMP_THREADS)
#pragma omp parallel for private(d)
#endif	
	       for (i = 0; i < nthr; ++i) {
		    d.max = (d.min = i * block_size) + block_size;
                    if (d.max > loopmax)
                         d.max = loopmax;
		    d.thr_num = i;
		    d.data = data;
		    proc(&d);
	       }
#if defined(USING_SGIMP_THREADS)
	  }
#endif

#elif defined(HAVE_SEMAPHORES)

	  --nthr;
	  for (w = workers, i = 0; i < nthr; ++i) {
	       A(w);
	       w->d.max = (w->d.min = i * block_size) + block_size;
	       w->d.thr_num = i;
	       w->d.data = data;
	       w->proc = proc;
	       fftw_sem_post(&w->sid_ready);
	       w = w->next;
	  }
	  d.min = i * block_size;
	  d.max = loopmax;
	  d.thr_num = i;
	  d.data = data;
	  proc(&d);

	  for (w = workers, i = 0; i < nthr; ++i) {
	       A(w);
	       fftw_sem_wait(&w->sid_done);
	       w = w->next;
	  }

#else /* explicit thread spawning: */

	  STACK_MALLOC(spawn_data *, d, sizeof(spawn_data) * nthr);
	  STACK_MALLOC(fftw_thr_id *, tid, sizeof(fftw_thr_id) * (--nthr));

	  for (i = 0; i < nthr; ++i) {
	       d[i].max = (d[i].min = i * block_size) + block_size;
	       d[i].thr_num = i;
	       d[i].data = data;
	       fftw_thr_spawn(&tid[i], (fftw_thr_function) proc,
			      (void *) (d + i));
	  }
	  d[i].min = i * block_size;
	  d[i].max = loopmax;
	  d[i].thr_num = i;
	  d[i].data = data;
	  proc(&d[i]);
	  
	  for (i = 0; i < nthr; ++i)
	       fftw_thr_wait(tid[i]);

	  STACK_FREE(tid);
	  STACK_FREE(d);

#endif /* ! USING_COMPILER_THREADS */

	  THREAD_OFF; /* prevent debugging mode from failing under threads */
     }
}

#else /* ! HAVE_THREADS */
void X(spawn_loop)(int loopmax, int nthr,
		   spawn_function proc, void *data)
{
     spawn_data d;
     UNUSED(nthr);
     d.min = 0; d.max = loopmax;
     d.thr_num = 0;
     d.data = data;
     proc(&d);
}
#endif

/* X(ithreads_init) does any initialization that is necessary to use
   threads.  It must be called before calling any fftw threads functions.
   
   Returns 0 if successful, and non-zero if there is an error.
   Do not call any fftw threads routines if X(ithreads_init)
   is not successful! */

int X(ithreads_init)(void)
{
#ifdef USING_POSIX_THREADS
     /* Set the thread creation attributes as necessary.  If we don't
	change anything, just use the default attributes (NULL). */
     int err, attr, attr_changed = 0;

     err = pthread_attr_init(&fftw_pthread_attributes); /* set to defaults */
     if (err) return err;

     /* Make sure that threads are joinable!  (they aren't on AIX) */
     err = pthread_attr_getdetachstate(&fftw_pthread_attributes, &attr);
     if (err) return err;
     if (attr != PTHREAD_CREATE_JOINABLE) {
	  err = pthread_attr_setdetachstate(&fftw_pthread_attributes,
					    PTHREAD_CREATE_JOINABLE);
	  if (err) return err;
	  attr_changed = 1;
     }

     /* Make sure threads parallelize (they don't by default on Solaris).
	
        Note, however that the POSIX standard does not *require*
	implementations to support PTHREAD_SCOPE_SYSTEM.  They may
        only support PTHREAD_SCOPE_PROCESS (e.g. IRIX, Cygwin).  In
        this case, how the threads interact with other resources on
        the system is undefined by the standard, and we have to
        hope for the best. */
     err = pthread_attr_getscope(&fftw_pthread_attributes, &attr);
     if (err) return err;
     if (attr != PTHREAD_SCOPE_SYSTEM) {
	  err = pthread_attr_setscope(&fftw_pthread_attributes,
				      PTHREAD_SCOPE_SYSTEM);
	  if (err == 0) attr_changed = 1;
          /* ignore errors */
     }

     if (attr_changed)  /* we aren't using the defaults */
	  fftw_pthread_attributes_p = &fftw_pthread_attributes;
     else {
	  fftw_pthread_attributes_p = NULL;  /* use default attributes */
	  err = pthread_attr_destroy(&fftw_pthread_attributes);
	  if (err) return err;
     }
#endif /* USING_POSIX_THREADS */

#ifdef USING_MACOS_THREADS
     /* FIXME: don't have malloc hooks (yet) in fftw3 */
     /* Must use MPAllocate and MPFree instead of malloc and free: */
     if (MPLibraryIsLoaded()) {
	  MALLOC_hook = MPAllocate;
	  fftw_free_hook = MPFree;
     }
#endif /* USING_MACOS_THREADS */

#if defined(USING_OPENMP_THREADS) && ! defined(_OPENMP)
#error OpenMP enabled but not using an OpenMP compiler
#endif

#ifdef HAVE_THREADS
     X(mksolver_ct_hook) = X(mksolver_ct_threads);
     X(mksolver_hc2hc_hook) = X(mksolver_hc2hc_threads);
     return 0; /* no error */
#else
     return 0; /* no threads, no error */
#endif
}

/* This function must be called before using nthreads > 1, with
   the maximum number of threads that will be used. */
void X(threads_setmax)(int nthreads_max)
{
#ifdef HAVE_SEMAPHORES
     minimum_workforce(nthreads_max - 1);
#else
     UNUSED(nthreads_max);
#endif
}

void X(threads_cleanup)(void)
{
#ifdef USING_POSIX_THREADS
     if (fftw_pthread_attributes_p) {
	  pthread_attr_destroy(fftw_pthread_attributes_p);
	  fftw_pthread_attributes_p = 0;
     }
#endif /* USING_POSIX_THREADS */

#ifdef HAVE_SEMAPHORES
     kill_workforce();
#endif

#ifdef HAVE_THREADS
     X(mksolver_ct_hook) = 0;
     X(mksolver_hc2hc_hook) = 0;
#endif
}
