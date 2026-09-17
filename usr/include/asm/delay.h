/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: delay.h,v $
 * Revision 1.1.2.1  1997/02/27  11:10:43  bruel
 * 	First revision
 * 	[1997/02/27  11:01:10  bruel]
 *
 * $EndLog$
 */


#ifndef _ASM_OSFMACH3_MACHINE_DELAY_H
#define _ASM_OSFMACH3_MACHINE_DELAY_H

extern __inline__ void __delay(unsigned long );

#define  udelay(u) panic("udelay")
#define __udelay(u) panic("__udelay")

#endif	/* _ASM_OSFMACH3_MACHINE_DELAY_H */



