/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: irq.h,v $
 * Revision 1.1.2.1  1996/09/09  16:57:39  barbou
 * 	Added definition for NR_IRQS.
 * 	[96/08/21            barbou]
 *
 * 	Created.
 * 	[1996/08/21  16:22:02  barbou]
 *
 * $EndLog$
 */

#ifndef _OSFMACH3_IRQ_H
#define _OSFMACH3_IRQ_H

#include <linux/linkage.h>
#include <asm/segment.h>

#define NR_IRQS 0

extern void disable_irq(unsigned int);
extern void enable_irq(unsigned int);

#endif	/* _OSFMACH3_IRQ_H */
