/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: rpc.h,v $
 * Revision 1.1.15.15  1996/01/09  19:22:24  devrcs
 * 	Conditionalized the machine dependent function protos for
 * 	machine_rpc_simple() and klthread_depress_abort().
 * 	[1995/12/01  19:49:32  jfraser]
 *
 * 	Merged '64-bit safe' changes from DEC alpha port.
 * 	[1995/11/21  18:09:40  jfraser]
 *
 * Revision 1.1.15.14  1995/10/09  17:21:17  devrcs
 * 	Merge forward.
 * 	[1995/08/24  22:36:01  watkins]
 * 
 * Revision 1.1.51.1  1995/08/04  17:08:22  watkins
 * 	Use vm_size_t in copyfunc_t.
 * 	[1995/08/04  15:22:27  watkins]
 * 
 * Revision 1.1.15.13  1995/09/18  19:20:50  devrcs
 * 	Merge forward.
 * 	[1995/08/24  22:36:01  watkins]
 * 
 * Revision 1.1.51.1  1995/08/04  17:08:22  watkins
 * 	Use vm_size_t in copyfunc_t.
 * 	[1995/08/04  15:22:27  watkins]
 * 
 * Revision 1.1.15.12  1995/05/14  18:20:08  dwm
 * 	ri-osc CR1304 - merge (nmk19_latest - nmk19b1) diffs into mainline.
 * 	Declare the libmach function mach_subsystem_join, for creating the union
 * 	of two subsystems.  [ condict]
 * 	[1995/05/14  17:34:53  dwm]
 * 
 * Revision 1.1.15.11  1995/03/15  17:19:47  bruel
 * 	put back machine dependant code where it belongs.
 * 	[95/03/07            bruel]
 * 
 * Revision 1.1.15.10  1995/02/24  14:48:39  alanl
 * 	Moved machine independent definitions and declarations out of the
 * 	i386 directory.
 * 	[95/01/11            sjs]
 * 
 * Revision 1.1.28.2  1994/12/09  22:26:56  dwm
 * 	mk6 CR801 - merge up from nmk18b4 to nmk18b7
 * 	Comments from merged code below, as marked
 * 	[1994/12/09  21:14:04  dwm]
 * 
 * Revision 1.1.15.8  1994/11/23  16:03:28  devrcs
 * 	BEGIN comments from merge of nmk18b4 - nmk18b7
 * 	Reinstate declaration for unwind_invoke_state().
 * 	[1994/11/10  16:28:12  watkins]
 * 
 * 	Remove declarartion for unwind_invoke_state().
 * 	[1994/11/09  21:50:25  watkins]
 * 
 * 	Check in for merge.
 * 	[1994/11/08  22:15:45  watkins]
 * 
 * Revision 1.1.22.5  1994/11/08  21:47:18  rkc
 * 	Modified definition of `function_ptr_t' again.
 * 	[1994/11/08  14:00:25  rkc]
 * 
 * 	Added prototype for `unwind_invoke_state()' and changed
 * 	definition of `function_ptr_t.'
 * 	[1994/10/19  19:07:49  rkc]
 * 
 * Revision 1.1.31.2  1994/11/08  15:34:16  watkins
 * 	Add declaration for rpc return error function.
 * 
 * Revision 1.1.15.7  1994/10/28  16:34:42  condict
 * 	Remove obsolete rpc_set_obj_value_type, rpc_get_obj_type/value macros.
 * 	These have been replaced by the libmach "port_set/get" macros, which
 * 	do not depend on port names being pointers to ports.  See
 * 	mach_services/include/port_obj.h.
 * 	[1994/10/24  18:37:52  condict]
 * 
 * Revision 1.1.15.6  1994/10/19  16:25:42  watkins
 * 	Make kernel buffer size smaller.
 * 	[1994/10/11  15:05:18  watkins]
 * 
 * Revision 1.1.15.5  1994/10/14  10:38:01  bolinger
 * 	Work around ri-osc CR705:  change rpc_error() not to output
 * 	GREEN errors by default.
 * 	[1994/10/13  22:35:42  bolinger]
 * 
 * Revision 1.1.15.4  1994/10/07  16:48:08  bolinger
 * 	ri-osc CR686: Enable mach_rpc() to work to a collocated
 * 	target task.
 * 	END comments from merge of nmk18b4 - nmk18b7
 * 	[1994/10/07  16:33:47  bolinger]
 * 
 * Revision 1.1.28.1  1994/11/02  18:37:58  dwm
 * 	mk6 CR668 - 1.3b26 merge
 * 	rename ROUNDUP to ROUNDUP2, since it deals only with power-of-2
 * 	rounding, and conflicts with profiling/profile-internal.h
 * 	[1994/11/02  18:17:51  dwm]
 * 
 * Revision 1.1.15.3  1994/09/23  02:42:14  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:42:38  ezf]
 * 
 * Revision 1.1.15.2  1994/09/02  02:40:57  watkins
 * 	Put internal declarations under ifdef.
 * 	[1994/09/02  02:38:08  watkins]
 * 
 * 	Add prototype for clean_port_array.
 * 	[1994/08/31  13:59:29  watkins]
 * 
 * Revision 1.1.15.1  1994/08/26  20:48:54  watkins
 * 	Merge with rt2_shared.
 * 	[1994/08/26  18:42:55  watkins]
 * 
 * Revision 1.1.12.4  1994/08/24  20:23:22  watkins
 * 	For now, all port arrays are variable arrays.
 * 	[1994/08/24  20:22:47  watkins]
 * 
 * Revision 1.1.12.3  1994/08/18  18:56:11  watkins
 * 	Use element size and element count in arg descriptor.
 * 	[1994/08/18  18:36:54  watkins]
 * 
 * Revision 1.1.12.2  1994/08/17  14:24:34  watkins
 * 	Conditionalize debug code. Add cpp synonyms.
 * 	[1994/08/17  14:23:56  watkins]
 * 
 * Revision 1.1.12.1  1994/07/18  22:04:10  burke
 * 	Check-in for merge.
 * 	[1994/07/15  21:04:58  burke]
 * 
 * Revision 1.1.10.3  1994/07/05  14:35:37  watkins
 * 	Merge with rpc.
 * 	[1994/07/05  14:32:34  watkins]
 * 
 * Revision 1.1.3.7  1994/06/08  20:17:46  dswartz
 * 	Preemption merge.
 * 	[1994/06/08  20:17:17  dswartz]
 * 
 * Revision 1.1.3.6  1994/05/03  20:51:00  condict
 * 	Add max_reply_msg field to the subsystem, as part of the MIG -maxonstack
 * 	changes.
 * 	[1994/05/03  20:48:33  condict]
 * 
 * 	Add a max_reply_msg field to the routine descriptor, for Mach.
 * 	[1994/04/30  02:16:30  condict]
 * 
 * Revision 1.1.3.5  1994/04/29  20:55:58  dwm
 * 	Pull out elements in common with ipc_port and ipc_pset into a
 * 	separate structure, making the dependencies explicit.
 * 	Renamed those fields to facilitate the usual macro tricks.
 * 	[1994/04/29  20:54:08  dwm]
 * 
 * Revision 1.1.3.4  1994/01/22  03:39:59  bolinger
 * 	Temporarily add new fields to struct rpc_port for use by server
 * 	as "object" type and value, since currently their kernel-known
 * 	equivalents can't reliably be used by a server.
 * 	[1994/01/21  22:49:02  bolinger]
 * 
 * Revision 1.1.3.3  1994/01/21  02:51:09  bolinger
 * 	Change object handle interface to reflect latest port naming
 * 	conventions.
 * 	[1994/01/21  02:50:51  bolinger]
 * 
 * Revision 1.1.3.2  1994/01/20  17:47:10  emcmanus
 * 	Quick fixes to allow the server to compile.  This version must be
 * 	corrected, if only to make the ipc_kobject_set call correctly.
 * 	[1994/01/20  17:46:46  emcmanus]
 * 
 * Revision 1.1.3.1  1994/01/20  10:58:39  emcmanus
 * 	Copied for submission.
 * 	[1994/01/20  10:57:45  emcmanus]
 * 
 * Revision 1.1.1.3  1994/01/20  02:49:30  condict
 * 	Moved rpc_subsystem and routine declarations here, from subsystem.h.
 * 
 * Revision 1.1.1.2  1994/01/15  22:06:06  condict
 * 	Add defs of things for fast RPC.
 * 
 * $EndLog$
 */

/*
 * Mach RPC Subsystem Interfaces
 */

#ifndef	_MACH_RPC_H_
#define _MACH_RPC_H_

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/vm_types.h>

#include <mach/mig_errors.h>
#include <mach/machine/rpc.h>
#include <mach/ipc_common.h>

#ifdef	MACH_KERNEL
#include <ipc/ipc_object.h>
#endif	/* MACH_KERNEL */

/*
 * The various bits of the type field of the routine_arg_descriptor
 */

/* The basic types */

#define TYPE_SHIFT                      0
#define MACH_RPC_PORT			(1 << TYPE_SHIFT)
#define MACH_RPC_ARRAY                  (1 << (TYPE_SHIFT + 1))
#define MACH_RPC_VARIABLE               (1 << (TYPE_SHIFT + 2))
#define LAST_TYPE_BIT                   (TYPE_SHIFT+3)

/* XXX Port arrays need not be variable arrays, as assumed below. Fixme. */
#define MACH_RPC_ARRAY_FIX		(MACH_RPC_ARRAY)
#define MACH_RPC_ARRAY_FIXED		(MACH_RPC_ARRAY)
#define MACH_RPC_ARRAY_VAR		(MACH_RPC_ARRAY | MACH_RPC_VARIABLE)
#define MACH_RPC_ARRAY_VARIABLE		(MACH_RPC_ARRAY | MACH_RPC_VARIABLE)
#define MACH_RPC_PORT_ARRAY		(MACH_RPC_PORT  | MACH_RPC_ARRAY_VAR)

/* Argument direction bits */

#define DIRECT_SHIFT            	LAST_TYPE_BIT
#define DIRECTION_SHIFT                 LAST_TYPE_BIT
#define MACH_RPC_IN			(1 << DIRECTION_SHIFT)
#define MACH_RPC_OUT			(1 << (DIRECTION_SHIFT + 1))
#define LAST_DIRECT_BIT         	(DIRECTION_SHIFT + 2)
#define LAST_DIRECTION_BIT              (DIRECTION_SHIFT + 2)

#define MACH_RPC_INOUT			(MACH_RPC_IN | MACH_RPC_OUT)

/* Persist and pointer bit */

#define POINTER_SHIFT                   LAST_DIRECTION_BIT
#define MACH_RPC_POINTER                (1 << POINTER_SHIFT)
#define LAST_POINTER_BIT                (POINTER_SHIFT + 1)

/* Port disposition bits */

#define NAME_SHIFT                    	LAST_POINTER_BIT
#define MACH_RPC_RECEIVE                (1 << NAME_SHIFT)
#define MACH_RPC_SEND                   (2 << NAME_SHIFT)
#define MACH_RPC_SEND_ONCE              (3 << NAME_SHIFT)
#define LAST_NAME_BIT			(NAME_SHIFT + 2)

#define ACTION_SHIFT			LAST_NAME_BIT
#define MACH_RPC_MOVE                   (1 << ACTION_SHIFT)
#define MACH_RPC_COPY                   (2 << ACTION_SHIFT)
#define MACH_RPC_MAKE                   (3 << ACTION_SHIFT)
#define LAST_ACTION_BIT                 (ACTION_SHIFT + 2)

#define MACH_RPC_MOVE_RECEIVE		(MACH_RPC_MOVE | MACH_RPC_RECEIVE)
#define MACH_RPC_MOVE_SEND		(MACH_RPC_MOVE | MACH_RPC_SEND)
#define MACH_RPC_COPY_SEND		(MACH_RPC_COPY | MACH_RPC_SEND)
#define MACH_RPC_MAKE_SEND		(MACH_RPC_MAKE | MACH_RPC_SEND)
#define MACH_RPC_MOVE_SEND_ONCE		(MACH_RPC_MOVE | MACH_RPC_SEND_ONCE)
#define MACH_RPC_MAKE_SEND_ONCE		(MACH_RPC_MAKE | MACH_RPC_SEND_ONCE)

/* Hint for virtual vs. physical copy */ 

#define OPTION_SHIFT                    LAST_ACTION_BIT
#define MACH_RPC_PHYSICAL_COPY		(1 << OPTION_SHIFT)
#define MACH_RPC_VIRTUAL_COPY		(1 << (OPTION_SHIFT + 1))
#define LAST_OPTION_BIT                 (OPTION_SHIFT + 2)

/* Deallocate? */

#define DEALLOCATE_SHIFT                LAST_OPTION_BIT
#define MACH_RPC_DEALLOCATE		(1 << DEALLOCATE_SHIFT)
#define LAST_DEALLOCATE_BIT             (DEALLOCATE_SHIFT + 1)

/* Argument is already on the stack */

#define ONSTACK_SHIFT                   LAST_DEALLOCATE_BIT
#define MACH_RPC_ONSTACK		(1 << ONSTACK_SHIFT)
#define LAST_ONSTACK_BIT                (ONSTACK_SHIFT + 1)

/* Is variable array bounded? Derived from type and arg.size */

#define BOUND_SHIFT			LAST_ONSTACK_BIT
#define MACH_RPC_BOUND			(1 << BOUND_SHIFT)
#define MACH_RPC_UNBOUND		(0)
#define BOUND				MACH_RPC_BOUND
#define UNBND				MACH_RPC_UNBOUND
#define LAST_BOUND_BIT			(BOUND_SHIFT + 1)

/*
 * Temporarily map old MiG names to the new names, until the 
 * new MiG is ready. This allows us to continue developing the
 * RPC path while MiG is being enhanced. *Temporary*
 */
#define OLD	
#ifdef OLD
#define ROUTINE_ARG_PORT	MACH_RPC_PORT
#define ROUTINE_ARG_FIXED_ARRAY	MACH_RPC_ARRAY_FIXED
#define	ROUTINE_ARG_VAR_ARRAY	MACH_RPC_ARRAY_VARIABLE

#define	ROUTINE_ARG_IN		MACH_RPC_IN
#define ROUTINE_ARG_OUT		MACH_RPC_OUT
#define ROUTINE_ARG_INOUT	MACH_RPC_INOUT	

#define direction		type
#endif 	/* OLD */


/*
 * Basic mach rpc types.
 */
typedef unsigned int    routine_arg_type;
typedef unsigned int	routine_arg_offset;
typedef unsigned int	routine_arg_size;

/*
 * Definition for MIG-generated server stub routines.  These routines
 * unpack the request message, call the server procedure, and pack the
 * reply message.
 */
typedef void	(*mig_stub_routine_t) (mach_msg_header_t *InHeadP,
				       mach_msg_header_t *OutHeadP);

typedef mig_stub_routine_t mig_routine_t;

/*
 * Definition for server implementation routines.  This is the routine
 * called by the MIG-generated server stub routine.
 */
typedef kern_return_t   (*mig_impl_routine_t)(void);

/*
 * Definitions for a signature's argument and routine descriptor's.
 */
struct routine_arg_descriptor {
	routine_arg_type	type;	   /* Port, Array, etc. */
        routine_arg_size        size;      /* element size in bytes */
        routine_arg_size        count;     /* number of elements */
	routine_arg_offset	offset;	   /* Offset in list of routine args */
};
typedef struct routine_arg_descriptor *routine_arg_descriptor_t;

struct routine_descriptor {
	mig_impl_routine_t	impl_routine;	/* Server work func pointer   */
	mig_stub_routine_t	stub_routine;	/* Unmarshalling func pointer */
	unsigned int		argc;		/* Number of argument words   */
	unsigned int		descr_count;	/* Number of complex argument */
					        /* descriptors                */
	struct routine_arg_descriptor *
				arg_descr;	/* Pointer to beginning of    */
						/* the arg_descr array        */
	unsigned int		max_reply_msg;	/* Max size for reply msg     */
};
typedef struct routine_descriptor *routine_descriptor_t;

struct mach_rpc_signature {
    struct routine_descriptor rd;
    struct routine_arg_descriptor rad[1];
};
typedef struct mach_rpc_signature *mach_rpc_signature_t;

/*
 *	A subsystem describes a set of server routines that can be invoked by
 *	mach_rpc() on the ports that are registered with the subsystem.  For
 *	each routine, the routine number is given, along with the
 *	address of the implementation function in the server and a
 *	description of the arguments of the routine (it's "signature").
 *
 *	This structure definition is only a template for what is really a
 *	variable-length structure (generated by MIG for each subsystem).
 *	The actual structures do not always have one entry in the routine
 *	array, and also have a varying number of entries in the arg_descr
 *	array.  Each routine has an array of zero or more arg descriptors
 *	one for each complex arg.  These arrays are all catenated together
 *	to form the arg_descr field of the subsystem struct.  The
 *	arg_descr field of each routine entry points to a unique sub-sequence
 *	within this catenated array.  The goal is to keep everything
 *	contiguous.
 */
struct rpc_subsystem {
	struct subsystem *subsystem;	/* Reserved for system use */

	mach_msg_id_t	start;		/* Min routine number */
	mach_msg_id_t	end;		/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max mach_msg size */
	vm_address_t	base_addr;	/* Address of this struct in user */

	struct routine_descriptor	/* Array of routine descriptors */
			routine[1       /* Actually, (start-end+1) */
				 ];

	struct routine_arg_descriptor
			arg_descriptor[1   /* Actually, the sum of the descr_ */
					]; /* count fields for all routines   */
};
typedef struct rpc_subsystem  *rpc_subsystem_t;

#define RPC_SUBSYSTEM_NULL	((rpc_subsystem_t) 0)

/*
 * This structure is user-visible, both to user-mode RPC code
 * and to kernel-loaded servers.
 */
typedef struct rpc_port {

	/*
	 * Initial sub-structure in common with ipc_pset and rpc_port
	 * First element is an ipc_object
	 */
	struct ipc_common_data rpc_port_comm;

	unsigned int rpc_unused_pad[1];	/* XXX unused */

} *rpc_port_t;

#define rp_object		rpc_port_comm.icd_object
				/* access to rp_object internals not needed */
#define rp_kobject		rpc_port_comm.icd_kobject
#define rp_subsystem		rpc_port_comm.icd_subsystem
#define rp_sobject		rpc_port_comm.icd_sobject
#define rp_sbits		rpc_port_comm.icd_sbits
#define rp_receiver_name	rpc_port_comm.icd_receiver_name


/* 
 * 	New RPC declarations
 *
 *	First pass at definitions and types for the new rpc service.
 *	This is subject to revision.
 */

/*
 *	RPC macros
 */

#define RPC_MASK(shift,last)                                            \
        ( ((1 << ((last)-(shift)))-1) << (shift) )

#define RPC_FIELD(field,shift,last)                                     \
        ( (field) & (((1 << ((last)-(shift)))-1) << (shift)) )

#define RPC_BOUND(dsc)                                                  \
        (((RPC_FIELD((dsc).type,TYPE_SHIFT+1,TYPE_SHIFT+3) ==           \
           MACH_RPC_ARRAY_VARIABLE) && (dsc).count != 0) ? MACH_RPC_BOUND : 0)

#define ROUNDUP2(x,n)    ((((unsigned)(x)) + (n) - 1) & ~((n)-1))
#define ROUNDWORD(x)    ROUNDUP2(x,sizeof(int))

/*
 *      RPC errors
 *
 *      Display and process errors of different severity, from just for
 *      information only to fatal (panic). Error code colors indicate how
 *      difficult it is for the subsystem to handle the error correctly.
 *      The implication is that, for example, early versions of the code may
 *      not be handling code red errors properly. The code should use this
 *      facility instead of regular printf's.
 */

#define	MACH_RPC_DEBUG	1

#define ERR_INFO        1               /* purely informational */
#define ERR_GREEN       2               /* easily handled error */
#define ERR_YELLOW      3               /* medium difficult error */
#define ERR_RED         4               /* difficult to handle error */
#define ERR_FATAL       5               /* unrecoverable error, panic */

#if MACH_RPC_DEBUG > 1
#define rpc_error(E,S)                                                  \
        printf("RPC error ");                                           \
        rpc_error_show_severity(S);                                     \
        printf("in file \"%s\", line %d: ", __FILE__, __LINE__);        \
        printf E ;                                                      \
        printf("\n");                                                   \
        rpc_error_severity(S)
#else
#define rpc_error(E,S)                                                  \
	if ((S) == ERR_FATAL || (S) == ERR_RED) {			\
        printf("RPC error ");                                           \
        rpc_error_show_severity(S);                                     \
        printf("in file \"%s\", line %d: ", __FILE__, __LINE__);        \
        printf E ;                                                      \
        printf("\n");                                                   \
        rpc_error_severity(S);						\
	}
#endif	/* MACH_RPC_DEBUG */

/*
 *      RPC buffer size and break points
 *
 *      These values define the rpc buffer size on the kernel stack,
 *      and break point values for switching to virtual copy (cow).
 *      This should be in a machine dependent include file. All sizes
 *      are in word (sizeof(int)) units.
 */

#define RPC_KBUF_SIZE   16              /* kernel stack buffer size (ints) */
#define RPC_COW_SIZE    1024            /* size where COW is a win (ints) */
#define RPC_DESC_COUNT  4               /* default descriptor count */


/*
 *      RPC copy state
 *
 *      Record the rpc copy state for arrays, so we can unwind our state
 *      during error processing. There is one entry per complex (signatured)
 *      argument. The first entry is marked COPY_TYPE_ALLOC_KRN if this record
 *      itself was kalloc'd because the number of complex arg descriptors
 *      exceeded the default value (RPC_DESC_COUNT). This is not a conflict
 *      since the first argument is always the destination port, never an array.
 */

#define COPY_TYPE_NO_COPY               0       /* nothing special */
#define COPY_TYPE_ON_KSTACK             1       /* array is on kernel stack */
#define COPY_TYPE_ON_SSTACK             2       /* array is on server stack */
#define COPY_TYPE_VIRTUAL_IN            3       /* vm_map_copyin part of cow */
#define COPY_TYPE_VIRTUAL_OUT_SVR       4       /* map cpyout svr part of cow */
#define COPY_TYPE_VIRTUAL_OUT_CLN       5       /* map cpyout cln part of cow */
#define COPY_TYPE_ALLOC_KRN             6       /* kernel kalloc'd for array */
#define COPY_TYPE_ALLOC_SVR             7       /* vm_alloc'd in server space */
#define COPY_TYPE_ALLOC_CLN             8       /* vm_alloc'd in client space */
#define COPY_TYPE_PORT                  9       /* plain port translated */
#define COPY_TYPE_PORT_ARRAY            10      /* port array translated */


/*
 * 	RPC types
 */

typedef int 			mach_rpc_id_t;
typedef int 			mach_rpc_return_t;
typedef unsigned int		mach_rpc_size_t;
typedef unsigned int		mach_rpc_offset_t;

struct rpc_copy_state {
        unsigned                copy_type;      /* what kind of copy */
        vm_offset_t             alloc_addr;     /* address to free */
};
typedef struct rpc_copy_state *rpc_copy_state_t;
typedef struct rpc_copy_state  rpc_copy_state_data_t;

typedef boolean_t (*copyfunc_t)(const char *, char *, vm_size_t);


/*
 *	RPC function declarations
 */

#ifdef	MACH_KERNEL

extern 
mach_rpc_return_t	mach_rpc_trap(
				mach_port_t		dest_port,
				mach_rpc_id_t		routine_num,
				mach_rpc_signature_t	signature_ptr,
				mach_rpc_size_t 	signature_size );
										
extern 
mach_rpc_return_t	mach_rpc_return_trap( void );

extern 
mach_rpc_return_t	mach_rpc_return_error( void );

void			mach_rpc_return_wrapper( void );

void            	rpc_upcall( 
				vm_offset_t		stack,
				vm_offset_t		new_stack, 
				vm_offset_t		server_func, 
				int 			return_code );

void            	rpc_error_severity( int severity );
void            	rpc_error_show_severity( int severity );
unsigned int    	name_rpc_to_ipc( unsigned int action );

void			clean_port_array(
				ipc_object_t *		array,
				unsigned		count,
				unsigned		cooked,
				unsigned		direct );

void            	unwind_rpc_state( 
				routine_descriptor_t	routine, 
				rpc_copy_state_t	state, 
				int * 			arg_buf );

kern_return_t		unwind_invoke_state( 
				thread_act_t		thr_act );

kern_return_t   	rpc_invke_args_in( 
				routine_descriptor_t	routine, 
				rpc_copy_state_t	state,
				int *			arg_buf,
				copyfunc_t		infunc );

kern_return_t   	rpc_invke_args_out( 
				routine_descriptor_t	routine, 
				rpc_copy_state_t	state,
				int *			arg_buf, 
				int ** 			new_sp,
				copyfunc_t		outfunc );

kern_return_t   	rpc_reply_args_in( 
				routine_descriptor_t	routine, 
				rpc_copy_state_t	state,
				int *			svr_buf,
				copyfunc_t		infunc );

kern_return_t   	rpc_reply_args_out( 
				routine_descriptor_t	routine, 
				rpc_copy_state_t	state,
				int *			svr_buf, 
				int * 			cln_buf,
				copyfunc_t		outfunc );

#endif	/* MACH_KERNEL */

#ifndef __alpha
/*
 * This is machine dependent and doesn't belong here.
 * jfraser
 */

/*
 * Glue function extern declarations
 */
extern kern_return_t	machine_rpc_simple(
				int, 
				int,
				void *);

extern kern_return_t	klthread_depress_abort(
				mach_port_t);

#endif /* __alpha */
/*
 * An rpc_glue_vector_t defined either by the kernel or by crt0
 */
extern rpc_glue_vector_t _rpc_glue_vector;

/*
 * libmach helper functions:
 */
extern rpc_subsystem_t	mach_subsystem_join(
				rpc_subsystem_t,
				rpc_subsystem_t,
				unsigned int *,
				void *(* )(int));

#endif	/* _MACH_RPC_H_ */


