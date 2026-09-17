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
 * $Log:	gq_misc.c,v $
 * Revision 2.3  93/02/05  08:06:25  danner
 * 	Mods to *compile* on Flamingo.
 * 	[93/02/04  01:52:10  af]
 * 
 * Revision 2.2  92/05/22  15:47:17  jfriedl
 * 	Made it work for real.
 * 	[92/05/20  22:54:37  af]
 * 
 * 	Created, with some Ultrix code.
 * 	[92/05/09            af]
 * 
 */
/*
 *	File: gq_misc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	Driver for the PMAG-DA/FA graphic boards (3D).
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

/*
 * NOTE: A lot of code is common to this and the 2D driver,
 * and lives in gx_misc.c
 */

#include <device/device_types.h>
#include <chips/screen_defs.h>
#include <chips/dec_lic/gx_defs.h>

#include <platforms.h>

#ifdef	DECSTATION
#include <mips/PMAX/pmag_da.h>
#define	pmag_da_rstc(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_RESET_N10))
#define	pmag_da_strtc(x)	((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_START_N10))
#define	pmag_da_inth(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_INT_TO_HOST))
#endif

#ifdef	FLAMINGO
extern vm_offset_t kvtophys( vm_offset_t virt );
#include <mips/PMAX/pmag_da.h>	/* XXX fixme */
#define	pmag_da_rstc(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_RESET_N10))
#define	pmag_da_strtc(x)	((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_START_N10))
#define	pmag_da_inth(x)		((volatile int*)(((vm_offset_t)x) + GQ_OFFSET_INT_TO_HOST))
#endif

/*
 * Device-specific set status
 */
gq_set_status(sc, flavor, status, status_count)
	screen_softc_t	sc;
	int		flavor;
	dev_status_t	status;
	unsigned int	status_count;
{
	gx_softc_t     *gq = (gx_softc_t *) sc->hw_state;

	switch (flavor) {

	case SCREEN_ADJ_MAPPED_INFO: {

		user_info_t    *up = sc->up;
		vm_offset_t	user_base = *(vm_offset_t *) status;
		vm_offset_t	user_addr;

		/* Say what the eventcount id is */
		if (gq->event.sanity == 0) {
			evc_init(&gq->event);
		}
		up->event_id = gq->event.ev_id;

		user_addr = user_base + SILLY_OFFSET;

		/* Make it point to the event_queue, in user virtual */
		up->evque.events = (screen_event_t *)(user_addr +
			((char*)up->event_queue - (char*)up));

		/* Make it point to the point_track, in user virtual */
		up->evque.track = (screen_timed_point_t *)(user_addr +
			((char*)up->point_track - (char*)up));

		/* Make it point to the ring buffer, in user virtual */
		up->dev_dep_1.gx.rb_addr = (int *)user_addr +
			gq->ring_buffer_offset;

		up->dev_dep_1.gx.rb_phys = kvtophys((vm_offset_t)gq->ring_buffer);
		up->dev_dep_1.gx.rb_size = GQ_RING_SIZE;

		up->dev_dep_2.gx.ptpt_phys = kvtophys((vm_offset_t)gq->ptpt);
		up->dev_dep_2.gx.ptpt_size = 512;

		/*
		 * Map 3D board into user virtual
		 */
		user_base += GQ_MAPPED_MEM_SIZE;
		up->dev_dep_2.gx.gxo = (char*) user_base;
		up->dev_dep_1.gx.gram = (int *) (user_base + GQ_OFFSET_SRAM);
		up->dev_dep_2.gx.stic_reg = (int *) (user_base + GQ_OFFSET_STIC);

		break;

	}

	case _IO('q', 17):	/* N10RESET */

		gq->coproc_active = FALSE;
		*pmag_da_rstc(gq->board_base) = 0;
		break;

	case _IO('q', 18):	/* N10START */

		bzero(gq->ptpt, GQ_PTPT_SIZE + GQ_PTTB_SIZE);
		gq->coproc_active = TRUE;
		*pmag_da_strtc(gq->board_base) = 0;
		break;

	case SCREEN_HARDWARE_DEP: {
		vm_map_t	map, port_name_to_map();
		extern int	gq_pmap_hacking();

		map = port_name_to_map(*(unsigned int *)status);
		if (map == VM_MAP_NULL)
			return D_IO_ERROR;
		gq->pmap = map->pmap;
		gq->pmap->hacking = gq_pmap_hacking;
		break;
	}

	default:
		return gx_set_status(sc, flavor, status, status_count);
	}

	return D_SUCCESS;
}

/*
 * Do what's needed when X exits
 */
gq_soft_reset(sc)
	screen_softc_t	sc;
{
	gx_softc_t	*gq = (gx_softc_t*) sc->hw_state;

	/* halt N10 */
	*pmag_da_rstc(gq->board_base) = 0;
	wbflush();
	delay(20000);
	* (pmag_da_inth(gq->board_base)) = GQ_INTR_ACK;

	gq->coproc_active = FALSE;

	/* Zero pagetables */
	bzero(gq->ptpt, GQ_PTPT_SIZE + GQ_PTTB_SIZE);
	gq->pt_sbrk = 0;
	gq->pmap = 0;

	gx_soft_reset(sc);
}

#endif	/* (NGX > 0) */
