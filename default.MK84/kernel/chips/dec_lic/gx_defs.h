/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	gx_defs.h,v $
 * Revision 2.2  92/05/22  15:47:21  jfriedl
 * 	Added a bunch of fields and defs for 3d board.
 * 	Fixed text foreground color to use colormap entry
 * 	no. 1, so that we can coexist with X11 running.
 * 	[92/05/20  23:01:02  af]
 * 
 * 	Created, with some Ultrix code.
 * 	[92/05/09            af]
 * 
 */
/*
 *	File: gx_defs.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	Definitions for the 2D/3D graphic boards.
 *
 */

/************************************************************************
 *									*
 *			Copyright (c) 1989 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/


#include <kern/eventcount.h>

/* Hardware state (to be held in the screen descriptor) */

typedef struct {

	char		*cursor_registers;	/* opaque, for sharing */
	unsigned short	cursor_state;		/* some regs are W-only */
	char		stamp_width;
	char		stamp_height;

	char		*cursor_reset;
	char		*board_base;		/* io slot, virt */

	int		*ring_buffer;
	unsigned	ring_buffer_offset;

	int		*ptpt;
	int		pt_sbrk;
	boolean_t	coproc_active;
	pmap_t		pmap;
	struct evc	event;

	boolean_t	mapped_to_user;	
	unsigned int	mapped_memory_size;
	unsigned int	mapped_io_size;

	int *		(*get_packet)();	/* to send to board */
	int		(*send_packet)();

	/* stats */

	unsigned int	dropped_packets;
	unsigned int	poll_timeouts;
	unsigned int	poll_count;

} gx_softc_t;

extern gx_softc_t	*gx_alloc();

/* Ultrix drivers compat */
#define	SILLY_OFFSET		(2*sizeof(int))

/* Parameters */
#define _TEXT_BG_	(0x00000000)
#define _TEXT_FG_	(0x00010101)
#define	_TEXT_LW_	((KfontHeight<<2)-1)
#define	_N_PLANES_	(8)

/*
 * Definitions for the 2D board
 */

#define	GA_MAPPED_MEM_SIZE	0x020000

#define	GA_RING_SIZE		0x18000
#define	GA_CONSOLE_PACKET_SIZE	0x2000

typedef struct {
    unsigned long minval;
    unsigned long maxval;
} gaStampClipRect;

typedef struct {
        long		numClipRects;
        long		refCount;
        gaStampClipRect	clipRects[8];
} gaClipList;

typedef struct _ga_Packet {
     union {
	struct {
	    long opcode;
	} un;
	struct {
	    long opcode;
	    long word_count;
	    long cliplist_sync;
	    long data[1];
	} PassPacket;
	struct {
	    long opcode;
	    long word_count;
	    long sram_phys_addr;
	    long r3000_virt_addr;
	} ReadSram;
	struct {
	    long opcode;
	    long word_count;
	    long sram_phys_addr;
	    long r3000_virt_addr;
	} WriteSram;
	struct {
	    long opcode;
	    long sram_phys_addr;
	    long data;
	} PutData;
	struct {
	    long opcode;
	    long cliplist_number;
	    long cliprect_count;
	    gaStampClipRect rect[1];
	} LoadClipList;
     } un;
} ga_Packet, *ga_PacketPtr;

#define GA_QUEUE_PACKETS	16

typedef struct {
    long pad1[3];			/* get to aligned boundary */
    long NoOp[10];			/* No-op packet area */
    long Stic_NoOp[10];			/* Store Stic NoOp */
    long Video_NoOp[4];			/* Store Video NoOp */
    volatile long intr_status;		/* interrupt status */
    volatile long lastRead;
    volatile long lastWritten;
    volatile long qDepth;
    gaStampClipRect *pCliprect;
    long numCliprect;
    gaStampClipRect *fixCliprect;
    volatile long *srv_qpoll[GA_QUEUE_PACKETS];
    volatile long *intr_qpoll[GA_QUEUE_PACKETS];
    volatile long *save_region[GA_QUEUE_PACKETS];
} Com2d, *Com2dPtr;

#define	N_PASSPACKET		5
#define	N_MAX_CLIPLISTS		16
#define	NUM_REQUIRED_CONTEXT	4

#define	GA_CMDBUF0_SIZE		0x1000
#define	GA_CMDBUF1_SIZE		0x1000
#define	GA_IMAGE_BUFFER_SIZE	0x3000-12
#define	GA_COMAREA_SRVCOM_SIZE	0x0800
#define	GA_2DCOM_SIZE		0x0800
#define	GA_CLIPLIST_SIZE	0x2000

typedef struct ga_ComArea {
    /*
     * Request Buffer 0
     */
    long CmdBuf0[ GA_CMDBUF0_SIZE >> 2 ];
    /*
     * Request Buffer 1
     */
    long CmdBuf1[ GA_CMDBUF1_SIZE >> 2 ];
    /*
     * Image Buffer (pad0 to quad align; leave word at end for count value)
     */
    char pad0[12];
    long image_buf[ (GA_IMAGE_BUFFER_SIZE+4) >> 2 ];
    /*
     * X Server Common Area (take away word to make up for count value)
     */
    long SRVCom[ (GA_COMAREA_SRVCOM_SIZE-4) >> 2 ];
    /*
     * 2d Server Common Area
     */
    Com2d SRV2DCom;
    char pad1[GA_2DCOM_SIZE-sizeof(Com2d)];
    /*
     * Cliplist
     */
    gaClipList ClipL[ N_MAX_CLIPLISTS ];
    char pad2[GA_CLIPLIST_SIZE-sizeof(gaClipList)*N_MAX_CLIPLISTS];
    /*
     * Interrupt-driven request buffers
     */
    long IntrBuf[GA_QUEUE_PACKETS][GA_CMDBUF0_SIZE >> 2];
} ga_ComArea, *ga_ComAreaPtr;

/* 
 * Given the address of a PixelStamp packet, return the 
 * index of the first word of the 2-word cliprect field
 */
#define DECODE_CLIP_INDEX(p)  ( NUM_REQUIRED_CONTEXT + \
	((((*p) & (1<<8))) >> 5) + ((((*p) & (1<<10))) >> 10) )

/*
 * Interrupt status bits: these bits are set in the gx_info 'intr_status'
 * field.
 *
 * INTR_ACTIVE is set by the server when it submits a packet to the STIC.
 * As long as it is set it means that there is at least one packet which
 * has not yet completed (and thus that the interrupt service routine can
 * be expected to be entered at some point in the future.  This bit
 * is cleared by the isr when it becomes blocked or idle.
 *
 * INTR_BLOCKED is set by the isr when it encounters a microcode packet
 * which it cannot deal with (e.g. one which must be emulated by the
 * server).  It is cleared by the server immediately prior to queueing
 * a packet which the isr can deal with.
 * 
 * INTR_CLIP is set by the isr when it is sending off a packet which
 * has specified a cliplist.
 *
 * INTR_ERR is set by the isr when it detects an error.
 *
 *   Value of intr_status	Meaning
 *   --------------------	-------
 *	       0                Idle.  No packet completions pending.
 *             1                A packet will complete in the future.
 *             2                Idle because isr can't emulate ucode pkt.
 *             3                ILLEGAL.
 */
#define GA_INTR_ACTIVE	(1<<0)
#define GA_INTR_BLOCKED (1<<1)
#define GA_INTR_CLIP    (1<<2)
#define GA_INTR_ERR     (1<<3)
#define GA_INTR_NEEDSIG (1<<4)

/*
 * How to get lost in the wires....
 */
/*
 * 2DA STIC polling address:
 *
 *   For 2DA, <15:17> aren't connected at all.  Ditto for <24:26>.
 *   Input <2:14+15:20> become the STIC's <2:14+18:23> and likewise
 *	for <21:22> to STIC <27:28>.
 *
 *   This means you can specify (input) 32K of physically contiguous
 *	memory to the STIC before bumping against some tie-lines, but...
 *
 *   The STIC can only "see" 23 bits of address (2da), and only if you form
 *	the polling address such that some bits get shift up into the
 *	STIC's <18:23> & <27:28>, since they're tied to <15:20> & <21:22>
 *	on the bus.  This means the packet buffers must be in the 1st
 *	8MB of physical memory.
 */


#define _0to14  (0x00007fff)	/* bits 00-14 set */
#define _0to23	(0x00ffffff)	/* bits 00-23 set */
#define _2to21  (0x003ffffc)	/* bits 02-21 set */
#define _6to8	(0x000001c0)	/* bits 00-08 set */
#define _11to28 (0x1ffff800)	/* bits 11-28 set */
#define _15to17	(0x00038000)	/* bits 15-17 set */
#define _15to20 (0x001f8000)	/* bits 15-20 set */
#define _18to23 (0x00fc0000)	/* bits 18-23 set */
#define _18to28 (0x1ffc0000)	/* bits 18-28 set */
#define _21to22 (0x00600000)	/* bits 21-22 set */
#define _24to28 (0x1f000000)	/* bits 24-28 set */
#define _27to28	(0x18000000)	/* bits 27-28 set */

/* convert a system physical address to a STIC space physical address */
#define GX_SYS2STIC(A) ((((A)&_21to22)<<6)|(((A)&_15to20)<<3)|((A)&_0to14))
#define GX_SYS_TO_STIC(A) GX_SYS2STIC(((vm_offset_t)(A)))

/* convert a STIC space physical address to a system physical address */
#define GX_STIC2SYS(A) ((((A)&_27to28)>>6)|(((A)&_18to23)>>3)|((A)&_0to14))
#define GX_STIC_TO_SYS(A) GX_STIC2SYS(((vm_offset_t)(A)))

/* convert a system physical address to STIC DMA encoding - architecture spec */
#define GX_PHYS_TO_DMA(A) ((((vm_offset_t)(A)) & _11to28) >> 9)

/* convert a system physical address to a DMA encoding - implementation spec */
#define GX_SYS2DMA(A)	( (((A)&~_0to14)<<3) | ((A)&_0to14) )
#define GX_SYS_TO_DMA(A)  GX_PHYS_TO_DMA(GX_SYS2DMA(((vm_offset_t)(A))))

/*
 * Ok, I took pity of the poor maintainer and here is an explanation
 * of the above nonsense.
 *
 * To tell the STIC there is something to do you touch (read or write)
 * a certain address in the board's space. The offset of this address
 * is mapped to a physical address by the board, in main memory. The
 * board then goes to look in that page, to see what we want it to do.
 * The above is trying to explain this in reverse, e.g. how to go
 * from a given physical address in main memory to the offset inside
 * the board that must be polled.
 *
 * Graphically, such a mapping process might look like
 *
 *  mem:170000 -->	0000 0000 0001 0111 0000 0000 0000 00xx
 *			now shift this as described above and..
 *  -->stic:b80000	   0 0??? 1011 10?? ?000 0000 0000 00xx
 *			now make this in pages
 *  -->polling:5c00	               00?? ?101 110? ??00 00xx
 */

/*
 * Definitions for the 3D board
 */

#define	GQ_MAPPED_MEM_SIZE	0xe000

#define	GQ_RING_SIZE		0x8000

/*
 * 128KB of SRAM on graphics accelerator option board.
 *
 * This stuff should EXACTLY correspond to the ucode layout, please...
 * We define the major organs here.  Internals are left with the ucode.
 * ANY changes here MUST be coordinated with the N10 ucode.
 */
#define GQ_RAM_SIZE		(0x20000)
#define	GQ_CONSOLE_PACKET_SIZE	(0x1000)


typedef int gqRAMReqBuf[ GQ_CONSOLE_PACKET_SIZE / sizeof(int) ];

typedef struct _gq_ram {
    /* graphics console packet area */
    gqRAMReqBuf reqbuf[2];
    /* read/write spans image buffer + trailing WIDTH for readspans */
    int pixbuf[ (2 * 1280 + 4 + 7) & ~7 ];
    /* begin 2 interrupt words quadword aligned */
    int intr_host;
    int intr_coproc;
    /* rest is a mystery */
    char memory[1];
} gqRAM;

/* Interrupt coproc --> host */
#define HST_INTR_MASK	0xfffff000
#define HST_INTR_SHFT	0x0000000c	/* 12. */

#define HST_INTR_WHAT	0x0000000f
#define HST_INTR_WSHF	0x00000000
#define HST_INTR_PGIN	0x00000001	/* pagein */
#define HST_INTR_XLAT	0x00000002	/* translate */
#define HST_INTR_VSYN	0x00000003	/* vblank sync */
#define HST_INTR_VRFY   0x00000004	/* verify valid map */
#define HST_INTR_PMSK	0x00000f00
#define HST_INTR_DRTY	0x00000800	/* pagein dirty page */
#define HST_INTR_PADD	0x00000700	/* add'l pageins */
#define HST_INTR_PSHF	0x00000010	/* 16. */


/* Interrupt host --> coproc */

#define GQ_INTR_TIMO	1500000		/* ~3 secs */
#define GQ_INTR_MASK	HST_INTR_MASK
#define GQ_INTR_SHFT	HST_INTR_SHFT
#define GQ_INTR_ACK	0x00000000
#define GQ_INTR_WHAT	0x000000f0
#define GQ_INTR_WSHF	0x00000008
#define GQ_INTR_INV1	0x00000010	/* invalidate 1 pte */
#define GQ_INTR_INVA	0x00000020	/* invalidate all ptes */
#define GQ_INTR_CEIL	GQ_INTR_INVA
#define GQ_INTR_PAUS	0x00000030	/* pause N10 */
#define GQ_INTR_FLSH	0x00000040	/* flush data cache */
#define GQ_INTR_BUF0	0x00000001
#define GQ_INTR_BUF1	0x00000002
#define GQ_INTR_HALT	0x000000f0	/* unused */

/*
 * hacking away..
 */

#define	GQ_PTPT_SIZE	0x1000
#define	GQ_PTTB_SIZE	0x10000
