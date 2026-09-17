/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: osfmach3_setjmp.h,v $
 * Revision 1.1.2.1  1997/02/27  11:10:47  bruel
 * 	First revision
 * 	[1997/02/27  11:01:12  bruel]
 *
 * $EndLog$
 */

#ifndef	_ASM_OSFMACH3_MACHINE_OSFMACH3_SETJMP_H_
#define _ASM_OSFMACH3_MACHINE_OSFMACH3_SETJMP_H_

#include <mach_setjmp.h>

#define osfmach3_setjmp  mach_setjmp
#define osfmach3_longjmp  mach_longjmp

typedef int osfmach3_jmp_buf[_JBLEN];

#endif	/* _ASM_OSFMACH3_MACHINE_OSFMACH3_SETJMP_H_ */

