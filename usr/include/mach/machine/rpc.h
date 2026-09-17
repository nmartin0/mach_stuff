/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: rpc.h,v $
 * Revision 1.1.12.4  1996/07/31  09:56:50  paire
 * 	Merged with nmk20b7_shared (1.1.19.1)
 * 	[96/06/10            paire]
 *
 * Revision 1.1.19.1  1996/03/22  16:06:06  bruel
 * 	register renaming cleanup.
 * 	[96/03/22            bruel]
 * 
 * Revision 1.1.12.3  1995/11/02  14:59:27  bruel
 * 	fixed klcopy* prototypes.
 * 	[95/09/26            bruel]
 * 
 * Revision 1.1.12.2  1995/05/15  18:29:50  bruel
 * 	Added MACH_RPC.
 * 	[95/05/15            bruel]
 * 
 * Revision 1.1.12.1  1995/03/15  17:47:31  bruel
 * 	hppa merge
 * 	[1995/03/15  09:45:37  bruel]
 * 
 * Revision 1.1.5.3  1994/07/08  17:39:01  bruel
 * 	fixed klthread_switch and klthread_depress_abort calls.
 * 	[94/07/08            bruel]
 * 
 * Revision 1.1.5.2  1994/07/05  08:37:36  bruel
 * 	fixed copyin/copyout definitions.
 * 	[94/07/04            bruel]
 * 
 * Revision 1.1.5.1  1994/05/25  13:15:59  bruel
 * 	Adapted from i386.
 * 	[94/05/19            bruel]
 * 
 * $EndLog$
 */

#ifndef _MACH_HP700_RPC_H_
#define _MACH_HP700_RPC_H_

#include <mach/kern_return.h>
#include <mach/port.h>

/*
 * Definition of RPC "glue code" operations vector -- entry
 * points needed to accomplish short-circuiting
 */
typedef struct rpc_glue_vector {
	kern_return_t	(*rpc_simple)(int, int, void *);
	boolean_t	(*copyin)(mach_port_t, const char *, char *, int);
	boolean_t	(*copyout)(mach_port_t, const char *, char *, int);
	boolean_t	(*copyinstr)(mach_port_t, const char *, char *, int, int*);
	kern_return_t	(*thread_switch)(mach_port_t, int, int);
	kern_return_t	(*thread_depress_abort)(mach_port_t);
} *rpc_glue_vector_t;

/*
 * Macros used to dereference glue code ops vector -- note
 * hard-wired references to global defined below. 
 */
#define CAN_SHCIRCUIT(name)	(_rpc_glue_vector->name != 0)
#define RPC_SIMPLE(port, rtn_num, argc, argv) \
	((*(_rpc_glue_vector->rpc_simple))(rtn_num, argc, (void *)(&(port))))
#define COPYIN(map, from, to, count) \
	((*(_rpc_glue_vector->copyin))(map, from, to, count))
#define COPYOUT(map, from, to, count) \
	((*(_rpc_glue_vector->copyout))(map, from, to, count))
#define COPYINSTR(map, from, to, max, actual) \
	((*(_rpc_glue_vector->copyinstr))(map, from, to, max, actual))
#define THREAD_SWITCH(thread, option, option_time) \
	((*(_rpc_glue_vector->thread_switch))(thread, option, option_time))
#define	THREAD_DEPRESS_ABORT(act)	\
	((*(_rpc_glue_vector->thread_depress_abort))(act))


/*
 * User machine dependent macros for mach rpc
 */
#define MACH_RPC(sig_ptr, sig_size, id, dest, arg_list)			   \
	mach_rpc_trap(dest, id, (mach_rpc_signature_t) sig_ptr, sig_size)

/*
 * The implementation of glue functions for hp_pa
 */
extern boolean_t klcopyin(mach_port_t, const char *, char *, int);
extern boolean_t klcopyout(mach_port_t, const char *, char *, int);
extern boolean_t klcopyinstr(mach_port_t, const char *, char *, int, int *);
extern kern_return_t klthread_switch(mach_port_t, int, int);

/*
 * An rpc_glue_vector_t defined either by the kernel or by crt0
 */
extern rpc_glue_vector_t _rpc_glue_vector;

#define MACH_RPC_ARGV(act)	(char*)(USER_REGS(act)->arg1)  
#define MACH_RPC_RET(act)	( USER_REGS(act)->rp ) 
#define MACH_RPC_UIP(act)	( USER_REGS(act)->iioq_head )
#define MACH_RPC_USP(act)	( USER_REGS(act)->sp )

#endif	/* _MACH_HP700_RPC_H_ */
