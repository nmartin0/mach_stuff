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
#include <mach/vm_param.h>
#include <ppc/misc_protos.h>
#include <ppc/io_map_entries.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/video_console.h>
#include <ppc/POWERMAC/video_board.h>
#include <ppc/POWERMAC/device_tree.h> 

int		ati_probe(caddr_t port);
io_return_t	ati_init(struct vc_info *);
io_return_t	ati_setcolor(int color, struct vc_color *);
io_return_t	ati_getmode(struct vc_info *);
io_return_t	ati_setmode(struct vc_info *);
extern int	vc_no_base_check;

volatile unsigned char *regBasePhys;
volatile unsigned char *regBase;

static unsigned int ati_chiptype;

#define CRTC_OFF_PITCH	0x14
#define DAC_W_INDEX	0xc0
#define DAC_DATA	0xc1
#define CUR_HORZ_POSN	0x6c
#define CONFIG_CHIP_ID	0xE0

#define MACH64_GX_ID		0xD7
#define MACH64_CX_ID		0x57
#define MACH64_CT_ID		0x4354
#define MACH64_ET_ID		0x4554
#define MACH64_VT_ID		0x5654
#define MACH64_VQ_ID		0x4749
#define MACH64_GT_ID		0x4754

struct video_board ati_video = {
	ati_init,
	ati_setcolor,
	ati_setmode,
	ati_getmode,
	NULL,
	ati_probe
};

struct vc_info		ati_info;
static device_node_t	*ati_node;

decl_simple_lock_data(,ati_lock)


/*
 * Initialize the screen.. lookup the monitor type and
 * figure out what mode it is in.
 */

#if 0
static char *ati_names[] = {
	"ATY,mach64",
	"ATY,XCLAIM",
	"ATY,264VT",
	"ATY,mach64ii",
	"ATY,mach64_3D_pcc",
	"ATY,XCLAIM3D",
	"ATY,XCLAIMVR",
	"ATY,RAGEII_M",
	"ATY,XCLAIMVRPro",
	"ATY,mach64_3DU",
	NULL
};
#endif

int
ati_probe(caddr_t addr)
{
  	unsigned int base;
	register unsigned int i,j;
	int retval;

	retval=0;
	base = (unsigned int)addr;
	for (ati_node = find_type("display"); ati_node;
	     ati_node=ati_node->next) {
		if (strncmp("ATY,", ati_node->name, 4) == 0) {
			unsigned long na;
			unsigned int *ap = (unsigned int *)
				get_property(ati_node, "AAPL,address", &na);
			/* check the address is in second 8M of aperture */
			if(base & (8 * 1024 * 1024) == 0)
				continue;
			if (ap != NULL) {
				if (ati_node->n_addrs) {
					if (vc_no_base_check ||
					    (base & 0xFF000000)
					     == (*ap & 0xFF000000)) {
						retval = 1;
						break;
					}
				} else {
					if (!ofw_is_device_compatible(ati_node,
								"ATY,264LTPro"))
						continue;
					for (na /= sizeof(unsigned int); na > 0;
								     --na, ++ap)
						if (vc_no_base_check ||
						    (*ap <= base &&
						     base < *ap + 0x1000000)) {
							retval = 1;
							break;
						}
				}
			}
		}
	}
	if (retval) {
		regBasePhys = (unsigned char *) (base & 0xff800000) - 0x400;
		regBase = regBasePhys;

		/* move HW cursor off screen */
		*(regBasePhys + CUR_HORZ_POSN + 1) = 0xff;

		/* find out chiptype */
		i = (unsigned int) (regBasePhys + CONFIG_CHIP_ID);
		__asm__ volatile ("lwbrx %0,0,%1" :  "=r" (j) : "r" (i));
		ati_chiptype = j & 0x0000FFFF;
	}

	return retval;
}


io_return_t
ati_init(struct vc_info * info) 
{
	extern Boot_Video       boot_video_info;
	unsigned char		val;

	if (kernel_map) {
		regBase = (volatile unsigned char *)
			io_map((unsigned int)regBasePhys, 0x1000);
		simple_lock_init(&ati_lock, ETAP_IO_TTY);
	}		

	strcpy(ati_info.v_name, "ATY Mach64");
	// was strcpy(ati_info.v_name, ati_node->name);

	ati_info.v_width = boot_video_info.v_width;
	ati_info.v_height = boot_video_info.v_height;
	ati_info.v_depth = boot_video_info.v_depth;
	ati_info.v_rowbytes = boot_video_info.v_rowBytes;
	ati_info.v_physaddr = boot_video_info.v_baseAddr;
	ati_info.v_baseaddr = ati_info.v_physaddr;
	ati_info.v_reserved[0] = (unsigned long)trunc_page(regBasePhys);
	ati_info.v_reserved[1] = (unsigned long)0x1000;
	ati_info.v_type = VC_TYPE_PCI;

	memcpy(info, &ati_info, sizeof(ati_info));

	return	D_SUCCESS;
}

/*
 * Set the colors...
 */

io_return_t
ati_setcolor(int count, struct vc_color *colors)
{
	int	i, scale;

	simple_lock(&ati_lock);
	scale = (ati_chiptype != MACH64_GX_ID)
		? ((ati_info.v_depth == 16) ? 3 : 0) : 0;

	for (i = 0; i < count ;i++, colors++) {
		*(regBase + DAC_W_INDEX)  = i << scale;;
		eieio();
		*(regBase + DAC_DATA) = colors->vp_red;
		eieio();
		*(regBase + DAC_DATA) = colors->vp_green;
		eieio();
		*(regBase + DAC_DATA) = colors->vp_blue;
		eieio();
	}
	simple_unlock(&ati_lock);

	return	D_SUCCESS;
}

/*
 * Set the video mode based on the screen dimensions
 * provided. 
 */

io_return_t
ati_setmode(struct vc_info * info)
{
	return	D_INVALID_OPERATION;
}

/*
 * Get the current video mode..
 */

io_return_t
ati_getmode(struct vc_info *info)
{
	memcpy(info, &ati_info, sizeof(*info));

	return	D_SUCCESS;
}
