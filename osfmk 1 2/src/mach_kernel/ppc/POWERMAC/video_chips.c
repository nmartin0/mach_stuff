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

/*
 * Driver for the CHIPS 65550 video for PowerBooks
 * (and possibly other Macs)
 * 
 */

#include <vc.h>
#include <platforms.h>

#include <mach_kdb.h>
#include <kern/spl.h>
#include <machine/machparam.h>          /* spl definitions */
#include <types.h>
#include <device/io_req.h>
#include <device/tty.h>
#include <device/conf.h>
#include <chips/busses.h>
#include <vm/vm_kern.h>
#include <ppc/misc_protos.h>
#include <ppc/io_map_entries.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/video_console.h>
#include <ppc/POWERMAC/video_board.h>
#include <ppc/POWERMAC/device_tree.h>
#include <ppc/POWERMAC/pci.h>

int		chips_probe(caddr_t port);
io_return_t	chips_init(struct vc_info *);
io_return_t	chips_setcolor(int color, struct vc_color *);
io_return_t	chips_getmode(struct vc_info *);
io_return_t	chips_setmode(struct vc_info *);

extern int      vc_no_base_check;

static volatile unsigned char *regBasePhys;
static volatile unsigned char *regBase;

#define	CHIPS_REG_OFFSET	0x400000

#define DAC_W_INDEX		(0x3c8 * 2)
#define DAC_DATA		((0x3c9 * 2) + 1)

struct video_board chips_video = {
	chips_init,
	chips_setcolor,
	chips_setmode,
	chips_getmode,
	NULL,
	chips_probe
};

struct vc_info	chips_info;
static device_node_t	*chips_node;

decl_simple_lock_data(,chips_lock)

/*
 * Initialize the screen.. lookup the monitor type and
 * figure out what mode it is in.
 */

static device_node_t *chips_node;

static char	*chips_names[] = {
	"chips65550",
	NULL
};

int
chips_probe(caddr_t addr)
{
	int	i;

	for (i = 0; chips_names[i]; i++)
		if (chips_node = find_devices(chips_names[i])) {
			regBase = regBasePhys = (volatile unsigned char *)
				chips_node->addrs[0].address + CHIPS_REG_OFFSET;

			/* see if active video device */
			if ((((unsigned long)regBase & 0xff000000) == (unsigned long)addr & 0xff000000) || vc_no_base_check)
				return 1;
		}

	return 0;
}


io_return_t
chips_init(struct vc_info * info) 
{
	extern Boot_Video       boot_video_info;
	unsigned char		val;

	if (kernel_map) {
		regBase = (volatile unsigned char *)
			io_map((unsigned int)regBasePhys, 4096);
		simple_lock_init(&chips_lock, ETAP_IO_TTY);
	}

	strcpy(chips_info.v_name, chips_node->name);
	chips_info.v_width = boot_video_info.v_width;
	chips_info.v_height = boot_video_info.v_height;
	chips_info.v_depth = boot_video_info.v_depth;
	chips_info.v_rowbytes = boot_video_info.v_rowBytes;
	chips_info.v_physaddr = boot_video_info.v_baseAddr;
	chips_info.v_baseaddr = chips_info.v_physaddr;
	chips_info.v_type = VC_TYPE_PCI;

	memcpy(info, &chips_info, sizeof(chips_info));

	return	D_SUCCESS;
}

/*
 * Set the colors...
 */

io_return_t
chips_setcolor(int count, struct vc_color *colors)
{
	int	i;

	simple_lock(&chips_lock);

	for (i = 0; i < count ;i++, colors++) {
		*(regBase + DAC_W_INDEX)  = i;
		delay(1);
		eieio();
		*(regBase + DAC_DATA) = colors->vp_red;
		eieio();
		*(regBase + DAC_DATA) = colors->vp_green;
		eieio();
		*(regBase + DAC_DATA) = colors->vp_blue;
		eieio();
	}

	simple_unlock(&chips_lock);

	return	D_SUCCESS;
}

/*
 * Set the video mode based on the screen dimensions
 * provided. 
 */

io_return_t
chips_setmode(struct vc_info * info)
{
	return	D_INVALID_OPERATION;
}

/*
 * Get the current video mode..
 */

io_return_t
chips_getmode(struct vc_info *info)
{
	memcpy(info, &chips_info, sizeof(*info));

	return	D_SUCCESS;
}
/*
 * @OSF_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: video_chips.c,v $
 * Revision 1.3  1998/11/12 05:21:12  dgatwood
 *
 * Hopefully a complete fix for the probe code issues.  The initial problem
 * was that probe order was critical if you had multiple video devices, as
 * the first driver that had valid hardware would attach for the purpose of
 * palette changing, etc., resulting in screwy console colors if that was
 * the wrong one.  That was fixed by an patch submitted several weeks ago
 * that checked to see if the hardware was reasonably close in address to
 * the address passed in as being the base address of the console.  While
 * this may fail if there are two devices fairly close in address space,
 * it seems to work in most cases.  The exceptions were Platinum video
 * (which was moved to the end of the list) and ATI on G3 desktop and MT
 * machines.  The new code tries twice.  If it fails the first time, it
 * tries again without caring about the address.  There's one more change
 * I just noticed and will commit in a minute.
 *
 * dgatwood@globegate.utm.edu
 *
 * Revision 1.2  1998/10/19 20:18:51  dgatwood
 *
 * This patches the probe code for various drivers to return false if the
 * card is not the currently active one.  This will eventually be undone,
 * when the general probe code is modified to allow for multiple displays.
 *
 *
 * dgatwood@globegate.utm.edu
 *
 * Revision 1.1.1.1  1998/09/28 15:10:25  root
 * This is the current Mach MK.
 *
 * --David
 * dgatwood@globegate.utm.edu
 *
 * Revision 1.1.2.1  1997/10/29  15:27:30  stephen
 * 	New file
 * 	[1997/10/29  15:23:28  stephen]
 *
 * Revision 1.1.1.2  1997/10/29  15:23:28  stephen
 * 	New file
 *
 */
