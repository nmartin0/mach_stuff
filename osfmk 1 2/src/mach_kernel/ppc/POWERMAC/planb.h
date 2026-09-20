/*
	This is a port of video-in driver, planb, originally written for
	Linux/PPC by Michel Lanners <mlan@cpu.lu>.

	Copyright (C) 1999 Takashi Oe <toe@unlinfo.unl.edu>
*/
/*
    Below is the original copyright notice appeared in planb-0.06.
*/
/* 
    planb - PlanB frame grabber driver

    PlanB is used in the 7x00/8x00 series of PowerMacintosh
    Computers as video input DMA controller.

    Copyright (C) 1997 Michel Lanners (mlan@cpu.lu)

    Based largely on the bttv driver by Ralph Metzler (rjkm@thp.uni-koeln.de)

    Additional debugging and coding by Takashi Oe (toe@unlinfo.unl.edu)


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _PLANB_H_
#define _PLANB_H_

#include <device/device_types.h>
#include <ppc/POWERMAC/dbdma.h>
#include <ppc/POWERMAC/videodev.h>
#include <ppc/POWERMAC/planb_io.h>

#define PLANB_DEVICE_NAME	"Apple PlanB Video-In"

/* This should be safe for both PAL and NTSC */
#define PLANB_MAXPIXELS 768
#define PLANB_MAXLINES 576
#define PLANB_NTSC_MAXLINES 480

/* Uncomment your preferred norm ;-) */
#define PLANB_DEF_NORM VIDEO_MODE_PAL
//#define PLANB_DEF_NORM VIDEO_MODE_NTSC
//#define PLANB_DEF_NORM VIDEO_MODE_SECAM

/* fields settings */
#define PLANB_GRAY	0x1	/*  8-bit mono? */
#define PLANB_COLOUR15	0x2	/* 16-bit mode */
#define PLANB_COLOUR32	0x4	/* 32-bit mode */
#define PLANB_CLIPMASK	0x8	/* hardware clipmasking */

/* misc. flags for PlanB DMA operation */
#define	CH_SYNC		0x1	/* synchronize channels (set by ch1;
				   cleared by ch2) */
#define FIELD_SYNC	0x2     /* used for the start of each field
				   (0 -> 1 -> 0 for ch1; 0 -> 1 for ch2) */
#define EVEN_FIELD	0x0	/* even field is detected if unset */
#define DMA_ABORT	0x2	/* error or just out of sync if set */
#define ODD_FIELD	0x4	/* odd field is detected if set */

/* Potentially useful macros */
#define PLANB_SET(x)	((x) << 16 | (x))
#define PLANB_CLR(x)	((x) << 16)

#define RUN	DBDMA_CNTRL_RUN
#define WAKE	DBDMA_CNTRL_WAKE
#define ACTIVE	DBDMA_CNTRL_ACTIVE
#define FLUSH	DBDMA_CNTRL_FLUSH
#define PAUSE	DBDMA_CNTRL_PAUSE

#define DBDMA_CMD(a)	((a) << 12)
#define DBDMA_NOP	DBDMA_CMD(DBDMA_CMD_NOP)
#define DBDMA_STOP	DBDMA_CMD(DBDMA_CMD_STOP)
#define STORE_WORD	DBDMA_CMD(DBDMA_CMD_STORE_QUAD)
#define INPUT_MORE	DBDMA_CMD(DBDMA_CMD_IN_MORE)
#define OUTPUT_MORE	DBDMA_CMD(DBDMA_CMD_OUT_MORE)

#define DBDMA_KEY(a)	((a) << 8)
#define KEY_SYSTEM	DBDMA_KEY(DBDMA_KEY_SYSTEM)
#define KEY_STREAM0	DBDMA_KEY(DBDMA_KEY_STREAM0)

#define DBDMA_INT(a)	((a) << 4)

#define DBDMA_BRANCH(a)	((a) << 2)
#define BR_ALWAYS	DBDMA_BRANCH(DBDMA_BRANCH_ALWAYS)
#define BR_IFSET	DBDMA_BRANCH(DBDMA_BRANCH_IF_TRUE)
#define BR_IFCLR	DBDMA_BRANCH(DBDMA_BRANCH_IF_FALSE)

#define WAIT_IFCLR	DBDMA_WAIT_IF_FALSE
#define WAIT_IFSET	DBDMA_WAIT_IF_TRUE

/* This represents the physical register layout */
struct planb_regmap {
	dbdma_regmap_t	ch1;			/* 0x00: video in */
unsigned int pad01[3];	/* could somebody fix dbdma_regmap_t, please? */
	volatile unsigned int		even;	/* 0x40: even field setting */
	volatile unsigned int		odd;	/* 0x44; odd field setting */
	unsigned int			pad1[14];	/* empty? */
	dbdma_regmap_t	ch2;			/* 0x80: clipmask out */
unsigned int pad02[3];	/* could somebody fix dbdma_regmap_t, please? */
	unsigned int			pad2[16];	/* 0xc0: empty? */
	volatile unsigned int		reg3;		/* 0x100: ???? */
	volatile unsigned int		intr_stat;	/* 0x104: irq status */
#define PLANB_CLR_IRQ		0x00		/* clear Plan B interrupt */
#define PLANB_GEN_IRQ		0x01		/* assert Plan B interrupt */
#define PLANB_FRM_IRQ		0x02		/* end of frame */
#define PLANB_IRQ_CMD_MASK	0x00000003U	/* reserve 2 lsbs for command */
	unsigned int			pad3[1];	/* empty? */
	volatile unsigned int		reg5;		/* 0x10c: ??? */
	unsigned int			pad4[60];	/* empty? */
	volatile unsigned char		saa_addr;	/* 0x200: SAA subadr */
	char				pad5[3];
	volatile unsigned char		saa_regval;	/* SAA7196 write reg. val */
	char				pad6[3];
	volatile unsigned char		saa_status;	/* SAA7196 status byte */
	/* There is more unused stuff here */
};
typedef struct planb_regmap planb_regmap_t;

struct planb_window {
	int		x, y;
	unsigned short	width, height;
	unsigned short	bpp, bpl, depth, pad;
	unsigned short	swidth, sheight;
	int		norm;
	int		interlace;
	unsigned int	color_fmt;
	int		chromakey;
	int		mode;	/* used to switch between TV/VTR modes */
};
typedef struct planb_window planb_window_t;

struct planb_suspend {
	int overlay;
	int frame;
	dbdma_command_t cmd;
};
typedef struct planb_suspend planb_suspend_t;

struct planb_misc {
	int channel;
	unsigned char addr;
};

struct planb
{
	decl_simple_lock_data(,lock)

	int dev;
	volatile planb_regmap_t *planb_base;	/* virt base of planb */
	planb_regmap_t *planb_base_phys;	/* phys base of planb */
	void	*priv_space;			/* Org. alloc. mem for kfree */
	vm_size_t priv_size;
	unsigned int tab_size;
	int     maxlines;
	unsigned char	irq;			/* interrupt number */
	volatile unsigned int intr_mask;

	struct video_picture picture;
	struct planb_misc misc;
	struct planb_info info;

	int	overlay;			/* overlay running? */
	planb_window_t win;
	unsigned long frame_buffer_phys;	/* We need phys for DMA */
	volatile dbdma_command_t *ch1_cmd;	/* Video In DMA cmd buffer */
	volatile dbdma_command_t *ch2_cmd;	/* Clip Out DMA cmd buffer */
	volatile dbdma_command_t *overlay_last1;
	volatile dbdma_command_t *overlay_last2;
	vm_offset_t ch1_cmd_phys;
	volatile unsigned char *mask;		/* Clipmask buffer */
	int suspend;
	planb_suspend_t suspended;
	int	cmd_buff_inited;		/* cmd buffer inited? */
};
typedef struct planb planb_t;

/*
 * For configuration
 */

io_return_t	planbopen(dev_t dev, dev_mode_t flag, io_req_t ior);
void		planbclose(dev_t dev);
io_return_t	planbread(dev_t dev, io_req_t ior);
io_return_t	planbwrite(dev_t dev, io_req_t ior);
boolean_t	planbportdeath(dev_t dev, ipc_port_t port);
io_return_t	planbgetstatus(dev_t dev, dev_flavor_t flavor,
		dev_status_t data, mach_msg_type_number_t *status_count);
io_return_t     planbsetstatus(dev_t dev, dev_flavor_t flavor,
		dev_status_t data, mach_msg_type_number_t status_count);

#endif /* _PLANB_H_ */
