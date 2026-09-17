/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: vm_types.h,v $
 * Revision 1.1.10.1  1995/03/15  17:47:48  bruel
 * 	hppa merge
 * 	[1995/03/15  09:45:45  bruel]
 *
 * Revision 1.1.3.1  1994/02/11  13:28:41  bruel
 * 	Created from Utah.
 * 	[93/11/19            bruel]
 * 
 * $EndLog$
 */

#ifndef	_MACH_HP700_VM_TYPES_H_
#define _MACH_HP700_VM_TYPES_H_

#ifndef	ASSEMBLER
typedef unsigned int	natural_t;
typedef int		integer_t;
typedef int		int32;
typedef unsigned int	uint32;
typedef	natural_t	vm_offset_t;
typedef	natural_t	vm_size_t;
typedef	unsigned int	prot_t;
typedef unsigned int    space_t;
#endif	/* ASSEMBLER */
#endif	/* _MACH_HP700_VM_TYPES_H_ */
