/* ==== machdep.h ============================================================
 * Copyright (c) 1993 Chris Provenzano, proven@athena.mit.edu
 *
 * machdep.h,v 1.2 1995/09/13 02:20:26 hjl Exp
 */

#ifndef _PTHREAD_MIT_MACHDEP_H
#define _PTHREAD_MIT_MACHDEP_H

#include <unistd.h>
#include <setjmp.h>
#include <sys/time.h>

/*
 * The first machine dependent functions are the SEMAPHORES
 * needing the test and set instruction.
 */
#define SEMAPHORE_CLEAR 0
#define SEMAPHORE_SET   1

#ifdef __i386__
#define SEMAPHORE_TEST_AND_SET(lock)    \
({										\
volatile long temp = SEMAPHORE_SET;     \
										\
__asm__("xchgl %0,(%2)"                 \
        :"=r" (temp)                    \
        :"0" (temp),"r" (lock));        \
temp;                                   \
})
#endif

#ifdef __mc68000__
#define SEMAPHORE_TEST_AND_SET(lock)    \
({					\
long temp = SEMAPHORE_SET;		\
					\
__asm__ __volatile__			\
	("movel %2@,%0\n\t"		\
	 "1:\t"				\
	 "casl %0,%1,%2@\n\t"		\
	 "jne 1b"			\
	 : "=&d" (temp)			\
	 : "d" (temp), "a" (lock));	\
temp;                                   \
})
#endif

#define SEMAPHORE_RESET(lock)           *lock = SEMAPHORE_CLEAR

/*
 * New types
 */
typedef long    semaphore;

#define SIGMAX	31

/*
 * New Strutures
 */
struct machdep_pthread {
    void        		*(*start_routine)(void *);
    void        		*start_argument;
    void        		*machdep_stack;
	struct itimerval	machdep_timer;
    jmp_buf     		machdep_state;
#ifdef __i386__
    char 	    		machdep_float_state[108];
#endif
#ifdef __mc68000__
    char 	    		machdep_float_state[8*12];
#endif
};

/*
 * Static machdep_pthread initialization values.
 * For initial thread only.
 */
#define MACHDEP_PTHREAD_INIT    \
{ NULL, NULL, NULL, { { 0, 0 }, { 0, 100000 } }, 0 }

/*
 * Minimum stack size
 */
#define PTHREAD_STACK_MIN	1024

/*
 * sigset_t macros
 */
#define	SIG_ANY(sig)		(sig)

/*
 * Some fd flag defines that are necessary to distinguish between posix
 * behavior and bsd4.3 behavior.
 */
#define __FD_NONBLOCK 		O_NONBLOCK

/*
 * New functions
 */

__BEGIN_DECLS

#if defined(PTHREAD_KERNEL)

int machdep_save_state      __P((void));

#endif

__END_DECLS

#endif /* _PTHREAD_MIT_MACHDEP_H */
