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
 * $Log:	bsd_ioctl.c,v $
 * Revision 2.3  91/12/19  20:27:20  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:07:18  rwd
 * 	First checkin
 * 	[90/08/31  13:31:17  rwd]
 * 
 */
/*
 *	File:	./bsd_ioctl.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <fnode.h>
#include <fentry.h>
#include <ux_user.h>
#include <bsd_ioctl.h>

/*
 * Copy server structure in or out.
 * Used by PCOPY definition in bsd_ioctl.h.
 */
pcopy(rw, server, user, size)
	int rw;
	char *server;
	char *user;
	int size;
{
	if (rw & IOC_IN) {
		bcopy(user, server, size);
	}
	if (rw & IOC_OUT) {
		bcopy(server, user, size);
	}
}

fe_ioctl(fe, rw, type, command, param, psize)
	struct fentry *fe;
	int rw;
	int type;
	int command;
	char *param;
	int psize;
{
	switch (command) {
		case 1:
			return 0;

		default:
			return ENOTTY;
	}
}

/* no bsd_1 form */
Bsd_ioctl(ut, rval, fd, request, argp)
	struct ux_task *ut;
	int rval[2];
	int fd;
	unsigned long request;
	char *argp;
{
	int psize, rw, type, command;
	char *param, _param[IOCPARM_MASK + 1];
	struct fentry *fe;
	int error;

	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	if ((request >> 16) == 0) {
		return EINVAL;
	}
	rw = (request & IOC_INOUT);
	psize = (request >> 16) & IOCPARM_MASK;
	type = (request & 0xff00) >> 8;
	command = request & 0xff;
	if (rw & IOC_IN) {
		error = copyin(ut, argp, psize, &param);
		if (error) {
			return error;
		}
	} else if (rw & IOC_OUT) {
		param = _param;
		bzero(param, psize);
	}
	if (type == 'f') {
		error = fe_ioctl(fe, rw, type, command, param, psize);
	} else {
		error = FOP_IOCTL(fe->fe_fnode, rw, type, command, param,
			psize);
	}
	if (error) {
		return error;
	}
	if (! error && (rw & IOC_OUT)) {
		error = copyout(ut, param, argp, psize);
	}
	if (rw & IOC_IN) {
		uncopyin(param, psize);
	}
	return error;
}
