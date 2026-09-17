/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


#ifndef _ASM_OSFMACH3_MACHINE_DELAY_H
#define _ASM_OSFMACH3_MACHINE_DELAY_H

extern __inline__ void __delay(unsigned long );

#define  udelay(u) panic("udelay")
#define __udelay(u) panic("__udelay")

#endif	/* _ASM_OSFMACH3_MACHINE_DELAY_H */



