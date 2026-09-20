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
 * $Log:	bsd_server_side.c,v $
 * Revision 2.5  92/02/02  13:02:13  rpd
 * 	Fixed vm_deallocate argument types.
 * 
 * Revision 2.4  91/12/19  20:27:46  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/12/04  21:55:23  rpd
 * 	Fixed Bsd1_settimeofday to return zero for now.
 * 	[90/12/04            rpd]
 * 
 * 	Implemented Bsd1_init_process.
 * 	[90/12/04            rpd]
 * 	Fixed Bsd1_pid_by_task, to use lookup_ux_task_by_task.
 * 	[90/11/24            rpd]
 * 
 * Revision 2.2  90/09/08  00:15:43  rwd
 * 	Add more unused stubs.  Convert to variable length
 * 	pathname.
 * 	[90/07/16            rwd]
 * 
 */
/*
 *	File:	./bsd_server_side.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <mach/message.h>
#include <errno.h>
#include <ux_user.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <bsd_types.h>

extern int add_crs;
extern int silent;

static int
closedown(ut, interrupt, error, rval)
	struct ux_task *ut;
	boolean_t	*interrupt;
	int		error;
	int		*rval;
{
	*interrupt = (ut->ut_sigtake != 0);
	if (silent) {
	} else if (error) {
		unix_error(") ", error);
	} else if (rval) {
		dprintf(") = %d\n", *rval);
	} else {
		dprintf(") = \n");
	}
	return error;
}

Bsd1_write_short(proc_port, interrupt, fd, buf, len, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fd;
	char *		buf;
	unsigned int	len;
	int		*rval;		/* out */
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_write_short", proc_port, &ut, interrupt);
	if (error) return error;
	error = bsd_write(ut, rval, fd, buf, len);
out:	return closedown(ut, interrupt, error, rval);
}

Bsd1_write_long(proc_port, interrupt, fd, buf, len, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fd;
	char *		buf;
	unsigned int	len;
	int		*rval;		/* out */
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_write_long", proc_port, &ut, interrupt);
	if (error) return error;
	error = bsd_write(ut, rval, fd, buf, len);
	vm_deallocate(mach_task_self(), (vm_offset_t) buf, len);
out:	return closedown(ut, interrupt, error, rval);
}

Bsd1_select(proc_port, interrupt, nd, in_set, ou_set, ex_set,
	    in_valid, ou_valid, ex_valid, do_timeout, tv, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		nd;
	fd_set		*in_set;	/* in/out */
	fd_set		*ou_set;	/* in/out */
	fd_set		*ex_set;	/* in/out */
	boolean_t	in_valid;
	boolean_t	ou_valid;
	boolean_t	ex_valid;
	boolean_t	do_timeout;
	struct timeval	tv;
	int		*rval;		/* out */
{
	int error;
	struct ux_task *ut;
	struct timeval *tvp;

	error = ut_lookup("bsd_select", proc_port, &ut, interrupt);
	if (error) return error;
	if (do_timeout) {
		tvp = &tv;
	} else {
		tvp = 0;
	}
	error = bsd_select(ut, rval, nd,
			   (in_valid)?in_set:0,
			   (ou_valid)?ou_set:0,
			   (ex_valid)?ex_set:0,
			   tvp);
out:	return closedown(ut, interrupt, error, rval);
}

/*
 * in uipc_syscalls
 */
Bsd1_send_short(proc_port, interrupt,
		fileno, flags, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	return EOPNOTSUPP;
}

Bsd1_send_long(proc_port, interrupt,
		fileno, flags, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	return EOPNOTSUPP;
}

Bsd1_sendto_short(proc_port, interrupt, fileno, flags,
		to, tolen, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*to;
	int		tolen;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	return EOPNOTSUPP;
}

Bsd1_sendto_long(proc_port, interrupt, fileno, flags,
		to, tolen, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*to;
	int		tolen;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	return EOPNOTSUPP;
}

Bsd1_recvfrom_short(proc_port, interrupt, fileno, flags, len,
		from, fromlen, data, data_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	int		len;
	char		*from;		/* pointer to OUT array */
	int		*fromlen;	/* in/out */
	char		*data;		/* pointer to OUT array */
	unsigned int	*data_count;	/* out */
{
	return EOPNOTSUPP;
}

Bsd1_recvfrom_long(proc_port, interrupt, fileno, flags, len,
		from, fromlen, data, data_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	int		len;
	char		*from;		/* pointer to OUT array */
	int		*fromlen;	/* in/out */
	char		**data;		/* out */
	unsigned int	*data_count;	/* out */
{
	return EOPNOTSUPP;
}

/*
 * in ufs_syscalls
 */
Bsd1_chdir(proc_port, interrupt, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_chdir", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_chdir(ut, 0, fname);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_chroot(proc_port, interrupt, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_chroot", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_chroot(ut, 0, fname);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_open(proc_port, interrupt, fname, fname_count, mode, crtmode, fileno)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
	int	mode;
	int	crtmode;
	int	*fileno;	/* OUT */
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_open", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_open(ut, fileno, fname, mode, crtmode);
out:	return closedown(ut, interrupt, error, fileno);
}

Bsd1_creat(proc_port, interrupt, fname, fname_count, fmode, fileno)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
	int	fmode;
	int	*fileno;	/* OUT */
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_creat", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_creat(ut, fileno, fname, fmode);
out:	return closedown(ut, interrupt, error, fileno);
}

Bsd1_mknod(proc_port, interrupt, fname, fname_count, fmode, dev)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	unsigned int fname_count;
	char	*fname;
	int	fmode;
	int	dev;
{
	return EPERM;
}

Bsd1_link(proc_port, interrupt, target, target_len, linkname, link_len)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*target;
	int	target_len;
	char	*linkname;
	int	link_len;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_link", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_link(ut, 0, target, linkname);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_symlink(proc_port, interrupt, target, target_len, linkname, link_len)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*target;
	int	target_len;
	char	*linkname;
	int	link_len;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_symlink", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_symlink(ut, 0, target, linkname);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_unlink(proc_port, interrupt, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_unlink", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_unlink(ut, 0, fname);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_access(proc_port, interrupt, fname, fname_count, fmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
	int	fmode;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_access", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_access(ut, 0, fname, fmode);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_stat(proc_port, interrupt, follow, name, name_size, stat)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	follow;
	char		*name;
	int		name_size;
	register statb_t *stat;	/* out */
{
	int error;
	struct stat st;
	struct ux_task *ut;

	error = ut_lookup("bsd_*stat", proc_port, &ut, interrupt);
	if (error) return error;
	if (follow) {
		error = bsd_stat(ut, 0, name, &st);
	} else {
		error = bsd_lstat(ut, 0, name, &st);
	}
	if (! error) {
		stat->s_dev	= st.st_dev;
		stat->s_ino	= st.st_ino;
		stat->s_mode	= st.st_mode;
		stat->s_nlink	= st.st_nlink;
		stat->s_uid	= st.st_uid;
		stat->s_gid	= st.st_gid;
		stat->s_rdev	= st.st_rdev;
		stat->s_size	= st.st_size;
		stat->s_atime	= st.st_atime;
		stat->s_mtime	= st.st_mtime;
		stat->s_ctime	= st.st_ctime;
		stat->s_blksize	= st.st_blksize;
		stat->s_blocks	= st.st_blocks;
	}
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_readlink(proc_port, interrupt, name, name_size, count, buf, bufCount)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*name;
	int	name_size;
	int	count;
	char	*buf;		/* pointer to OUT array */
	int	*bufCount;	/* out */
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_readlink", proc_port, &ut, interrupt);
	if (error) return error;
	error = bsd_readlink(ut, bufCount, name,  buf, count);
out:	return closedown(ut, interrupt, error, bufCount);
}

Bsd1_chmod(proc_port, interrupt, fname, fname_count, fmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
	int	fmode;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_chmod", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_chmod(ut, 0, fname, fmode);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_chown(proc_port, interrupt, fname, fname_count, uid, gid)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
	int	uid;
	int	gid;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_chown", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_chown(ut, 0, fname, uid, gid);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_utimes(proc_port, interrupt, fname, fname_count, times)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int fname_count;
	struct timeval	*times;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_utimes", proc_port, &ut, interrupt);
	if (error) return error;
	error = bsd_utimes(ut, 0, fname, times);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_truncate(proc_port, interrupt, fname, fname_count, length)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fname;
	unsigned int fname_count;
	int	length;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_truncate", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_truncate(ut, 0, fname, length);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_rename(proc_port, interrupt, from, from_size, to, to_size)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*from;
	int		from_size;
	char		*to;
	int		to_size;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_rename", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_rename(ut, 0, from, to);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_mkdir(proc_port, interrupt, name, name_size, dmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*name;
	int	name_size;
	int	dmode;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_mkdir", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_mkdir(ut, 0, name, dmode);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_rmdir(proc_port, interrupt, name, name_size)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*name;
	int	name_size;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_rmdir", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_rmdir(ut, 0, name);
out:	return closedown(ut, interrupt, error, 0);
}

/*
 * In cmu_syscalls
 */
Bsd1_xutimes(proc_port, interrupt, fname, fname_count, times)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int fname_count;
	struct timeval	*times;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_xutimes", proc_port, &ut, interrupt);
	if (error) return error;
	error = EINVAL;
out:	return closedown(ut, interrupt, error, 0);
}

/*
 * in ufs_mount
 */
Bsd1_mount(proc_port, interrupt, fspec, fspec_size, freg, freg_size, ronly)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fspec;
	int	fspec_size;
	char	*freg;
	int	freg_size;
	int	ronly;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_mount", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_ufs_mount(fspec, freg, ronly);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_umount(proc_port, interrupt, fspec, fspec_size)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fspec;
	int	fspec_size;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_umount", proc_port, &ut, interrupt);
	if (error) return error;
	error = Bsd_ufs_umount(fspec);
out:	return closedown(ut, interrupt, error, 0);
}

/*
 * in kern_acct
 */

Bsd1_acct(proc_port, interrupt, acct_on, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	acct_on;
	char		*fname;
	unsigned int fname_count;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_acct", proc_port, &ut, interrupt);
	if (error) return error;
	error = EPERM;
out:	return closedown(ut, interrupt, error, 0);
}

/*
 * in quota_sys
 */
Bsd1_setquota(proc_port, interrupt, fblk, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char	*fblk;
	char	*fname;
	unsigned int fname_count;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_setquota", proc_port, &ut, interrupt);
	if (error) return error;
	error = EPERM;
out:	return closedown(ut, interrupt, error, 0);
}

/*
 * More glue
 */
Bsd1_setgroups(proc_port, interrupt, gidsetsize, gidset)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	unsigned int	gidsetsize;
	int		*gidset;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_setgroups", proc_port, &ut, interrupt);
	if (error) return error;
	error = bsd_setgroups(ut, 0, gidsetsize, gidset);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_setrlimit(proc_port, interrupt, which, lim)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		which;
	struct rlimit	*lim;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_setrlimit", proc_port, &ut, interrupt);
	if (error) return error;
/*	error = bsd_setrlimit(ut, 0, which, lim); */
	error = EINVAL;
out:	return closedown(ut, interrupt, error, 0);
}

#if 0
/* XXX should be merged */
Bsd1_sigvec(proc_port, interrupt, signo, have_nsv, nsv, osv, tramp)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* OUT */
	int		signo;
	boolean_t	have_nsv;
	struct sigvec	nsv;
	struct sigvec	*osv;	/* OUT */
	int		tramp;
{
	int error;
	struct ux_task *ut;
	struct sigvec *vec;

	error = ut_lookup("bsd_sigvec", proc_port, &ut, interrupt);
	if (error) return error;
	if (have_nsv) {
		vec = &nsv;
	} else {
		vec = 0;
	}
	error = bsd_sigvec(ut, 0, signo, vec, osv, tramp);
out:	return closedown(ut, interrupt, error, 0);
}
#endif

Bsd1_sigstack(proc_port, interrupt, have_nss, nss, oss)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	have_nss;
	struct sigstack	nss;
	struct sigstack	*oss;		/* OUT */
{
	int error;
	struct ux_task *ut;
	struct sigstack *ss;

	error = ut_lookup("bsd_sigstack", proc_port, &ut, interrupt);
	if (error) return error;
	if (have_nss) {
		ss = &nss;
	} else {
		ss = 0;
	}
	error = bsd_sigstack(ut, 0, ss, oss);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_settimeofday(proc_port, interrupt, have_tv, tv, have_tz, tz)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	have_tv;
	struct timeval	tv;
	boolean_t	have_tz;
	struct timezone	tz;
{
	int error;
	struct ux_task *ut;
	struct timeval *tvp;
	struct timezone *tzp;

	error = ut_lookup("bsd_settimeofday", proc_port, &ut, interrupt);
	if (error) return error;
	tvp = (have_tv) ? &tv : 0;
	tzp = (have_tz) ? &tz : 0;
#if	0
	error = Bsd_settimeofday(ut, 0, tvp, tzp);
#else
	error = 0;
#endif
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_adjtime(proc_port, interrupt, delta, olddelta)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	struct timeval	*delta;
	struct timeval	*olddelta;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_adjtime", proc_port, &ut, interrupt);
	if (error) return error;
	error = EPERM;
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_setitimer(proc_port, interrupt, which, have_itv, itv, oitv)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		which;
	boolean_t	have_itv;
	struct itimerval itv;
	struct itimerval *oitv;		/* OUT */
{
	int error;
	struct ux_task *ut;
	struct itimerval *itvp;

	error = ut_lookup("bsd_setitimer", proc_port, &ut, interrupt);
	if (error) return error;
	itvp = (have_itv) ? &itv : 0;
	error = bsd_setitimer(ut, 0, which, itvp, oitv);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_sethostname(proc_port, interrupt, hostname, len)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*hostname;
	int		len;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_sethostname", proc_port, &ut, interrupt);
	if (error) return error;
	error = bsd_sethostname(ut, 0, hostname, len);
out:	return closedown(ut, interrupt, error, 0);
}

Bsd1_bind(proc_port, interrupt, s, name, namelen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	char		*name;
	int		namelen;
{
	return EOPNOTSUPP;
}

Bsd1_accept(proc_port, interrupt, s, name, namelen, new_s)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	char		*name;		/* OUT */
	int		*namelen;	/* OUT */
	int		*new_s;		/* OUT */
{
	return EOPNOTSUPP;
}

Bsd1_connect(proc_port, interrupt, s, name, namelen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	char		*name;
	int		namelen;
{
	return EOPNOTSUPP;
}

Bsd1_setsockopt(proc_port, interrupt, s, level, name, val, valsize)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	int		level;
	int		name;
	char		*val;
	int		valsize;
{
	return EOPNOTSUPP;
}

Bsd1_getsockopt(proc_port, interrupt, s, level, name, val, avalsize)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	int		level;
	int		name;
	char		*val;		/* OUT */
	int		*avalsize;	/* IN-OUT */
{
	return EOPNOTSUPP;
}

Bsd1_getsockname(proc_port, interrupt, fdes, asa, alen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fdes;
	char		*asa;		/* OUT */
	int		*alen;		/* OUT */
{
	return EOPNOTSUPP;
}

Bsd1_getpeername(proc_port, interrupt, fdes, asa, alen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fdes;
	char		*asa;		/* OUT */
	int		*alen;		/* OUT */
{
	return EOPNOTSUPP;
}

Bsd1_table_set(proc_port, interrupt, id, index, lel, nel,
		addr, count, nel_done)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* out */
	int		id;
	int		index;
	int		lel;
	int		nel;
	char		*addr;
	unsigned int	count;
	int		*nel_done;	/* out */
{
	return EINVAL;
}

Bsd1_table_get(proc_port, interrupt, id, index, lel, nel,
		addr, count, nel_done)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* out */
	int		id;
	int		index;
	int		lel;
	int		nel;
	char		**addr;		/* out */
	unsigned int	*count;		/* out */
	int		*nel_done;	/* out */
{
	return EINVAL;
}

Bsd1_pioctl(proc_port, interrupt, path, com, follow, in_data, in_data_count,
		out_data_wanted, out_data, out_data_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char *		path;
	int		com;
	boolean_t	follow;
	char *		in_data;
	unsigned int	in_data_count;
	int		out_data_wanted;
	char *		out_data;	 /* pointer to OUT array */
	unsigned int	*out_data_count; /* out */
{
	return EINVAL;
}

Bsd1_readwrite(proc_port, interrupt, which, fileno, size, amount)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	which;
	int		fileno;
	int		size;
	int		*amount;
{
	return EINVAL;
}

Bsd1_share_wakeup(proc_port, offset)
	mach_port_t	proc_port;
	int		offset;
{
	return EINVAL;
}

Bsd1_signals_wakeup(proc_port)
	mach_port_t	proc_port;
{
	return EINVAL;
}

extern mach_port_t privileged_host_port;
extern mach_port_t device_server_port;

Bsd1_task_by_pid(proc_port, interrupt, pid, t, tType)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		pid;
	task_t		*t;
	mach_msg_type_name_t *tType;
{
	register struct proc	*p;
	register int	error;
	struct ux_task *ut, *ut2;
	int rval[2];

	rval[0]=0;
	rval[1]=0;
	error = ut_lookup("bsd_task_by_pid", proc_port, &ut, interrupt);
	if (error) return error;

	if (pid == 0) {
	    *t = mach_task_self();
	} else if (pid == -1) {
	    *t = privileged_host_port;
	} else if (pid == -2) {
	    *t = device_server_port;
	} else if (!lookup_ux_task_by_pid(pid, &ut2)) {
	    *t = ut2->ut_task;
	} else
	    error = ESRCH;

	if (!error) {
		kern_return_t kr;

		/*
		 * Give ourself another send right for the task port,
		 * and specify that the send right should be moved
		 * into the reply message.  This way there is no problem
		 * if the task port should be destroyed before the
		 * the reply message is sent.
		 */

		kr = mach_port_mod_refs(mach_task_self(), *t,
					MACH_PORT_RIGHT_SEND, 1);
		if (kr == KERN_SUCCESS) {
			*tType = MACH_MSG_TYPE_MOVE_SEND;
		} else {
			printf("pid %d -> task 0x%x; mod_refs returned %d\n",
				pid, *t, kr);
			panic("Bsd1_task_by_pid");
		}
	}

	if (error) {
		/*
		 * If we can't produce a real send right,
		 * then give our caller MACH_PORT_NULL instead
		 * of a unix error code which he would probably
		 * mistake for a port.
		 */

		*t = MACH_PORT_NULL;
		*tType = MACH_MSG_TYPE_MOVE_SEND;
		error = 0;
	}

out:	return closedown(ut, interrupt, error, rval);
}

int
Bsd1_pid_by_task(proc_port, interrupt, task, pidp, comm, commlen, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	task_t		task;
	int		*pidp;
	char		*comm;
	int		*commlen;
	int		*rval;
{
	struct ux_task *ut;
	int len;

	if (task == mach_task_self()) {
	    *pidp = 0;
	    len = 3;
	    if (*commlen < len)
		len = *commlen;
	    bcopy("Poe", comm, len);
	    *commlen = len;
	    *rval = 0;
	} else if (lookup_ux_task_by_task(task, &ut)) {
	    *pidp = 0;
	    *commlen = 0;
	    *rval = -1;
	} else {
	    *pidp = ut->ut_pid;
	    len = strlen(ut->ut_comm);
	    if (*commlen < len)
		len = *commlen;
	    bcopy(ut->ut_comm, comm, len);
	    *commlen = len;
	    *rval = 0;
	}

	/* get rid of the send right */
	(void) mach_port_deallocate(mach_task_self(), task);

	*interrupt = FALSE;
	return ESUCCESS;
}

Bsd1_init_process(proc_port, interrupt)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
{
	int error;
	struct ux_task *ut;

	error = ut_lookup("bsd_init_process", proc_port, &ut, interrupt);
	if (error) return error;

	if (ut->ut_euid == 0)
		ut->ut_init_process = TRUE;
	else
		error = EPERM;

out:	return closedown(ut, interrupt, error, 0);
}

/*
 * XXX @@@ should have definitions
 */

Bsd1_getrusage()
{
	return EINVAL;
}

Bsd1_maprw_release_it()
{
	return EINVAL;
}

Bsd1_maprw_remap()
{
	return EINVAL;
}

Bsd1_maprw_request_it()
{
	return EINVAL;
}
