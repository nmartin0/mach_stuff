/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * Copyright 1991-1998 by Apple Computer, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */

#ifndef _POWERMAC_PERFORMA_H_
#define _POWERMAC_PERFORMA_H_

#include "conf.h"
#include <mach/ppc/vm_types.h>
#include <ppc/spl.h>

#ifndef ASSEMBLER

/* Physical address and size of IO control registers region */

#define PERFORMA_IO_BASE_ADDR	0x50f00000 // - 0x00f00000 ??
#define PERFORMA_IO_SIZE		0x42000


/* Interrupt control register */
// #define PERFORMA_ICR			POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x2a000)

/* PMU registers */
#define PERFORMA_PMU_BASE_PHYS	(PERFORMA_IO_BASE_ADDR)

/* Interrupt controlling VIAs */

#define PERFORMA_VIA1_IFR		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x01a00))
#define PERFORMA_VIA1_IER		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x01c00))
#define PERFORMA_VIA1_PCR		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x01800))

#define PERFORMA_VIA1_PORTB_IFR		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x0))

#define	PERFORMA_VIA1_AUXCONTROL	(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x01600))
#define	PERFORMA_VIA1_T1COUNTERLOW	(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x00800))
#define	PERFORMA_VIA1_T1COUNTERHIGH	(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x00A00))
#define	PERFORMA_VIA1_T1LATCHLOW	(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x00C00))
#define	PERFORMA_VIA1_T1LATCHHIGH	(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x00E00))

#define PERFORMA_VIA2_IFR		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x3a00))
#define PERFORMA_VIA2_IER		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x3c00))

#define PERFORMA_VIA2_SLOT_IFR		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x3e00))

#define	PERFORMA_F108_IFR		(POWERMAC_IO(PERFORMA_IO_BASE_ADDR+0x1a101))
#define	PERFORMA_IDE0_BASE		(0x50F1A000)
#define	PERFORMA_IDE1_BASE		(0x50F1A080)
#define PERFORMA_ASC_BASE_PHYS		(0x50F10000)
// I don't think so....
// #define PERFORMA_SCC_BASE_PHYS		(0x50FC0000)

/* Fish 'n' Chips */

#define PERFORMA_CUDA_BASE_PHYS	(0x50f16000)
#define PERFORMA_CAPELLA_BASE_PHYS (0x53000000)
#define PERFORMA_CAPELLA_SIZE (0x01000000)
#define CAPELLA_INT_REG_OFFSET (0x18)
#define PERFORMA_SCC_BASE_PHYS	(0x50f0C000) // Really, it's 5000C000


/* Miscellaneous Prototypes */

void f108_register_int(int device, spl_t level,
		void (*handler)(int, void *));

void   performa_interrupt_initialize(void);

#endif /* ndef ASSEMBLER */
#endif /* _POWERMAC_PERFORMA_H_ */
