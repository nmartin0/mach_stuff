/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	emul_vector.s,v $
 * Revision 2.4  94/03/25  18:21:29  mrt
 * 	Changed comments to c style.
 * 	[93/11/17            mrt]
 * 
 * Revision 2.3  91/12/19  20:25:30  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:41:03  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:06:03  rwd]
 * 
 * Revision 2.2  90/05/21  13:46:29  dbg
 * 	Wrote i386 version.
 * 	[90/03/14            dbg]
 * 
 */
/*
 * Author:  Alessandro Forin (af) at Carnegie-Mellon University
 */
#include <sys/syscall.h>
#include <sys/errno.h>

/*
 * Emulator vector entry - allocate new stack
 */
	.data
	.globl	_emul_stack_lock
_emul_stack_lock:
	.long	0
	.text

/*
 * Both of these sequences must be 8 bytes long, and leave the stacked
 * PC pointing at the end of the sequence.
 */
/*
 * Generic emulator entry
 *
 * ESP->	flags
 *		pc to return to
 * system call number is in %eax
 */

/*
 * Generic call
 */
	.globl	_emul_common
_emul_common:
	pushl	%eax			/* save registers that C does not */
	pushl	%ecx
	pushl	%edx
0:	movl	$1,%eax
	xchg	%eax,_emul_stack_lock	/* lock emul_stack lock */
	orl	%eax,%eax
	jne	0b
	movl	_emul_stack_list,%eax	/* grab an emulator stack */
	orl	%eax,%eax
	jne	3f			/* if none: */
	movl	$0,_emul_stack_lock	/*   release the lock */
	call	_emul_stack_alloc	/*   allocate a stack */
	jmp	4f			/*   and use it */
3:	movl	(%eax),%ecx		/*   else */
	movl	%ecx,_emul_stack_list	/*   remove the stack from list */
	movl	$0,_emul_stack_lock	/*   and unlock the lock */
4:	movl	%esp,(%eax)		/* Save user's SP at top of new stack */
	movl	%eax,%esp		/* switch to emulator's stack */
	pushl	%ebx			/* save the remaining registers */
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	pushl	%esp			/* push address of registers */
	call	_emul_syscall		/* call C code */
	addl	$4,%esp			/* pop parameter */
/*
 * Return
 */
	.globl	emul_exit
emul_exit:
	popl	%ebp			/* restore registers */
	popl	%edi
	popl	%esi
	popl	%ebx
	movl	%esp,%ecx		/* save emulator stack address */
	movl	(%esp),%esp		/* return to user's stack */

0:	movl	$1,%eax
	xchg	%eax,_emul_stack_lock	/* lock emul_stack lock */
	orl	%eax,%eax
	jne	0b
	movl	_emul_stack_list,%eax
	movl	%eax,(%ecx)		/* chain emulator stack on list */
	movl	%ecx,_emul_stack_list
	movl	$0,_emul_stack_lock	/* release the lock */

	popl	%edx			/* restore regs that C does not save */
	popl	%ecx
	popl	%eax
	popf				/* restore flags */
	ret				/* return to user */

/*
 * Child starts executing here with return values in eax/edx,
 * stack pointing to saved edx/ecx/eax/flags/eip.
 */
	.globl	_child_fork
_child_fork:
	movl	%eax,8(%esp)		/* save eax */
	movl	%edx,(%esp)		/* and edx */
	call	_child_init		/* initialize emulator for child */
	popl	%edx			/* restore registers that C does not */
	popl	%ecx
	popl	%eax
	popf				/* restore flags */
	clc				/* clear carry for Success return */
	ret				/* return to user */

/*
 * The easiest way to exec /etc/init is to take
 * an emulator trap (it sets up the registers nicely).
 */
	.globl	_emul_execve
_emul_execve:
	movl	$ SYS_execve,%eax	/* exec system call number */
	pushl	$emul_exec_error	/* push error return address */
	pushfl				/* push flags */
	jmp	_emul_common		/* take a system call 'trap' */

	/* we only get here on failure */
emul_exec_error:
	movl	$-1,%eax		/* failure return */
	ret

/*
 * Get segment registers
 */
	.globl	_get_seg_regs
_get_seg_regs:
	movl	4(%esp),%edx	/* point to register buffer */
	xorl	%eax,%eax	/* clear high 16 bits */
	mov	%cs,%ax
	movl	%eax,0(%edx)	/* cs to regs[0] */
	mov	%ss,%ax
	movl	%eax,4(%edx)	/* ss to regs[1] */
	mov	%ds,%ax
	movl	%eax,8(%edx)	/* ds to regs[2] */
	mov	%es,%ax
	movl	%eax,12(%edx)	/* es to regs[3] */
	mov	%fs,%ax
	movl	%eax,16(%edx)	/* fs to regs[4] */
	mov	%gs,%ax
	movl	%eax,20(%edx)	/* gs to regs[5] */
	ret
