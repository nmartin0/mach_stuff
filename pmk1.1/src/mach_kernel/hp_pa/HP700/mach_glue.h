/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
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
 * pmk1.1
 */
/* 
 * Copyright (c) 1992, The University of Utah and
 * the Center for Software Science at the University of Utah (CSS).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the Center
 * for Software Science at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSS ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSS DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSS requests users of this software to return to css-dist@cs.utah.edu any
 * improvements that they make and grant CSS redistribution rights.
 *
 * 	Utah $Hdr: mach_glue.h 1.5 92/07/20$
 */

/*
 * Glue to make OSF-1 or BSD drivers work in Mach 3.0
 * with minimal damage to driver code.
 */

#include <vm/vm_kern.h>

#define ssmv(m)		ssm(m)

#define LOG_WARNING	1

#define ENODEV		D_NO_SUCH_DEVICE
#define EROFS		D_READ_ONLY
#define	EMFILE		D_NO_MEMORY
#define	EWOULDBLOCK	D_WOULD_BLOCK
#define	EBADF		D_READ_ONLY
#define	FREAD		D_READ
#define	FWRITE		D_WRITE
#define	FNDELAY		D_NODELAY

#define	MAX(a,b)	((a) > (b) ? (a) : (b))
#define	MIN(a,b)	((a) < (b) ? (a) : (b))
#define	DELAY(n)	delay(n)

#define CTRL(c)		('c'&037)

/* HP OSF stuff */
#define pmap_flush_range(p, v, s)	\
{ \
	space_t sid = (p)->space; \
	if ((((int)(v) & 31) + (s)) <= 32) \
		fcacheline(sid, (vm_offset_t)(v)); \
	else if (sid == HP700_SID_KERNEL) \
		fdcache(sid, v, s); \
	else \
		panic("pmap_flush_range: not kernel"); \
}
#define pmap_purge_range(p, v, s)	\
{ \
	space_t sid = (p)->space; \
	if ((((int)(v) & 31) + (s)) <= 32) \
		pcacheline(sid, (vm_offset_t)(v)); \
	else if (sid == HP700_SID_KERNEL) \
		pdcache(sid, v, s); \
	else \
		panic("pmap_purge_range: not kernel"); \
}

/*
 * Skeletal structs for attach routines
 */
struct hp_businfo {
	int	bustype;
};

#define	MAXCSRS	1
typedef struct hp_ctlrinfo {
	int		   ctlrnum;
	struct hp_ctlrinfo *parent_ctlrinfop;
	struct hp_businfo  *ctlr_businfop;
	int                irq;
	int		   numcsrs;
	vm_offset_t        csrs[MAXCSRS];
	vm_offset_t	   vcsrs[MAXCSRS];
} hp_ctlrinfo_t;

#define BUSTYPE_NOSUCHBUS	0
#define BUSTYPE_SYS		1
#define BUSTYPE_EISA		2
#define BUSTYPE_ZALON		3

/*
 * For EISA
 */
#define RESOLVER_ADD_HANDLER 1
#define RESOLVER_DEL_HANDLER 2

#define	IO_EISA_IACK_PHYS	0xfc01f000
#define	IO_EISA_IACK_LEN	0x1000

#define io_map(pa, sz, nm)	(pa)
#define io_unmap(va, nm)



