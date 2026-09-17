/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: fake_interrupt.h,v $
 * Revision 1.1.2.1  1996/09/09  16:57:27  barbou
 * 	Created.
 * 	[1996/08/21  16:32:33  barbou]
 *
 * $EndLog$
 */

/*
 * Manages fake interrupts to force user threads running in user mode to
 * go back under server control.
 */

#ifndef	_OSFMACH3_FAKE_INTERRUPT_H_
#define _OSFMACH3_FAKE_INTERRUPT_H_

extern void generate_fake_interrupt(struct task_struct *p);
extern void cancel_fake_interrupt(struct task_struct *p);

#endif	/* _OSFMACH3_FAKE_INTERRUPT_H_ */
