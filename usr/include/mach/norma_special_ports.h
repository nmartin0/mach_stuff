/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: norma_special_ports.h,v $
 * Revision 1.2.10.2  1995/02/23  17:57:16  alanl
 * 	Merged with DIPC2_SHARED.
 * 	[1995/01/04  14:28:30  alanl]
 *
 * Revision 1.2.10.1  1994/09/23  02:41:00  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:42:04  ezf]
 * 
 * Revision 1.2.8.1  1994/08/04  02:28:34  mmp
 * 	NOTE: file was moved back to b11 version for dipc2_shared.
 * 
 * 	Added NORMA_TEST_PORT.
 * 	[1994/06/28  14:07:14  mmp]
 * 
 * 	DIPC:  no use appears to exist any longer for
 * 	the host_paging_self port or its associated
 * 	support code.
 * 	[1994/04/27  23:53:02  alanl]
 * 
 * Revision 1.2.2.2  1993/06/09  02:42:43  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:17:37  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:38:12  devrcs
 * 	Fixes for ANSI C
 * 	[1993/02/26  13:30:17  sp]
 * 
 * Revision 1.1  1992/09/30  02:31:53  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.3.2.2  92/06/24  18:05:35  jeffreyh
 * 	Added NORMA_HOST_PAGING_PORT and related macros. 
 * 	[92/06/17            jeffreyh]
 * 
 * Revision 2.3.2.1  92/01/09  18:44:29  jsb
 * 	Define MAX_SPECIAL_KERNEL_ID.
 * 	[92/01/04  18:16:44  jsb]
 * 
 * Revision 2.3  91/12/13  13:47:36  jsb
 * 	Moved MAX_SPECIAL_ID here from ipc/ipc_node.h.
 * 
 * Revision 2.2  91/08/03  18:19:11  jsb
 * 	First checkin.
 * 	[91/07/25  07:52:36  jsb]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 */
/*
 *	File:	mach/norma_special_ports.h
 *
 *	Defines codes for remote access to special ports.  These are NOT
 *	port identifiers - they are only used for the norma_get_special_port
 *	and norma_set_special_port routines.
 */

#ifndef	_MACH_NORMA_SPECIAL_PORTS_H_
#define _MACH_NORMA_SPECIAL_PORTS_H_

#define	MAX_SPECIAL_KERNEL_ID	10
#define	MAX_SPECIAL_ID		40

/*
 * Provided by kernel
 */
#define NORMA_DEVICE_PORT	1
#define NORMA_HOST_PORT		2
#define NORMA_HOST_PRIV_PORT	3

#ifdef	MACH_KERNEL

#include <kernel_test.h>
#if	KERNEL_TEST
#define	NORMA_TEST_PORT		(MAX_SPECIAL_ID - 1)
#endif	/* KERNEL_TEST */

#endif	/* MACH_KERNEL */

/*
 * Not provided by kernel
 */
#define NORMA_NAMESERVER_PORT	(1 + MAX_SPECIAL_KERNEL_ID)

/*
 * Definitions for ease of use.
 *
 * In the get call, the host parameter can be any host, but will generally
 * be the local node host port. In the set call, the host must the per-node
 * host port for the node being affected.
 */

#define norma_get_device_port(host, node, port)	\
	(norma_get_special_port((host), (node), NORMA_DEVICE_PORT, (port)))

#define norma_set_device_port(host, port)	\
	(norma_set_special_port((host), NORMA_DEVICE_PORT, (port)))

#define norma_get_host_port(host, node, port)	\
	(norma_get_special_port((host), (node), NORMA_HOST_PORT, (port)))

#define norma_set_host_port(host, port)	\
	(norma_set_special_port((host), NORMA_HOST_PORT, (port)))

#define norma_get_host_priv_port(host, node, port)	\
	(norma_get_special_port((host), (node), NORMA_HOST_PRIV_PORT, (port)))

#define norma_set_host_priv_port(host, port)	\
	(norma_set_special_port((host), NORMA_HOST_PRIV_PORT, (port)))

#define norma_get_nameserver_port(host, node, port)	\
	(norma_get_special_port((host), (node), NORMA_NAMESERVER_PORT, (port)))

#define norma_set_nameserver_port(host, port)	\
	(norma_set_special_port((host), NORMA_NAMESERVER_PORT, (port)))

#endif	/* _MACH_NORMA_SPECIAL_PORTS_H_ */
