/*
 * @OSF_FREE_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: cthreads.h,v $
 * Revision 1.1.12.3  1996/07/31  07:30:10  paire
 * 	Merged with nmk20b7_shared (1.1.17.1)
 * 	[96/05/29            paire]
 *
 * Revision 1.1.17.1  1996/04/12  06:20:12  paire
 * 	Added prototypes for spin_try_lock() and cthread_sp().
 * 	[96/01/29            paire]
 * 
 * Revision 1.1.12.2  1996/02/05  17:36:32  emcmanus
 * 	Copied from nmk20b5_shared.
 * 	[1996/02/05  17:23:18  emcmanus]
 * 
 * Revision 1.1.14.2  1996/01/02  13:33:40  bruel
 * 	fixed spin_try_lock __asm__ [paire].
 * 	[96/01/02            bruel]
 * 
 * Revision 1.1.14.1  1995/12/30  18:20:07  emcmanus
 * 	Provided inline gcc __asm__ versions of spin_try_lock() and
 * 	cthread_sp(), as is done on the i386.
 * 	[1995/12/30  18:12:07  emcmanus]
 * 
 * Revision 1.1.12.1  1995/03/24  13:59:06  emcmanus
 * 	CR1149: Merged HP/PA version into mainline.
 * 	Defined cthread_context_t and changed context parameters to
 * 	use it.  cthread_user_build now returns a value, used by
 * 	cthread_join.
 * 	[1995/03/24  13:58:14  emcmanus]
 * 
 * Revision 1.1.10.2  1995/02/23  14:13:46  emcmanus
 * 	hppa merge was broken.
 * 	[1995/02/23  14:02:28  emcmanus]
 * 
 * Revision 1.1.10.1  1995/02/06  07:57:56  bruel
 * 	hppa merge.
 * 	[1995/02/06  07:53:54  bruel]
 * 
 * Revision 1.1.7.4  1994/12/02  09:43:21  bruel
 * 	disabled debuging code.
 * 	[94/12/02            bruel]
 * 
 * Revision 1.1.7.3  1994/07/11  16:43:37  emcmanus
 * 	Define CTHREAD_STACK_FREE_START and CTHREAD_STACK_FREE_END.
 * 	Correct definition of STATE_STACK, although this interface is
 * 	not currently used.
 * 	[1994/07/11  16:43:04  emcmanus]
 * 
 * Revision 1.1.7.2  1994/06/24  16:27:13  emcmanus
 * 	Merged with changes from 1.1.7.1
 * 	[1994/06/24  16:26:59  emcmanus]
 * 
 * 	Added machine-dependent cthread_filter macro.  Passing 6 arguments
 * 	to a function that switches to determine its action is much less
 * 	efficient than passing 4 or fewer arguments to individual functions.
 * 	[1994/06/24  16:24:36  emcmanus]
 * 
 * Revision 1.1.7.1  1994/06/07  13:47:34  bruel
 * 	added state defines.
 * 	[94/06/07            bruel]
 * 
 * Revision 1.1.2.1  1994/02/11  15:16:43  bruel
 * 	Created from Utah.
 * 	[93/11/26            bruel]
 * 
 * $EndLog$
 */
/* 
 * Copyright (c) 1990,1991 The University of Utah and
 * the Center for Software Science (CSS).  All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the Center
 * for Software Science at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSS ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSS DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSS requests users of this software to return to css-dist@cs.utah.edu any
 * improvements that they make and grant CSS redistribution rights.
 *
 * 	Utah $Hdr$
 *	Author: University of Utah CSS
 */

#ifndef _MACHINE_CTHREADS_H_
#define _MACHINE_CTHREADS_H_

typedef struct { int _ldcws[4]; } spin_lock_t;

#define SPIN_LOCK_INITIALIZER { { -1, -1, -1, -1 } }

/*
 * At the base of the stack we allocate a region of 128 bytes.  The
 * pointer to the cthread structure goes at the start of this region,
 * and the continuation context goes at the end.  The space in between
 * is free.
 */
#define CTHREAD_STACK_OFFSET	 128
#define CTHREAD_STACK_FREE_START sizeof(cthread_t)
#define CTHREAD_STACK_FREE_END	 (CTHREAD_STACK_OFFSET - (2 * sizeof(int)))

/*
 * we'll always assign a block of 16 bytes to the spinlock and then assume
 * that the real lock is the one aligned on a 16 byte boundary
 */
#define	_align_spin_lock(s) \
  ((int *)(((int) (s) + sizeof(spin_lock_t) - 1) & ~(sizeof(spin_lock_t) - 1)))

#if 0
/* XXX for debugging */
#define spin_lock_init(s) \
	((s)->_ldcws[0]=(s)->_ldcws[1]=(s)->_ldcws[2]=(s)->_ldcws[3] = -1)
#else
#define spin_lock_init(s) (*_align_spin_lock(s) = -1)
#endif
#define spin_lock_locked(s) (*_align_spin_lock(s) == 0)
#define spin_unlock(s) spin_lock_init(s)

extern int spin_try_lock(spin_lock_t *);
extern void *cthread_sp(void);

#if __GNUC__ && !_NO_INLINE_SPINLOCKS && !defined(lint)

extern __inline__ int
spin_try_lock(spin_lock_t *p)
{
	int _r__;

	__asm__ volatile("ldcws 0(%2),%0" : "=r"(_r__), "=m"(*p) :
					    "r"(_align_spin_lock(p)));
	return (_r__ != 0);
}

extern __inline__ void *
cthread_sp(void)
{
	void *_sp__;

	__asm__ ("copy %%r30,%0" : "=r" (_sp__));
	return _sp__;
}

#endif	/* __GNUC__ */


#define STATE_FLAVOR HP700_THREAD_STATE
#define STATE_COUNT HP700_THREAD_STATE_COUNT
typedef struct hp700_thread_state thread_state;
#define STATE_STACK(state) ((state)->r30)

#define cthread_filter(contextp, action, arg1, arg2, arg3, arg4) \
	CTHREAD_FILTER_CALL_ ## action(contextp, arg1, arg2, arg3, arg4)
/*
 * Unfortunately cpp expands the `action' to an integer, when we would
 * prefer to leave it as CTHREAD_FILTER_USER_BUILD or whatever.  This is
 * a consequence of going through more than one level of macros.  But
 * hardwiring the constants here is no worse than the hardwired numbers
 * in AT386/csw.s.
 *
 * The machine-independent CTHREAD_FILTER macro wants to force everything
 * into `void *', which is vexing because we have to cast some of it back,
 * and we lose typechecking.
 */

typedef vm_offset_t cthread_context_t;

extern void *cthread_user_build(cthread_context_t *ctxp,
				void (*f)(void *, void *), void *a1, void *a2);
#define CTHREAD_FILTER_CALL_0(ctxp, arg1, arg2, arg3, arg4) \
	cthread_user_build(ctxp, (void (*)(void *, void *))arg1, arg2, arg3)

extern void cthread_internal_build(cthread_context_t ctx, void (*f)(void *),
				   void *a1, spin_lock_t *lockp, int newctx);
#define CTHREAD_FILTER_CALL_1(ctxp, arg1, arg2, arg3, arg4) \
	cthread_internal_build(*ctxp, (void (*)(void *))arg1, arg2, arg3, \
			       *(int *)arg4)

/* The `result' parameter is not currently used, so we don't pass it. */
extern void cthread_user_invoke(cthread_context_t ctx);
#define CTHREAD_FILTER_CALL_3(ctxp, arg1, arg2, arg3, arg4) \
	cthread_user_invoke(*ctxp)

extern void cthread_prepare(cthread_context_t ctx, void (*f)(void *), void *a1);
#define CTHREAD_FILTER_CALL_4(ctxp, arg1, arg2, arg3, arg4) \
	cthread_prepare(*ctxp, (void (*)(void *))arg1, arg2)

extern void cthread_invoke_self(cthread_context_t ctx, void (*f)(void *),
				void *a1);
#define CTHREAD_FILTER_CALL_7(ctxp, arg1, arg2, arg3, arg4) \
	cthread_invoke_self(*ctxp, (void (*)(void *))arg1, arg2)

#endif /* _MACHINE_CTHREADS_H_ */
