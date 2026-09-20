/* 
 * MACH Operating System
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
 * $Log:	bsd_io.c,v $
 * Revision 2.7  94/03/25  18:21:52  mrt
 * 	Added support for reading CMU style FASTLINKS.
 * 	Do not create FASTLINKS
 * 	[94/02/18            mrt]
 * 
 * Revision 2.6  92/02/02  13:01:40  rpd
 * 	Fixed vm_allocate/vm_deallocate argument types.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.5  91/12/19  20:27:18  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  91/06/18  13:06:36  jjc
 * 	Fixed Bsd_xread() to check to see if the buffer will fit on the stack
 * 	even after rounding up the beginning of the buffer to a page boundary.
 * 	[91/04/16            jjc]
 * 
 * Revision 2.3  90/09/27  13:53:57  rwd
 * 	Use map_file_lock/unlock.
 * 	[90/09/11            rwd]
 * 
 * Revision 2.2  90/09/08  00:07:15  rwd
 * 	Convert files to mapped window.
 * 	[90/08/28            rwd]
 * 
 */
/*
 *	File:	./bsd_io.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <fnode.h>
#include <ufs_fops.h>
#include <ufs_disk.h>
#include <fentry.h>
#include <ux_param.h>
#include <ux_user.h>

bsd_readlink(ut, rval, pathname, buf, len)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	char *buf;
	int len;
{
	int error, resid, out_len;
	char*	fstlnk_str;
	struct fnode *fn;
	struct unode *ufn;

	error = fn_path(ut, pathname, FALSE, &fn);
	if (error) {
		return error;
	}
	error = fn_checktype(fn, S_IFLNK, EINVAL);
	if (error) {
		FOP_DECR(fn);
		return error;
	}

	ufn = (struct unode *) fn;
#if DEBUG
	printf ("readlink: start #%d\n", ufn->un_ino);
#endif
	if ( ufn ->i_ic.ic_flags & IC_FASTLINK != 0 ) {
	    out_len = (long)ufn->i_size;
	    if (out_len > len) out_len = len;
	    resid = len - out_len;
	    fstlnk_str = ufn->i_ic.ic_Mun.ic_Msymlink;
#if DEBUG
	    printf ("readlink: #%d ut=0x%x, len=%d out_len=%d resid=%d str= '%s'\n",
		ufn->un_ino, ut, len, out_len, resid, fstlnk_str);
#endif

	    if (resid > 0 ) {
		bcopy(fstlnk_str, buf, out_len);
	    }
	    FOP_DECR(fn);
	}
	else {
	    error = FOP_READ(fn, 0, buf, len, &resid);
	    FOP_DECR(fn);
	    if (error) {
		return error;
	    }
	}
	if (resid == 0) {
		return ENAMETOOLONG;
	}
	rval[0] = len - resid;
	return 0;
}
	
Bsd_xread(ut, rval, fd, buf, len, offsetp)
	struct ux_task *ut;
	int rval[2];
	int fd;
	char *buf;
	int len;
	long *offsetp;
{
	struct fentry *fe;
	int error;

	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	if (! (fe->fe_rdwr & FREAD)) {
		return EBADF;
	}
	if (offsetp) {
		/* called from getdirentries, so make dir check */
		error = fn_checktype(fe->fe_fnode, S_IFDIR, ENOTDIR);
		if (error) {
			return error;
		}
		/* now copy out current offset */
		copyout(ut, &fe->fe_offset, offsetp, sizeof(*offsetp));
	}
	if (fe->fe_fnode->fn_maymap) {
		struct stat st;
		int addr;

		error = bsd_map(fe->fe_fnode);
		if (error) {
			return error;
		}
		error = FOP_GETSTAT(fe->fe_fnode, &st, FATTR_SIZE);
		if (error) {
			return error;
		}
		if (fe->fe_offset >= st.st_size) {
			rval[0] = 0;
			return 0;
		}
		if (len > st.st_size - fe->fe_offset) {
			len = st.st_size - fe->fe_offset;
		}
		map_file_lock(&fe->fe_fnode->fn_map_info);
		map_file_remap(&fe->fe_fnode->fn_map_info, fe->fe_offset, len);
		addr = fe->fe_fnode->fn_map_info.mi_address -
			fe->fe_fnode->fn_map_info.mi_offset + fe->fe_offset;
		error = copyout(ut, addr, buf, len);
		map_file_unlock(&fe->fe_fnode->fn_map_info);
		if (error) {
			return error;
		}
		rval[0] = len;
		fe->fe_offset += len;
		return 0;
	} else {
		vm_size_t resid;
		char *buffer;
		char _buffer[2 * 8192 + 4096];
		int onstack;

		/*
		 * Check to see if there's enough room on the stack to put
		 * our buffer even after we round the beginning of the buffer
		 * up to the next page boundary.
		 */
		buffer = (char *)round_page((int)_buffer);
		onstack = ( ((int)buffer - (int)_buffer + len)
				<= sizeof(_buffer) );
		if (!onstack) {
			buffer = 0;
			error = vm_allocate(mach_task_self(),
					    (vm_offset_t *)&buffer, len,
					    TRUE);
			if (error) {
				return ENOMEM;
			}
		}
		error = FOP_READ(fe->fe_fnode, fe->fe_offset, buffer, len,
				 &resid);
		if (! error) {
			rval[0] = len - resid;
			copyout(ut, buffer, buf, rval[0]);
			fe->fe_offset += rval[0];
		}
		if (! onstack) {
			vm_deallocate(mach_task_self(),
				      (vm_offset_t) buffer, len);
		}
		return error;
	}
}

Bsd_read(ut, rval, fd, buf, len)
	struct ux_task *ut;
	int rval[2];
	int fd;
	char *buf;
	int len;
{
	return Bsd_xread(ut, rval, fd, buf, len, 0);
}

Bsd_getdirentries(ut, rval, fd, buf, len, offsetp)
	struct ux_task *ut;
	int rval[2];
	int fd;
	char *buf;
	int len;
	long *offsetp;
{
	if (offsetp == 0) {
		return EFAULT;
	}
	return Bsd_xread(ut, rval, fd, buf, len, offsetp);
}

bsd_write(ut, rval, fd, buf, len)
	struct ux_task *ut;
	int rval[2];
	int fd;
	char *buf;
	int len;
{
	struct fentry *fe;
	vm_size_t resid;
	int error;
	off_t offset;

	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	if (! (fe->fe_rdwr & FWRITE)) {
		return EBADF;
	}
	if (fe->fe_append) {
		struct stat st;
		error = FOP_GETSTAT(fe->fe_fnode, &st, FATTR_SIZE);
		if (error) {
			return error;
		}
		offset = st.st_size;
	} else {
		offset = fe->fe_offset;
	}
	error = FOP_WRITE(fe->fe_fnode, offset, buf, len, &resid);
	if (error == EPIPE) {
		bsd_sendsig(ut, SIGPIPE);
		return EINTR;
	}
	if (! error) {
		rval[0] = len - resid;
		fe->fe_offset = offset + rval[0];
	}
	return error;
}

/* no bsd_1 form */
Bsd_writev(ut, rval, fd, iov, iovcnt)
	struct ux_task *ut;
	int rval[2];
	int fd;
	struct iovec *iov;
	int iovcnt;
{
	int j;
	int total = 0;
	int rv;
	char *buf;
	struct fentry *fe;
	vm_size_t resid;
	int error;

	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	if (! (fe->fe_rdwr & FWRITE)) {
		return EBADF;
	}
	if (fe->fe_append) {
		printf("\n[oops! writev+append!]\n");
	}
	error = copyin(ut, iov, iovcnt * sizeof(*iov), &iov);
	if (error) {
		return error;
	}
	for (j = 0; j < iovcnt; j++) {
		error = copyin(ut, iov[j].iov_base, iov[j].iov_len, &buf);
		if (error) {
			return error;
		}
		error = FOP_WRITE(fe->fe_fnode, fe->fe_offset, buf,
				  iov[j].iov_len, &resid);
		total += (iov[j].iov_len - resid);
		if (error || resid) {
			break;
		}
	}
	/* XXX uncopyin */
	if (error == ESPIPE) {
		bsd_sendsig(ut, SIGPIPE);
	}
	rval[0] = total;
	return error;
}

Bsd_lseek(ut, rval, fd, offset, whence)
	struct ux_task *ut;
	int rval[2];
	int fd;
	off_t offset;
	int whence;
{
	struct fentry *fe;
	struct stat st;
	int error;
	off_t new_offset;

	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	if (! fe->fe_fnode->fn_mayseek) {
		return ESPIPE;
	}
	switch (whence) {
		case L_SET:
			new_offset = offset;
			break;

		case L_INCR:
			new_offset = fe->fe_offset + offset;
			break;

		case L_XTND:
			error = FOP_GETSTAT(fe->fe_fnode, &st, FATTR_SIZE);
			if (error) {
				return error;
			}
			new_offset = st.st_size + offset;
			break;

		default:
			return EINVAL;
	}
	if (new_offset < 0) {
		return EINVAL;
	}
	fe->fe_offset = new_offset;
	rval[0] = (int) new_offset;
	return 0;
}
