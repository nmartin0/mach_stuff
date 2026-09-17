/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: setjmp.h,v $
 * Revision 1.1.2.1  1996/09/09  16:58:44  barbou
 * 	Include asm/osfmach3_setjmp.h instead of asm/setjmp.h.
 * 	[96/08/21            barbou]
 *
 * 	Created.
 * 	[1996/08/21  15:55:44  barbou]
 *
 * $EndLog$
 */

#ifndef	_OSFMACH3_SETJMP_H_
#define _OSFMACH3_SETJMP_H_

#include <asm/osfmach3_setjmp.h>

extern int osfmach3_setjmp(osfmach3_jmp_buf *jmp_buf);
extern void osfmach3_longjmp(osfmach3_jmp_buf *jmp_buf, int val);

#endif	/* _OSFMACH3_SETJMP_H_ */
