/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	gx_misc.c,v $
 * Revision 2.4  93/11/17  16:10:52  dbg
 * 	Changed mmap routines to return physical address instead of
 * 	physical page number.
 * 	[93/06/16            dbg]
 * 
 * Revision 2.3  93/02/05  08:06:37  danner
 * 	Mods to *compile* on Flamingo.
 * 	[93/02/04  01:52:39  af]
 * 
 * Revision 2.2  92/05/22  15:47:27  jfriedl
 * 	Took away spl protection around (very expensive) loading
 * 	of cursor bitmap. The bt459 does it fine-grain, and with
 * 	retry so it really was not necessary. Maxine was not happy.
 * 	Avoid spls and use SCREEN_BEING_UPDATED instead.
 * 	[92/05/21            af]
 * 	Added open routine, reverse video, fixed line insertion,
 * 	added callups to terminal emulator on scrolls (took a
 * 	while to figure this one out), added third return value
 * 	to SCREEN_GET_OFFSETS to tell if 3d board, spl protection
 * 	where needed, spruced up.
 * 	[92/05/20  22:59:00  af]
 * 
 * 	Created, with some Ultrix code.
 * 	[92/05/13            af]
 * 
 */
/*
 *	File: gx_misc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Common code for the 3max 2D and 3D Display driver.
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

#include <device/device_types.h>
#include <chips/screen_defs.h>
#include <chips/dec_lic/gx_defs.h>
#include <chips/dec_lic/stamp.h>

#include <platforms.h>

#ifdef	DECSTATION
#define machine_btop mips_btop
#endif

#ifdef	FLAMINGO
#define	machine_btop alpha_btop
#endif


/* not all compilers are smart about multiply by 15 */
#if	(KfontHeight==15)
#	define TIMES_KfontHeight(w)	(((w)<<4)-(w))
#else
#	define TIMES_KfontHeight(w)	((w)*KfontHeight)
#endif

#define	min(a,b)	((a < b) ? a : b)

/* Hardware state */
gx_softc_t	gx_softc_data[NGX];

gx_softc_t*
gx_alloc(	unit, base, cur, creset, ring, ring_offs,
	 	mapped_memory_size, mapped_io_size )

	char		*base, *cur, *creset;
	int		*ring;
	unsigned int	ring_offs;
	unsigned int	mapped_memory_size, mapped_io_size;
{
	gx_softc_t     *gx = &gx_softc_data[unit];

	gx->cursor_registers	= cur;
	gx->cursor_reset	= creset;
	gx->board_base		= base;
	gx->ring_buffer 	= ring;
	gx->ring_buffer_offset	= ring_offs;
	gx->mapped_memory_size	= mapped_memory_size;
	gx->mapped_io_size	= mapped_io_size;

	screen_attach(unit, (char *) gx);

	return gx;
}

/*
 * Initialize color map, for kernel use
 */
gx_init_colormap(sc)
	screen_softc_t	sc;
{
	gx_softc_t	*gx = (gx_softc_t*)sc->hw_state;
	user_info_t	*up = sc->up;
	color_map_t	Bg_Fg[2];
	register int	i;

	bzero(	up->dev_dep_2.gx.colormap,
		sizeof(up->dev_dep_2.gx.colormap));
	up->dev_dep_2.gx.colormap[  0] = 0;
	up->dev_dep_2.gx.colormap[  1] = 0xffffff;
	up->dev_dep_2.gx.colormap[255] = 0xffffff;
	bt459_init_colormap( gx->cursor_registers );

	/* init bg/fg cursor colors */
	for (i = 0; i < 3; i++) {
		up->dev_dep_2.gx.Bg_color[i] = 0x00;
		up->dev_dep_2.gx.Fg_color[i] = 0xff;
	}

	Bg_Fg[0].red = Bg_Fg[0].green = Bg_Fg[0].blue = 0x00;
	Bg_Fg[1].red = Bg_Fg[1].green = Bg_Fg[1].blue = 0xff;
	bt459_cursor_color( gx->cursor_registers, Bg_Fg);
}

/*
 * Initialize cursor
 */
gx_init_cursor_sprite(up)
	user_info_t	*up;
{
	register int    i;
	register unsigned char *bits = up->dev_dep_2.gx.cursor_sprite;

	bzero(bits, 1024);

	/* This is for our 16hX8w font */
	for (i = 0; i < (16 * (64 / 4)); i += (64 / 4)) {
		bits[i] = 0x3f;
		bits[i + 1] = 0xfc;
	}
}

/*
 * Hardware initialization
 */
gx_init_screen(gx)
	gx_softc_t *gx;
{
	bt459_init( gx->cursor_registers, gx->cursor_reset, 5  /* 5:1 MUX */);
	/* ... stic .... */
}


/*
 * When user gets control
 */
gx_graphic_open(gx)
	gx_softc_t *gx;
{
	gx->mapped_to_user = TRUE;
}

/*
 * Do what's (typically) needed when X exits
 */
gx_soft_reset(sc)
	screen_softc_t	sc;
{
	gx_softc_t	*gx = (gx_softc_t*) sc->hw_state;
	user_info_t	*up =  sc->up;

	gx->mapped_to_user = FALSE;

	/*
	 * Restore params in mapped structure
	 */
	gx_init_screen_params(sc,up);
	up->row = up->max_row - 1;

	/*
	 * Restore RAMDAC chip to default state
	 */
	gx_init_screen(gx);

	/*
	 * Load kernel's cursor sprite
	 */
	gx_init_cursor_sprite(up);
	bt459_cursor_sprite(gx->cursor_registers, up->dev_dep_2.gx.cursor_sprite);

	/*
	 * Color map and cursor color
	 */
	gx_init_colormap(sc);
}

/*
 * Routine to paint a char on the 2-3D boards
 */
gx_char_paint(sc, c, row, col)
	screen_softc_t	sc;
{
	register int *pp, *xyp;
	int *stampPacket, xpix, ypix, xy, v1, v2;
	register unsigned char	*font;
	gx_softc_t		*gx = (gx_softc_t*) sc->hw_state;
	boolean_t		rev_video;

	rev_video = sc->standout;

	xpix = col * KfontWidth;
	ypix = TIMES_KfontHeight(row);
	font = &kfont_7x14[ TIMES_KfontHeight( (int)(c - ' ') )];

	if ((stampPacket = (*gx->get_packet)(gx)) == 0)
		return;

	pp = stampPacket;
	*pp++ = CMD_LINES| RGB_FLAT| XY_PERPRIM| LW_PERPKT;
	*pp++ = (2 << 24) | 0xffffff;
	*pp++ = 0x0;
	*pp++ = UPD_ENABLE| WE_XYMASK| UMET_COPY;
	*pp++ = _TEXT_LW_;

	xyp = pp;
	/* fg = character to draw */
	*pp++ = (*(font+1))<<16 | (*font); font += 2;
	*pp++ = (*(font+1))<<16 | (*font); font += 2;
	*pp++ = (*(font+1))<<16 | (*font); font += 2;
	*pp++ = (*(font+1))<<16 | (*font); font += 2;
	*pp++ = (*(font+1))<<16 | (*font); font += 2;
	*pp++ = (*(font+1))<<16 | (*font); font += 2;
	*pp++ = (*(font+1))<<16 | (*font); font += 2;
	*pp++ =                   (*font);
	*pp++ = xy = CONSXYADDR(xpix, ypix);
	*pp++ = v1 =    (xpix          <<19) | ((ypix<<3)+_TEXT_LW_);
	*pp++ = v2 = ((((xpix+8)<<3)-1)<<16) | (v1 & 0xffff);
	*pp++ = (rev_video) ? _TEXT_BG_ : _TEXT_FG_;

	/* opaque background */
	*pp++ = *xyp++ ^ 0xffffffff;
	*pp++ = *xyp++ ^ 0xffffffff;
	*pp++ = *xyp++ ^ 0xffffffff;
	*pp++ = *xyp++ ^ 0xffffffff;
	*pp++ = *xyp++ ^ 0xffffffff;
	*pp++ = *xyp++ ^ 0xffffffff;
	*pp++ = *xyp++ ^ 0xffffffff;
	*pp++ = *xyp   ^ 0xffffffff;
	*pp++ = xy;
	*pp++ = v1;
	*pp++ = v2;
	*pp   = (rev_video) ? _TEXT_FG_ : _TEXT_BG_;

	(*gx->send_packet)(gx, stampPacket);
}

/*
 * Delete the line at the given row.
 */
gx_remove_line(sc, row)
	screen_softc_t	sc;
	short row;
{
	register int   *pp, fY, tY, h;
	int            *stampPacket;
	gx_softc_t     *gx = (gx_softc_t *) sc->hw_state;
	register int	text_rows, text_cols;

	text_rows = sc->up->max_row - 1;
	text_cols = sc->up->max_col;

	tY = TIMES_KfontHeight( row * _N_PLANES_);	/* to */
	fY = tY + (KfontHeight * _N_PLANES_);		/* from */
	h  = TIMES_KfontHeight(text_rows - row);	/* # scanlines */

	while (h > 0) {
		register int    n = min(h, STAMP_MAX_CMDS);

		h -= n;

		if ((stampPacket = (*gx->get_packet) (gx)) == 0)
			break;

		pp = stampPacket;
		*pp++ = CMD_COPYSPANS | LW_PERPKT;
		*pp++ = (n << 24) | 0xffffff;
		*pp++ = 0x0;
		*pp++ = UPD_ENABLE | UMET_COPY | SPAN;
		*pp++ = 1;	/* linewidth */
		for (; n > 0; n--, fY += _N_PLANES_, tY += _N_PLANES_) {
			*pp++ = text_cols * KfontWidth * _N_PLANES_;
			*pp++ = fY;	/* x := 0 */
			*pp++ = tY;
		}
		(*gx->send_packet) (gx, stampPacket);
	}

	/* do it now to pipeline with hdw doing the copies */
	ascii_screen_rem_update(sc, row);

	/* clear out bottom row lines */
	gx_rect(sc, 0, TIMES_KfontHeight(text_rows),
		KfontHeight, text_cols * KfontWidth,
		0x0);
}

/*
 * Open a new blank line at the given row.
 */
gx_insert_line(sc, row)
	screen_softc_t	sc;
	short row;
{
	register int   *pp, fY, tY, h;
	int            *stampPacket;
	gx_softc_t     *gx = (gx_softc_t *) sc->hw_state;
	register int	text_rows, text_cols;

	text_rows = sc->up->max_row - 1;
	text_cols = sc->up->max_col;


	tY = TIMES_KfontHeight(text_rows * _N_PLANES_);	/* from */
	fY = tY - (KfontHeight * _N_PLANES_);		/* to */
	h = TIMES_KfontHeight(text_rows - row);		/* # scanlines */

	while (h > 0) {
		register int    n = min(h, STAMP_MAX_CMDS);

		h -= n;

		if ((stampPacket = (*gx->get_packet) (gx)) == 0)
			break;

		pp = stampPacket;
		*pp++ = CMD_COPYSPANS | LW_PERPKT;
		*pp++ = (n << 24) | 0xffffff;
		*pp++ = 0x0;
		*pp++ = UPD_ENABLE | UMET_COPY | SPAN;
		*pp++ = 1;	/* linewidth */
		for (; n > 0; n--, fY -= _N_PLANES_, tY -= _N_PLANES_) {
			*pp++ = text_cols * KfontWidth * _N_PLANES_;
			*pp++ = fY;	/* x := 0 */
			*pp++ = tY;
		}
		(*gx->send_packet) (gx, stampPacket);
	}

	/* do it now to pipeline with hdw doing the copies */
	ascii_screen_ins_update(sc, row);

	/* clear out the line */
	gx_rect(sc, 0, TIMES_KfontHeight(row),
		KfontHeight, text_cols * KfontWidth,
		0x0);
}

/*
 * Clear the screen
 */
gx_clear_bitmap(sc)
	screen_softc_t	sc;
{
	gx_rect(sc, 0, 0, 1024, 1280, 0x0);

	/* clear ascii screenmap */
	ascii_screen_fill(sc, ' ');
}

/*
 * Draw a rectangle, <x,y> = top-left corner
 */
gx_rect(sc, top_x, top_y, height, width, rgb)
	screen_softc_t	sc;
	int top_x, top_y, height, width, rgb;
{
	register int   *pp, *stampPacket, lw;
	gx_softc_t     *gx = (gx_softc_t *) sc->hw_state;

	if ((stampPacket = (*gx->get_packet) (gx)) == 0)
		return;

	pp = stampPacket;
	*pp++ = CMD_LINES | RGB_CONST | LW_PERPKT;
	*pp++ = (1 << 24) | 0xffffff;
	*pp++ = 0x0;
	*pp++ = UPD_ENABLE | UMET_COPY;

	lw = (height << 2) - 1;
	top_y = (top_y << 3) + lw;

	*pp++ = lw;
	*pp++ = rgb;		/* rgb */
	*pp++ = (top_x << 19) | top_y;
	*pp   = ((((top_x + width) << 3) - 1) << 16) | top_y;

	(*gx->send_packet) (gx, stampPacket);
}


/*
 * Initialize screen parameters, and in the
 * user-mapped descriptor.
 */
gx_init_screen_params(sc, up)
	screen_softc_t	sc;
	user_info_t	*up;
{
	register int	vis_x, vis_y;

	up->frame_scanline_width = sc->frame_scanline_width;
	up->frame_height = sc->frame_height;

	vis_x = sc->frame_visible_width;
	vis_y = sc->frame_visible_height;

	up->max_x		= vis_x;
	up->max_y		= vis_y;
	up->max_cur_x		= vis_x - 1;
	up->max_cur_y		= vis_y - 1;
	up->min_cur_x		= -63;
	up->min_cur_y		= -63;
	up->max_row		= vis_y / KfontHeight;
	up->max_col		= vis_x / KfontWidth;

	up->version		= 11;

	up->mouse_threshold	= 4;	
	up->mouse_scale		= 2;

#if 0
	up->dev_dep_2.gx.tablet_scale_x	= ((vis_x - 1) * 1000) / 2200;
	up->dev_dep_2.gx.tablet_scale_y	= ((vis_y - 1) * 1000) / 2200;
#endif
}

gx_decode_stamp_configuration(gx, up, config)
	gx_softc_t	*gx;
	user_info_t	*up;
	sticCf          config;
{
	gx->stamp_width = config.xconfig ? 5 : 4;
	gx->stamp_height = 1 << config.yconfig;

	up->dev_dep_2.gx.stamp_width  = gx->stamp_width;
	up->dev_dep_2.gx.stamp_height = gx->stamp_height;

	/* primary display buffer */
	up->dev_dep_2.gx.nplanes = (config.option & STIC_CF_PLANES) ? 24 : 8;

	up->dev_dep_2.gx.zzplanes = 0;

	switch (config.option) {
	    case STIC_OPT_2DA:
		up->dev_dep_2.gx.zplanes = 0;
		break;
	    default:
		if (up->dev_dep_2.gx.stamp_height >= 2) {
			up->dev_dep_2.gx.zplanes = 24;
			up->dev_dep_2.gx.zzplanes = 24;
		} else {
			up->dev_dep_2.gx.zplanes = 8;
		}
		break;
	}
}

/*
 * Initialize the STAMP hardware
 */
gx_init_stamp(stic, stamp)
	sticRegs	*stic;
	vm_offset_t	stamp;
{
	int             modtype, xconfig, yconfig, config;

	/*
	 * initialize STIC registers (bmk) 
	 */
	stic->sticsr = 0x00000030;	/* sticcsr */
	wbflush();
	delay(8000);			/* > 4mS */
	stic->sticsr = 0x00000000;	/* sticcsr */
	stic->buscsr = 0xffffffff;	/* buscsr */
	delay(40000);			/* long time... */

	/*
	 * Initialize Stamp config register 
	 */
	modtype = stic->modcl;
	xconfig = (modtype & 0x800) >> 11;
	yconfig = (modtype & 0x600) >> 9;
	config = (yconfig << 1) | xconfig;

	/* stamp0 config */
	*(int *) (stamp + 0x000b0) = config;
	*(int *) (stamp + 0x000b4) = 0x0;

	if (yconfig > 0) {
		/* stamp1 config */
		*(int *) (stamp + 0x100b0) = 0x8 | config;
		*(int *) (stamp + 0x100b4) = 0x0;
		if (yconfig > 1) {
			/* stamp 2 & 3 config */
		}
	}
	/*
	 * Initialize STIC video registers 
	 */
	stic->vblank = (1024 << 16) | 1063;	/* vblank */
	stic->vsync = (1027 << 16) | 1030;	/* vsync */
	stic->hblank = (255 << 16) | 340;	/* hblank */
	stic->hsync2 = 245;	/* hsync2 */
	stic->hsync = (261 << 16) | 293;	/* hsync */

	stic->ipdvint = STIC_INT_CLR;		/* ipdvint */
	stic->sticsr  = STIC_CSR_STARTVT;	/* sticcsr */
	wbflush();
}

/*
 * Position cursor (correction from cfb settings)
 */
gx_pos_cursor(regs, x, y)
{
	bt459_pos_cursor( regs, x + 370 - 219, y + 37 - 34);
}

/*
 * Get status ops
 */
gx_get_status(sc, flavor, status, status_count)
	screen_softc_t	sc;
	int		flavor;
	dev_status_t	status;
	unsigned int	*status_count;
{
	gx_softc_t	*gx = (gx_softc_t*) sc->hw_state;

	if (flavor == SCREEN_GET_OFFSETS) {
		unsigned	*offs = (unsigned *) status;

						/* virtual size */
		offs[0] = gx->mapped_memory_size + gx->mapped_io_size;

						/* offset of user_info_t */
		offs[1] = SILLY_OFFSET;	

						/* isa 3d board */
		offs[2] = (gx->ptpt) ? TRUE : FALSE;

		*status_count = 3;

		return D_SUCCESS;
	} else
		return D_INVALID_OPERATION;
}

/*
 * Set status ops
 */
gx_set_status(sc, flavor, status, status_count)
	screen_softc_t	sc;
	int		flavor;
	dev_status_t	status;
	unsigned int	status_count;
{
	gx_softc_t		*gx = (gx_softc_t*) sc->hw_state;
	user_info_t		*up =  sc->up;

	switch (flavor) {

	case _IO('q', 14): {	/* WCCOLOR */
		color_map_t		c[2];

		c[0].red   = up->dev_dep_2.gx.Bg_color[2];
		c[0].green = up->dev_dep_2.gx.Bg_color[1];
		c[0].blue  = up->dev_dep_2.gx.Bg_color[0];
		c[1].red   = up->dev_dep_2.gx.Fg_color[2];
		c[1].green = up->dev_dep_2.gx.Fg_color[1];
		c[1].blue  = up->dev_dep_2.gx.Fg_color[0];

		sc->flags |= SCREEN_BEING_UPDATED;
		bt459_cursor_color (gx->cursor_registers, c );
		sc->flags &= ~SCREEN_BEING_UPDATED;

		break;
	}

	case _IO('q', 15): {	/* WCURSOR */

		sc->flags |= SCREEN_BEING_UPDATED;
		bt459_cursor_sprite( gx->cursor_registers,
				     sc->up->dev_dep_2.gx.cursor_sprite);
		sc->flags &= ~SCREEN_BEING_UPDATED;

		break;
	}
	     
	case _IO('q', 16): {	/* _SETCMAP */
		unsigned int		from_entry, to_entry;

		from_entry = up->dev_dep_2.gx.cmap_index;
		to_entry = from_entry + up->dev_dep_2.gx.cmap_count;

		if (to_entry > 256)
			return D_INVALID_RECNUM;

		for ( ; from_entry < to_entry; from_entry++) {
			color_map_t	m;
			unsigned int	c;

			c       = up->dev_dep_2.gx.colormap[from_entry];
			m.red   = (c >> 16) & 0xff;
			m.green = (c >>  8) & 0xff;
			m.blue  = (c >>  0) & 0xff;

			sc->flags |= SCREEN_BEING_UPDATED;
			bt459_load_colormap_entry( gx->cursor_registers,
						   from_entry, &m);
			sc->flags &= ~SCREEN_BEING_UPDATED;
		}

		break;
	}

	default:
		return D_INVALID_OPERATION;
	}
	return D_SUCCESS;
}


/*
 * Map pages to user space
 */
vm_offset_t
gx_map_page(
	screen_softc_t	sc,
	vm_offset_t	off,
	int		prot)
{
	vm_offset_t	addr;
	gx_softc_t	*gx = (gx_softc_t*) sc->hw_state;

	if (off < gx->mapped_memory_size) {

		addr = pmap_extract(kernel_pmap, (vm_offset_t) sc->up)
			 - SILLY_OFFSET;

	} else if (off < gx->mapped_memory_size + gx->mapped_io_size) {

		addr = pmap_extract(kernel_pmap, (vm_offset_t) gx->board_base);
		off -= gx->mapped_memory_size;

	} else
		return D_INVALID_SIZE;

	return addr + off;
}


#endif	/* NGX > 0 */
