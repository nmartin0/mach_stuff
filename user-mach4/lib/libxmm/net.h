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
 * $Log: net.h,v $
 * Revision 1.1.1.1  1995/05/04  06:56:41  sclawson
 * New files.
 *
 * Revision 2.3  92/01/22  22:53:40  rpd
 * 	Added M_CHANGE_COMPLETED_ID.
 * 	[92/01/18            rpd]
 * 
 * Revision 2.2  91/07/06  15:04:33  jsb
 * 	First checkin.
 * 
 */
/*
 *	File:	net.h
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Definitions for xmm_net.c and various protocols.
 */

/*
 * Eventually will have to decide whether we want rpcs or just ipc.
 * E.g., return values from (simple)routines.
 */

#define	M_INIT_ID		0
#define	M_TERMINATE_ID		1
#define	M_COPY_ID		2
#define	M_DATA_REQUEST_ID	3
#define	M_DATA_UNLOCK_ID	4
#define	M_DATA_WRITE_ID		5
#define	M_LOCK_COMPLETED_ID	6
#define	M_SUPPLY_COMPLETED_ID	7
#define	M_DATA_RETURN_ID	8
#define	M_CHANGE_COMPLETED_ID	9

#define	K_DATA_PROVIDED_ID	10
#define	K_DATA_UNAVAILABLE_ID	11
#define	K_GET_ATTRIBUTES_ID	12
#define	K_LOCK_REQUEST_ID	13
#define	K_DATA_ERROR_ID		14
#define	K_SET_ATTRIBUTES_ID	15
#define	K_DESTROY_ID		16
#define	K_DATA_SUPPLY_ID	17

typedef struct net_msg		*net_msg_t;
typedef struct net_addr		*net_addr_t;
typedef struct net_proto	*net_proto_t;

struct net_addr {
	unsigned long	a[4];
};

#define	NET_ADDR_EQ(na1, na2)		\
	((na1)->a[0] == (na2)->a[0] &&	\
	 (na1)->a[1] == (na2)->a[1] &&	\
	 (na1)->a[2] == (na2)->a[2] &&	\
	 (na1)->a[3] == (na2)->a[3])

#define	NET_ADDR_NULL(na)		\
	((na)->a[0] == 0 &&		\
	 (na)->a[1] == 0 &&		\
	 (na)->a[2] == 0 &&		\
	 (na)->a[3] == 0)

struct net_msg {
	unsigned long	id;
	unsigned long	m_call;		/* boolean */
	unsigned long	arg1;
	unsigned long	arg2;
	unsigned long	arg3;
	unsigned long	arg4;
	unsigned long	arg5;
};

struct net_proto {
	kern_routine_t	server_init;
	kern_routine_t	server_send;
	struct net_addr	server_addr;
	char *		server_name;
};

#if 0
#ifdef	lint
#undef	ntohl
#undef	htonl
static unsigned long __ntohl(x) unsigned long x; { return x; }
static unsigned long __htonl(x) unsigned long x; { return x; }
#define	ntohl(x) __ntohl((unsigned long)(x))
#define	htonl(x) __htonl((unsigned long)(x))
#endif	lint
#endif
