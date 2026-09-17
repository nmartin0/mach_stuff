/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: exception.h,v $
 * Revision 1.2.15.5  1995/04/07  19:05:02  barbou
 * 	Backed out previous submission.
 * 	[95/03/29            barbou]
 *
 * Revision 1.2.15.4  1995/03/15  17:19:40  bruel
 * 	EXC_TYPES_COUNT is machine independant.
 * 	(the machine exception type is given in the code argument).
 * 	[95/03/06            bruel]
 * 
 * Revision 1.2.15.3  1995/01/10  05:16:13  devrcs
 * 	mk6 CR801 - merge up from nmk18b4 to nmk18b7
 * 	* Rev 1.2.13.3  1994/11/08  21:50:43  rkc
 * 	  Define symbols related to alert processing.
 * 	[1994/12/09  21:11:17  dwm]
 * 
 * 	mk6 CR668 - 1.3b26 merge
 * 	64bit: exception_data_type_t ==> integer_t
 * 	[1994/10/14  03:42:34  dwm]
 * 
 * Revision 1.2.15.1  1994/09/23  02:36:11  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:39:37  ezf]
 * 
 * Revision 1.2.2.4  1993/09/03  15:53:52  jeffc
 * 	CR9255 - Remove MACH_EXC_COMPAT
 * 	[1993/08/26  15:10:53  jeffc]
 * 
 * Revision 1.2.2.3  1993/08/03  18:29:27  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	[1993/08/02  17:49:44  gm]
 * 
 * Revision 1.2.2.2  1993/06/09  02:40:09  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:15:55  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:33:16  devrcs
 * 	changes for EXC_MACH_SYSCALL
 * 	[1993/04/05  12:06:01  david]
 * 
 * 	ansi C conformance changes
 * 	[1993/02/02  18:52:54  david]
 * 
 * 	Added types for new exception interface.
 * 	[1992/12/23  13:08:31  david]
 * 
 * Revision 1.1  1992/09/30  02:30:36  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.3  91/05/14  16:51:41  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:31:55  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:17:06  mrt]
 * 
 * Revision 2.1  89/08/03  16:02:18  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:13:29  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:52:07  mwyoung
 * Relocated from sys/exception.h
 * 
 * Revision 2.2  88/08/24  02:26:52  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:12:09  mwyoung]
 * 
 *
 * 29-Sep-87  David Black (dlb) at Carnegie-Mellon University
 *	Created.
 *
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
/*
 */

#ifndef	EXCEPTION_H_
#define	EXCEPTION_H_

#ifndef	ASSEMBLER
#include <mach/port.h>
#include <mach/thread_status.h>
#include <mach/machine/vm_types.h>
#endif	/* ASSEMBLER */
#include <mach/machine/exception.h>

/*
 *	Machine-independent exception definitions.
 */


#define EXC_BAD_ACCESS		1	/* Could not access memory */
		/* Code contains kern_return_t describing error. */
		/* Subcode contains bad memory address. */

#define EXC_BAD_INSTRUCTION	2	/* Instruction failed */
		/* Illegal or undefined instruction or operand */

#define EXC_ARITHMETIC		3	/* Arithmetic exception */
		/* Exact nature of exception is in code field */

#define EXC_EMULATION		4	/* Emulation instruction */
		/* Emulation support instruction encountered */
		/* Details in code and subcode fields	*/

#define EXC_SOFTWARE		5	/* Software generated exception */
		/* Exact exception is in code field. */
		/* Codes 0 - 0xFFFF reserved to hardware */
		/* Codes 0x10000 - 0x1FFFF reserved for OS emulation (Unix) */

#define EXC_BREAKPOINT		6	/* Trace, breakpoint, etc. */
		/* Details in code field. */

#define EXC_SYSCALL		7	/* System calls. */

#define EXC_MACH_SYSCALL	8	/* Mach system calls. */

#define EXC_RPC_ALERT		9	/* RPC alert */

/*
 *	Machine-independent exception behaviors
 */

# define EXCEPTION_DEFAULT		1
/*	Send a catch_exception_raise message including the identity.
 */

# define EXCEPTION_STATE		2
/*	Send a catch_exception_raise_state message including the
 *	thread state.
 */

# define EXCEPTION_STATE_IDENTITY	3
/*	Send a catch_exception_raise_state_identity message including
 *	the thread identity and state.
 */

/*
 * Masks for exception definitions, above
 * bit zero is unused, therefore 1 word = 31 exception types
 */

#define EXC_MASK_BAD_ACCESS		(1 << EXC_BAD_ACCESS)
#define EXC_MASK_BAD_INSTRUCTION	(1 << EXC_BAD_INSTRUCTION)
#define EXC_MASK_ARITHMETIC		(1 << EXC_ARITHMETIC)
#define EXC_MASK_EMULATION		(1 << EXC_EMULATION)
#define EXC_MASK_SOFTWARE		(1 << EXC_SOFTWARE)
#define EXC_MASK_BREAKPOINT		(1 << EXC_BREAKPOINT)
#define EXC_MASK_SYSCALL		(1 << EXC_SYSCALL)
#define EXC_MASK_MACH_SYSCALL		(1 << EXC_MACH_SYSCALL)
#define EXC_MASK_RPC_ALERT		(1 << EXC_RPC_ALERT)

#define EXC_MASK_ALL	(EXC_MASK_BAD_ACCESS |			\
			 EXC_MASK_BAD_INSTRUCTION |		\
			 EXC_MASK_ARITHMETIC |			\
			 EXC_MASK_EMULATION |			\
			 EXC_MASK_SOFTWARE |			\
			 EXC_MASK_BREAKPOINT |			\
			 EXC_MASK_SYSCALL |			\
			 EXC_MASK_MACH_SYSCALL |		\
			 EXC_MASK_RPC_ALERT |			\
			 EXC_MASK_MACHINE)


#define FIRST_EXCEPTION		1	/* ZERO is illegal */

#ifndef	ASSEMBLER


/*
 * Exported types
 */

typedef int			exception_type_t;
typedef	integer_t		exception_data_type_t;
typedef int			exception_behavior_t;
typedef integer_t		*exception_data_t;
typedef	unsigned int		exception_mask_t;
typedef	exception_mask_t	*exception_mask_array_t;
typedef exception_behavior_t	*exception_behavior_array_t;
typedef thread_state_flavor_t	*exception_flavor_array_t;
# ifdef MACH_KERNEL		/* stupid MIG */
typedef struct	ipc_port	**exception_port_array_t;
# else
typedef mach_port_t		*exception_port_array_t;
# endif /* MACH_KERNEL */

# ifdef MACH_KERNEL
/*
 * Exception structures
 */

struct exception_action{
	struct ipc_port		*port;		/* exception port */
	thread_state_flavor_t	flavor;		/* state flavor to send */
	exception_behavior_t	behavior;	/* exception type to raise */
};
# endif /* MACH_KERNEL */
#endif	/* ASSEMBLER */
#endif	/* EXCEPTION_H_ */
