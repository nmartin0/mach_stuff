/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: mig_errors.h,v $
 * Revision 1.2.21.4  1996/07/31  09:57:31  paire
 * 	Merged with nmk20b7_shared (1.2.30.1)
 * 	[96/06/10            paire]
 *
 * Revision 1.2.30.1  1996/04/12  06:12:29  paire
 * 	Moved mig_user_{de}allocate() prototypes from kern/ipc_mig.h
 * 	[96/04/05            paire]
 * 
 * Revision 1.2.21.3  1995/04/13  23:03:20  barbou
 * 	Added "reply_port" argument to mig_dealloc_reply_port and added new
 * 	mig_put_reply_port routine declaration.
 * 	[95/04/14            barbou]
 * 
 * Revision 1.2.21.2  1994/09/23  02:40:24  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:41:49  ezf]
 * 
 * Revision 1.2.21.1  1994/06/13  20:49:59  dlb
 * 	Merge MK6 and NMK17
 * 	[1994/06/13  20:48:10  dlb]
 * 
 * Revision 1.2.13.1  1994/01/20  04:39:09  condict
 * 	Delete mig_entry_t.  (Superceded by user_subsystem_t)
 * 	[1994/01/15  22:05:05  condict]
 * 
 * Revision 1.2.19.1  1994/02/08  11:01:16  bernadat
 * 	Keep replysize for each individual service in the mig_entry_t
 * 	structure. This allows to allocate the right size on the kernel
 * 	side instead of allocating the maximum reply size (more than 8K
 * 	for some subsystems)
 * 	[94/01/27            bernadat]
 * 	[94/02/04            bernadat]
 * 
 * Revision 1.2.9.6  1993/08/05  19:09:33  jeffc
 * 	CR9508 - delete dead code - remove MACH_IPC_TYPED and MACH_IPC_COMPAT
 * 	[1993/08/03  21:26:42  jeffc]
 * 
 * Revision 1.2.9.5  1993/07/28  22:03:25  travos
 * 	CR9523 - Define and use mig_routine_t type.
 * 	Clean up some of the ifdef.
 * 	Add three extern that need to used in kernel and user
 * 	space (mig_strncpy, mig_get_reply_port, mig_dealloc_reply_port)
 * 	[1993/07/28  20:49:55  travos]
 * 
 * Revision 1.2.9.4  1993/07/19  18:38:17  travos
 * 	In mig_entry_t, use the type mach_msg_id_t.
 * 	[1993/07/19  18:37:50  travos]
 * 
 * Revision 1.2.9.3  1993/07/12  18:07:36  gm
 * 	CR9339: Added an (incorrect) full ANSI-C prototype for ms_routine
 * 	function pointer.
 * 	[1993/07/12  13:37:08  gm]
 * 
 * Revision 1.2.9.2  1993/06/09  02:42:22  gm
 * 	Add MIG_TRAILER_ERROR.  CR #8992.
 * 	[1993/04/30  21:10:32  travos]
 * 
 * 	Define mig_reply_header_t if MACH_IPC_TYPED or
 * 	MACH_IPC_COMPAT. CR #8972.
 * 	[1993/04/27  11:29:26  rod]
 * 
 * Revision 1.2  1993/04/19  16:36:24  devrcs
 * 	Merge untyped ipc:
 * 	Compile errors when MACH_IPC_TYPED=0
 * 	[1993/02/19  15:48:34  travos]
 * 	New structures for Untyped IPC:  mig_reply_error_t and mig_entry_t
 * 	[1993/02/17  15:16:03  travos]
 * 
 * 	ansi C conformance changes
 * 	[1993/02/02  18:53:46  david]
 * 
 * Revision 1.1  1992/09/30  02:31:30  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.5.3.1  92/03/03  16:22:20  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  12:10:53  jeffreyh]
 * 
 * Revision 2.7  92/01/15  13:44:38  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.6  92/01/03  20:21:52  dbg
 * 	Add mig_routine_t.
 * 	[91/11/11            dbg]
 * 
 * Revision 2.5  91/08/28  11:15:31  jsb
 * 	Added MIG_SERVER_DIED.
 * 	[91/08/21            rpd]
 * 
 * Revision 2.4  91/05/14  16:56:33  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:34:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:19:44  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:14  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:37:01  rpd]
 * 
 * Revision 2.1  89/08/03  16:03:33  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:38:41  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  01:01:21  mwyoung
 * Relocated from sys/mig_errors.h
 * 
 * Revision 2.2  88/07/20  21:05:51  rpd
 * Added definition of mig_symtab_t.
 * 
 *  2-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Added MIG_ARRAY_TOO_LARGE.
 *
 * 25-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Added definition of death_pill_t.
 *
 * 31-Jul-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Created.
 *
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
 * Mach Interface Generator errors
 *
 */

#ifndef	_MACH_MIG_ERRORS_H_
#define _MACH_MIG_ERRORS_H_

#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/rpc.h>
#include <mach/vm_types.h>

/*
 *	These error codes should be specified as system 4, subsytem 2.
 *	But alas backwards compatibility makes that impossible.
 *	The problem is old clients of new servers (eg, the kernel)
 *	which get strange large error codes when there is a Mig problem
 *	in the server.  Unfortunately, the IPC system doesn't have
 *	the knowledge to convert the codes in this situation.
 */

#define MIG_TYPE_ERROR		-300	/* client type check failure */
#define MIG_REPLY_MISMATCH	-301	/* wrong reply message ID */
#define MIG_REMOTE_ERROR	-302	/* server detected error */
#define MIG_BAD_ID		-303	/* bad request message ID */
#define MIG_BAD_ARGUMENTS	-304	/* server type check failure */
#define MIG_NO_REPLY		-305	/* no reply should be send */
#define MIG_EXCEPTION		-306	/* server raised exception */
#define MIG_ARRAY_TOO_LARGE	-307	/* array not large enough */
#define MIG_SERVER_DIED		-308	/* server died */
#define MIG_TRAILER_ERROR       -309    /* trailer has an unknown format */

#include <mach/ndr.h>

typedef struct {
	mach_msg_header_t	Head;
	NDR_record_t		NDR;
	kern_return_t		RetCode;
} mig_reply_error_t;

typedef struct mig_symtab {
	char	*ms_routine_name;
	int	ms_routine_number;
	void    (*ms_routine)(void);	/* Since the functions in the
					 * symbol table have unknown
					 * signatures, this is the best
					 * we can do...
					 */
} mig_symtab_t;

/* Client side reply port allocate */
extern mach_port_t mig_get_reply_port(void);

/* Client side reply port deallocate */
extern void mig_dealloc_reply_port(mach_port_t reply_port);

/* Client side reply port "deallocation" */
extern void mig_put_reply_port(mach_port_t reply_port);

/* Allocate memory for out-of-stack mig structures */
extern char *mig_user_allocate(vm_size_t size);

/* Deallocate memory used for out-of-stack mig structures */
extern void mig_user_deallocate(char *data, vm_size_t size);

/* Bounded string copy */
extern int mig_strncpy(
	char	*dest,
	char	*src,
	int	len);

#endif	/* _MACH_MIG_ERRORS_H_ */
