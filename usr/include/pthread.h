#ifndef _PTHREAD_H_
#define	_PTHREAD_H_

#ifndef _POSIX_THREADS
#define _POSIX_THREADS
#endif

#ifdef _MIT_POSIX_THREADS
#include <pthread/mit/pthread.h>
#endif

extern int __pthreaded;

/* Used to test if we are MT or not. */
#define threaded()	__pthreaded

#endif
