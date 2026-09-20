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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	task_special_user.c,v $
 * Revision 2.3  91/02/08  16:05:34  mrt
 * 	Added new Mach copyright and author notices
 * 
 * Revision 2.2  90/01/24  23:33:55  mrt
 * 	Changed include of mach/mach_interface.h to mach_interface.h
 * 	[90/01/22            mrt]
 * 
 * 	Updated to reflect new mig.
 * 	[89/08/04            dlb]
 * 
 * 	Created
 * 	[89/03/11            dlb]
 * 
 */
/*
 *	File: 	task_special_user.c
 *	Author:	David Black, Carnegie Mellon University
 *	Date:	Jan 1990
 *
 *	Special versions of task_{suspend,resume} cut down from mach_user.c.
 *	These versions are modified in two ways to protect servers from
 *	users who supply bogus ports:
 *		1.  There is no reply message requested or expected.
 *		2.  There is a send timeout of 0.
 *	This works fine for the expected usage case: server performing
 *	operations on tasks on the LOCAL machine.  This should not be used
 *	if the task may be remote!
 */

#include <mach_interface.h>
#include <mach/message.h>
#include <mach/mach_types.h>
#include <mach/mig_errors.h>
#include <mach/msg_type.h>

#ifndef	TypeCheck
#define	TypeCheck 1
#endif	TypeCheck

#ifndef	UseExternRCSId
#if	hc
#define	UseExternRCSId		1
#endif	hc
#endif	UseExternRCSId

#ifndef UseStaticMsgType
#if     !defined(hc) || defined(__STDC__)
#define UseStaticMsgType        1
#endif
#endif

/* Routine task_suspend_special */
mig_external kern_return_t task_suspend_special(
#if     (defined(__STDC__) || defined(c_plusplus))
		task_t target_task)
#else
	target_task)
		task_t target_task;
#endif
{
	typedef struct {
		msg_header_t Head;
	} Request;

	union {
		Request In;
	} Mess;

	register Request *InP = &Mess.In;

	msg_return_t msg_result;

	InP->Head.msg_simple = TRUE;
	InP->Head.msg_size = sizeof(Request);
	InP->Head.msg_type = MSG_TYPE_NORMAL;
	InP->Head.msg_remote_port = target_task;
	InP->Head.msg_local_port = PORT_NULL;
	InP->Head.msg_id = 2056;

	msg_result = msg_send(&InP->Head, SEND_TIMEOUT, 0);

	return(msg_result);
}

/* Routine task_resume_special */
mig_external kern_return_t task_resume_special(
#if     (defined(__STDC__) || defined(c_plusplus))
		task_t target_task)
#else
	target_task)
		task_t target_task;
#endif
{
	typedef struct {
		msg_header_t Head;
	} Request;

	union {
		Request In;
	} Mess;

	register Request *InP = &Mess.In;

	msg_return_t msg_result;

	InP->Head.msg_simple = TRUE;
	InP->Head.msg_size = sizeof(Request);
	InP->Head.msg_type = MSG_TYPE_NORMAL;
	InP->Head.msg_remote_port = target_task;
	InP->Head.msg_local_port = PORT_NULL;
	InP->Head.msg_id = 2057;

	msg_result = msg_send(&InP->Head, SEND_TIMEOUT, 0);

	return(msg_result);
}
