/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	bsd_msg.h,v $
 * Revision 2.3  91/12/19  20:27:36  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:15:26  rwd
 * 	First checkin
 * 	[90/08/31  13:33:11  rwd]
 * 
 * Revision 2.3.2.1  90/05/03  01:17:13  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  20:08:07  rpd]
 * 
 * Revision 2.3  89/11/29  15:30:13  af
 * 	Added rval2, because some syscalls do not modify it and it
 * 	must be preserved.
 * 	[89/11/20            af]
 * 
 * Revision 2.2  89/10/17  11:26:51  rwd
 * 	Added interrupt return parameter.  Removed INTERRUPT
 * 	and ERROR macros.
 * 	[89/09/21            dbg]
 * 
 */
/*
 *	File:	./bsd_msg.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

/*
 * Request and reply message for generic BSD kernel call.
 */

#include <mach/boolean.h>
#include <mach/message.h>

struct	bsd_request {
    mach_msg_header_t	hdr;
    mach_msg_type_t	int_type;	/* int[8] */
    int			rval2;
    int			syscode;
    int			arg[6];
};

struct	bsd_reply {
    mach_msg_header_t	hdr;
    mach_msg_type_t	int_type;	/* int[4] */
    int			retcode;
    int			rval[2];
    boolean_t		interrupt;
};

union bsd_msg {
    struct bsd_request	req;
    struct bsd_reply	rep;
    char		msg[8192];
};

#define	BSD_REQ_MSG_ID		100000
