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
 * $Log:	machine_ptrace.c,v $
 * Revision 2.3  91/12/19  20:28:20  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/27  13:54:38  rwd
 * 	First Checkin
 * 	[90/09/10  19:05:58  rwd]
 * 
 */
/*
 *	File:	./i386/machine_ptrace.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <ux_user.h>

Bsd_ptrace(ut, rval, request, pid, address, data)
	struct ux_task *ut;
	int rval[2];
	int request;
	int pid;
	int *address;
	int data;
{
	return EINVAL;
}
