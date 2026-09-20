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
 * $Log:	bsd_host.c,v $
 * Revision 2.3  91/12/19  20:27:16  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:07:11  rwd
 * 	First checkin
 * 	[90/08/31  13:30:25  rwd]
 * 
 */
/*
 *	File:	./bsd_host.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <ux_user.h>

static char hostname[1024] = "jsb.mach.cs.cmu.edu";	/* XXX */
static unsigned long hostid = 0x8002d134;		/* XXX */

Bsd_gethostname(ut, rval, name, namelen)
	struct ux_task *ut;
	int rval[2];
	char *name;
	int namelen;
{
	int len;

	len = strlen(hostname) + 1;
	if (len > namelen) {
		len = namelen;
	}
	copyout(ut, hostname, name, len); /* no bsd_1 form */
	return 0;
}

bsd_sethostname(ut, rval, name, namelen)
	struct ux_task *ut;
	int rval[2];
	char *name;
	int namelen;
{
	if (ut->ut_euid != 0) {
		return EPERM;
	}
	if (namelen < 0) {
		return EINVAL;
	}
	if (namelen > sizeof(hostname) - 1) {
		namelen = sizeof(hostname) - 1;
	}
	bcopy(name, hostname, namelen);
	hostname[namelen] = 0;
	return 0;
}

Bsd_gethostid(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	rval[0] = hostid;
	return 0;
}

Bsd_sethostid(ut, rval, id)
	struct ux_task *ut;
	int rval[2];
	long id;
{
	if (ut->ut_euid != 0) {
		return EPERM;
	}
	hostid = id;
	return 0;
}
