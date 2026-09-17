/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: memory_object.h,v $
 * Revision 1.3.12.6  1995/11/02  14:59:41  bruel
 * 	Added advisory_pageout and silent_overwrite fields
 * 	to memory_object_behave_info.
 * 	Renamed MEMORY_OBJECT_BEHAVIOR_INFO for backward compatibility
 * 	MEMORY_OBJECT_BEHAVIOR_INFO_OLD should go away shortly.
 * 	[95/07/03            bernadat]
 * 	[95/09/29            bruel]
 *
 * Revision 1.3.12.5  1995/05/25  20:50:49  mmp
 * 	Remove object_ready and write_completions from attributes.
 * 	[1995/05/25  19:32:58  mmp]
 * 
 * Revision 1.3.12.4  1995/04/07  19:05:40  barbou
 * 	Added "object_ready" to memory_object_attr_info. It should
 * 	be removed when we switch back to m_o_notify/establish.
 * 	[95/03/10            barbou]
 * 
 * 	VM Merge - Pager Clustering.
 * 	Upgraded the memory_object_attr_info structure to the new
 * 	specifications to allow memory managers to specify the cluster size.
 * 	[94/10/11            barbou]
 * 	[95/03/10            barbou]
 * 
 * Revision 1.3.12.3  1995/01/06  19:51:21  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	b26 removed copy_call; back since norma needs it; likewise
 * 	restore write_completions to struct memory_object_behave_info
 * 	[1994/11/02  18:17:47  dwm]
 * 
 * Revision 1.3.12.2  1994/09/23  02:39:57  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:41:35  ezf]
 * 
 * Revision 1.3.12.1  1994/06/13  20:49:53  dlb
 * 	Merge MK6 and NMK17
 * 	[1994/06/13  20:48:06  dlb]
 * 
 * Revision 1.3.10.1  1994/03/07  16:41:41  paire
 * 	Added memory_object attributes structure for NORMA interfaces.
 * 	[94/02/21            paire]
 * 
 * Revision 1.3.2.3  1993/10/05  22:23:16  watkins
 * 	Merge forward.
 * 	[1993/10/05  22:04:54  watkins]
 * 
 * Revision 1.3.4.2  1993/09/28  19:42:43  watkins
 * 	Memory object attribute interfaces comply with spec.
 * 
 * Revision 1.3.2.2  1993/06/09  02:42:06  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:17:21  jeffc]
 * 
 * Revision 1.3  1993/04/19  16:36:03  devrcs
 * 	ansi C conformance changes
 * 	[1993/02/02  18:53:12  david]
 * 
 * Revision 1.2  1992/12/07  21:29:26  robert
 * 	integrate any changes below for 14.0 (branch from 13.16 base)
 * 
 * 	Joseph Barrera (jsb) at Carnegie-Mellon University 05-Aug-92
 * 	Added MEMORY_OBJECT_COPY_SYMMETRIC, currently only for internal
 * 	norma kernel use. Added MEMORY_OBJECT_RETURN_ANYTHING, so that
 * 	a memory manager can ask for a page that is neither precious
 * 	nor dirty. Added MEMORY_OBJECT_COPY_INVALID to simplify code.
 * 	[1992/12/06  20:25:43  robert]
 * 
 * Revision 1.1  1992/09/30  02:31:24  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.6.2.1  92/02/21  11:24:08  jsb
 * 	Added MEMORY_OBJECT_COPY_TEMPORARY.
 * 	[92/02/11  07:56:35  jsb]
 * 
 * Revision 2.6  91/08/28  11:15:22  jsb
 * 	Add defs for memory_object_return_t.
 * 	[91/07/03  14:06:26  dlb]
 * 
 * Revision 2.5  91/05/18  14:35:05  rpd
 * 	Removed memory_manager_default.
 * 	[91/03/22            rpd]
 * 
 * Revision 2.4  91/05/14  16:55:55  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:34:01  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:19:11  mrt]
 * 
 * Revision 2.2  90/06/02  14:58:56  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:35:26  rpd]
 * 
 * Revision 2.1  89/08/03  16:02:52  rwd
 * Created.
 * 
 * Revision 2.5  89/02/25  18:38:23  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.4  89/02/07  00:54:07  mwyoung
 * Relocated from vm/memory_object.h
 * 
 * Revision 2.3  89/01/30  22:08:42  rpd
 * 	Updated includes to the new style.  Fixed log.
 * 	Made variable declarations use "extern".
 * 	[89/01/25  15:25:20  rpd]
 */
/* CMU_ENDHIST */
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
 *	File:	memory_object.h
 *	Author:	Michael Wayne Young
 *
 *	External memory management interface definition.
 */

#ifndef	_MACH_MEMORY_OBJECT_H_
#define _MACH_MEMORY_OBJECT_H_

/*
 *	User-visible types used in the external memory
 *	management interface:
 */

#include <mach/port.h>
#include <mach/machine/vm_types.h>

typedef	mach_port_t	memory_object_t;
					/* A memory object ... */
					/*  Used by the kernel to retrieve */
					/*  or store data */

typedef	mach_port_t	memory_object_control_t;
					/* Provided to a memory manager; ... */
					/*  used to control a memory object */

typedef	mach_port_t	memory_object_name_t;
					/* Used to describe the memory ... */
					/*  object in vm_regions() calls */

typedef mach_port_t     memory_object_rep_t;
					/* Per-client handle for mem object */
					/*  Used by user programs to specify */
					/*  the object to map */

typedef	int		memory_object_copy_strategy_t;
					/* How memory manager handles copy: */
#define		MEMORY_OBJECT_COPY_NONE		0
					/* ... No special support */
#define		MEMORY_OBJECT_COPY_CALL		1
					/* ... Make call on memory manager */
#define		MEMORY_OBJECT_COPY_DELAY 	2
					/* ... Memory manager doesn't
					 *     change data externally.
					 */
#define		MEMORY_OBJECT_COPY_TEMPORARY 	3
					/* ... Memory manager doesn't
					 *     change data externally, and
					 *     doesn't need to see changes.
					 */
#define		MEMORY_OBJECT_COPY_SYMMETRIC 	4
					/* ... Memory manager doesn't
					 *     change data externally,
					 *     doesn't need to see changes,
					 *     and object will not be
					 *     multiply mapped.
					 *
					 *     XXX
					 *     Not yet safe for non-kernel use.
					 */

#define		MEMORY_OBJECT_COPY_INVALID	5
					/* ...	An invalid copy strategy,
					 *	for external objects which
					 *	have not been initialized.
					 *	Allows copy_strategy to be
					 *	examined without also
					 *	examining pager_ready and
					 *	internal.
					 */

typedef	int		memory_object_return_t;
					/* Which pages to return to manager
					   this time (lock_request) */
#define		MEMORY_OBJECT_RETURN_NONE	0
					/* ... don't return any. */
#define		MEMORY_OBJECT_RETURN_DIRTY	1
					/* ... only dirty pages. */
#define		MEMORY_OBJECT_RETURN_ALL	2
					/* ... dirty and precious pages. */
#define		MEMORY_OBJECT_RETURN_ANYTHING	3
					/* ... any resident page. */

#define		MEMORY_OBJECT_NULL	MACH_PORT_NULL


/*
 *	Types for the memory object flavor interfaces
 */

#define MEMORY_OBJECT_INFO_MAX      (1024) 
typedef int     *memory_object_info_t;      
typedef int	 memory_object_flavor_t;
typedef int      memory_object_info_data_t[MEMORY_OBJECT_INFO_MAX];


#define OLD_MEMORY_OBJECT_BEHAVIOR_INFO 	10	
#define MEMORY_OBJECT_PERFORMANCE_INFO	11
#define OLD_MEMORY_OBJECT_ATTRIBUTE_INFO	12
#define MEMORY_OBJECT_NORMA_INFO	13
#define MEMORY_OBJECT_ATTRIBUTE_INFO	14
#define MEMORY_OBJECT_BEHAVIOR_INFO 	15	


struct old_memory_object_behave_info {
	memory_object_copy_strategy_t	copy_strategy;	
	boolean_t			temporary;
	boolean_t			invalidate;
};

struct memory_object_perf_info {
	vm_size_t			cluster_size;
	boolean_t			may_cache;
};

struct old_memory_object_attr_info {			/* old attr list */
        boolean_t       		object_ready;
        boolean_t       		may_cache;
        memory_object_copy_strategy_t 	copy_strategy;
};

/* NOTE: this is now only by the ad1 server. */
struct memory_object_norma_info {			/* old norma list */
        boolean_t       		object_ready;
        boolean_t       		may_cache;
	boolean_t			write_completions;
        memory_object_copy_strategy_t 	copy_strategy;
	vm_size_t			cluster_size;
};

struct memory_object_attr_info {
	memory_object_copy_strategy_t	copy_strategy;
	vm_offset_t			cluster_size;
	boolean_t			may_cache_object;
	boolean_t			temporary;
};

struct memory_object_behave_info {
	memory_object_copy_strategy_t	copy_strategy;	
	boolean_t			temporary;
	boolean_t			invalidate;
	boolean_t			silent_overwrite;
	boolean_t			advisory_pageout;
};

typedef struct old_memory_object_behave_info *old_memory_object_behave_info_t;
typedef struct old_memory_object_behave_info old_memory_object_behave_info_data_t;

typedef struct memory_object_behave_info *memory_object_behave_info_t;
typedef struct memory_object_behave_info memory_object_behave_info_data_t;

typedef struct memory_object_perf_info 	*memory_object_perf_info_t;
typedef struct memory_object_perf_info	memory_object_perf_info_data_t;

typedef struct old_memory_object_attr_info *old_memory_object_attr_info_t;
typedef struct old_memory_object_attr_info old_memory_object_attr_info_data_t;

typedef struct memory_object_attr_info	*memory_object_attr_info_t;
typedef struct memory_object_attr_info	memory_object_attr_info_data_t;

typedef struct memory_object_norma_info	*memory_object_norma_info_t;
typedef struct memory_object_norma_info	memory_object_norma_info_data_t;


#define OLD_MEMORY_OBJECT_BEHAVE_INFO_COUNT   	\
                (sizeof(old_memory_object_behave_info_data_t)/sizeof(int))
#define MEMORY_OBJECT_BEHAVE_INFO_COUNT   	\
                (sizeof(memory_object_behave_info_data_t)/sizeof(int))
#define MEMORY_OBJECT_PERF_INFO_COUNT		\
		(sizeof(memory_object_perf_info_data_t)/sizeof(int))
#define OLD_MEMORY_OBJECT_ATTR_INFO_COUNT		\
		(sizeof(old_memory_object_attr_info_data_t)/sizeof(int))
#define MEMORY_OBJECT_ATTR_INFO_COUNT		\
		(sizeof(memory_object_attr_info_data_t)/sizeof(int))
#define MEMORY_OBJECT_NORMA_INFO_COUNT		\
		(sizeof(memory_object_norma_info_data_t)/sizeof(int))

#define invalid_memory_object_flavor(f)					\
	(f != MEMORY_OBJECT_NORMA_INFO && 				\
	 f != MEMORY_OBJECT_ATTRIBUTE_INFO && 				\
	 f != MEMORY_OBJECT_PERFORMANCE_INFO && 			\
	 f != OLD_MEMORY_OBJECT_BEHAVIOR_INFO &&				\
	 f != MEMORY_OBJECT_BEHAVIOR_INFO &&				\
	 f != OLD_MEMORY_OBJECT_ATTRIBUTE_INFO)


#endif	/* _MACH_MEMORY_OBJECT_H_ */
