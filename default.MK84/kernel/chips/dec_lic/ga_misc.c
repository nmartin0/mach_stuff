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
 * $Log:	ga_misc.c,v $
 * Revision 2.3  93/02/05  08:06:14  danner
 * 	Mods to *compile* on Flamingo.
 * 	[93/02/04  01:51:39  af]
 * 
 * Revision 2.2  92/05/22  15:47:02  jfriedl
 * 	Disable interrupts on exit.
 * 	[92/05/20  22:53:19  af]
 * 
 * 	Created, with some Ultrix code.
 * 	[92/05/09            af]
 * 
 */
/*
 *	File: ga_misc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	Driver for the PMAG-CA 2D graphic board.
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


#include <gx.h>
#if	(NGX > 0)
#include <platforms.h>

/*
 * NOTE: A lot of code is common to this and the 3D driver,
 * and lives in gx_misc.c
 */

#include <device/device_types.h>
#include <chips/screen_defs.h>
#include <chips/dec_lic/gx_defs.h>
#include <chips/dec_lic/stamp.h>

#include <chips/bt459.h>

#ifdef	DECSTATION
#include <mips/PMAX/pmag_ca.h>
#define	pmag_ca_stic(x)		((sticRegs*)(((vm_offset_t)x)+GA_OFFSET_STIC))
#endif

#ifdef	FLAMINGO
extern vm_offset_t kvtophys( vm_offset_t virt );
#include <mips/PMAX/pmag_ca.h>	/* XXXX fixme */
#define	pmag_ca_stic(x)		((sticRegs*)(((vm_offset_t)x)+GA_OFFSET_STIC))
#endif

/*
 * Device-specific set status
 */
ga_set_status(sc, flavor, status, status_count)
	screen_softc_t	sc;
	int		flavor;
	dev_status_t	status;
	unsigned int	status_count;
{
	if (flavor == SCREEN_ADJ_MAPPED_INFO) {

		gx_softc_t     *ga = (gx_softc_t *) sc->hw_state;
		user_info_t    *up = sc->up;
		vm_offset_t	user_base = *(vm_offset_t *) status;
		vm_offset_t	user_addr, rb_phys_addr, rb_poll_off;
		char		*poll_addr;

		user_addr = user_base + SILLY_OFFSET;

		/* Make it point to the event_queue, in user virtual */
		up->evque.events = (screen_event_t *)(user_addr +
			((char*)up->event_queue - (char*)up));

		/* Make it point to the point_track, in user virtual */
		up->evque.track = (screen_timed_point_t *)(user_addr +
			((char*)up->point_track - (char*)up));

		/* Make it point to the ring buffer, in user virtual */
		up->dev_dep_1.gx.rb_addr = (int *)user_addr +
			ga->ring_buffer_offset;

	        /*
	         * The server uses the "physical" address to figure out
	         * which address in STIC polling space to read in order to
	         * dispatch a packet.  However, the STIC's physical addresses
	         * do not map directly onto host physical addresses.
		 * Furthermore, we are not interested in the address of the
		 * ucode packet, but rather the embedded STIC packet within.
		 * This is the origin of the mysterious fudge factor of 12.
		 * The physical address points to the start of the
		 * ring buffer, but the STIC address points to the
		 * first STIC packet in the ring buffer.
	         */

		rb_phys_addr = kvtophys( (vm_offset_t) ga->ring_buffer);
		rb_poll_off = GX_SYS_TO_DMA(rb_phys_addr + 12);
		up->dev_dep_1.gx.rb_phys = GX_SYS_TO_STIC(rb_phys_addr + 12);
		up->dev_dep_1.gx.rb_size = GA_RING_SIZE;

		/*
		 *	DMA for the ring buffer is, again, weird
		 */
	        poll_addr = (char *) (user_base + GA_MAPPED_MEM_SIZE +
					GA_OFFSET_POLL);
		up->dev_dep_2.gx.stic_dma_rb = (int *)(poll_addr + rb_poll_off);

		/*
		 * Map STIC control registers into user virtual
		 */
/* unused	up->dev_dep_2.gx.gxo = (char*)user_base + GA_MAPPED_MEM_SIZE; */
		up->dev_dep_2.gx.stic_reg = (int *) (user_base + GA_MAPPED_MEM_SIZE +
					GA_OFFSET_STIC);

	        /*
	         * These addresses are used by the server when it is directly
	         * submitting packets to the STIC.
	         */
		{
			register int	i;
			register struct ga_ComArea * Com;

			Com = (struct ga_ComArea *)ga->ring_buffer;

		        for (i = 0; i < GA_QUEUE_PACKETS; i++) {
			    Com->SRV2DCom.srv_qpoll[i] = (volatile long *)
			    	(poll_addr + GX_SYS_TO_DMA(&Com->IntrBuf[i][3]));
			}
		}

		return D_SUCCESS;
	} else
		return gx_set_status(sc, flavor, status, status_count);
}

/*
 * Do what's needed when X exits
 */
ga_soft_reset(sc)
	screen_softc_t	sc;
{
	register int	i;
	register struct ga_ComArea * Com;
	sticRegs	*stic;
	Com2dPtr	pCom2d;
	gx_softc_t	*ga = (gx_softc_t*) sc->hw_state;

	stic = pmag_ca_stic(ga->board_base);
	stic->ipdvint = STIC_INT_CLR;

	Com = (struct ga_ComArea *)ga->ring_buffer;

	pCom2d = (Com2dPtr) & Com->SRV2DCom;
	pCom2d->intr_status = 0;

	for (i = 0; i < GA_QUEUE_PACKETS; i++)
	    Com->SRV2DCom.intr_qpoll[i] =
		(volatile long *)((char *)ga->board_base +
			 GX_SYS_TO_DMA(&Com->IntrBuf[i][3]));

	gx_soft_reset(sc);

}


#endif	/* (NGX > 0) */
