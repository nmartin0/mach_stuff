/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#ifndef	_OSFMACH3_SETJMP_H_
#define _OSFMACH3_SETJMP_H_

#include <asm/osfmach3_setjmp.h>

extern int osfmach3_setjmp(osfmach3_jmp_buf *jmp_buf);
extern void osfmach3_longjmp(osfmach3_jmp_buf *jmp_buf, int val);

#endif	/* _OSFMACH3_SETJMP_H_ */
