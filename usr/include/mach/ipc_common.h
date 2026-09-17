/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: ipc_common.h,v $
 * Revision 1.1.4.3  1995/02/28  02:03:54  dwm
 * 	mk6 CR1120 - Merge mk6pro_shared into cnmk_shared
 * 	* Rev 1.1.8.1  1995/02/22  18:09:16  bolinger
 * 	* Fix ri-osc CR 1095: remove members of struct {i,r}pc_common_data
 * 	* once but no longer needed for RPC to collocated tasks.
 * 	[1995/02/28  01:17:24  dwm]
 *
 * Revision 1.1.4.2  1995/01/06  19:50:51  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	change typedefs moved from ipc/ipc_object.h to natural_t
 * 	[1994/11/02  18:17:36  dwm]
 * 
 * Revision 1.1.4.1  1994/09/23  02:38:10  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:40:33  ezf]
 * 
 * Revision 1.1.2.1  1994/04/29  20:55:34  dwm
 * 	Common sub-structure at the start of each ipc_port, ipc_pset,
 * 	and rpc_port, which contains the ipc_object used to tag each.
 * 	[1994/04/29  20:53:52  dwm]
 * 
 * $EndLog$
 */

#ifndef	_MACH_IPC_COMMON_H_
#define _MACH_IPC_COMMON_H_

typedef vm_offset_t ipc_kobject_t;	/* for kern/ipc_kobject.h	*/

typedef natural_t ipc_object_refs_t;	/* for ipc/ipc_object.h		*/
typedef natural_t ipc_object_bits_t;
typedef natural_t ipc_object_type_t;

/*
 * There is no lock in the ipc_object; it is in the enclosing kernel
 * data structure (rpc_common_data) used by both ipc_port and ipc_pset.
 * The ipc_object is used to both tag and reference count these two data
 * structures, and (Noto Bene!) pointers to either of these or the
 * ipc_object at the head of these are freely cast back and forth; hence
 * the ipc_object MUST BE FIRST in the ipc_common_data.
 * 
 * If the RPC implementation enabled user-mode code to use kernel-level
 * data structures (as ours used to), this peculiar structuring would
 * avoid having anything in user code depend on the kernel configuration
 * (with which lock size varies).
 */
struct ipc_object {
	ipc_object_refs_t io_references;
	ipc_object_bits_t io_bits;
};

/*
 * Common sub-structure at the head of each
 * ipc_port and ipc_pset.  This sub-structure could
 * also safely be made common to user-mode RPC
 * code.
 */
typedef struct ipc_common_data {
	struct ipc_object	icd_object;
	ipc_kobject_t		icd_kobject;
	struct rpc_subsystem *	icd_subsystem;
	mach_port_t		icd_receiver_name;
} *ipc_common_t;

#endif	/* _MACH_IPC_COMMON_H_ */
