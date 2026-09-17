/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#ifndef _ASM_OSFMACH3_PPC_CHECKSUM_H
#define _ASM_OSFMACH3_PPC_CHECKSUM_H

#include <asm-ppc/checksum.h>

extern unsigned short csum_fold(unsigned int sum);

#undef	csum_partial_copy_fromuser
extern unsigned int csum_partial_copy_fromuser(const char *src,
					       char *dst,
					       int len,
					       int sum);

#endif	/* _ASM_OSFMACH3_PPC_CHECKSUM_H */
