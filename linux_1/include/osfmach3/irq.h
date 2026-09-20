/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#ifndef _OSFMACH3_IRQ_H
#define _OSFMACH3_IRQ_H

#include <linux/linkage.h>
#include <asm/segment.h>

#define NR_IRQS 0

extern void disable_irq(unsigned int);
extern void enable_irq(unsigned int);

#endif	/* _OSFMACH3_IRQ_H */
