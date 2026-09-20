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
 * $Log:	syscall_table.c,v $
 * Revision 2.4  92/02/02  13:01:24  rpd
 * 	Removed MAP_FILE and MAP_UAREA code.
 * 	[92/01/30            rpd]
 * 
 * Revision 2.3  91/12/19  20:25:45  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:41:41  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:21:13  rwd]
 * 
 * Revision 2.9  90/06/19  23:07:03  rpd
 * 	Added pid_by_task.
 * 	[90/06/14            rpd]
 * 
 * Revision 2.8  90/05/29  20:22:36  rwd
 * 	Make all path_name calls go through glue routines to pass length
 * 	to MiG interfaces.
 * 	[90/04/06            dbg]
 * 
 * 	Add lseek for MAP_FILE case.
 * 	[90/03/27            rwd]
 * 
 * Revision 2.7  90/05/21  13:48:19  dbg
 * 	sysents at end of file cannot be initialized with array
 * 	constructors (gcc).
 * 	[90/04/20            dbg]
 * 
 * 	Make all path_name calls go through glue routines to pass length
 * 	to MiG interfaces.
 * 	[90/04/06            dbg]
 * 
 * Revision 2.6  90/03/14  21:24:22  rwd
 * 	Add sigsetmask to mapped calls.
 * 	[90/02/23            rwd]
 * 	Add MAP_UAREA version of obreak.
 * 	[90/01/23            rwd]
 * 
 * Revision 2.5  89/12/08  20:14:01  rwd
 * 	Changed mkdir to 2 params
 * 	[89/11/01            rwd]
 * 
 * Revision 2.4  89/11/29  15:26:58  af
 * 	Made sigvec take 4 args.
 * 	Conditionally expand the table (mips) to 256 entries.
 * 	[89/11/16  15:17:06  af]
 * 
 * Revision 2.3  89/11/15  13:26:37  dbg
 * 	Add e_readv, e_writev, e_pioctl.
 * 	[89/11/07            dbg]
 * 	Add table call.  Fix cmusysent to be zero-based.
 * 	[89/10/25            dbg]
 * 
 * 	Correct number of arguments for mkdir.
 * 	[89/10/24            dbg]
 * 
 * Revision 2.2  89/10/17  11:24:18  rwd
 * 	Added init_process syscall at -41.
 * 	[89/09/29            rwd]
 * 
 */
#include <syscall_table.h>

#define	syss(routine, nargs)	{ nargs, routine }
#define	sysg			{ E_GENERIC, emul_generic }
#define	sysr(routine)		{ E_CHANGE_REGS, routine }
#ifdef	ECACHE
#define syse(routine)		{ E_GENERIC, routine }
#endif

/*
 * Routines in system call table
 */
int	emul_generic();		/* catchall */

int	e_fork(), e_execv(), e_execve(), e_sigreturn(), e_osigcleanup(),
	e_wait();
int	e_htg_syscall();

int	e_write(), e_stat(), e_lstat(), e_acct(), e_readlink(), e_select();
int	e_send(), e_getrusage(), e_recvfrom(), e_sendto();
int	e_sigvec(), e_sigstack(), e_settimeofday(), e_setitimer();
int	e_accept(), e_setsockopt(), e_getsockopt();
int	e_getsockname(), e_getpeername();
int	e_table();
int	e_readv(), e_writev(), e_pioctl();

#ifdef	ECACHE
int	e_getpid(), e_getdtablesize(), e_getpagesize();
#endif

int	e_open(), e_creat(), e_link(), e_unlink(), e_chdir();
int	e_mknod(), e_chmod(), e_chown(), e_mount(), e_umount();
int	e_access(), e_symlink(), e_chroot(), e_rename();
int	e_truncate(), e_mkdir(), e_rmdir(), e_utimes();
int	e_setquota(), e_xutimes();
int	Bsd1_task_by_pid(), Bsd1_pid_by_task(), Bsd1_init_process();
int	Bsd1_setgroups(), Bsd1_setrlimit(), Bsd1_adjtime(), Bsd1_sethostname();
int	Bsd1_bind(), Bsd1_connect();

/*
 * System call table
 */
struct sysent sysent[] = {
	sysg, sysg,			/* 0 */
	sysr(e_fork),			/* 2 */
	sysg,				/* 3 */
	syss(e_write, 3),		/* 4 */
	syss(e_open, 3),		/* 5 */
	sysg,
	sysg,
	syss(e_creat, 2),		/* 8 */
	syss(e_link, 2),		/* 9 */
	syss(e_unlink, 1),		/* 10 */
	sysr(e_execv),			/* 11 */
	syss(e_chdir, 1),		/* 12 */
	sysg,
	syss(e_mknod, 3),		/* 14 */
	syss(e_chmod, 2),		/* 15 */
	syss(e_chown, 3),		/* 16 */
	sysg,
	sysg,
	sysg,
#ifdef	ECACHE
	syse(e_getpid),			/* 20 */
#else
	sysg,
#endif
	syss(e_mount, 3),		/* 21 */
	syss(e_umount, 1),		/* 22 */
	sysg,
	sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	syss(e_access, 2),		/* 33 */
	sysg, sysg, sysg, sysg,
	syss(e_stat, 2),		/* 38 */
	sysg,
	syss(e_lstat, 2),		/* 40 */
	sysg, sysg, sysg, sysg, sysg, sysg,
	sysg,
	sysg, sysg,
	sysg,
	syss(e_acct, 1),		/* 51 */
	sysg, sysg, sysg, sysg, sysg,
	syss(e_symlink, 2),		/* 57 */
	syss(e_readlink, 3),		/* 58 */
	sysr(e_execve),			/* 59 */
	sysg,
	syss(e_chroot, 1),		/* 61 */
	sysg, sysg,
#ifdef	ECACHE
	syse(e_getpagesize),		/* 64 */
#else
	sysg,
#endif
	sysg,
	sysr(e_fork),			/* 66 (vfork) */
	sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg,
	syss(Bsd1_setgroups, 2),		/* 80 */
	sysg, sysg,
	syss(e_setitimer, 3),		/* 83 */
	sysr(e_wait),			/* 84 */
	sysg, sysg, sysg,
	syss(Bsd1_sethostname, 2),	/* 88 */
#ifdef	ECACHE
	syse(e_getdtablesize),		/* 89 */
#else
	sysg,
#endif
	sysg, sysg, sysg,
	syss(e_select, 5),		/* 93 */
	sysg, sysg, sysg, sysg,
	syss(Bsd1_connect, 3),		/* 98 */
	syss(e_accept, 3),		/* 99 */
	sysg,
	syss(e_send, 4),		/* 101 */
	sysg,
	sysr(e_sigreturn),		/* 103 */
	syss(Bsd1_bind, 3),		/* 104 */
	syss(e_setsockopt, 5),		/* 105 */
	sysg, sysg,
	/*
	 * Sigvec is defined to take 3 args, the fourth
	 * one is added inside libc on those
	 * machines that use the sigtramp device
	 */
	syss(e_sigvec, 4),		/* 108 */
	sysg,sysg,
	sysg,
	syss(e_sigstack, 2),		/* 112 */
	sysg, sysg, sysg, sysg,
	syss(e_getrusage, 2),		/* 117 */
	syss(e_getsockopt, 5),		/* 118 */
	sysg,
	syss(e_readv, 3),		/* 120 */
	syss(e_writev, 3),		/* 121 */
	syss(e_settimeofday, 2),	/* 122 */
	sysg, sysg,
	syss(e_recvfrom, 6),		/* 125 */
	sysg, sysg,
	syss(e_rename, 2),		/* 128 */
	syss(e_truncate, 2),		/* 129 */
	sysg, sysg, sysg,
	syss(e_sendto, 6),		/* 133 */
	sysg, sysg,
	syss(e_mkdir, 2),		/* 136 */
	syss(e_rmdir, 1),		/* 137 */
	syss(e_utimes, 2),		/* 138 */
	sysr(e_osigcleanup),		/* 139 */
	syss(Bsd1_adjtime, 2),		/* 140 */
	syss(e_getpeername, 3),		/* 141 */
	sysg, sysg,
	sysg,
	syss(Bsd1_setrlimit, 2),		/* 145 */
	sysg, sysg,
	syss(e_setquota, 2),		/* 148 */
	sysg,
	syss(e_getsockname, 3),		/* 150 */
#ifdef	sun3
	      sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	syss(e_pioctl, 4),		/* 168 */
							      sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
#endif	sun3
#if	defined(vax) || defined(i386)
	      sysg, sysg, sysg, sysg, sysg, sysg,
	syss(e_pioctl, 4),		/* 157 */
							sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg,
	syss(e_pioctl, 4),		/* 171 */
		    sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
#endif	defined(vax) || defined(i386)
#ifdef	mips
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	syss(e_pioctl, 4),		/* 168 */
	sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg,
	syss(e_pioctl, 4),		/* 195 */
	sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg, sysg,
	sysg, sysg, sysg, sysg, sysg, sysg, sysg
#endif	mips
};

int	nsysent = sizeof(sysent)/sizeof(struct sysent);

struct sysent	cmusysent[] = {
	sysg,				/*  0 (make it zero-based) */
	sysg,				/* -1 */
	sysg,				/* -2 */
	sysg,				/* -3 */
	syss(e_xutimes, 2),		/* -4 */
	sysg,				/* -5 */
	syss(e_table, 5),		/* -6 */
	sysg,				/* -7 */
	sysg,				/* -8 */
	sysg				/* -9 */
};

int	ncmusysent = sizeof(cmusysent)/sizeof(struct sysent);

struct sysent	sysent_task_by_pid =
	syss(Bsd1_task_by_pid, 1);

struct sysent	sysent_pid_by_task =
	syss(Bsd1_pid_by_task, 4);

struct sysent	sysent_htg_ux_syscall =
	sysr(e_htg_syscall);

struct sysent	sysent_init_process =
	syss(Bsd1_init_process, 0);
