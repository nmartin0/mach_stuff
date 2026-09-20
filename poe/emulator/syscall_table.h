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
 * $Log:	syscall_table.h,v $
 * Revision 2.3  91/12/19  20:25:47  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:41:47  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:21:33  rwd]
 * 
 * Revision 2.3  90/06/19  23:07:11  rpd
 * 	Added pid_by_task.
 * 	[90/06/14            rpd]
 * 
 * Revision 2.2  89/10/17  11:24:24  rwd
 * 	Added sysent_init_process.
 * 	[89/09/29            rwd]
 * 
 */
/*
 * Definition of system call table.
 */
struct sysent {
	int	nargs;		/* number of arguments, or special code */
	int	(*routine)();
};

/*
 * Special arguments:
 */
#define	E_GENERIC	(-1)
				/* no specialized routine */
#define	E_CHANGE_REGS	(-2)
				/* may change registers */

/*
 * Exported system call table
 */
extern struct sysent	sysent[];	/* normal system calls */
extern int		nsysent;

extern struct sysent	cmusysent[];	/* CMU extensions */
extern int		ncmusysent;

extern struct sysent	sysent_task_by_pid;
extern struct sysent	sysent_pid_by_task;
extern struct sysent	sysent_htg_ux_syscall;
extern struct sysent	sysent_init_process;
