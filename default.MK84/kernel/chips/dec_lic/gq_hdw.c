/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	gq_hdw.c,v $
 * Revision 2.6  93/05/17  17:09:45  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 	[93/05/17            rvb]
 * 
 * Revision 2.5  93/05/15  19:35:44  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.4  93/02/05  08:06:31  danner
 * 	Mods to *compile* on Flamingo.
 * 	[93/02/04  01:50:39  af]
 * 
 * Revision 2.3  93/01/14  17:15:33  danner
 * 	static/extern fixup.
 * 	[93/01/14            danner]
 * 
 * 	Fixed GCC complaint in an #if 0 branch.
 * 	[92/12/16            pds]
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 	Commented out random text on line 364 which contained a single quote.
 * 	[92/11/06            cmaeda]
 * 
 * Revision 2.2  92/05/22  15:47:07  jfriedl
 * 	Made it work for real, added some Ultrix code.
 * 	Moved in copyright-infected directory.
 * 	[92/05/16            af]
 * 
 * Revision 2.3  90/12/05  23:31:10  af
 * 	Created (almost) empty.
 * 	[90/12/03  23:17:49  af]
 */
/*
 *	File: gq_hdw.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Hardware-level routines for the 3max 3D display.
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



#include <bm.h>
#include <gx.h>
#if	(NGX > 0)

#include <sys/types.h>
#include <mach/std_types.h>
#include <sys/types.h>
#include <chips/busses.h>
#include <machine/machspl.h>
#include <chips/screen_defs.h>

#include <chips/dec_lic/gx_defs.h>
#include <chips/dec_lic/stamp.h>

#include <platforms.h>

#ifdef	DECSTATION
#include <mips/mips_cpu.h>
#include <mips/PMAX/tc.h>
#include <mips/PMAX/pmag_da.h>
#define	pmag_da_stic(x)		((sticRegs*)(((vm_offset_t)x)+GQ_OFFSET_STIC))
#define	pmag_da_stamp(x)	(char*)(((vm_offset_t)x) + GQ_OFFSET_STAMP)
#define	pmag_da_poll(x)		(char*)(((vm_offset_t)x) + GQ_OFFSET_POLL)
#define	pmag_da_sram(x)		((gqRAM*)(((vm_offset_t)x) + GQ_OFFSET_SRAM))
#define	pmag_da_inth(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_INT_TO_HOST))
#define	pmag_da_intc(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_INT_TO_N10))
#define	pmag_da_rstc(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_RESET_N10))
#endif

#ifdef	FLAMINGO
#include <alpha/DEC/tc.h>
#include <mips/PMAX/pmag_da.h>	/* XXX */
#define	pmag_da_stic(x)		((sticRegs*)(((vm_offset_t)x)+GQ_OFFSET_STIC))
#define	pmag_da_stamp(x)	(char*)(((vm_offset_t)x) + GQ_OFFSET_STAMP)
#define	pmag_da_poll(x)		(char*)(((vm_offset_t)x) + GQ_OFFSET_POLL)
#define	pmag_da_sram(x)		((gqRAM*)(((vm_offset_t)x) + GQ_OFFSET_SRAM))
#define	pmag_da_inth(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_INT_TO_HOST))
#define	pmag_da_intc(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_INT_TO_N10))
#define	pmag_da_rstc(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_RESET_N10))
#endif

/*
 * Definition of the driver for the auto-configuration program.
 */

int	gq_probe(), gq_intr();
void	gq_attach();

vm_offset_t	gq_std[NGX] = { 0 };
struct	bus_device *gq_info[NGX];
struct	bus_driver gq_driver = 
        { gq_probe, 0, gq_attach, 0, gq_std, "gq", gq_info,};

int gq_dont = 0;

/* forward declarations */
static gq_intr_pagein(gx_softc_t *,unsigned int,screen_softc_t);
static gq_request_pagein();
/*
 * Probe/Attach functions
 */

gq_probe( /* reg, ui */)
{
	static probed_once = 0;

	/*
	 * Probing was really done sweeping the TC long ago
	 */
	if (tc_probe("gq") == 0)
		return 0;
	if (probed_once++ > 1)
		printf("[mappable] ");
	return 1;
}

void
gq_attach(ui)
	struct bus_device *ui;
{
	printf(": 3D color display");
}


/*
 * Interrupt routine, and associated actions
 */
gq_intr(unit,spllevel)
	spl_t	spllevel;
{
	static int	gq_intr_noop(), gq_intr_pagein(),
			gq_intr_xlate(), gq_intr_vblank(),
			gq_intr_vrfy();
	static int      (*gq_intr_vec[]) () = {
		                gq_intr_noop,	/* 0 */
		                gq_intr_pagein,	/* 1 */
		                gq_intr_xlate,	/* 2 */
		                gq_intr_vblank,	/* 3 */
		                gq_intr_vrfy,	/* 4 */
		                gq_intr_noop,	/* 5 */
		                gq_intr_noop,	/* 6 */
		                gq_intr_noop,	/* 7 */
		                gq_intr_noop,	/* 8 */
		                gq_intr_noop,	/* 9 */
		                gq_intr_noop,	/* 10 */
		                gq_intr_noop,	/* 11 */
		                gq_intr_noop,	/* 12 */
		                gq_intr_noop,	/* 13 */
		                gq_intr_noop,	/* 14 */
		                gq_intr_noop,	/* 15 */
	};
	gx_softc_t	*gq;
	screen_softc_t	sc;

#ifdef	mips
	splx(spllevel);
#endif

	sc = screen(unit);
	gq = (gx_softc_t *) sc->hw_state;
	if (gq->mapped_to_user) {
		unsigned int	cause;
		gqRAM		*ram;

		ram = pmag_da_sram(gq->board_base);
		cause = ram->intr_host;

		(*gq_intr_vec[ cause & HST_INTR_WHAT ]) (gq, cause, sc);
	} else
		gq_intr_noop(gq, 0);
}

/*
 * This one is a bit dubious (af)
 */
static
gq_intr_stic(gq, cause)
	gx_softc_t	*gq;
	unsigned int	cause;
{
	sticRegs       *stic = pmag_da_stic(gq->board_base);

	if (stic->ipdvint & STIC_INT_E) {
		unsigned int	ipdvint = stic->ipdvint;
		stic->ipdvint = STIC_INT_CLR;
		printf("gq_intr_stic: x%x x%x x%x x%x x%x\n",
		      ipdvint, stic->sticsr, stic->buscsr,
		      stic->busadr, stic->busdat);
	}
}

/*
 * Catchall
 */
static
gq_intr_noop(gq, cause, sc)
	gx_softc_t	*gq;
	unsigned int	cause;
	screen_softc_t	sc;
{
	* (pmag_da_inth(gq->board_base)) = GQ_INTR_ACK;
	wbflush();

	gq_intr_stic(gq, cause);
}

/*
 * For the HE3D 3MAX board, N10 receives VINT.  VINT is normally off.
 * When an N10 packet with the sync bit arrives at the head of the
 * packet queue, N10 enables VINT and interrupts R3 when STIC interrupts
 * N10 for VINT.  We load the colormap during this time.  Unused.
 */
static
gq_intr_vblank(gq, cause, sc)
	gx_softc_t	*gq;
	unsigned int	cause;
	screen_softc_t	sc;
{
	gq_intr_noop(gq, cause, sc);

	/* gx_load_colormap(); */
}

/*
 * Coprocessor page tables
 */

#define	coproc_vtovsn(x)	((((vm_offset_t)x) >> 22) & 0x1ff)
#define coproc_vtopn(x)		((((vm_offset_t)x) >> 12) & 0x3ff)
#define coproc_pdetophys(p)	((p) & 0xfffff000)
#define coproc_phystopte(h)	((h) >> 8)

/*
 * Read/write miss on coproc pagetables
 */
static
gq_tlbmiss(gq, cause, write_miss, sc)
	gx_softc_t	*gq;
	unsigned int	cause;
	boolean_t	write_miss;
	screen_softc_t	sc;
{
	register vm_offset_t virtual_address, vsn, phys_address;
	vm_offset_t     page_table;
	register unsigned int *pte, *pde;

	virtual_address = (cause & HST_INTR_MASK);
	vsn = coproc_vtovsn(virtual_address);	/* virt seg num */

	/*
	 * Which page is it 
	 */
	phys_address = pmap_extract(gq->pmap, virtual_address);
	if (phys_address == 0) {
		return gq_request_pagein(gq, cause, sc);
	}

	/*
	 * Fill pde, if necessary
	 */
	pde = (unsigned int *) (gq->ptpt + vsn);
	if (*pde) {
		page_table = (vm_offset_t)
			(PHYS_TO_K1SEG(coproc_pdetophys(*pde)));
	} else {
		page_table = ((vm_offset_t) gq->ptpt) + GQ_PTPT_SIZE + gq->pt_sbrk;
		gq->pt_sbrk += 0x1000;
		if (gq->pt_sbrk > GQ_PTTB_SIZE) panic("GQ_PTTB_SIZE");
		*pde = kvtophys(page_table) | vsn;
	}

	/*
	 * Fill pte 
	 */
	pte = (unsigned int *) (page_table + coproc_vtopn(virtual_address) * 4);
	*pte = coproc_phystopte(phys_address) | 2; /* make valid */

	/*
	 * Make writeable if necessary, then tell coproc
	 */
	if (write_miss) {
		*pte |= 4;
		pmap_set_modify(phys_address);
		*(pmag_da_inth(gq->board_base)) = 0;
	} else {
		*(pmag_da_inth(gq->board_base)) = phys_address;
	}
#if debug
	if (gq_dont)
		printf("miss(%d) x%x -> x%x\n",
			write_miss, virtual_address, phys_address);
#endif
}

/*
 * Coproc looked at a paged out page ?
 */
static
gq_intr_pagein(
	gx_softc_t	*gq,
	unsigned int	cause,
	screen_softc_t	sc)
{
	return gq_tlbmiss(gq, cause, TRUE, sc);
}


/*
 * Coproc took a translation fault (pde invalid)
 */
static
gq_intr_xlate(gq, cause, sc)
	gx_softc_t	*gq;
	unsigned int	cause;
	screen_softc_t	sc;
{
	return gq_tlbmiss(gq, cause, FALSE, sc);
}


static
gq_intr_vrfy(gq, cause, sc)
	gx_softc_t	*gq;
	unsigned int	cause;
{
    register int virtual_address;
    register struct pte *pte;

    virtual_address = (cause & HST_INTR_MASK);

#if 0

    pte = vtopte(gx_serverp, btop(virtual_address));

    if (pte == 0) {
	/*GQ_INTRH(gqo) = cause;		/* clear intr line */
	/*wbflush();*/
	printf("gq_intr_vrfy: bad VA 0x%x\n", virtual_address);
	/* let server segv while handling signal */
	return gq_request_pagein(gq, cause);
    }

    if (pte->pg_v)
    {
	/* page is already valid */
	*GQ_INTRH(gqo) = (pte->pg_pfnum << GQ_INTR_SHFT);
	wbflush();
	return 0;
    }
    else
    {
	*GQ_INTRH(gqo) = GQ_INTR_ACK;	/* N10 will choke on this */
	wbflush();
	psignal(gx_serverp, SIGSEGV); /* not valid - SEGV the server */
/* we'd do a */
	up->interrupt_info = 1;
	evc_signal(&gq->event);
    }

#else
	/* That must have been for debugging */
	gimmeabreak();
#endif
}

/*
 * Helper functions
 */
static
gq_request_pagein(gq, cause, sc)
	gx_softc_t	*gq;
	unsigned int	cause;
	screen_softc_t	sc;
{
	register user_info_t	*up = sc->up;

	*(pmag_da_inth(gq->board_base)) = cause;	/* clear the R3K
							 * interrupt line */

	up->dev_dep_2.gx.ptpt_pgin = (int *)
		(cause & (HST_INTR_MASK | HST_INTR_PMSK));

	/*
	 * Need to do a pagein
	 * Server is notified of this with a Unix signal #30 (SIGUSR1)
	 */
	up->interrupt_info = 2;
	evc_signal(&gq->event);

#if debug
	if (gq_dont)
		printf("pgin %x\n", cause);
#endif
}

/*
 * This one should be called when a virtual mapping
 * is removed from the X11 server's address space.
 * I am ashamed of having written such abomination.
 */
gq_pmap_hacking(pmap, virtual_address)
	pmap_t		pmap;
	vm_offset_t	virtual_address;
{
	gx_softc_t	*gq;
	vm_offset_t	vsn, page_table;
	unsigned int	*pte, *pde;
	int		i;
	screen_softc_t	sc;
	
	for (i = 0; i < NBM; i++) {

		sc = screen(i);
		if (sc == 0) continue;
		gq = (gx_softc_t *) sc->hw_state;
		if (gq->pmap == pmap)
			break;
	}
	if (i == NBM) return;

	vsn = coproc_vtovsn(virtual_address);	/* virt seg num */
	pde = (unsigned int *) (gq->ptpt + vsn);
	if (*pde == 0)
		return;

	page_table = (vm_offset_t) (PHYS_TO_K1SEG(coproc_pdetophys(*pde)));
	pte = (unsigned int *) (page_table + coproc_vtopn(virtual_address) * 4);
#if debug
	vsn = *pte;
#endif
	if (*pte == 0)
		return;

	*pte = 0; /* invalidate */

	/* Now tell coproc to do the same */
	virtual_address &= GQ_INTR_MASK;
	i = gq_intr_coproc( gq, virtual_address | GQ_INTR_INV1, sc);
#if debug
	if (gq_dont)
		printf("inval %x [%x %x] --> %x\n",
			virtual_address, *pde, vsn, i);
#endif
}

/*
 * Get/send packets to the board
 */
int *
gq_get_packet(gq)
	gx_softc_t	*gq;
{
	static int      whichBuffer = 0;	/* 0 || 1 */
	register int   *buf;

	if (gq_dont)
		return (int *) 0;

	/*
	 * don`t collide with N10 over SRAM when xcons not enabled!
	 * should not be the common case. 
	 */
	if (gq->coproc_active) {
		register int    i;

		/*
		 * ask N10 which packet buffer we may use.
		 */
		whichBuffer = gq_intr_coproc(gq, GQ_INTR_PAUS, 0);

		if (whichBuffer < 0 || whichBuffer > 1) {
			whichBuffer = 0;
			return (int *) 0;
		}

		/*
		 * Then wait for stic to be idle
		 * since this buffer may be currently executing
		 */
		gq_poll_stic_pint(gq, pmag_da_stic(gq->board_base));

	}

	buf = pmag_da_sram(gq->board_base)->reqbuf[whichBuffer];
	whichBuffer ^= 0x1;

	bzero(buf, GQ_CONSOLE_PACKET_SIZE);

	return (buf);
}

/*
 * wait for stic to be ready to accept next packet.  note that we never
 * wait forever.  we'll time out and go ahead and see if the stic will
 * accept a packet anyway.
 */
gq_poll_stic_pint(gq, stic)
	gx_softc_t	*gq;
	sticRegs	*stic;
{
	register int i;

	for (i = 0; i < STAMP_RETRIES; i++) {
		if (stic->ipdvint & STIC_INT_P)
			break;
		gq->poll_count++;
		delay(STAMP_DELAY);
	}
	if (i == STAMP_RETRIES)
		gq->poll_timeouts++;
}

gq_send_packet(gq, buf)
	gx_softc_t	*gq;
	int		*buf;	/* virtual */
{
	int             i;
	vm_offset_t	off;
	volatile int   *poll;
	sticRegs       *stic = pmag_da_stic(gq->board_base);

	if (gq_dont) return;

	/* ... to sram phys addr ... */
	off = (vm_offset_t)(buf) - (vm_offset_t)pmag_da_sram(gq->board_base);

	poll = (volatile int *)
		(pmag_da_poll(gq->board_base) + GX_SYS_TO_DMA(off));

	gq_poll_stic_pint(gq, stic);

	/* clear pkt done intr bit */
	stic->ipdvint = STIC_INT_P_WE;

	/* make sure all writes completed */
	wbflush();

	if (*poll != STAMP_GOOD) {
		gq->dropped_packets++;
		return;
	}

	/*
	 * If N10 running, then must have asked permission to do console
	 * output, so N10 must be waiting for OK to proceed... 
	 */
	if (gq->coproc_active) {
		gq_poll_stic_pint(gq, stic);
		pmag_da_sram(gq->board_base)->intr_coproc = 0;
		wbflush();
	}
}

/*
 * Synch with coproc
 */
gq_intr_coproc(gq, cmd, sc)
	gx_softc_t	*gq;
	int cmd;
	screen_softc_t	sc;
{
	register int    i, ans;
	volatile int   *intr_coproc;
	volatile int   *read_coproc;
	int             rval = 4;

	intr_coproc = pmag_da_intc(gq->board_base);
	read_coproc = &(pmag_da_sram(gq->board_base)->intr_coproc);

	*intr_coproc = cmd;
	wbflush();

	/* timeout */
	i = gq->coproc_active ? GQ_INTR_TIMO : (GQ_INTR_TIMO >> 2);
	for ( ; i > 0; i--) {

		delay(10);	/* 150 cycles for N10 intr roundtrip */
		switch (ans = *read_coproc) {	/* N10 running at 25-30ns
						 * clock */
		    case 0xdeadbabe:
		    case 0x00000000:
			return -1;
		    case GQ_INTR_BUF0:	/* 1 */
			return 0;
		    case GQ_INTR_BUF1:	/* 2 */
			return 1;
		    default:
			if (ans != cmd) {
				rval = 2;	/* > 1 */
				goto BadReply;
			}
		}
	}
BadReply:

	*pmag_da_rstc(gq->board_base) = 0;	/* kill the N10 */
	wbflush();
	delay(20000);
	* (pmag_da_inth(gq->board_base)) = GQ_INTR_ACK;

	gq->coproc_active = FALSE;
	printf("gq_intr_coproc: wrong answer.\n");

	/* Have the server killed */

	if (sc) {
		sc->up->interrupt_info = 3;
		evc_signal(&gq->event);
	}
	return rval;
}


/*
 * Boot time initialization: must make device
 * usable as console asap.
 */
gq_mem_need()
{
	/*
	 * 5 pages for user info (roundup from 4c04)
	 * 8 pages for ring buffer (what for ??)
	 * 1 page for pagetables
	 */
	return GQ_MAPPED_MEM_SIZE + GQ_PTTB_SIZE;
}

extern int
	gq_soft_reset(), gq_set_status(),
	gx_pos_cursor(), gx_graphic_open(),
	bt459_video_on(), bt459_video_off(),
	/* XXXX gq_vretrace(), */
	gx_get_status(), gx_char_paint(),
	gx_insert_line(), gx_remove_line(),
	gx_clear_bitmap(), gx_map_page();

static struct screen_switch gq_sw = {
	gx_graphic_open,	/* graphic_open */
	gq_soft_reset,		/* graphic_close */
	gq_set_status,		/* set_status */
	gx_get_status,		/* get_status */
	gx_char_paint,		/* char_paint */
	gx_pos_cursor,		/* pos_cursor */
	gx_insert_line,		/* insert_line */
	gx_remove_line,		/* remove_line */
	gx_clear_bitmap,	/* clear_bitmap */
	bt459_video_on,		/* video_on */
	bt459_video_off,	/* video_off */
	screen_noop /*gq_vretrace XXX */,		/* intr_enable */
	gx_map_page		/* map_page */
};

gq_cold_init(unit, up)
{
	gx_softc_t	*gq;
	screen_softc_t	sc = screen(unit);
	int		base = tc_probe("gq");
	vm_offset_t	ring, ring_offset;

	bcopy(&gq_sw, &sc->sw, sizeof(sc->sw));
	sc->flags |= COLOR_SCREEN;
	sc->frame_scanline_width = 2048;
	sc->frame_height = 1024;
	sc->frame_visible_width = 1280;
	sc->frame_visible_height = 1024;

	up = K0SEG_TO_K1SEG(up);
	ring = up + 0x5000;

	up += SILLY_OFFSET;
	ring_offset = (ring - up) / sizeof(int);

	gx_init_screen_params(sc,up);
	(void) screen_up(unit, up);

	gq = gx_alloc(	unit, base, 
			base + GQ_OFFSET_BT459, base + GQ_OFFSET_RESET_BT459,
			ring, ring_offset,
			GQ_MAPPED_MEM_SIZE, GQ_SLOT_SIZE);
	gq->ptpt = (int *) (ring + GQ_RING_SIZE);
	gq->get_packet = gq_get_packet;
	gq->send_packet = gq_send_packet;

	gx_init_stamp( pmag_da_stic(base), pmag_da_stamp(base) );
	gx_decode_stamp_configuration( gq, up,
		(pmag_da_stic(base)->modcl & ~STIC_CF_CONFIG_OPTION)
		      | STIC_OPT_3DA_SH);

	screen_default_colors(up);

	gx_soft_reset(sc);

	/*
	 * Clearing the screen at boot saves from scrolling
	 * much, and speeds up booting quite a bit.
	 */
	screen_blitc( unit, 'C'-'@');/* clear screen */
}

#endif	/* NGX > 0 */
