/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: system.h,v $
 * Revision 1.1.2.1  1997/02/27  11:10:09  bruel
 * 	First revision
 * 	[1997/02/27  11:00:50  bruel]
 *
 * $EndLog$
 */


#ifndef _ASM_OSFMACH3_MACHINE_SYSTEM_H
#define _ASM_OSFMACH3_MACHINE_SYSTEM_H


#include <osfmach3/system.h>

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
#define tas(ptr) (xchg((ptr),1))

extern inline unsigned long xchg_u32(volatile unsigned long *m, unsigned long val)
{
	unsigned long retval;

	retval = *m;
	*m = val;
	return retval;
}

/*
 * This function doesn't exist, so you'll get a linker error
 * if something tries to do an invalid xchg().
 */
extern void __xchg_called_with_bad_pointer(void);

static inline unsigned long __xchg(unsigned long x, void * ptr, int size)
{
	switch (size) {
	case 4:
		return xchg_u32(ptr, x);
	};
	__xchg_called_with_bad_pointer();
	return x;
}

#endif	/* _ASM_OSFMACH3_MACHINE_SYSTEM_H */
