/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: segment.h,v $
 * Revision 1.1.2.1  1996/09/09  16:58:31  barbou
 * 	Added prototype for copystr_fromfs().
 * 	[96/08/22            barbou]
 *
 * 	Added prototypes for copyin() and copyout().
 * 	[96/08/21            barbou]
 *
 * 	Created.
 * 	[1996/08/21  15:06:16  barbou]
 *
 * $EndLog$
 */

#ifndef _OSFMACH3_SEGMENT_H
#define _OSFMACH3_SEGMENT_H

#include <mach/mach_types.h>

#define KERNEL_DS	0x100
#define USER_DS		0x101

extern void memcpy_tofs(void * to, const void * from, unsigned long n);
extern void memcpy_fromfs(void * to, const void * from, unsigned long n);
extern int strlen_fromfs(const char *addr);
extern int copystr_fromfs(char *to, const char *from, int n);
extern void copyout(char *from, vm_address_t to, unsigned long count);
extern void copyin(vm_address_t from, char *to, unsigned long count);

extern unsigned long get_fs(void);
extern void set_fs(unsigned long val);
static __inline__ unsigned long
get_ds(void)
{
	return KERNEL_DS;
}

#endif /* _OSFMACH3_SEGMENT_H */
