/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
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
