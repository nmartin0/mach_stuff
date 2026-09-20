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
/*
 * Added EAsyColor1600 support, IBM RGB624/640 tagged RGB+YUV
 * support in 15/16 bpp and blitter register mapping by R. Oikawa.
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

int		ims_probe(caddr_t port);
io_return_t	ims_init(struct vc_info *);
io_return_t	ims_setcolor(int color, struct vc_color *);
io_return_t	ims_getmode(struct vc_info *);
io_return_t	ims_setmode(struct vc_info *);

extern int      vc_no_base_check;

volatile unsigned char *regBasePhys;
volatile unsigned char *regBase;
extern Boot_Video       boot_video_info;

#define RAMDAC_OFFSET      0x40000
#define DAC_W_INDEX	   0x00
#define DAC_DATA	   0x04
#define DAC_PIXMASK        0x08
#define DAC_PADDR          0x0C
#define DAC_INDEX_LOW      0x10
#define DAC_INDEX_HIGH     0x14
#define DAC_INDEX_DATA     0x18
#define DAC_INDEX_CTRL     0x1C
#define DAC_REVISION_LEVEL 0x00
#define DAC_ID             0x01 /* = 0x30 if exists YUV->RGB function */
#define DAC_PIXEL_FORMAT   0x0a
#define DAC_8BPP_CONTROL   0x0b
#define DAC_16BPP_CONTROL  0x0c
#define DAC_24BPP_CONTROL  0x0d
#define DAC_32BPP_CONTROL  0x0e
#define DAC_K1_YUV         0xa0
#define DAC_K2_YUV         0xa1
#define DAC_K3_YUV         0xa2
#define DAC_K4_YUV         0xa3

#define TAGGED_RGB_YUV_8_7 0xcc
#define YUV_RGB_K1         ((unsigned char)((0.419/0.299)*256.0/2.0))
#define YUV_RGB_K2         ((unsigned char)((0.299/0.419)*256.0/2.0))
#define YUV_RGB_K3         ((unsigned char)((0.114/0.331)*256.0/2.0))
#define YUV_RGB_K4         ((unsigned char)((0.587/0.331)*256.0/2.0))


struct video_board ims_video = {
	ims_init,
	ims_setcolor,
	ims_setmode,
	ims_getmode,
	NULL,
	ims_probe
};

struct vc_info	ims_info;
static device_node_t	*ims_node;

decl_simple_lock_data(,ims_lock)

/*
 * Initialize the screen.. lookup the monitor type and
 * figure out what mode it is in.
 */

static device_node_t *ims_node;

static char	*ims_names[] = {
	"IMS,tt128mb",
	"IMS,tt128mbA",
	"IMS,tt128mb8",
	"IMS,tt128mb4",
	"IMS,tt128mb2",
	"EA,Arrow",
	NULL
};

int
ims_probe(caddr_t addr)
{
	int	i;

	ims_node = find_type("display");
	
	while (ims_node) {
		if (strncmp("IMS,", ims_node->name, 4) == 0)
			/* see if active video device */
			if (((unsigned long)ims_node->addrs[0].address & 0xff000000) ==
			    ((unsigned long)addr & 0xff000000)
			    || vc_no_base_check)
				goto found;

		ims_node = ims_node->next;
	}

	for (i = 0; ims_names[i]; i++)
		if (ims_node = find_devices(ims_names[i])) {
			break;
		}

	if (ims_node == NULL)
		return 0;

found:

	if((boot_video_info.v_baseAddr & 0xfff00000) \
	   != (ims_node->addrs[0].address & 0xfff00000))
	  return 0;

	regBase = regBasePhys = (volatile unsigned char *)
		(ims_node->addrs[0].address + 0x00800000); // was 00840000

	return 1;
}


static void setup_tagged_yuv(void)
{
  if(ims_info.v_depth != 16)
    return;

  simple_lock(&ims_lock);

  *(regBase + RAMDAC_OFFSET + DAC_INDEX_HIGH) = 0x00; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_LOW) = DAC_ID; eieio();
  if(*(regBase + RAMDAC_OFFSET + DAC_INDEX_DATA) != 0x30) {
    simple_unlock(&ims_lock);
    return;
  }
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_LOW) = DAC_16BPP_CONTROL; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_DATA) = TAGGED_RGB_YUV_8_7; eieio();

  *(regBase + RAMDAC_OFFSET + DAC_INDEX_LOW ) = DAC_K1_YUV; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_DATA) = YUV_RGB_K1; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_LOW ) = DAC_K2_YUV; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_DATA) = YUV_RGB_K2; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_LOW ) = DAC_K3_YUV; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_DATA) = YUV_RGB_K3; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_LOW ) = DAC_K4_YUV; eieio();
  *(regBase + RAMDAC_OFFSET + DAC_INDEX_DATA) = YUV_RGB_K4; eieio();
  simple_unlock(&ims_lock);
}


io_return_t
ims_init(struct vc_info * info) 
{
extern Boot_Video	boot_video_info;
unsigned char		val;

	if (kernel_map) {
		regBase = (volatile unsigned char *)
			io_map((unsigned int)regBasePhys, 0x41000); // was 0x400
		simple_lock_init(&ims_lock, ETAP_IO_TTY);
	}

	// strcpy(ims_info.v_name, ims_node->name);
	strcpy(ims_info.v_name, "IMS,tt128mb");
	ims_info.v_width = boot_video_info.v_width;
	ims_info.v_height = boot_video_info.v_height;
	ims_info.v_depth = boot_video_info.v_depth;
	ims_info.v_rowbytes = boot_video_info.v_rowBytes;
	ims_info.v_physaddr = boot_video_info.v_baseAddr;
	ims_info.v_baseaddr = ims_info.v_physaddr;
	ims_info.v_type = VC_TYPE_PCI;

	ims_info.v_reserved[0] = (unsigned long)regBasePhys;
	ims_info.v_reserved[1] = (unsigned long)0x41000;
	setup_tagged_yuv();
	memcpy(info, &ims_info, sizeof(ims_info));

	return	D_SUCCESS;
}

/*
 * Set the colors...
 */

io_return_t
ims_setcolor(int count, struct vc_color *colors)
{
	int	i;

	simple_lock(&ims_lock);

	// RAMDAC_OFFSET added to each line....
	for (i = 0; i < count ;i++, colors++) {
		*(regBase + RAMDAC_OFFSET + DAC_W_INDEX)  = i;
		eieio();
		*(regBase + RAMDAC_OFFSET + DAC_DATA) = colors->vp_red;
		eieio();
		*(regBase + RAMDAC_OFFSET + DAC_DATA) = colors->vp_green;
		eieio();
		*(regBase + RAMDAC_OFFSET + DAC_DATA) = colors->vp_blue;
		eieio();
	}

	simple_unlock(&ims_lock);

	return	D_SUCCESS;
}

/*
 * Set the video mode based on the screen dimensions
 * provided. 
 */

io_return_t
ims_setmode(struct vc_info * info)
{
	return	D_INVALID_OPERATION;
}

/*
 * Get the current video mode..
 */

io_return_t
ims_getmode(struct vc_info *info)
{
	memcpy(info, &ims_info, sizeof(*info));

	return	D_SUCCESS;
}
