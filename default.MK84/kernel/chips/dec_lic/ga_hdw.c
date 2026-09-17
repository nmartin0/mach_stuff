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
 * $Log:	ga_hdw.c,v $
 * Revision 2.5  93/05/17  15:16:52  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 	[93/05/17            rvb]
 * 
 * Revision 2.4  93/05/15  19:35:35  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.3  93/02/05  08:06:19  danner
 * 	Mods to *compile* on Flamingo.
 * 	[93/02/04  01:51:04  af]
 * 
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.2  92/05/22  15:46:57  jfriedl
 * 	Made it work for real. Added some Ultrix code.
 * 	Moved in a copyright-infected directory.
 * 	[92/05/20            af]
 * 
 * Revision 2.7  91/06/19  11:56:02  rvb
 * 	The busses.h and other header files have moved to the "chips"
 * 	directory.
 * 	[91/06/07            rvb]
 * 
 * Revision 2.6  91/05/14  17:21:28  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:33:49  mrt
 * 	In interrupt routine, drop priority as now required.
 * 	[91/02/12  12:43:54  af]
 * 
 * Revision 2.4  91/02/05  17:40:46  mrt
 * 	Added author notices
 * 	[91/02/04  11:13:20  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:11:12  mrt]
 * 
 * Revision 2.3  90/12/05  23:31:08  af
 * 	Created, (almost) empty.
 * 	[90/12/03  23:17:23  af]
 */
/*
 *	File: ga_hdw.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Hardware-level routines for the 3max 2D display.
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


#include <gx.h>
#if	(NGX > 0)
#include <platforms.h>

#include <sys/types.h>
#include <mach/std_types.h>
#include <machine/machspl.h>
#include <sys/types.h>
#include <chips/busses.h>
#include <chips/screen_defs.h>

#include <chips/dec_lic/gx_defs.h>
#include <chips/dec_lic/stamp.h>

#ifdef	DECSTATION
#include <mips/mips_cpu.h>
#define	phystokvu(x)	PHYS_TO_K1SEG(x)
#include <mips/PMAX/tc.h>
#include <mips/PMAX/pmag_ca.h>
#define	pmag_ca_stic(x)		((sticRegs*)(((vm_offset_t)x)+GA_OFFSET_STIC))
#define	pmag_ca_stamp(x)	(char*)(((vm_offset_t)x) + GA_OFFSET_STAMP)
#define	pmag_ca_poll(x)		(char*)(((vm_offset_t)x) + GA_OFFSET_POLL)
#endif

#ifdef	FLAMINGO
#define	phystokvu(x)	PHYS_TO_K0SEG(x)
extern vm_offset_t kvtophys( vm_offset_t virt );
#include <alpha/DEC/tc.h>
#include <mips/PMAX/pmag_ca.h>	/* XXX fixme */
#define	pmag_ca_stic(x)		((sticRegs*)(((vm_offset_t)x)+GA_OFFSET_STIC))
#define	pmag_ca_stamp(x)	(char*)(((vm_offset_t)x) + GA_OFFSET_STAMP)
#define	pmag_ca_poll(x)		(char*)(((vm_offset_t)x) + GA_OFFSET_POLL)
#endif

/*
 * Definition of the driver for the auto-configuration program.
 */

int	ga_probe(), ga_intr();
void	ga_attach();

vm_offset_t	ga_std[NGX] = { 0 };
struct	bus_device *ga_info[NGX];
struct	bus_driver ga_driver = 
        { ga_probe, 0, ga_attach, 0, ga_std, "ga", ga_info,};


/*
 * Probe/Attach functions
 */

ga_probe( /* reg, ui */)
{
	static probed_once = 0;

	/*
	 * Probing was really done sweeping the TC long ago
	 */
	if (tc_probe("ga") == 0)
		return 0;
	if (probed_once++ > 1)
		printf("[mappable] ");
	return 1;
}

void
ga_attach(ui)
	struct bus_device *ui;
{
	/* ... */
	printf(": 2D color display");
}


/*
 * Interrupt routine
 */
#define SUBMIT_PACKET(i) \
	status = *pCom2d->intr_qpoll[i]
#define	NEXT_BUF(p,i) \
	(((i) == (GA_QUEUE_PACKETS-1)) ? 0 : ((i) + 1))

ga_intr(unit,spllevel)
	register int unit;
	spl_t	spllevel;
{
	screen_softc_t  sc = screen(unit);
	gx_softc_t	*ga = (gx_softc_t*)sc->hw_state;
	register sticRegs *stic = pmag_ca_stic(ga->board_base);
	struct ga_ComArea *Com = (struct ga_ComArea *)ga->ring_buffer;

	if (stic->ipdvint & STIC_INT_P) {	/* Packet Intr */

		register Com2dPtr pCom2d = (Com2dPtr) & Com->SRV2DCom;
		register int    cp;
		register ga_PacketPtr pPkt;
		register int    status;


		/*
		 * Clear *only* packet done interrupt
		 */
		stic->ipdvint = (stic->ipdvint | STIC_INT_P_WE) &
			~(STIC_INT_E_WE | STIC_INT_V_WE | STIC_INT_P);

		/*
		 * If were idle, dismiss this as a spurious interrupt
		 */
		if (pCom2d->lastRead == pCom2d->lastWritten ||
		    !(pCom2d->intr_status & GA_INTR_ACTIVE))
			return 1;

		/*
		 * If we are in the process of sending a packet through a
		 * cliplist, fixup packet with next cliprect and resubmit.  If
		 * the last cliprect just completed, clear the clipping
		 * status. 
		 */
		if (pCom2d->intr_status & GA_INTR_CLIP) {
			if (pCom2d->numCliprect-- > 0) {
				*pCom2d->fixCliprect = *pCom2d->pCliprect++;
				SUBMIT_PACKET(pCom2d->lastRead);
				return 1;
			} else {
				pCom2d->intr_status &= ~GA_INTR_CLIP;
			}
		}
loop:
		/*
		 * Point past packet just completed
		 */
		cp = pCom2d->lastRead = NEXT_BUF(pCom2d, pCom2d->lastRead);
		/* if no more packets to process, set idle status */
		if (cp == pCom2d->lastWritten) {
			pCom2d->intr_status &= ~GA_INTR_ACTIVE;
			return 1;
		}
		pPkt = (ga_PacketPtr) Com->IntrBuf[cp];

		if (pPkt->un.un.opcode == N_PASSPACKET) {
			unsigned int             clip_idx;

			clip_idx = (unsigned int)pPkt->un.PassPacket.cliplist_sync;

			if (clip_idx < N_MAX_CLIPLISTS) {
				int             nrects;
				int             fix_off;
				gaStampClipRect *pFixup;

				nrects = Com->ClipL[clip_idx].numClipRects;
				fix_off = DECODE_CLIP_INDEX(pPkt->un.PassPacket.data);
				pFixup = (gaStampClipRect *) & pPkt->un.PassPacket.data[fix_off];

				*pFixup = Com->ClipL[clip_idx].clipRects[0];
				if (nrects > 1) {
					pCom2d->pCliprect =
						&Com->ClipL[clip_idx].clipRects[1];
					pCom2d->numCliprect = nrects - 1;
					pCom2d->fixCliprect = pFixup;
					pCom2d->intr_status |= GA_INTR_CLIP;
				}
			}
			SUBMIT_PACKET(cp);
			return 1;
		} else {
			/*
			 * Not a pass packet, skip it
			 */
			goto loop;
		}

	} else if (stic->ipdvint & STIC_INT_V) {	/* Vert Int */

		stic->ipdvint = (stic->ipdvint | STIC_INT_V_WE) &
			~(STIC_INT_E_WE | STIC_INT_P_WE | STIC_INT_V);
		lk201_led(unit);

	} else /*if (stic->ipdvint & STIC_INT_E)*/ {	/* Error, stray */

		panic("ga_intr %x %x %x %x %x", stic->ipdvint,
		       stic->sticsr, stic->buscsr,
		       stic->busadr, stic->busdat);
	}
}

/*
 * Turn vert retrace interrupt on/off
 */
ga_vretrace(ga, on)
	gx_softc_t	*ga;
	boolean_t	on;
{
	register sticRegs *stic = pmag_ca_stic(ga->board_base);

	if (on)
		stic->ipdvint = STIC_INT_V_WE | STIC_INT_V_EN;	
	else
		stic->ipdvint = STIC_INT_V_WE;	
}

/*
 * Get/send packets to the board
 */
int *ga_get_packet(ga)
	gx_softc_t	*ga;
{
	static int      i = 1;
	register int   *buf;

	buf = ga->ring_buffer + ((GA_CONSOLE_PACKET_SIZE >> 2) * i) + 3;
	i ^= 1;			/* toggle between buffers */

	bzero( (char*) buf, GA_CONSOLE_PACKET_SIZE);

	return (buf);
}


int ga_dont = 0;

ga_send_packet(ga, buf)
	gx_softc_t	*ga;
	int		*buf;	/* virtual */
{
	register int    i = 0;
	volatile int   *poll;
	register sticRegs *stic = pmag_ca_stic(ga->board_base);
	register int    save_ipdvint;

	if (ga_dont)
		return;

	/*
	 * The X11 server does not tolerate anyone else
	 * around while it plays with the stic.
	 */
	if (ga->mapped_to_user) {

		delay(10000);
		goto kick_it;
	}

	poll = (volatile int *)
		(pmag_ca_poll(ga->board_base) + GX_SYS_TO_DMA(buf));

	/* disable packet-done interrupts */
	stic->ipdvint = ((save_ipdvint = stic->ipdvint) | STIC_INT_P_WE) &
		~(STIC_INT_E_WE | STIC_INT_V_WE | STIC_INT_P_EN);

	wbflush();		/* make sure all writes completed */

	/*
	 * wait for stic to be ready to accept next packet.  note that we
	 * never wait forever.  we`ll time out and go ahead and see if the
	 * stic will accept a packet anyway.  if not, _then_ we complain... 
	 */
	i = 0;
	if ((save_ipdvint & STIC_INT_P) == 0)
		for (i = 0; i < STAMP_RETRIES; i++) {
			if (stic->ipdvint & STIC_INT_P)
				break;
			ga->poll_count++;
			delay(STAMP_DELAY);
		}

	if (i == STAMP_RETRIES)
		ga->poll_timeouts++;

	/*
	 * Restore ipdvint state, clearing the STIC_INT_P bit.  If interrupts
	 * were enabled before, they will be re-enabled.  ga_intr() is capable
	 * of dealing with spurious interrupts generated while the server is
	 * running. 
	 */
	stic->ipdvint = (save_ipdvint | STIC_INT_P_WE) &
		~(STIC_INT_E_WE | STIC_INT_V_WE | STIC_INT_P);
	wbflush();

kick_it:
	/*
	 * See if it got ok the last packet we send from there
	 */
	if (*poll != STAMP_GOOD) {
		i = -1;
		stic->ipdvint = save_ipdvint | STIC_INT_WE;
		wbflush();
		ga->dropped_packets++;
	}
}

/*
 * Boot time initialization: must make device
 * usable as console asap.
 */
vm_offset_t
ga_where_do_you_want_it(phys_addr)
	register vm_offset_t phys_addr;
{
	/* From experiments, it looks like we want the damn
	   thing eight pages below a 64k aligned point, and
	   that point should not be 128k aligned.

	   If you think that`s magic you are right.  The catch
	   is in the way the wires are twisted, look at the
	   comments in the gx_defs.h file for the whole story */


#	define	_128k_		0x20000
#	define	_64k_		0x10000
#	define	_32k_		0x8000
#	define	_sentinel_	0x1000
#	define	roundup(p,b)	((((vm_offset_t)p) + (b) - 1) & ~((b) - 1))

	phys_addr = roundup( phys_addr, _128k_ );
	if (phys_addr & _64k_)
		phys_addr += _64k_;
	return phys_addr + _32k_;
}

ga_mem_need()
{
	register vm_offset_t	free_phys;

	/* screen_data points to free memory in k0 space,
	   we care where our phys mem goes, unfortunately.
	   *how* we care is a mistery, lost in the wires */

	free_phys = kvtophys((vm_offset_t) screen_data);
	return ((ga_where_do_you_want_it(free_phys) - free_phys)
		+ GA_MAPPED_MEM_SIZE + _sentinel_);
#undef	_sentinel_
#undef	roundupn
}

extern int
	ga_soft_reset(), ga_set_status(),
	gx_pos_cursor(), gx_graphic_open(),
	bt459_video_on(), bt459_video_off(),
	gx_get_status(), gx_char_paint(),
	gx_insert_line(), gx_remove_line(),
	gx_clear_bitmap(), gx_map_page();

static struct screen_switch ga_sw = {
	gx_graphic_open,		/* graphic_open */
	ga_soft_reset,		/* graphic_close */
	ga_set_status,		/* set_status */
	gx_get_status,		/* get_status */
	gx_char_paint,		/* char_paint */
	gx_pos_cursor,		/* pos_cursor */
	gx_insert_line,		/* insert_line */
	gx_remove_line,		/* remove_line */
	gx_clear_bitmap,	/* clear_bitmap */
	bt459_video_on,		/* video_on */
	bt459_video_off,	/* video_off */
	ga_vretrace,		/* intr_enable */
	gx_map_page		/* map_page */
};

ga_cold_init(unit, up)
	vm_offset_t	up;
{
	gx_softc_t	*ga;
	screen_softc_t	sc = screen(unit);
	int		base = tc_probe("ga");
	vm_offset_t	ring, ring_offset;

	bcopy(&ga_sw, &sc->sw, sizeof(sc->sw));
	sc->flags |= COLOR_SCREEN;
	sc->frame_scanline_width = 2048;
	sc->frame_height = 1024;
	sc->frame_visible_width = 1280;
	sc->frame_visible_height = 1024;

	up = PHYS_TO_K1SEG(ga_where_do_you_want_it(kvtophys(up)));
	ring = up + 0x8000 - 12;

	up += SILLY_OFFSET;
	ring_offset = (ring - up) / sizeof(int);

	gx_init_screen_params(sc,up);
	(void) screen_up(unit, up);

	ga = gx_alloc(	unit, base,
			base + GA_OFFSET_BT459, base + GA_OFFSET_RESET_BT459,
			ring, ring_offset,
			GA_MAPPED_MEM_SIZE, GA_SLOT_SIZE);
	ga->get_packet = ga_get_packet;
	ga->send_packet = ga_send_packet;

	gx_init_stamp( pmag_ca_stic(base), pmag_ca_stamp(base) );
	gx_decode_stamp_configuration( ga, up, pmag_ca_stic(base)->modcl );

	screen_default_colors(up);

	ga_soft_reset(sc);

	/*
	 * Clearing the screen at boot saves from scrolling
	 * much, and speeds up booting quite a bit.
	 */
	screen_blitc( unit, 'C'-'@');/* clear screen */
}

#endif	/* NGX > 0 */
