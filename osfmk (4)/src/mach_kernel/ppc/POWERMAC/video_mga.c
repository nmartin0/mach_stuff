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
 * Driver for the ATI video for PCI class of machines.
 */
/* Modified for Millenium II for PowerMacintosh by R. Oikawa, 1998. */
/* Modified to support accelerated Xpmac.                           */

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

#define FRAME_BUFFER_ADDRESS 0
#define ILOAD_WINDOW_ADDRESS 1
#define ROM_BASE_ADDRESS     2
#define MGA_CONTROL_ADDRESS  3
#define RAMDAC_OFFSET		0x3c00
#define TVP3026_INDEX		0x00
#define TVP3026_WADR_PAL	0x00
#define TVP3026_COL_PAL		0x01
#define TVP3026_PIX_RD_MSK	0x02
#define TVP3026_RADR_PAL	0x03
#define TVP3026_CUR_COL_ADDR	0x04
#define TVP3026_CUR_COL_DATA	0x05
#define TVP3026_DATA		0x0a
#define TVP3026_CUR_RAM		0x0b
#define TVP3026_CUR_XLOW	0x0c
#define TVP3026_CUR_XHI		0x0d
#define TVP3026_CUR_YLOW	0x0e
#define TVP3026_CUR_YHI		0x0f

int		mga_probe(caddr_t port);
io_return_t	mga_init(struct vc_info *);
io_return_t	mga_setcolor(int color, struct vc_color *);
io_return_t	mga_getmode(struct vc_info *);
io_return_t	mga_setmode(struct vc_info *);

extern int      vc_no_base_check;

volatile unsigned char *MGAMMIOBasePhys;
volatile unsigned char *MGAMMIOBase;
extern Boot_Video       boot_video_info;

struct video_board mga_video = {
	mga_init,
	mga_setcolor,
	mga_setmode,
	mga_getmode,
	NULL,
	mga_probe
};

struct vc_info	mga_info;
static device_node_t	*mga_node;

decl_simple_lock_data(,mga_lock)

/*
 * Initialize the screen.. lookup the monitor type and
 * figure out what mode it is in.
 */


static device_node_t *mga_node;


static char	*mga_names[] = {
	"MTRX,Mistral",
	NULL
};


int
mga_probe(caddr_t addr)
{
	int	i;

	mga_node = find_type("display");
	
	while (mga_node) {
		if (strncmp("MTRX,", mga_node->name, 5) == 0)
			/* see if active video device */
			if (((unsigned long)addr & 0xFF000000) ==
			    ((unsigned long)mga_node->addrs[0].address &
			      0xFF000000) || vc_no_base_check)
				goto found;
		mga_node = mga_node->next;
	}

	for (i = 0; mga_names[i]; i++)
		if (mga_node = find_devices(mga_names[i])) {
			break;
		}

	if (mga_node == NULL)
		return 0;

found:

	if((boot_video_info.v_baseAddr & 0xfff00000) \
	   != (mga_node->addrs[FRAME_BUFFER_ADDRESS].address & 0xfff00000))
	  return 0;

	MGAMMIOBase = MGAMMIOBasePhys = (volatile unsigned char *) \
	  (mga_node->addrs[MGA_CONTROL_ADDRESS].address);

	return 1;
}


io_return_t
mga_init(struct vc_info * info) 
{



	if (kernel_map) {
		MGAMMIOBase = (volatile unsigned char *)
			io_map((unsigned int)MGAMMIOBasePhys, 0x4000);
		simple_lock_init(&mga_lock, ETAP_IO_TTY);
	}
	strcpy(mga_info.v_name, "MTRX,Mistral");
	mga_info.v_width = boot_video_info.v_width;
	mga_info.v_height = boot_video_info.v_height;
	mga_info.v_depth = boot_video_info.v_depth;
	mga_info.v_rowbytes = boot_video_info.v_rowBytes;
	mga_info.v_physaddr = boot_video_info.v_baseAddr;
	mga_info.v_baseaddr = mga_info.v_physaddr;
	mga_info.v_type = VC_TYPE_PCI;
	mga_info.v_reserved[0] = (unsigned long)MGAMMIOBasePhys;
	mga_info.v_reserved[1] = (unsigned long)0x4000;
	memcpy(info, &mga_info, sizeof(mga_info));

	return	D_SUCCESS;
}

/*
 * Set the colors...
 */

io_return_t
mga_setcolor(int count, struct vc_color *colors)
{
	int	i;

	if(mga_info.v_depth == 15 || mga_info.v_depth == 16)
	  return	D_SUCCESS;

	simple_lock(&mga_lock);

	for (i = 0; i < count ;i++, colors++) {
		*(MGAMMIOBase + RAMDAC_OFFSET + TVP3026_WADR_PAL)  = i;
		eieio();
		*(MGAMMIOBase + RAMDAC_OFFSET + TVP3026_COL_PAL) = colors->vp_red;
		eieio();
		*(MGAMMIOBase + RAMDAC_OFFSET + TVP3026_COL_PAL) = colors->vp_green;
		eieio();
		*(MGAMMIOBase + RAMDAC_OFFSET + TVP3026_COL_PAL) = colors->vp_blue;
		eieio();
	}

	simple_unlock(&mga_lock);

	return	D_SUCCESS;
}

/*
 * Set the video mode based on the screen dimensions
 * provided. 
 */

io_return_t
mga_setmode(struct vc_info * info)
{
	return	D_INVALID_OPERATION;
}

/*
 * Get the current video mode..
 */

io_return_t
mga_getmode(struct vc_info *info)
{
	memcpy(info, &mga_info, sizeof(*info));

	return	D_SUCCESS;
}
