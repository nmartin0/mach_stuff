/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <osfmach3/mach3_debug.h>
#include <linux/sched.h>
#include <asm/syscalls.h>

void console_verbose(void)
{
	extern int console_loglevel;
	console_loglevel = 15;
}

void trap_init(void)
{
}


void die_if_kernel(char * str, struct pt_regs * regs, long err)
{
	struct task_struct	*tsk;

	if (current != FIRST_TASK) {
		/* it's a user process ... */
		extern boolean_t server_exception_in_progress;
		if (!server_exception_in_progress) {
			return;
		}
		/* ... but it's a server exception ! */
	}

	console_verbose();
	printk("%s: %04lx\n", str, err & 0xffff);
	show_regs(regs);
	tsk = current;
	if (tsk != NULL) {
		printk("Process %s (pid: %d, stackpage=%08lx)\nStack: ",
		       tsk->comm, tsk->pid, (long)regs->state.sp);
	}

	Debugger("die_if_kernel");
}

struct sysent sys_call_table[] = {
	sys_setup,    0,     /* 0 */
	sys_exit,     1,
	sys_fork,     0, 
	sys_read,     3,
	sys_write,    3,
	sys_open,     3,     /* 5 */
	sys_close,    1,
	sys_waitpid,  3,
	sys_creat,    2,
	sys_link,     2,
	sys_unlink,   1,     /* 10 */
	sys_execve,   3,
	sys_chdir,    1,
	sys_time,     1,
	sys_mknod,    3,
	sys_chmod,    2,     /* 15 */
	sys_chown,    3,
	sys_break,    0,
	sys_stat,     2,
	sys_lseek,    3,
	sys_getpid,   0,     /* 20 */
	sys_mount,    5,
	sys_umount,   1,
	sys_setuid,   1,
	sys_getuid,   0,
	sys_stime,    1,     /* 25 */
	sys_ptrace,   4,
	sys_alarm,    1,
	sys_fstat,    2,
	sys_pause,    0,
	sys_utime,    2,     /* 30 */
	0,            0,
	0,            0,
	sys_access,   2,
	0,            0,
	0,            0,     /* 35 */
	sys_sync,     0,
	sys_kill,     2,
	sys_rename,   2,
	sys_mkdir,    2,
	sys_rmdir,    1,     /* 40 */
	sys_dup,      1,
	sys_pipe,     1,
	sys_times,    1,
	0,            0,
	sys_brk,      1,     /* 45 */
	sys_setgid,   1,
	sys_getgid,   0,
	0,            0,
	sys_geteuid,  0,
	sys_getegid,  0,     /* 50 */
	0,            0,
	0,            0,
	0,            0,
	sys_ioctl,    3,
	sys_fcntl,    3,     /* 55 */
	0,            0,
	sys_setpgid,  2,
	0,            0,
	0,            0,
	sys_umask,    1,     /* 60 */
	0,            0,
	0,            0,
	sys_dup2,     2,
	sys_getppid,  0,
	sys_getpgrp,  0,     /* 65 */
	sys_setsid,   0,
	sys_sigaction, 3,
	0,            0,
	0,            0,
	sys_setreuid, 2,     /* 70 */
	sys_setregid, 2,
	sys_sigsuspend, 1,
	sys_sigpending, 1,
	sys_sethostname, 2,
	sys_setrlimit,  2,     /* 75 */
	sys_getrlimit, 2,
	sys_getrusage, 2,
	sys_gettimeofday,  2,
	sys_settimeofday,  2,
	sys_getgroups, 2,     /* 80 */
	sys_setgroups, 2,
	sys_select,   5,
	sys_symlink,  2,
	0,            0,
	sys_readlink, 3,     /* 85 */
	0,            0,
	sys_swapon,   2,
	sys_reboot,   3,
	old_readdir,  3,
	sys_mmap,     6,     /* 90 */
	sys_munmap,   2,
	sys_truncate, 2,
	sys_ftruncate, 2,
	sys_fchmod,   2,
	sys_fchown,   3,     /* 95 */
	sys_getpriority, 2,
	sys_setpriority, 3,
	0,            0,
	sys_statfs,   2,
	sys_fstatfs,  2,     /* 100,*/
	0,            0,
	sys_socketcall,  2,
	0,            0,
	sys_setitimer, 3,
	sys_getitimer, 2,     /* 105 */
	sys_newstat,  2,
	sys_newlstat, 2,
	sys_newfstat, 2,
	0,            0,
	0,            0,     /* 110 */
	sys_vhangup, 0,
	0,            0,
	0,            0,
	sys_wait4,    4,
	sys_swapoff,  1,     /* 115 */
	sys_sysinfo,  1,
	sys_ipc,      6,
	sys_fsync,    1,
	sys_sigreturn,  0,
	0,            0,     /* 120 */
	0,            0,
	sys_uname,    1,
	0,            0,
	0,            0,
	sys_mprotect,  3,     /* 125 */
	sys_sigprocmask, 3,
	0,            0,
	0,            0,
	0,            0,
	0,            0,     /* 130 */
	0,            0,
	sys_getpgid,  1,
	sys_fchdir,   1,
	0,            0,
	0,            0,    /* 135 */
	0,            0,
	0,            0,
	sys_setfsuid, 1,
	sys_setfsgid, 1,
	0,            0,     /* 140 */
	sys_getdents, 3,
	0,            0,
	sys_flock,    2,
	0,            0,
	sys_readv,    3,     /* 145 */
	sys_writev,   3,
	sys_getsid,   1,
};

char * sys_call_name_table[] = {
	"setup",      /* 0 */
	"exit",
	"fork",
	"read",
	"write",
	"open",       /* 5 */
	"close",
	"waitpid",
	"creat",
	"link",
	"unlink",     /* 10 */
	"execve",
	"chdir",
	"time",
	"mknod",
	"chmod",      /* 15 */
	"chown",
	"break",
	"stat",
	"lseek",
	"getpid",     /* 20 */
	"mount",
	"umount",
	"setuid",
	"getuid",
	"stime",      /* 25 */
	"ptrace",
	"alarm",
	"fstat",
	"pause",
	"utime",      /* 30 */
	0,
	0,
	"access",
	0,
	0,            /* 35 */
	"sync",
	"kill",
	"rename",
	"mkdir",
	"rmdir",       /* 40 */
	"dup",
	"pipe",
	"times",
	0,
	"brk",        /* 45 */
	"setgid",
	"getgid",
	0,
	"geteuid",
	"getegid",    /* 50 */
	0,
	0,
	0,
	"ioctl",
	"fcntl",      /* 55 */
	0,
	"setpgid",
	0,
	0,
	"umask",      /* 60 */
	0,
	0,
	"dup2",
	"getppid",
	"getpgrp",     /* 65 */
	"setsid",
	"sigaction",
	0,
	0,
	"setreuid",   /* 70 */
	"setregid",
	"sigsuspend",
	"sigpending",
	"sethostname",
	"setrlimit",    /* 75 */
	"getrlimit",
	"getrusage",
	"gettimeofday",
	"settimeofday",
	"getgroups", /* 80 */
	"setgroups",
	"select",
	"symlink",
	0,
	"readlink",    /* 85 */
	0,
	"swapon",
	"reboot",
	"old readdir",
	"mmap",   /* 90 */
	"munmap",
	"truncate",
	"ftruncate",
	"fchmod",
	"fchown",     /* 95 */
	"getpriority",
	"setpriority",
	0,
	"statfs",
	"fstatfs",     /* 100 */
	0,
	"socketcall",
	0,
	"setitimer",
	"getitimer",   /* 105 */
	"newstat",
	"newlstat",
	"newfstat",
	0,
	0,            /* 110 */
	"vhangup",
	0,
	0,
	"wait4",
	"swapoff",     /* 115 */
	"sysinfo",
	"sysVipc",
	"fsync",
	"sigreturn",
	0,            /* 120 */
	0,
	"uname",
	0,
	0,
	"mprotect",   /* 125 */
	"sigprocmask",
	0,
	0,
	0,
	0,           /* 130 */
	0,
	"getpgid",
	"fchdir",
	0,
	0,           /* 135 */
	0,
	0,
	"setfsuid",
	"setfsgid",
	0,           /* 140 */
	"getdents",
	0,
	"flock",
	0,
	"readv",           /* 145 */
	"writev",
	"getsid"
};

asmlinkage void do_debug(int exc, struct pt_regs * regs, long error_code)
{
	force_sig(SIGTRAP, current);
	current->osfmach3.thread->trap_no = 1;
	current->osfmach3.thread->error_code = error_code;

	die_if_kernel("debug",regs,error_code);
}
