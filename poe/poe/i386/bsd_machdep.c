/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	bsd_machdep.c,v $
 * Revision 2.5  94/03/25  18:22:47  mrt
 * 	Changed struct exec.a_info to a_magic to match change in
 * 	i386/exec.h.
 * 	[93/11/17            mrt]
 * 
 * Revision 2.4  92/02/02  13:02:21  rpd
 * 	Fixed for new <i386/exec.h>.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.3  91/12/19  20:28:08  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/27  13:54:18  rwd
 * 	Set efl.
 * 	[90/09/11            rwd]
 * 	Taken from XUX16 and XMK23 versions.
 * 	[90/09/10            rwd]
 * 
 */
/*
 *	File:	./i386/bsd_machdep.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <mach/message.h>
#include <errno.h>
#include <fnode.h>
#include <loader_info.h>
#include <ux_user.h>
#include <ux_param.h>
#include <i386/exec.h>
#include <i386/vmparam.h>
#include <i386/eflags.h>

/*
 *	Object:
 *		ex_get_header			EXPORTED function
 *
 *		Reads the exec header for the loader's benefit
 *
 */
int ex_get_header(fn, lp)
	struct fnode *fn;
	register struct loader_info *lp;
{
	struct exec	x;
	register int	result;
	vm_size_t	resid;

	result = FOP_READ(fn, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result)
		return (result);
	if (resid)
		return (ENOEXEC);

	switch (x.a_magic) {

	    case OMAGIC:
		lp->format = EX_READIN;
		lp->text_start  = 0;
		lp->text_size   = 0;
		lp->text_offset = 0;
		lp->data_start  = 0x10000;
		lp->data_size   = x.a_text + x.a_data;
		lp->data_offset = sizeof(struct exec);
		lp->bss_size    = x.a_bss;
		break;

	    case NMAGIC:
		if (x.a_text == 0) {
			return(ENOEXEC);
		}
		lp->format = EX_SHAREABLE;
		lp->text_start  = 0x10000;
		lp->text_size   = x.a_text;
		lp->text_offset = sizeof(struct exec);
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = x.a_data;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a_bss;
		break;

	    case ZMAGIC:
		if (x.a_text == 0) {
			return(ENOEXEC);
		}
		lp->format = EX_PAGEABLE;
		lp->text_start  = 0x10000;
		lp->text_size   = sizeof(struct exec) + x.a_text;
		lp->text_offset = 0;
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = x.a_data;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a_bss;
		break;
	    default:
		return (ENOEXEC);
	}
	lp->entry_1 = x.a_entry;
	lp->entry_2 = 0;

	return(0);
}

xx_thread_set_state(user_thread, lp, arg_pos)
	mach_port_t user_thread;
	struct loader_info *lp;
	int arg_pos;
{
	struct i386_thread_state regs;
	kern_return_t kr;
	unsigned int		reg_size;

	reg_size = i386_THREAD_STATE_COUNT;
	kr = thread_get_state(user_thread,
				i386_THREAD_STATE,
				(thread_state_t)&regs,
				&reg_size);
	if (kr) mach_error("vm_get_state", kr);


	dprintf("entry = %x\n\r",lp->entry_1);
	regs.eip = lp->entry_1;
	regs.uesp = arg_pos;
	regs.efl = EFL_USER_SET;

	kr = thread_set_state(user_thread,
				i386_THREAD_STATE,
				(thread_state_t)&regs,
				reg_size);
	if (kr) mach_error("thread_set_state", kr);
}

/* XXX this should be made obsolete */
thread_get_pc(thread, pc)
	mach_port_t thread;
	int *pc;
{
	struct i386_thread_state regs;
	int error;
	unsigned int reg_size;

	reg_size = i386_THREAD_STATE_COUNT;
	error = thread_get_state(thread, i386_THREAD_STATE,
				(thread_state_t)&regs, &reg_size);
	if (error) {
		return error;
	}
	*pc = regs.eip;
	return 0;
}

/*This is obsolete, now that ptrace sorta works*/

thread_dump(thread)
	mach_port_t thread;
{
	struct i386_thread_state state;
	mach_msg_type_number_t count;
	register kern_return_t kr;

	printf("i386 thread_dump(%x)\n\r",thread);
	count = i386_THREAD_STATE_COUNT;
	kr = thread_get_state(thread, i386_THREAD_STATE,
			     (thread_state_t) &state, &count);
	if (kr != KERN_SUCCESS || count != i386_THREAD_STATE_COUNT)
		return 0;

	printf("gs = 0x%8.8x\tfs = 0x%8.8x\tes = 0x%8.8x\tds= 0x%8.8x\n\r",
	       state.gs, state.fs, state.es, state.ds);
	printf("edi= 0x%8.8x\tesi= 0x%8.8x\tebp= 0x%8.8x\tesp= 0x%8.8x\n\r",
	       state.edi, state.esi, state.ebp, state.esp);
	printf("ebx= 0x%8.8x\tedx= 0x%8.8x\tecx= 0x%8.8x\teax= 0x%8.8x\n\r",
	       state.ebx, state.edx, state.ecx, state.eax);
	printf("eip= 0x%8.8x\tcs = 0x%8.8x\tefl= 0x%8.8x\tuesp=0x%8.8x\n\r",
	       state.eip, state.cs, state.efl, state.uesp);
	printf("ss = 0x%8.8x\n\r",
	       state.ss);
	return 0;
}

get_threadstate(ut)
	struct ux_task *ut;
{
	int error, size;
	struct i386_thread_state *sr;

	if (ut->ut_threadstate) {
		return 0;
	}
	sr = (struct i386_thread_state *) malloc(sizeof(*sr));
	size = i386_THREAD_STATE_COUNT;
	error = thread_get_state(ut->ut_thread, i386_THREAD_STATE,
				 (thread_state_t) sr, &size);
	if (error) {
		free(sr);
printf("AAA.2\n\r");
		return ESRCH;
	}
	ut->ut_threadstate = (char *) sr;
	return 0;
}

set_entry_address(lp, entry, entry_count)
	struct loader_info *lp;
	int		*entry;		/* pointer to OUT array */
	unsigned int	*entry_count;	/* out */
{
	entry[0] = lp->entry_1;
	*entry_count = 1;
}

/*
 * Clone the parent's registers into the child thread for fork.
 */
boolean_t
thread_dup(child_thread, new_state, new_state_count, parent_pid, rc)
	thread_t	child_thread;
	thread_state_t	new_state;
	unsigned int	new_state_count;
	int		parent_pid, rc;
{
	struct i386_thread_state *regs = (struct i386_thread_state *)new_state;

	if (new_state_count != i386_THREAD_STATE_COUNT)
	    return (KERN_INVALID_ARGUMENT);

	regs->eax = parent_pid;
	regs->edx = rc;

	(void) thread_set_state(child_thread, i386_THREAD_STATE,
				new_state, new_state_count);
	return (KERN_SUCCESS);
}

kern_return_t
machine_deallocate_for_exec(task)
task_t task;
{
	kern_return_t error;

	error = vm_deallocate(task, 0, EMULATOR_BASE);
	if (error) return error;
	error = vm_deallocate(task, EMULATOR_END,
			      STACK_END - STACK_SIZE - EMULATOR_END);
	if (error) return error;
}

int
machine_adjust_for_emulator(li)
struct loader_info *li;
{
	li->data_start += EMULATOR_BASE - li->text_start;
	li->text_start = EMULATOR_BASE;
	return 0;
}
