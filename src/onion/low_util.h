/*
	Onion HTTP server library
	Copyright (C) 2010-2014 David Moreno Montero and othes

	This library is free software; you can redistribute it and/or
	modify it under the terms of, at your choice:
	
	a. the Apache License Version 2.0. 
	
	b. the GNU General Public License as published by the 
		Free Software Foundation; either version 2.0 of the License, 
		or (at your option) any later version.
	 
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of both libraries, if not see 
	<http://www.gnu.org/licenses/> and 
	<http://www.apache.org/licenses/LICENSE-2.0>.
	*/

#ifndef ONION_LOW_UTIL_H
#define ONION_LOW_UTIL_H

/* File low_util.h provides low level utilities, notably wrapping
   memory allocation and thread creation. Adventurous users could even
   customize them during early initialization, e.g. when using Hans
   Boehm conservative garbage collector from http://hboehm.info/gc/ or
   when using their own malloc variant so we specialize general data -
   which might contain pointers and could be allocated with GC_MALLOC
   into scalar data which is guaranteed to be pointer free and could
   be allocated with GC_MALLOC_ATOMIC ...*/

#include <stdlib.h>
#include <signal.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /******* NEVER FAILING MEMORY ALLOCATORS *****/
  /***
      These allocators should not fail: if memory is exhausted, they
      invoke the memory failure routine then abort with a short
      failure message.
   ***/
/* Our malloc wrapper for any kind of data, including data
   containing pointers.  */
  void *onionlow_malloc (size_t sz);

/* Our malloc wrapper for scalar data which does not contain any
   pointers inside. Knowing that a given zone does not contain any
   pointer can be useful, e.g. to Hans Boehm's conservative garbage
   collector on http://hboehm.info/gc/ using GC_MALLOC_ATOMIC.... */
  void *onionlow_scalar_malloc (size_t sz);

/* Our calloc wrapper for any kind of data, even scalar one.  */
  void *onionlow_calloc (size_t nmemb, size_t size);

/* Our realloc wrapper for any kind of data, even scalar one.  */
  void *onionlow_realloc (void *ptr, size_t size);

/* Our strdup wrapper. */
  char *onionlow_strdup (const char *str);

  /***** POSSIBLY FAILING MEMORY ALLOCATORS *****/
  /***
      These allocators could fail by returning NULL. Their caller is
      requested to handle that failure.
  ***/
/* Our malloc wrapper for any kind of data, including data
   containing pointers.  */
  void *onionlow_try_malloc (size_t sz);

/* Our malloc wrapper for scalar data which does not contain any
   pointers inside.  */
  void *onionlow_try_scalar_malloc (size_t sz);

/* Our calloc wrapper for any kind of data, even scalar one.  */
  void *onionlow_try_calloc (size_t nmemb, size_t size);

/* Our realloc wrapper for any kind of data, even scalar one.  */
  void *onionlow_try_realloc (void *ptr, size_t size);

/* Our strdup wrapper. */
  char *onionlow_try_strdup (const char *str);

  /******** FREE WRAPPER ******/
/* Our free wrapper for any kind of data, even scalar one.  */
  void onionlow_free (void *ptr);

/* Signatures of user configurable memory routine replacement.  */
  typedef void *onionlow_malloc_sigt (size_t sz);
  typedef void *onionlow_scalar_malloc_sigt (size_t sz);
  typedef void *onionlow_calloc_sigt (size_t nmemb, size_t size);
  typedef void *onionlow_realloc_sigt (void *ptr, size_t size);
  typedef char *onionlow_strdup_sigt (const char *ptr);
  typedef void onionlow_free_sigt (void *ptr);

/* The memory failure handler is called with a short message. It
   generally should not return, i.e. should exit, abort, or perhaps
   setjmp.... */
  typedef void onionlow_memoryfailure_sigt (const char *msg);

/* Our configurator for memory routines. To be called once before any
   other onion processing at initialization. All the routines should
   be explicitly provided. */
  void onionlow_initialize_memory_allocation
    (onionlow_malloc_sigt * mallocrout,
     onionlow_scalar_malloc_sigt * scalarmallocrout,
     onionlow_calloc_sigt * callocrout,
     onionlow_realloc_sigt * reallocrout,
     onionlow_strdup_sigt * strduprout,
     onionlow_free_sigt * freerout,
     onionlow_memoryfailure_sigt * memoryfailurerout);


  /* We also offer a mean to wrap thread creation, join, cancel,
     detach, exit. This is needed for Boehm's garbage collector -
     which has GC_pthread_create, GC_pthread_join, ... and could be
     useful to others, e.g. for calling pthread_setname_np on Linux
     system.  There is no need to wrap mutexes... The wrapper functions
     can fail and their caller is expected to check for failure. */
#ifdef HAVE_PTHREADS
  int onionlow_pthread_create (pthread_t * thread,
			       const pthread_attr_t * attr,
			       void *(*start_routine) (void *), void *arg);
  typedef int onionlow_pthread_create_sigt (pthread_t * thread,
					    const pthread_attr_t * attr,
					    void *(*start_routine) (void *),
					    void *arg);

  int onionlow_pthread_join (pthread_t thread, void **retval);
  typedef int onionlow_pthread_join_sigt (pthread_t thread, void **retval);

  int onionlow_pthread_cancel (pthread_t thread);
  typedef int onionlow_pthread_cancel_sigt (pthread_t thread);

  int onionlow_pthread_detach (pthread_t thread);
  typedef int onionlow_pthread_detach_sigt (pthread_t thread);

  void onionlow_pthread_exit (void *retval);
  typedef void onionlow_pthread_exit_sigt (void *retval);

  int onionlow_pthread_sigmask (int how, const sigset_t * set,
				sigset_t * oldset);
  typedef int onionlow_pthread_sigmask_sigt (int how, const sigset_t * set,
					     sigset_t * oldset);

  /* Our configurator for pthread wrapping. Every routine should be
     provided. This initialization should happen early, at the same
     time as onionlow_initialize_memory_allocation, and before any
     other onion calls. If using Boehm GC you probably want to pass
     GC_pthread_create, GC_pthread_join, etc, etc... */

  void onionlow_initialize_threads
    (onionlow_pthread_create_sigt * thrcreator,
     onionlow_pthread_join_sigt * thrjoiner,
     onionlow_pthread_cancel_sigt * thrcanceler,
     onionlow_pthread_detach_sigt * thrdetacher,
     onionlow_pthread_exit_sigt * threxiter,
     onionlow_pthread_sigmask_sigt * thrsigmasker);

#endif				/*HAVE_PTHREADS */

#ifdef __cplusplus
}				/* end extern "C" */
#endif

#endif				/*ONION_LOW_UTIL_H */