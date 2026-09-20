/*
 * Copyright (c) 1992 Carnegie Mellon University
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
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 *
 * The Mdos condition code.
 *
 * HISTORY:
 * $Log:	bios_cond.c,v $
 * Revision 2.2  92/07/01  14:24:33  grm
 * 	Ifdef out idle code.
 * 	[92/07/01            grm]
 * 	Created.
 * 	[92/06/30  13:46:26  grm]
 * 
 *
 */
#include "base.h"
#include "bios.h"

#ifdef	IDLE_WORK_IN_PROGRESS
#include "bios_cond.h"


extern condition_t exit_condition;

mach_port_t cond_port;

typedef struct {
	mach_msg_header_t Head;
	mach_msg_type_t numType;
	int num;
} cond_msg_t;

void condition_init(port) 
	mach_port_t * port;
{
	mach_port_t right;
	mach_msg_type_name_t right_type;
	kern_return_t ret;

	ret = mach_port_allocate(mach_task_self(),
				 MACH_PORT_RIGHT_RECEIVE,
				 port);
	if (ret != KERN_SUCCESS) {
		mach_error("condition_init: port_allocate of condition", ret);
		exit(0);
	}

	ret = mach_port_extract_right(mach_task_self(),
				      *port,
				      MACH_MSG_TYPE_MAKE_SEND,
				      &right,
				      &right_type);
	if (ret != KERN_SUCCESS) {
		mach_error("condition_init: port_extract_right",ret);
		exit(0);
	}
}

void condition_wait(port)
	mach_port_t port;
{
	cond_msg_t wait_msg;
	cond_msg_t * wait_msg_ptr;
	kern_return_t ret;

	wait_msg_ptr = &wait_msg;

	ret = mach_msg(&wait_msg_ptr->Head, MACH_RCV_MSG, 0,
		       sizeof(wait_msg), port, 0, MACH_PORT_NULL);
	if (ret != KERN_SUCCESS) {
		mach_error("conditional wait message receive",ret);
		exit(0);
	}
}

void condition_signal(port)
	mach_port_t port;
{
	cond_msg_t sig_msg;
	cond_msg_t * sig_msg_ptr;
	kern_return_t ret;

	sig_msg_ptr = &sig_msg;

	sig_msg_ptr->numType.msgt_name = 2;
	sig_msg_ptr->numType.msgt_size = 32;
	sig_msg_ptr->numType.msgt_number = 1;
	sig_msg_ptr->numType.msgt_inline = TRUE;
	sig_msg_ptr->numType.msgt_longform = FALSE;
	sig_msg_ptr->numType.msgt_deallocate = FALSE;
	sig_msg_ptr->numType.msgt_unused = 0;

	sig_msg_ptr->num = 123;

	sig_msg_ptr->Head.msgh_bits =
		MACH_MSGH_BITS(19,0);
	/* msgh_size passed as argument */
	sig_msg_ptr->Head.msgh_remote_port = port;
	sig_msg_ptr->Head.msgh_local_port = MACH_PORT_NULL;
	sig_msg_ptr->Head.msgh_seqno = 0;
	sig_msg_ptr->Head.msgh_id = 500;

	ret = mach_msg(&sig_msg_ptr->Head,
		       MACH_SEND_MSG|MACH_MSG_OPTION_NONE,
		       sizeof(sig_msg),
		       0,
		       MACH_PORT_NULL,
		       MACH_MSG_TIMEOUT_NONE,
		       MACH_PORT_NULL);
	if (ret != KERN_SUCCESS) {
		mach_error("conditional signal message send",ret);
		exit(0);
	}
}
#endif	/* IDLE_WORK_IN_PROGRESS */
