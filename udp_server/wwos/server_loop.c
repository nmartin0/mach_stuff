/*
 * Simpleminded UDP Server
 *
 * Copyright (C) 1992  Chris Maeda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Chris Maeda (cmaeda@cs.cmu.edu)
 * School of Computer Science
 * Carnegie Mellon University
 * 5000 Forbes Ave
 * Pittsburgh PA  15213-3890
 * USA
 */
/*
 * udp server loop
 */

#include <stdio.h>
#include <mach.h>
#include <cthreads.h>
#include <mach/message.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/message.h>
#include <mach/mig_errors.h>


#define MAX_MSG_SIZE 4096

extern mach_port_t udp_service_port;
extern kern_return_t udp_server();



/*
 *	Routine:	mach_msg_server
 *	Purpose:
 *		A simple generic server function.
 */

mach_msg_return_t
cthread_mach_msg_server(demux, max_size, rcv_name)
    boolean_t (*demux)();
    mach_msg_size_t max_size;
    mach_port_t rcv_name;
{
    register mig_reply_header_t *bufRequest, *bufReply, *bufTemp;
    register mach_msg_return_t mr;

    bufRequest = (mig_reply_header_t *) malloc(max_size);
    if (bufRequest == 0)
	return KERN_RESOURCE_SHORTAGE;
    bufReply = (mig_reply_header_t *) malloc(max_size);
    if (bufReply == 0)
	return KERN_RESOURCE_SHORTAGE;

    for (;;) {
      get_request:
	mr = cthread_mach_msg(&bufRequest->Head, MACH_RCV_MSG,
		      0, max_size, rcv_name,
		      MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL, 1, 5);
	while (mr == MACH_MSG_SUCCESS) {
	    /* we have a request message */

	    (void) (*demux)(&bufRequest->Head, &bufReply->Head);

	    if (bufReply->RetCode != KERN_SUCCESS) {
		if (bufReply->RetCode == MIG_NO_REPLY)
		    goto get_request;

		/* don't destroy the reply port right,
		   so we can send an error message */
		bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
		mach_msg_destroy(&bufRequest->Head);
	    }

	    if (bufReply->Head.msgh_remote_port == MACH_PORT_NULL) {
		/* no reply port, so destroy the reply */
		if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)
		    mach_msg_destroy(&bufReply->Head);

		goto get_request;
	    }

	    /* send reply and get next request */

	    bufTemp = bufRequest;
	    bufRequest = bufReply;
	    bufReply = bufTemp;

	    mr = cthread_mach_msg(&bufRequest->Head, MACH_SEND_MSG|MACH_RCV_MSG,
			  bufRequest->Head.msgh_size, max_size, rcv_name,
			  MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL,1, 5);
	}

	/* a message error occurred */

	if (mr != MACH_SEND_INVALID_DEST)
	    break;

	/* the reply can't be delivered, so destroy it */
	mach_msg_destroy(&bufRequest->Head);
    }

    free((char *) bufRequest);
    free((char *) bufReply);
    return mr;
}


server_root()
{
    int ret;

/*    cthread_wire();*/
    
    while (1)  {
	if (cthread_mach_msg_server(udp_server, MAX_MSG_SIZE, udp_service_port) != MACH_MSG_SUCCESS)  {
	    printf("server_loop: returned %d\n", ret);
	    shutdown(0);
	}
    }
}


