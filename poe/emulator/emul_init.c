/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	emul_init.c,v $
 * Revision 2.5  94/03/25  18:21:16  mrt
 * 	Corrected include of mach/mach.h to mach.h.
 * 	[93/11/15            mrt]
 * 
 * Revision 2.4  92/02/02  13:00:21  rpd
 * 	Removed MAP_FILE and MAP_UAREA code.
 * 	[92/01/30            rpd]
 * 
 * Revision 2.3  91/12/19  20:20:09  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:39:52  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:20:32  rwd]
 * 
 * Revision 2.4  90/08/06  15:30:13  rwd
 * 	Comment potential problem in main().
 * 	[90/07/19            rwd]
 * 
 * Revision 2.3  90/06/02  15:20:29  rpd
 * 	Converted to new IPC.
 * 	Added emul_panic.
 * 	[90/03/26  19:28:17  rpd]
 * 
 * Revision 2.2  90/03/14  21:22:55  rwd
 * 	Added conditional call to emul_mapped_init.
 * 	[90/01/23            rwd]
 * 
 */
/*
 * Setup emulator entry points.
 */
#include <mach.h>
#include <mach/message.h>
#include "emul_stack.h"

/*
 * Initialize emulator.  Our bootstrap port is the BSD request port.
 */
mach_port_t	our_bsd_server_port;

emul_stack_t
emul_init()
{
	emul_stack_t stack;

	/*
	 * Bootstrap port is the request port.
	 */
	(void) task_get_bootstrap_port(mach_task_self(), &our_bsd_server_port);

	/*
	 * Set up emulator stack support.
	 */
	stack = emul_stack_init();

#ifdef ECACHE
	/*
	 * Setup cache.
	 */
	emul_cache_init();
#endif ECACHE

	return stack;
}

main(argc, argv)
	int	argc;
	char	**argv;
{
	emul_stack_t stack;

	/*
	 * Initialize the emulator.
	 */
	stack = emul_init();

	/*
	 * Set up the emulator vectors.
	 */
	emul_setup(mach_task_self());

	/*
	 * It is now OK to use the emulator stack reply port
	 */
	mig_init(stack);

	/*
	 * Don't on your life try to use a mig stub here without
	 * specifying the reply_port.  The stack has not been switched
	 * yet, and it won't work.  The emul_execve call results in the
	 * desired stack switch occuring (though it itself never returns)
	 */

	/*
	 * Exec the first program.
	 */
	return (emul_execve(argv[0], argv, (char **)0));
}


/*
 * Initialize emulator on child branch of fork.
 */
void
child_init()
{
	emul_stack_t stack;

	/*
	 * Re-initialize ALL of the emulator - saved
	 * port numbers are no longer valid, and
	 * we have a new request port.
	 */
	mach_init();		/* reset mach_task_self_ ! */
	stack = emul_init();

	/*
	 * It is now OK to use the emulator stack reply port
	 */
	mig_init(stack);
}

/*
 * Fail in an interesting, easy-to-debug way.
 */
void
emul_panic(msg)
	char *msg;
{
	for (;;)
		(void) mach_msg_trap((mach_msg_header_t *) msg,
				     MACH_MSG_OPTION_NONE, /* do nothing */
				     0, 0, MACH_PORT_NULL,
				     MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
}
