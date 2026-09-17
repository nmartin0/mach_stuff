/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: port.h,v $
 * Revision 1.2.24.9  1996/01/09  19:22:12  devrcs
 * 	Changed mach_port_info_t from natural_t to integer_t to match
 * 	values expected by existing code (resolves compiler warnings).
 * 	in MACH_PORT_GEN(name), mask name with 0xff to avoid putting data
 * 	in the upper longword on 64-bit machines.
 * 	[1995/12/01  19:49:26  jfraser]
 *
 * 	Merged '64-bit safe' changes from DEC alpha port.
 * 	[1995/11/21  18:09:22  jfraser]
 *
 * Revision 1.2.24.8  1995/10/09  17:21:12  devrcs
 * 	Merge RT IPC
 * 	[1995/09/21  23:10:38  widyono]
 * 
 * Revision 1.2.24.7  1995/02/24  14:48:35  alanl
 * 	Merged with DIPC2_SHARED.
 * 	[1995/01/04  14:38:58  alanl]
 * 
 * Revision 1.2.27.4  1994/12/09  22:26:46  dwm
 * 	mk6 CR801 - merge up from nmk18b4 to nmk18b7
 * 	* Rev 1.2.24.5  1994/10/14  10:31:04  bolinger
 * 	  Fix ri-osc CR665:  export kernel port naming conventions to
 * 	  user space, where (e.g.) new libmach port-name-to-data mapping
 * 	  server can access them.
 * 	[1994/12/09  21:11:57  dwm]
 * 
 * Revision 1.2.27.3  1994/10/14  03:51:14  dwm
 * 	mk6 CR668 - 1.3b26 merge
 * 	64bits, casts
 * 	[1994/10/14  03:43:05  dwm]
 * 
 * Revision 1.2.27.1  1994/10/04  19:20:15  bolinger
 * 	Update mk6_shared (based on nmk18b4) to cnmk_shared.
 * 	[1994/10/04  19:06:38  bolinger]
 * 
 * Revision 1.2.24.4  1994/09/28  17:47:45  dswartz
 * 	Merged with changes from 1.2.24.3
 * 	[1994/09/28  17:46:18  dswartz]
 * 
 * 	mach_port_allocate* changes [CR 461]
 * 	[1994/09/28  17:39:04  dswartz]
 * 
 * Revision 1.2.24.3  1994/09/23  02:41:44  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:42:23  ezf]
 * 
 * Revision 1.2.24.2  1994/08/18  23:13:50  widyono
 * 	o struct mach_port_status, add mps_rt field
 * 	o add MACH_PORT_DNREQUESTS_SIZE flavor, count
 * 	[1994/07/28  22:35:40  widyono]
 * 
 * Revision 1.2.24.1  1994/08/07  20:49:47  bolinger
 * 	Merge up to colo_b7.
 * 	[1994/08/01  21:01:55  bolinger]
 * 
 * Revision 1.2.18.1  1994/06/25  03:47:14  dwm
 * 	mk6 CR98 - add flavor interface typedefs (mach_port_flavor_t).
 * 	[1994/06/24  21:54:53  dwm]
 * 
 * Revision 1.2.23.2  1994/12/27  20:05:23  alanl
 * 	Define MACH_PORT_RECEIVE_NO_NMS to allow applications with
 * 	special knowledge to allocate ports that don't offer NMS
 * 	detection.  Later, this option will become the default.
 * 	[94/12/27            alanl]
 * 
 * Revision 1.2.23.1  1994/08/04  02:28:40  mmp
 * 	NOTE: file was moved back to b11 version for dipc2_shared.
 * 	DIPC:  mach_port_status structure changes to permit
 * 	ports without no more senders detection.
 * 	[1994/05/20  18:30:10  alanl]
 * 
 * Revision 1.2.5.6  1993/09/09  16:07:49  jeffc
 * 	CR9745 - Delete message accepted notifications
 * 	[1993/09/03  20:45:45  jeffc]
 * 
 * Revision 1.2.5.5  1993/08/05  19:09:38  jeffc
 * 	CR9508 - Delete dead code. Remove MACH_IPC_COMPAT
 * 	[1993/08/04  21:00:39  jeffc]
 * 
 * 	CR9508 - Delete dead code. Remove MACH_IPC_COMPAT
 * 	[1993/08/03  17:09:23  jeffc]
 * 
 * Revision 1.2.5.4  1993/08/03  19:05:09  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	CR9600: Add mach_port_name_t typedef.
 * 	[1993/08/02  18:27:55  gm]
 * 
 * Revision 1.2.5.3  1993/06/29  17:54:40  brezak
 * 	Add flavors for mach_port_{set,get}_attibutes().
 * 	Remove old_mach_port_status_t.
 * 	[1993/06/22  18:32:20  brezak]
 * 
 * Revision 1.2.5.2  1993/06/09  02:43:00  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:17:55  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:38:36  devrcs
 * 	ansi C conformance changes
 * 	[1993/02/02  18:54:18  david]
 * 
 * Revision 1.1  1992/09/30  02:31:59  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.6.2.1  92/03/03  16:22:27  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  12:19:28  jeffreyh]
 * 
 * Revision 2.7  92/01/15  13:44:48  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.6  91/08/28  11:15:36  jsb
 * 	Added mach_port_seqno_t.  Added mps_seqno to mach_port_status_t
 * 	and created old_mach_port_status_t for compatibility purposes.
 * 	[91/08/09            rpd]
 * 
 * Revision 2.5  91/08/24  12:17:54  af
 * 	Added a set of parenthesis to MACH_PORT_TYPE to shut up gcc2.
 * 	[91/07/19            danner]
 * 
 * Revision 2.4.4.1  91/08/19  13:48:02  danner
 * 	Added a set of parenthesis to MACH_PORT_TYPE to shut up gcc2.
 * 	[91/07/19            danner]
 * 
 * Revision 2.4  91/05/14  16:58:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:27  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:17  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:44  rpd
 * 	Added mach_port_status_t.
 * 	[90/05/13            rpd]
 * 	Converted to new IPC.
 * 	[90/03/26  22:38:36  rpd]
 * 
 *
 * Condensed history:
 *	Put ownership rights under MACH_IPC_XXXHACK (rpd).
 *	Put PORT_ENABLED under MACH_IPC_XXXHACK (rpd).
 *	Removed PORT_INVALID (rpd).
 *	Added port set, port name, port type declarations (rpd).
 *	Added PORT_INVALID (mwyoung).
 *	Added port_*_t types (mwyoung).
 *	Added PORT_ENABLED (mwyoung).
 *	Created (mwyoung).
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 *	File:	mach/port.h
 *
 *	Definition of a port
 *
 *	[The basic mach_port_t type should probably be machine-dependent,
 *	as it must be represented by a 32-bit integer.]
 */

#ifndef	_MACH_PORT_H_
#define _MACH_PORT_H_

#if	MACH_KERNEL
#include <dipc.h>
#endif	/* MACH_KERNEL */

#include <mach/boolean.h>
#include <mach/machine/vm_types.h>

typedef natural_t mach_port_t;
typedef mach_port_t mach_port_name_t;
typedef mach_port_t *mach_port_array_t;

/*
 *  MACH_PORT_NULL is a legal value that can be carried in messages.
 *  It indicates the absence of any port or port rights.  (A port
 *  argument keeps the message from being "simple", even if the
 *  value is MACH_PORT_NULL.)  The value MACH_PORT_DEAD is also
 *  a legal value that can be carried in messages.  It indicates
 *  that a port right was present, but it died.
 */

#define MACH_PORT_NULL		((mach_port_t) 0)
#define MACH_PORT_DEAD		((mach_port_t) ~0)

/*
 *  mach_port_t must be an unsigned type.  Port values
 *  have two parts, a generation number and an index.
 *  These macros encapsulate all knowledge of how
 *  a mach_port_t is laid out.  They are made visible 
 *  to user tasks so that packages to map from a mach_port_t
 *  to associated user data can discount the generation
 *  nuber (if desired) in doing the mapping.
 *
 *  Within the kernel, ipc/ipc_entry.c implicitly assumes
 *  when it uses the splay tree functions that the generation
 *  number is in the low bits, so that names are ordered first
 *  by index and then by generation.  If the size of generation
 *  numbers changes, be sure to update IE_BITS_GEN_MASK and
 *  friends in ipc/ipc_entry.h.
 */
#ifndef NO_PORT_GEN
#define	MACH_PORT_INDEX(name)		((name) >> 8)
#define	MACH_PORT_GEN(name)		(((name) & 0xff) << 24)
#define	MACH_PORT_MAKE(index, gen)	\
		(((index) << 8) | (gen) >> 24)
#else
#define	MACH_PORT_INDEX(name)		(name)
#define	MACH_PORT_GEN(name)		(0)
#define	MACH_PORT_MAKE(index, gen)	(index)
#endif	/* !NO_PORT_GEN */
#define	MACH_PORT_MAKEB(index, bits)	\
		MACH_PORT_MAKE(index, IE_BITS_GEN(bits))

#define	MACH_PORT_VALID(name)	\
		(((name) != MACH_PORT_NULL) && ((name) != MACH_PORT_DEAD))

/*
 *  These are the different rights a task may have.
 *  The MACH_PORT_RIGHT_* definitions are used as arguments
 *  to mach_port_allocate, mach_port_get_refs, etc, to specify
 *  a particular right to act upon.  The mach_port_names and
 *  mach_port_type calls return bitmasks using the MACH_PORT_TYPE_*
 *  definitions.  This is because a single name may denote
 *  multiple rights.
 */

typedef natural_t mach_port_right_t;

#define MACH_PORT_RIGHT_SEND		((mach_port_right_t) 0)
#define MACH_PORT_RIGHT_RECEIVE		((mach_port_right_t) 1)
#define MACH_PORT_RIGHT_SEND_ONCE	((mach_port_right_t) 2)
#define MACH_PORT_RIGHT_PORT_SET	((mach_port_right_t) 3)
#define MACH_PORT_RIGHT_DEAD_NAME	((mach_port_right_t) 4)
#define MACH_PORT_RIGHT_NUMBER		((mach_port_right_t) 5)
#define	TEMPORARY_NO_NMS	DIPC
#if	TEMPORARY_NO_NMS
/*
 *	XXX This definition is TEMPORARY, to permit
 *	users in-the-know to allocate ports that don't
 *	use NMS detection.  Later, mach_port_allocate
 *	will DEFAULT to this case, and an option will
 *	permit allocating ports that DO support NMS.
 *	Confused yet?  XXX
 */
#define	MACH_PORT_RIGHT_RECEIVE_NO_NMS	((mach_port_right_t) 6)
#endif	/* TEMPORARY_NO_NMS */

typedef natural_t mach_port_type_t;
typedef mach_port_type_t *mach_port_type_array_t;

#define MACH_PORT_TYPE(right)						\
		((mach_port_type_t)(((mach_port_type_t) 1)  		\
		<< ((right) + ((mach_port_right_t) 16))))	
#define MACH_PORT_TYPE_NONE	    ((mach_port_type_t) 0L)
#define MACH_PORT_TYPE_SEND	    MACH_PORT_TYPE(MACH_PORT_RIGHT_SEND)
#define MACH_PORT_TYPE_RECEIVE	    MACH_PORT_TYPE(MACH_PORT_RIGHT_RECEIVE)
#define MACH_PORT_TYPE_SEND_ONCE    MACH_PORT_TYPE(MACH_PORT_RIGHT_SEND_ONCE)
#define MACH_PORT_TYPE_PORT_SET	    MACH_PORT_TYPE(MACH_PORT_RIGHT_PORT_SET)
#define MACH_PORT_TYPE_DEAD_NAME    MACH_PORT_TYPE(MACH_PORT_RIGHT_DEAD_NAME)

/* Convenient combinations. */

#define MACH_PORT_TYPE_SEND_RECEIVE					\
		(MACH_PORT_TYPE_SEND|MACH_PORT_TYPE_RECEIVE)
#define	MACH_PORT_TYPE_SEND_RIGHTS					\
		(MACH_PORT_TYPE_SEND|MACH_PORT_TYPE_SEND_ONCE)
#define	MACH_PORT_TYPE_PORT_RIGHTS					\
		(MACH_PORT_TYPE_SEND_RIGHTS|MACH_PORT_TYPE_RECEIVE)
#define	MACH_PORT_TYPE_PORT_OR_DEAD					\
		(MACH_PORT_TYPE_PORT_RIGHTS|MACH_PORT_TYPE_DEAD_NAME)
#define MACH_PORT_TYPE_ALL_RIGHTS					\
		(MACH_PORT_TYPE_PORT_OR_DEAD|MACH_PORT_TYPE_PORT_SET)

/* Dummy type bits that mach_port_type/mach_port_names can return. */

#define MACH_PORT_TYPE_DNREQUEST	0x80000000

/* User-references for capabilities. */

typedef natural_t mach_port_urefs_t;
typedef integer_t mach_port_delta_t;			/* change in urefs */

/* Attributes of ports.  (See mach_port_get_receive_status.) */

typedef natural_t mach_port_seqno_t;		/* sequence number */
typedef natural_t mach_port_mscount_t;		/* make-send count */
typedef natural_t mach_port_msgcount_t;		/* number of msgs */
typedef natural_t mach_port_rights_t;		/* number of rights */

/*
 *	A port may have NMS detection enabled, in which case
 *	it tracks outstanding send rights.  Otherwise, there
 *	is no information available about outstanding srights.
 *	The return values are deliberately chosen to match
 *	the old boolean (0=FALSE=no srights, 1=TRUE=srights,
 *	2=xxx=no information available).
 */
#define	MACH_PORT_SRIGHTS_NONE		0		/* NMS:  no srights */
#define	MACH_PORT_SRIGHTS_PRESENT	1		/* NMS:  srights */
#define	MACH_PORT_SRIGHTS_NO_INFO	2		/* no NMS */
typedef unsigned int mach_port_srights_t;	/* status of send rights */

typedef struct mach_port_status {
	mach_port_t		mps_pset;	/* containing port set */
	mach_port_seqno_t	mps_seqno;	/* sequence number */
	mach_port_mscount_t	mps_mscount;	/* make-send count */
	mach_port_msgcount_t	mps_qlimit;	/* queue limit */
	mach_port_msgcount_t	mps_msgcount;	/* number in the queue */
	mach_port_rights_t	mps_sorights;	/* how many send-once rights */
#if	DIPC
	mach_port_srights_t	mps_srights;	/* do send rights exist? */
#else	/* DIPC */
	boolean_t		mps_srights;	/* do send rights exist? */
#endif	/* DIPC */
	boolean_t		mps_pdrequest;	/* port-deleted requested? */
	boolean_t		mps_nsrequest;	/* no-senders requested? */
	unsigned int		mps_flags;	/* port flags */
} mach_port_status_t;

#define MACH_PORT_QLIMIT_DEFAULT	((mach_port_msgcount_t) 5)
#define MACH_PORT_QLIMIT_MAX		((mach_port_msgcount_t) 16)

typedef struct mach_port_limits {
	mach_port_msgcount_t	mpl_qlimit;	/* number of msgs */
} mach_port_limits_t;

typedef integer_t *mach_port_info_t;		/* varying array of natural_t */

/* Flavors for mach_port_get/set_attributes() */
typedef int	mach_port_flavor_t;
#define MACH_PORT_LIMITS_INFO		1	/* uses mach_port_status_t */
#define MACH_PORT_RECEIVE_STATUS	2	/* uses mach_port_limits_t */
#define MACH_PORT_DNREQUESTS_SIZE	3	/* info is int */

#define MACH_PORT_LIMITS_INFO_COUNT \
	(sizeof(mach_port_limits_t)/sizeof(natural_t))
#define MACH_PORT_RECEIVE_STATUS_COUNT \
	(sizeof(mach_port_status_t)/sizeof(natural_t))
#define MACH_PORT_DNREQUESTS_SIZE_COUNT 1

/*
 * Structure used to pass information about port allocation requests.
 * Must be padded to 64-bits total length.
 */

typedef struct mach_port_qos {
	boolean_t		name:1;		/* name given */
	boolean_t		rt:1;		/* real-time port */
	boolean_t		pad1:30;
	boolean_t		pad2:32;
} mach_port_qos_t;

#endif	/* _MACH_PORT_H_ */
