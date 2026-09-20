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
 * $Log:	bsd_prot.c,v $
 * Revision 2.3  91/12/19  20:27:37  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:15:35  rwd
 * 	First checkin
 * 	[90/08/31  13:33:28  rwd]
 * 
 */
/*
 *	File:	./bsd_prot.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <ux_user.h>

bsd_getgroups(ut, rval, ngids, gids)
	struct ux_task *ut;
	int rval[2];
	int ngids;
	int *gids;
{
	if (ngids < ut->ut_ngids) {
		return EINVAL;
	}
	bcopy(ut->ut_gids, gids, ut->ut_ngids * sizeof(int));
	rval[0] = ut->ut_ngids;
	return 0;
}

bsd_setgroups(ut, rval, ngids, gids)
	struct ux_task *ut;
	int rval[2];
	int ngids;
	int *gids;
{
	int error;

	if (ngids < 0 || ngids > NGIDS) {
		return EINVAL;
	}
	bcopy(gids, ut->ut_gids, ngids * sizeof(int));
	ut->ut_ngids = ngids;
	return 0;
}

Bsd_getgid(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	rval[0] = ut->ut_rgid;
	rval[1] = ut->ut_egid;
	return 0;
}

Bsd_getuid(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	rval[0] = ut->ut_ruid;
	rval[1] = ut->ut_euid;
	return 0;
}

Bsd_setreuid(ut, rval, ruid, euid)
	struct ux_task *ut;
	int rval[2];
	int ruid;
	int euid;
{
	if (ruid != -1) ut->ut_ruid = ruid;
	if (euid != -1) ut->ut_euid = euid;
	return 0;
}
	
Bsd_setregid(ut, rval, rgid, egid)
	struct ux_task *ut;
	int rval[2];
	int rgid;
	int egid;
{
	if (rgid != -1) ut->ut_rgid = rgid;
	if (egid != -1) ut->ut_egid = egid;
	return 0;
}
