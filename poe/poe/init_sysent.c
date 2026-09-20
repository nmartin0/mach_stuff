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
 * $Log:	init_sysent.c,v $
 * Revision 2.7  92/02/02  13:02:27  rpd
 * 	Fixed for Ansi C.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.6  91/12/19  20:28:26  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.5  91/03/09  14:32:20  rpd
 * 	Changed all uses of sys_nope to sys_null.
 * 	[90/12/14            rpd]
 * 
 * Revision 2.4  90/12/04  21:55:37  rpd
 * 	Added Bsd_reboot.
 * 	Changed settimeofday to sys_null.
 * 	[90/12/04            rpd]
 * 
 * Revision 2.3  90/09/27  13:55:02  rwd
 * 	Add i386 entries.
 * 	[90/09/19            rwd]
 * 
 * Revision 2.2  90/09/08  00:19:02  rwd
 * 	First checkin
 * 	[90/08/31  13:41:56  rwd]
 * 
 */
/*
 *	File:	./init_sysent.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <sysent.h>
#include <ux_user.h>

extern int Bsd_nothing();
extern int Bsd_null();
extern int Bsd_sock();
extern int Bsd_quot();
extern int Bsd_eperm();

extern int Bsd_access();
extern int Bsd_chdir();
extern int Bsd_chmod();
extern int Bsd_chown();
extern int Bsd_chroot();
extern int Bsd_close();
extern int Bsd_creat();
extern int Bsd_dup();
extern int Bsd_dup2();
extern int Bsd_exit();
extern int Bsd_fchmod();
extern int Bsd_fchown();
extern int Bsd_fcntl();
extern int Bsd_fstat();
extern int Bsd_ftruncate();
extern int Bsd_getdirentries();
extern int Bsd_getdtablesize();
extern int Bsd_getgid();
extern int Bsd_getgroups();
extern int Bsd_gethostid();
extern int Bsd_gethostname();
extern int Bsd_getitimer();
extern int Bsd_getpagesize();
extern int Bsd_getpgrp();
extern int Bsd_getpid();
extern int Bsd_getrlimit();
extern int Bsd_gettimeofday();
extern int Bsd_getuid();
extern int Bsd_ioctl();
extern int Bsd_kill();
extern int Bsd_killpg();
extern int Bsd_link();
extern int Bsd_lstat();
extern int Bsd_lseek();
extern int Bsd_mkdir();
extern int Bsd_obreak();
extern int Bsd_open();
extern int Bsd_pipe();
extern int Bsd_ptrace();
extern int Bsd_read();
extern int Bsd_readlink();
extern int Bsd_reboot();
extern int Bsd_rename();
extern int Bsd_rmdir();
extern int Bsd_sbrk();
extern int Bsd_select();
extern int Bsd_setgroups();
extern int Bsd_sethostid();
extern int Bsd_sethostname();
extern int Bsd_setitimer();
extern int Bsd_setpgrp();
extern int Bsd_setregid();
extern int Bsd_setreuid();
extern int Bsd_sigblock();
extern int Bsd_sigpause();
extern int Bsd_sigsetmask();
extern int Bsd_sigstack();
extern int Bsd_sigvec();
extern int Bsd_stat();
extern int Bsd_symlink();
extern int Bsd_truncate();
extern int Bsd_ttyc();
extern int Bsd_umask();
extern int Bsd_unlink();
extern int Bsd_utimes();
extern int Bsd_wait();
extern int Bsd_write();
extern int Bsd_writev();

#if	__STDC__
#define	sys(xxx,fmt)		{ Bsd_ ## xxx,	"xxx", fmt }
#else
#define	sys(xxx,fmt)		{ Bsd_/**/xxx,	"xxx", fmt }
#endif
#define	sys_eperm(xxx,fmt)	{ Bsd_eperm,	"xxx", fmt }
#define	sys_sock(xxx,fmt)	{ Bsd_sock,	"xxx", fmt }
#define	sys_quot(xxx,fmt)	{ Bsd_quot,	"xxx", fmt }
#define	sys_nope(xxx,fmt)	{ Bsd_nothing,	"xxx", fmt }
#define	sys_null(xxx,fmt)	{ Bsd_null,	"xxx", fmt }
#define	sys_nothing		{ Bsd_nothing,	0,     0  }

struct sysent sysent[] = {
	sys_nothing,			/* 0 */
	sys(exit,"d"),			/* 1 */
	sys_null(fork,0),		/* 2 */
	sys(read,"dxd"),		/* 3 */
	sys(write,"dxd"),		/* 4 */
	sys(open,"soo"),		/* 5 */
	sys(close,"d"),			/* 6 */
	sys_nothing,			/* 7 */
	sys(creat,"so"),		/* 8 */
	sys(link,"ss"),			/* 9 */
	sys(unlink,"s"),		/* 10 */
	sys_null(execv,"sx"),		/* 11 */
	sys(chdir,"s"),			/* 12 */
	sys(ttyc,"d"),			/* 13 */
	sys_eperm(mknod,"soo"),		/* 14 */
	sys(chmod,"so"),		/* 15 */
	sys(chown,"sdd"),		/* 16 */
	sys(obreak,"x"),		/* 17 */
	sys_nothing,			/* 18 */
	sys(lseek,"ddd"),		/* 19 */
	sys(getpid,0),			/* 20 */
	sys_null(mount,"ssd"),		/* 21 */
	sys_null(umount,"s"),		/* 22 */
	sys_nothing,			/* 23 */
	sys(getuid,0),			/* 24 */
	sys_nothing,			/* 25 */
	sys(ptrace,0),			/* 26 */
	sys_nothing,			/* 27 */
	sys_nothing,			/* 28 */
	sys_nothing,			/* 29 */
	sys_nothing,			/* 30 */
	sys_nothing,			/* 31 */
	sys_nothing,			/* 32 */
	sys(access,"so"),		/* 33 */
	sys_nothing,			/* 34 */
	sys_nothing,			/* 35 */
	sys_null(sync,0),		/* 36 */
	sys(kill,"dd"),			/* 37 */
	sys(stat,"sx"),			/* 38 */
	sys_nothing,			/* 39 */
	sys(lstat,"sx"),		/* 40 */
	sys(dup,"d"),			/* 41 */
	sys(pipe,"x"),			/* 42 */
	sys_nothing,			/* 43 */
	sys_null(profil,"xddd"),	/* 44 */
	sys_nothing,			/* 45 */
	sys_nothing,			/* 46 */
	sys(getgid,0),			/* 47 */
	sys_nothing,			/* 48 */
	sys_nothing,			/* 49 */
	sys_nothing,			/* 50 */
	sys_eperm(acct,"s"),		/* 51 */
	sys_nothing,			/* 52 */
	sys_nothing,			/* 53 */
	sys(ioctl,"dIx"),		/* 54 */
	sys(reboot,"x"),		/* 55 */
	sys_nothing,			/* 56 */
	sys(symlink,"ddx"),		/* 57 */
	sys(readlink,"sxd"),		/* 58 */
	sys_null(execve,"sxx"),		/* 59 */
	sys(umask,"o"),			/* 60 */
	sys(chroot,"s"),		/* 61 */
	sys(fstat,"dx"),		/* 62 */
	sys_nothing,			/* 63 */
	sys(getpagesize,0),		/* 64 */
	sys_null(mremap,0),		/* 65 */
	sys_nothing,			/* 66 */
	sys_nothing,			/* 67 */
	sys_nothing,			/* 68 */
	sys(sbrk,"x"),			/* 69 */
	sys_null(sstk,0),		/* 70 */
	sys_null(mmap,0),		/* 71 */
	sys_nothing,			/* 72 */
	sys_null(munmap,0),		/* 73 */
	sys_null(mprotect,0),		/* 74 */
	sys_null(madvise,0),		/* 75 */
	sys_null(vhangup,0),		/* 76 */
	sys_nothing,			/* 77 */
	sys_null(mincore,0),		/* 78 */
	sys(getgroups,"dx"),		/* 79 */
	sys(setgroups,"dx"),		/* 80 */
	sys(getpgrp,"d"),		/* 81 */
	sys(setpgrp,"dd"),		/* 82 */
	sys(setitimer,"dxx"),		/* 83 */
	sys(wait,"xxx"),		/* 84 */
	sys_eperm(swapon,0),		/* 85 */
	sys(getitimer,"dx"),		/* 86 */
	sys(gethostname,"xd"),		/* 87 */
	sys(sethostname,"sd"),		/* 88 */
	sys(getdtablesize,0),		/* 89 */
	sys(dup2,"dd"),			/* 90 */
	sys_null(getdopt,0),		/* 91 */
	sys(fcntl,"ddd"),		/* 92 */
	sys(select,"dxxxx"),		/* 93 */
	sys_null(setdopt,0),		/* 94 */
	sys_null(fsync,"d"),		/* 95 */
	sys_null(setpriority,0),	/* 96 */
	sys_sock(socket,0),		/* 97 */
	sys_sock(connect,0),		/* 98 */
	sys_sock(accept,0),		/* 99 */
	sys_null(getpriority,0),	/* 100 */
	sys_sock(send,0),		/* 101 */
	sys_sock(recv,0),		/* 102 */
	sys_null(sigreturn,0),		/* 103 */
	sys_sock(bind,0),		/* 104 */
	sys_sock(setsockopt,0),		/* 105 */
	sys_sock(listen,0),		/* 106 */
	sys_nothing,			/* 107 */
	sys(sigvec,"dxx"),		/* 108 */
	sys(sigblock,"x"),		/* 109 */
	sys(sigsetmask,"x"),		/* 110 */
	sys(sigpause,"x"),		/* 111 */
	sys(sigstack,"xx"),		/* 112 */
	sys_sock(recvmsg,0),		/* 113 */
	sys_sock(sendmsg,0),		/* 114 */
	sys_nothing,			/* 115 */
	sys(gettimeofday,"x"),		/* 116 */
	sys_null(getrusage,0),		/* 117 */
	sys_sock(getsockopt,0),		/* 118 */
	sys_nothing,			/* 119 */
	sys_null(readv,0),		/* 120 */
	sys(writev,"dxd"),		/* 121 */
	sys_null(settimeofday,"xx"),	/* 122 */
	sys(fchown,"ddd"),		/* 123 */
	sys(fchmod,"do"),		/* 124 */
	sys_sock(recvfrom,0),		/* 125 */
	sys(setreuid,"dd"),		/* 126 */
	sys(setregid,"dd"),		/* 127 */
	sys(rename,"ss"),		/* 128 */
	sys(truncate,"sd"),		/* 129 */
	sys(ftruncate,"dd"),		/* 130 */
	sys_null(flock,"dd"),		/* 131 */
	sys_nothing,			/* 132 */
	sys_sock(sendto,0),		/* 133 */
	sys_sock(shutdown,0),		/* 134 */
	sys_sock(socketpair,0),		/* 135 */
	sys(mkdir,"so"),		/* 136 */
	sys(rmdir,"s"),			/* 137 */
	sys(utimes,"sx"),		/* 138 */
	sys_nothing,			/* 139 */
	sys_eperm(adjtime,0),		/* 140 */
	sys_sock(getpeername,0),	/* 141 */
	sys(gethostid,""),		/* 142 */
	sys(sethostid,"x"),		/* 143 */
	sys(getrlimit,"dx"),		/* 144 */
	sys_null(setrlimit,"dx"),	/* 145 */
	sys(killpg,"dd"),		/* 146 */
	sys_nothing,			/* 147 */
	sys_quot(setquota,0),		/* 148 */
	sys_quot(quota,0),		/* 149 */
	sys_sock(getsockname,0),	/* 150 */
#if	sun3
	sys_nothing,			/* 151 */
	sys_nothing,			/* 152 */
	sys_nothing,			/* 153 */
	sys_nothing,			/* 154 */
	sys_nothing,			/* 155 */
	sys(getdirentries,"dxdx"),	/* 156 */
	sys_nothing,			/* 157 */
	sys_nothing,			/* 158 */
	sys_nothing,			/* 159 */
	sys_nothing,			/* 160 */
	sys_nothing,			/* 161 */
	sys_nothing,			/* 162 */
	sys_nothing,			/* 163 */
	sys_nothing,			/* 164 */
	sys_nothing,			/* 165 */
	sys_nothing,			/* 166 */
	sys_nothing,			/* 167 */
	sys_nothing,			/* 168 */
	sys_nothing,			/* 169 */
	sys_nothing,			/* 170 */
	sys_nothing,			/* 171 */
	sys_nothing,			/* 172 */
	sys_nothing,			/* 173 */
	sys_nothing,			/* 174 */
	sys_nothing,			/* 175 */
	sys_nothing,			/* 176 */
	sys_nothing,			/* 177 */
	sys_nothing,			/* 178 */
	sys_nothing,			/* 179 */
	sys_nothing,			/* 180 */
#endif	sun3
#if	mips
	sys_nothing,			/* 151 */
	sys_nothing,			/* 152 */
	sys_nothing,			/* 153 */
	sys_nothing,			/* 154 */
	sys_nothing,			/* 155 */
	sys_nothing,			/* 156 */
	sys_nothing,			/* 157 */
	sys_nothing,			/* 158 */
	sys(getdirentries,"dxdx"),	/* 159 */
	sys_nothing,			/* 160 */
	sys_nothing,			/* 161 */
	sys_nothing,			/* 162 */
	sys_nothing,			/* 163 */
	sys_nothing,			/* 164 */
	sys_nothing,			/* 165 */
	sys_nothing,			/* 166 */
	sys_nothing,			/* 167 */
	sys_nothing,			/* 168 */
	sys_nothing,			/* 169 */
	sys_nothing,			/* 170 */
	sys_nothing,			/* 171 */
	sys_nothing,			/* 172 */
	sys_nothing,			/* 173 */
	sys_nothing,			/* 174 */
	sys_nothing,			/* 175 */
	sys_nothing,			/* 176 */
	sys_nothing,			/* 177 */
	sys_nothing,			/* 178 */
	sys_nothing,			/* 179 */
	sys_nothing,			/* 180 */
#endif	mips
#ifdef	i386
	sys_nothing,			/* 151 */
	sys_nothing,			/* 152 */
	sys_nothing,			/* 153 */
	sys_nothing,			/* 154 */
	sys_nothing,			/* 155 */
	sys_nothing,			/* 156 */
	sys_nothing,			/* 157 */
	sys_nothing,			/* 158 */
	sys_nothing,			/* 159 */
	sys_nothing,			/* 160 */
	sys_nothing,			/* 161 */
	sys_nothing,			/* 162 */
	sys_nothing,			/* 163 */
	sys(getdirentries,"dxdx"),	/* 164 */
	sys_nothing,			/* 165 */
	sys_nothing,			/* 166 */
	sys_nothing,			/* 167 */
	sys_nothing,			/* 168 */
	sys_nothing,			/* 169 */
	sys_nothing,			/* 170 */
	sys_nothing,			/* 171 */
	sys_nothing,			/* 172 */
	sys_nothing,			/* 173 */
	sys_nothing,			/* 174 */
	sys_nothing,			/* 175 */
	sys_nothing,			/* 176 */
	sys_nothing,			/* 177 */
	sys_nothing,			/* 178 */
	sys_nothing,			/* 179 */
	sys_nothing,			/* 180 */
#endif	i386
};

int nsysent = sizeof(sysent) / sizeof(*sysent);
