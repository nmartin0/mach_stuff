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
 * Driver for the ATI128 video for PCI class of machines.
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
#include <ppc/POWERMAC/io.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/video_console.h>
#include <ppc/POWERMAC/video_board.h>
#include <ppc/POWERMAC/device_tree.h> 

int		ati128_probe(caddr_t port);
io_return_t	ati128_init(struct vc_info *);
io_return_t	ati128_setcolor(int color, struct vc_color *);
io_return_t	ati128_getmode(struct vc_info *);
io_return_t	ati128_setmode(struct vc_info *);
extern int	vc_no_base_check;

static volatile unsigned char *regBasePhys;
static volatile unsigned char *regBase;

#define PALETTE_INDEX	0x0b0
#define PALETTE_DATA	0x0b4

struct video_board ati128_video = {
	ati128_init,
	ati128_setcolor,
	ati128_setmode,
	ati128_getmode,
	NULL,
	ati128_probe
};

struct vc_info		ati128_info;
static device_node_t	*ati128_node;

decl_simple_lock_data(,ati128_lock)


/*
 * Initialize the screen.. lookup the monitor type and
 * figure out what mode it is in.
 */

int
ati128_probe(caddr_t addr)
{
	register unsigned int i,j;
	int retval;

	retval=0;
	for (ati128_node = find_type("display"); ati128_node; ati128_node=ati128_node->next) {

		if (strncmp("ATY,Rage128", ati128_node->name, 11) == 0) {
			/* see if active video device */
			if (((unsigned long)addr & 0xFF000000) == ((unsigned long)ati128_node->addrs[0].address & 0xFF000000) || vc_no_base_check)
				retval=1; /* an ati128 devices is the active display. */
		}
		else continue;

		if (ati128_node->n_addrs == 3) {
			regBasePhys = (unsigned char *)
					ati128_node->addrs[2].address;
			regBase = regBasePhys;
		} else {
			retval = 0;
			continue;
		}
	}

	return retval;
}


io_return_t
ati128_init(struct vc_info * info) 
{
	extern Boot_Video       boot_video_info;
	unsigned char		val;

	if (kernel_map) {
		regBase = (volatile unsigned char *)
			io_map((unsigned int)regBasePhys, 0x2000);
		simple_lock_init(&ati128_lock, ETAP_IO_TTY);
	}		

	strcpy(ati128_info.v_name, "ATY Rage128");

	ati128_info.v_width = boot_video_info.v_width;
	ati128_info.v_height = boot_video_info.v_height;
	ati128_info.v_depth = boot_video_info.v_depth;
	ati128_info.v_rowbytes = boot_video_info.v_rowBytes;
	ati128_info.v_physaddr = boot_video_info.v_baseAddr;
	ati128_info.v_baseaddr = ati128_info.v_physaddr;
	ati128_info.v_reserved[0] = (unsigned long)regBasePhys;
	ati128_info.v_reserved[1] = (unsigned long)0x2000;
	ati128_info.v_type = VC_TYPE_PCI;

	memcpy(info, &ati128_info, sizeof(ati128_info));

	return	D_SUCCESS;
}

/*
 * Set the colors...
 */

io_return_t
ati128_setcolor(int count, struct vc_color *colors)
{
	unsigned int col;
	int	i;

	simple_lock(&ati128_lock);
	for (i = 0; i < count ;i++, colors++) {
		*(regBase + PALETTE_INDEX)  = i;
		eieio();
		col = colors->vp_red << 16 |
		      colors->vp_green << 8 |
		      colors->vp_blue;
		outl_le((unsigned long)(regBase + PALETTE_DATA), col);
	}
	simple_unlock(&ati128_lock);

	return	D_SUCCESS;
}

/*
 * Set the video mode based on the screen dimensions
 * provided. 
 */

io_return_t
ati128_setmode(struct vc_info * info)
{
	return	D_INVALID_OPERATION;
}

/*
 * Get the current video mode..
 */

io_return_t
ati128_getmode(struct vc_info *info)
{
	memcpy(info, &ati128_info, sizeof(*info));

	return	D_SUCCESS;
}
