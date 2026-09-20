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
 * Revision 2.4  91/12/19  20:28:43  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:55:20  rwd
 * 	Fix typo.
 * 	[90/09/10  20:00:15  rwd]
 * 
 * Revision 2.2  90/09/08  00:19:37  rwd
 * 	Added machine_deallocate_for_exec(). Added
 * 	machine_adjust_for_emulator().
 * 	[90/07/14            rwd]
 * 
 */
/*
 *	File:	./mips/bsd_machdep.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <fnode.h>
#include <loader_info.h>
#include <ux_user.h>
#include <mips/coff.h>
#include <ux_param.h>
#include <mips/vmparam.h>

/*
 *	Object:
 *		ex_get_header			EXPORTED function
 *
 *		Reads the exec header for the loader's benefit
 *
 */
int ex_get_header(fn, lp)
	struct fnode	*fn;
	struct loader_info	*lp;
{
	struct {
		struct filehdr	f;
		struct aouthdr	a;
	} x;
	register int	result;
	vm_size_t	resid;


	result = FOP_READ(fn, 0, &x, sizeof(x), &resid);
	if (result)
		return (result);
	if (resid || (x.f.f_magic != MIPSMAGIC))
		return (ENOEXEC);

	switch (x.a.magic) {

	    case 0407:
		lp->format = EX_READIN;
		lp->text_start  = 0;
		lp->text_size   = 0;
		lp->text_offset = 0;
		lp->data_start  = x.a.text_start;
		lp->data_size   = x.a.tsize + x.a.dsize;
		lp->data_offset = N_TXTOFF(x.f,x.a);
		lp->bss_size    = x.a.bsize;
		break;

	    case 0410:
	    case 0413:
		if (x.a.tsize == 0) {
			return(ENOEXEC);
		}
		lp->format = (x.a.magic == 0410) ? EX_SHAREABLE : EX_PAGEABLE;
		lp->text_start  = x.a.text_start;
		lp->text_size   = x.a.tsize;
		lp->text_offset = N_TXTOFF(x.f,x.a);
		lp->data_start  = x.a.data_start;
		lp->data_size   = x.a.dsize;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a.bsize;
		break;

	    default:
		return (ENOEXEC);
	}
	lp->entry_1 = x.a.entry;
	lp->entry_2 = x.a.gp_value;

	return(0);
}

xx_thread_set_state(user_thread, lp, arg_pos)
	mach_port_t user_thread;
	struct loader_info *lp;
	int arg_pos;
{
	struct mips_thread_state regs;
	kern_return_t kr;
	unsigned int		reg_size;

	reg_size = MIPS_THREAD_STATE_COUNT;
	kr = thread_get_state(user_thread,
				MIPS_THREAD_STATE,
				(thread_state_t)&regs,
				&reg_size);
	if (kr) mach_error("vm_get_state", kr);


	regs.pc = lp->entry_1;
	regs.r28 = lp->entry_2;
	regs.r29 = arg_pos;

	kr = thread_set_state(user_thread,
				MIPS_THREAD_STATE,
				(thread_state_t)&regs,
				reg_size);
	if (kr) mach_error("thread_set_state", kr);
}

/* XXX this should be made obsolete */
thread_get_pc(thread, pc)
	mach_port_t thread;
	int *pc;
{
	struct mips_thread_state regs;
	int error;
	unsigned int reg_size;

	reg_size = MIPS_THREAD_STATE_COUNT;
	error = thread_get_state(thread, MIPS_THREAD_STATE,
				(thread_state_t)&regs, &reg_size);
	if (error) {
		return error;
	}
	*pc = regs.pc;
	return 0;
}

#if XXXXXXXXXXXXXXXXXXXXXXXXXXXX
#else

/*This is obsolete, now that ptrace sorta works*/

thread_dump(thread)
	mach_port_t thread;
{
	struct mips_thread_state regs;
	int error;
	unsigned int reg_size;

	reg_size = MIPS_THREAD_STATE_COUNT;
	error = thread_get_state(thread, MIPS_THREAD_STATE,
				(thread_state_t)&regs, &reg_size);
	if (error) {
		return error;
	}
	printf("\tat\t= 0x%x\n", regs.r1);
	printf("\tv0\t= 0x%x\n", regs.r2);
	printf("\tv1\t= 0x%x\n", regs.r3);
	printf("\ta0\t= 0x%x\n", regs.r4);
	printf("\ta1\t= 0x%x\n", regs.r5);
	printf("\ta2\t= 0x%x\n", regs.r6);
	printf("\ta3\t= 0x%x\n", regs.r7);
	printf("\tt0\t= 0x%x\n", regs.r8);
	printf("\tt1\t= 0x%x\n", regs.r9);
	printf("\tt2\t= 0x%x\n", regs.r10);
	printf("\tt3\t= 0x%x\n", regs.r11);
	printf("\tt4\t= 0x%x\n", regs.r12);
	printf("\tt5\t= 0x%x\n", regs.r13);
	printf("\tt6\t= 0x%x\n", regs.r14);
	printf("\tt7\t= 0x%x\n", regs.r15);
	printf("\ts0\t= 0x%x\n", regs.r16);
	printf("\ts1\t= 0x%x\n", regs.r17);
	printf("\ts2\t= 0x%x\n", regs.r18);
	printf("\ts3\t= 0x%x\n", regs.r19);
	printf("\ts4\t= 0x%x\n", regs.r20);
	printf("\ts5\t= 0x%x\n", regs.r21);
	printf("\ts6\t= 0x%x\n", regs.r22);
	printf("\ts7\t= 0x%x\n", regs.r23);
	printf("\tt8\t= 0x%x\n", regs.r24);
	printf("\tt9\t= 0x%x\n", regs.r25);
	printf("\tk0\t= 0x%x\n", regs.r26);
	printf("\tk1\t= 0x%x\n", regs.r27);
	printf("\tgp\t= 0x%x\n", regs.r28);
	printf("\tsp\t= 0x%x\n", regs.r29);
	printf("\tfp\t= 0x%x\n", regs.r30);
	printf("\tra\t= 0x%x\n", regs.r31);
	printf("\tlmul\t= 0x%x\n", regs.mdlo);
	printf("\thmul\t= 0x%x\n", regs.mdhi);
	printf("\tpc\t= 0x%x\n", regs.pc);
	return 0;
}
#endif XXXXXXXXXXXXXXXXXXXXXXXXXXXX

get_threadstate(ut)
	struct ux_task *ut;
{
	int error, size;
	struct mips_regs {
		struct mips_thread_state	ts;
		struct mips_float_state		fs;
		struct mips_exc_state		es;
	} *mr;

	if (ut->ut_threadstate) {
		return 0;
	}
	mr = (struct mips_regs *) malloc(sizeof(*mr));
	size = MIPS_THREAD_STATE_COUNT;
	error = thread_get_state(ut->ut_thread, MIPS_THREAD_STATE,
				 &mr->ts, &size);
	if (error) {
		free(mr);
printf("AAA.2\n");
		return ESRCH;
	}
	size = MIPS_FLOAT_STATE_COUNT;
	error = thread_get_state(ut->ut_thread, MIPS_FLOAT_STATE,
				 &mr->fs, &size);
	if (error) {
		free(mr);
printf("AAA.3\n");
		return ESRCH;
	}
	size = MIPS_EXC_STATE_COUNT;
	error = thread_get_state(ut->ut_thread, MIPS_EXC_STATE,
				 &mr->es, &size);
	if (error) {
		free(mr);
printf("AAA.4\n");
		return ESRCH;
	}
	ut->ut_threadstate = (char *) mr;
	return 0;
}

set_entry_address(lp, entry, entry_count)
	struct loader_info *lp;
	int		*entry;		/* pointer to OUT array */
	unsigned int	*entry_count;	/* out */
{
	entry[0] = lp->entry_1;
	entry[1] = lp->entry_2;
	*entry_count = 2;
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
	struct mips_thread_state *regs = (struct mips_thread_state *)new_state;

	if (new_state_count != MIPS_THREAD_STATE_COUNT) {
		return KERN_INVALID_ARGUMENT;
	}
	regs->r2 = parent_pid;
	regs->r3 = rc;
	regs->r7 = 0;
	return thread_set_state(child_thread, MIPS_THREAD_STATE,
				new_state, new_state_count);
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
	return 0;
}
