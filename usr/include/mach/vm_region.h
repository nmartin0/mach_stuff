/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: vm_region.h,v $
 * Revision 1.1.5.1  1995/01/16  17:22:27  bolinger
 * 	Import files unchanged from osc1.3b11 into cnmk_shared.
 * 	[1995/01/16  17:20:37  bolinger]
 *
 * Revision 1.1.3.2  1993/10/05  22:23:22  watkins
 * 	Merge forward.
 * 	[1993/10/05  22:05:05  watkins]
 * 
 * Revision 1.1.1.2  1993/09/28  19:42:50  watkins
 * 	Created to comply with spec.
 * 
 * $EndLog$
 */
/*
 *	File:	mach/vm_region.h
 *
 *	Define the attributes of a task's memory region
 *
 */

#ifndef	_MACH_VM_REGION_H_
#define _MACH_VM_REGION_H_

#include <mach/boolean.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <mach/vm_behavior.h>

/*
 *	Types defined:
 *
 *	vm_region_info_t	memory region attributes
 */

#define VM_REGION_INFO_MAX      (1024)
typedef int	*vm_region_info_t;
typedef int	 vm_region_flavor_t;
typedef int	 vm_region_info_data_t[VM_REGION_INFO_MAX];

#define VM_REGION_BASIC_INFO	10

struct vm_region_basic_info {
	vm_prot_t		protection;
	vm_prot_t		max_protection;
	vm_inherit_t		inheritance;
	boolean_t		shared;
	boolean_t		reserved;
	vm_offset_t		offset;
	vm_behavior_t		behavior;
	unsigned short		user_wired_count;
};

typedef struct vm_region_basic_info		*vm_region_basic_info_t;
typedef struct vm_region_basic_info		 vm_region_basic_info_data_t;

#define VM_REGION_BASIC_INFO_COUNT		\
	(sizeof(vm_region_basic_info_data_t)/sizeof(int))

#endif	/*_MACH_VM_REGION_H_*/
