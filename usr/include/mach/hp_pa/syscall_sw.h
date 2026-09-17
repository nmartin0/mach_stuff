/*
 * @OSF_FREE_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: syscall_sw.h,v $
 * Revision 1.1.16.1  1997/06/02  17:01:49  bruel
 * 	Linux self build: changed 'gas' separators and comments.
 * 	[97/06/02            bruel]
 *
 * Revision 1.1.9.3  1995/11/02  14:59:30  bruel
 * 	there is no frame pointer when compiling with -O (new gcc).
 * 	cleaned up for gas.
 * 	[95/09/21            bruel]
 * 
 * Revision 1.1.9.2  1995/08/21  20:45:42  devrcs
 * 	CR1428: Added "stw %r4,(%sp)" to the template for kernel_trap and
 * 	rpc_trap.  This helps gdb find the caller's stack frame.
 * 	[1995/06/26  14:03:21  emcmanus]
 * 
 * Revision 1.1.9.1  1995/03/15  17:48:08  bruel
 * 	hppa merge
 * 	[1995/03/15  09:45:52  bruel]
 * 
 * Revision 1.1.2.1  1994/02/11  13:28:24  bruel
 * 	Created from Utah.
 * 	[93/11/30            bruel]
 * 
 * $EndLog$
 */
/*
 * Mach Operating System
 * Copyright (c) 1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#ifndef	_MACHINE_SYSCALL_SW_
#define	_MACHINE_SYSCALL_SW_	1

	.space	$TEXT$
	.subspa	$CODE$

#define kernel_trap(trap_name,trap_number,number_args)\
trap_name				        ; \
	.proc					; \
	.callinfo				; \
	ldil	L%0xC0000000,%r1		; \
	ble	4(%sr7,%r1)			; \
	ldi	trap_number,%r22		; \
	bv      0(%r2)                          ; \
        nop                                     ; \
	.procend				; \
	.export	trap_name

#define rpc_trap(trap_name,trap_number,number_args)\
trap_name					; \
	.proc					; \
	.callinfo				; \
	ldil	L%0xC0000000,%r1		; \
	ble	4(%sr7,%r1)			; \
	ldi	trap_number,%r22		; \
	bv      0(%r2)                          ; \
        nop                                     ; \
	.procend				; \
	.export	trap_name

#endif	/* _MACHINE_SYSCALL_SW_ */









