/*
	This is a port of video-in driver, planb, originally written for
	Linux/PPC by Michel Lanners <mlan@cpu.lu>.

	Copyright (C) 1999 Takashi Oe <toe@unlserve.unl.edu>

	This code distributed under Berkeley-compatible licensing terms
	by permission of the original author, Michel Lanners.
*/
/*
    Below is the original copyright notice appeared in planb-0.06.
*/
/* 
    planb - PlanB frame grabber driver

    PlanB is used in the 7x00/8x00 series of PowerMacintosh
    Computers as video input DMA controller.

    Copyright (C) 1998 Michel Lanners (mlan@cpu.lu)

    Based largely on the bttv driver by Ralph Metzler (rjkm@thp.uni-koeln.de)

    Additional debugging and coding by Takashi Oe (toe@unlinfo.unl.edu)
	(Some codes are stolen from proposed v4l2 videodev.c
		of Bill Dirks <dirks@rendition.com>)

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

#include <planb.h>
#include <platforms.h>
 
#include <mach_kdb.h>
#include <kern/spl.h>
#include <machine/machparam.h>
#include <types.h>
#include <sys/ioctl.h>
#include <device/io_req.h>
#include <device/tty.h> 
#include <device/conf.h>
#include <chips/busses.h>
#include <vm/vm_kern.h>
#include <ppc/misc_protos.h>
#include <ppc/proc_reg.h>
#include <ppc/io_map_entries.h>
#include <ppc/POWERMAC/io.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/interrupts.h>
#include <ppc/POWERMAC/powermac_gestalt.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/device_tree.h>
#include <ppc/POWERMAC/pcireg.h>
#include <ppc/POWERMAC/pcivar.h>
#include <ppc/POWERMAC/dbdma.h>
#include <ppc/POWERMAC/pram.h>

#include <ppc/POWERMAC/planb_io.h>
#include <ppc/POWERMAC/videodev.h>
#include <ppc/POWERMAC/planb.h>
#include <ppc/POWERMAC/saa7196.h>

#define IDEBUG(x)

void	planbattach(struct bus_device *device);
int	planbprobe(caddr_t addr, void *ui);

static kern_return_t planb_prepare_open(planb_t *);
static void planb_prepare_close(planb_t *);
static void saa_write_reg(unsigned char, unsigned char, planb_t *);
static unsigned char saa_status(int, planb_t *);
static void saa_set(unsigned char, unsigned char, planb_t *);
static void saa_init_regs(planb_t *);
static void add_clip(planb_t *, struct video_clip *);
static void fill_cmd_buff(planb_t *);
static void cmd_buff(planb_t *);
static volatile dbdma_command_t *setup_grab_cmd(int, planb_t *);
static void overlay_start(planb_t *);
static void overlay_stop(planb_t *);
static inline void tab_cmd_dbdma(volatile dbdma_command_t *, unsigned short,
	unsigned int);
static inline void tab_cmd_store(volatile dbdma_command_t *, unsigned int,
	unsigned int);
static inline void tab_cmd_gen(volatile dbdma_command_t *, unsigned short,
	unsigned short, unsigned int, unsigned int);
static volatile dbdma_command_t *cmd_geo_setup(volatile dbdma_command_t *,
					int, int, int, int, int, planb_t *);
static inline void planb_dbdma_stop(dbdma_regmap_t *);
static unsigned int saa_geo_setup(int, int, int, int, planb_t *);
static inline int overlay_is_active(planb_t *);

struct bus_device	*planb_info[NPLANB];
struct bus_driver	planb_driver = { planbprobe, 0, planbattach, 0, NULL,
						"planb", planb_info, 0, 0, 0 };

static planb_t planbs[NPLANB];
static int planb_num = 0;

struct pci_address {
	unsigned int a_hi;
	unsigned int a_mid;
	unsigned int a_lo;
};

struct pci_reg_property {
	struct pci_address addr;
	unsigned int size_hi;
	unsigned int size_lo;
};
typedef struct pci_reg_property pci_reg_property_t;

extern pcicfgregs *pci_devices;

planb_t *
planb_dev_to_pb(int dev)
{
	int          i;

	for (i=0; i < planb_num; i++) {
		if (planbs[i].dev == dev) {
			return &planbs[i];
		}
	}
	return 0;
}

/*****************************/
/* Hardware access functions */
/*****************************/

static void saa_write_reg(unsigned char addr, unsigned char val, planb_t *pb)
{
	pb->planb_base->saa_addr = addr; eieio();
	pb->planb_base->saa_regval = val; eieio();
	return;
}

/* return  status byte 0 or 1: */
static unsigned char saa_status(int byte, planb_t *pb)
{
	saa_regs[pb->win.norm][SAA7196_STDC] =
		(saa_regs[pb->win.norm][SAA7196_STDC] & ~2) | ((byte & 1) << 1);
	saa_write_reg(SAA7196_STDC, saa_regs[pb->win.norm][SAA7196_STDC], pb);
	delay(30000); /* XXX */
	return (unsigned char)inb((unsigned long)&pb->planb_base->saa_status);
}

static void saa_set(unsigned char addr, unsigned char val, planb_t *pb)
{
	if(saa_regs[pb->win.norm][addr] != val) {
		saa_regs[pb->win.norm][addr] = val;
		saa_write_reg(addr, val, pb);
	}
	return;
}

static void saa_init_regs(planb_t *pb)
{
	int i;

	for (i = 0; i < SAA7196_NUMREGS; i++)
		saa_write_reg(i, saa_regs[pb->win.norm][i], pb);
}

static unsigned int saa_geo_setup(int width, int height, int interlace, int bpp,
						planb_t *pb) {

	int ht, norm = pb->win.norm;

	switch(bpp) {
	case 2:
		/* RGB555+a 1x16-bit + 16-bit transparent */
		saa_regs[norm][SAA7196_FMTS] &= ~0x3;
		break;
	case 1:
	case 4:
		/* RGB888 1x24-bit + 8-bit transparent */
		saa_regs[norm][SAA7196_FMTS] &= ~0x1;
		saa_regs[norm][SAA7196_FMTS] |= 0x2;
		break;
	default:
		return D_INVALID_OPERATION;
	}
	ht = (interlace ? height / 2 : height);
	saa_regs[norm][SAA7196_OUTPIX] = (unsigned char) (width & 0x00ff);
	saa_regs[norm][SAA7196_HFILT] = (saa_regs[norm][SAA7196_HFILT] & ~0x3)
						| (width >> 8 & 0x3);
	saa_regs[norm][SAA7196_OUTLINE] = (unsigned char) (ht & 0xff);
	saa_regs[norm][SAA7196_VYP] = (saa_regs[norm][SAA7196_VYP] & ~0x3)
						| (ht >> 8 & 0x3);
	/* feed both fields if interlaced, or else feed only even fields */
	saa_regs[norm][SAA7196_FMTS] = (interlace) ?
					(saa_regs[norm][SAA7196_FMTS] & ~0x60)
					: (saa_regs[norm][SAA7196_FMTS] | 0x60);
	/* transparent mode; extended format enabled */
	saa_regs[norm][SAA7196_DPATH] |= 0x3;

	return 0;
}

/***************************/
/* DBDMA support functions */
/***************************/

static inline int overlay_is_active(planb_t *pb)
{
	unsigned int size = pb->tab_size * sizeof(dbdma_command_t);
	unsigned int caddr = (unsigned)
			inl_le((unsigned long)&pb->planb_base->ch1.d_cmdptrlo);

	return (inl_le((unsigned long)&pb->overlay_last1->d_cmddep)
						== pb->ch1_cmd_phys)
			&& (caddr < (pb->ch1_cmd_phys + size))
			&& (caddr >= (unsigned)pb->ch1_cmd_phys);
}

static inline void planb_dbdma_restart(dbdma_regmap_t *ch) {
	outl_le((unsigned long)&ch->d_control, PLANB_CLR(RUN));
	outl_le((unsigned long)&ch->d_control,
					PLANB_SET(RUN|WAKE) | PLANB_CLR(PAUSE));
}

static inline void planb_dbdma_stop(dbdma_regmap_t *ch) {
	int i = 0;

	outl_le((unsigned long)&ch->d_control,
					PLANB_CLR(RUN) | PLANB_SET(FLUSH));
	while((inl_le((unsigned long)&ch->d_status)
					== (ACTIVE | FLUSH)) && (i < 999)) {
	IDEBUG(printf("planb0: waiting for DMA to stop\n"));
		i++;
	}
}

static inline void tab_cmd_dbdma(volatile dbdma_command_t *ch,
	unsigned short command, unsigned int cmd_dep)
{
	volatile unsigned short *cmd =
				(volatile unsigned short *)&ch->d_cmd_count;

	outw_le((unsigned long)&cmd[1], command);
	outl_le((unsigned long)&ch->d_cmddep, cmd_dep);
}

static inline void tab_cmd_store(volatile dbdma_command_t *ch,
	unsigned int phy_addr, unsigned int cmd_dep)
{
	volatile unsigned short *cmd =
				(volatile unsigned short *)&ch->d_cmd_count;

	outw_le((unsigned long)&cmd[1], (unsigned short)(STORE_WORD
								| KEY_SYSTEM));
	outw_le((unsigned long)&cmd[0], 4);
	outl_le((unsigned long)&ch->d_address, phy_addr);
	outl_le((unsigned long)&ch->d_cmddep, cmd_dep);
}

static inline void tab_cmd_gen(volatile dbdma_command_t *ch,
	unsigned short command, unsigned short req_count,
	unsigned int phy_addr, unsigned int cmd_dep)
{
	volatile unsigned short *cmd =
				(volatile unsigned short *)&ch->d_cmd_count;

	outw_le((unsigned long)&cmd[1], command);
	outw_le((unsigned long)&cmd[0], req_count);
	outl_le((unsigned long)&ch->d_address, phy_addr);
	outl_le((unsigned long)&ch->d_cmddep, cmd_dep);
}

static volatile dbdma_command_t *cmd_geo_setup(volatile dbdma_command_t *c1,
			int width, int height, int interlace, int bpp, int clip,
							planb_t *pb)
{
	int norm = pb->win.norm;

	if((saa_geo_setup(width, height, interlace, bpp, pb)) != 0)
		return (volatile dbdma_command_t *)NULL;
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_addr),
							SAA7196_FMTS);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_regval),
					saa_regs[norm][SAA7196_FMTS]);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_addr),
							SAA7196_DPATH);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_regval),
					saa_regs[norm][SAA7196_DPATH]);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->even),
					bpp | ((clip)? PLANB_CLIPMASK: 0));
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->odd),
					bpp | ((clip)? PLANB_CLIPMASK: 0));
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_addr),
							SAA7196_OUTPIX);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_regval),
					saa_regs[norm][SAA7196_OUTPIX]);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_addr),
							SAA7196_HFILT);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_regval),
					saa_regs[norm][SAA7196_HFILT]);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_addr),
							SAA7196_OUTLINE);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_regval),
					saa_regs[norm][SAA7196_OUTLINE]);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_addr),
							SAA7196_VYP);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->saa_regval),
					saa_regs[norm][SAA7196_VYP]);
	return c1;
}

/***************/
/* Driver Core */
/***************/

static kern_return_t planb_prepare_open(planb_t *pb)
{
	int	i;
	vm_size_t size;
	kern_return_t	kr;

	/* allocate memory for two plus alpha command buffers (size: max lines,
	   plus 40 commands handling, plus 1 alignment),
	   plus clipmask buffer */
	pb->priv_size = (pb->tab_size*(2)+1)*sizeof(dbdma_command_t)
		+(PLANB_MAXLINES*((PLANB_MAXPIXELS+7)& ~7))/8;
	if(!(kernel_map))
		return D_NO_MEMORY;
	kr = kmem_alloc_contig(kernel_map, (vm_offset_t *)&pb->priv_space,
				pb->priv_size, 0, 0);
	if (kr != KERN_SUCCESS)
		return kr;
	if(!(pb->priv_space))
		return D_NO_MEMORY;
	memset((void *)pb->priv_space, 0, pb->priv_size);
	pb->overlay_last1 = pb->ch1_cmd = (volatile dbdma_command_t *)
				DBDMA_ALIGN((unsigned int *)pb->priv_space);
	pb->overlay_last2 = pb->ch2_cmd = pb->ch1_cmd + pb->tab_size;
	pb->ch1_cmd_phys = kvtophys((vm_offset_t)pb->ch1_cmd);
	pb->mask = (unsigned char *)(pb->ch2_cmd + pb->tab_size);
	pb->suspend = 0;
	return D_SUCCESS;
}

static void planb_prepare_close(planb_t *pb)
{
	/* make sure the dma's are idle */
	planb_dbdma_stop(&pb->planb_base->ch2);
	planb_dbdma_stop(&pb->planb_base->ch1);
	/* free kernel memory of command buffers */
	if(pb->priv_space != 0) {
		kmem_free(kernel_map, (vm_offset_t)pb->priv_space,
								pb->priv_size);
		pb->priv_space = 0;
		pb->cmd_buff_inited = 0;
	}
}

static void release_planb(void)
{
	int i;
	planb_t *pb;

	for (i=0;i<planb_num; i++) 
	{
		pb=&planbs[i];

		/* stop and flash DMAs unconditionally */
		planb_dbdma_stop(&pb->planb_base->ch2);
		planb_dbdma_stop(&pb->planb_base->ch1);

		/* clear and free interrupts */
		pb->intr_mask = PLANB_CLR_IRQ;
		outl_le((unsigned long)&pb->planb_base->intr_stat,
								PLANB_CLR_IRQ);
//		free_irq(pb->irq, pb);

		/* make sure all allocated memory are freed */
		planb_prepare_close(pb);

		/* how do you free io_map()ed vm? */
//		iounmap ((void *)pb->planb_base);
	}
}

/*****************************/
/* overlay support functions */
/*****************************/

static void overlay_start(planb_t *pb)
{
	volatile unsigned short *cmd =
			(volatile unsigned short *)&pb->ch1_cmd->d_cmd_count;

	planb_dbdma_stop(&pb->planb_base->ch2);
	planb_dbdma_stop(&pb->planb_base->ch1);
	outl_le((unsigned long)&pb->planb_base->ch2.d_cmdptrlo,
					kvtophys((vm_offset_t)pb->ch2_cmd));
	outl_le((unsigned long)&pb->planb_base->ch1.d_cmdptrlo,
					kvtophys((vm_offset_t)pb->ch1_cmd));
	outw_le((unsigned long)&cmd[1], DBDMA_NOP);
	planb_dbdma_restart(&pb->planb_base->ch2);
	planb_dbdma_restart(&pb->planb_base->ch1);

	return;
}

static void overlay_stop(planb_t *pb)
{
	planb_dbdma_stop(&pb->planb_base->ch2);
	planb_dbdma_stop(&pb->planb_base->ch1);

	return;
}

static void suspend_overlay(planb_t *pb)
{
	int fr = -1;
	dbdma_command_t last;

	if(pb->suspend++)
		return;
	if(ACTIVE & inl_le((unsigned long)&pb->planb_base->ch1.d_status)) {
		if(overlay_is_active(pb)) {
			planb_dbdma_stop(&pb->planb_base->ch2);
			planb_dbdma_stop(&pb->planb_base->ch1);
			pb->suspended.overlay = 1;
			pb->suspended.frame = fr;
			memcpy(&pb->suspended.cmd, &last, sizeof(last));
			return;
		}
	}
	pb->suspended.overlay = 0;
	pb->suspended.frame = fr;
	memcpy(&pb->suspended.cmd, &last, sizeof(last));
	return;
}

static void resume_overlay(planb_t *pb)
{
	if(pb->suspend > 1)
		return;
	if(ACTIVE & inl_le((unsigned long)&pb->planb_base->ch1.d_status)) {
		goto finish;
	}
	if(pb->suspended.overlay) {
		volatile unsigned short *cmd1 =
			(volatile unsigned short *)&pb->ch1_cmd->d_cmd_count;
		volatile unsigned short *cmd2 =
			(volatile unsigned short *)&pb->ch2_cmd->d_cmd_count;

		outw_le((unsigned long)&cmd1[1], DBDMA_NOP);
		outw_le((unsigned long)&cmd2[1], DBDMA_NOP);
		/* Set command buffer addresses */
		outl_le((unsigned long)&pb->planb_base->ch1.d_cmdptrlo,
				kvtophys((vm_offset_t)pb->overlay_last1));
		outl_le((unsigned long)&pb->planb_base->ch2.d_cmdptrlo,
				kvtophys((vm_offset_t)pb->overlay_last2));
		/* Start the DMA controller */
		outl_le((unsigned long)&pb->planb_base->ch2.d_control,
				PLANB_CLR(PAUSE) | PLANB_SET(RUN|WAKE));
		outl_le((unsigned long)&pb->planb_base->ch1.d_control,
				PLANB_CLR(PAUSE) | PLANB_SET(RUN|WAKE));
	}

finish:
	pb->suspend--;
}

static void add_clip(planb_t *pb, struct video_clip *clip) 
{
	volatile unsigned char	*base;
	int	xc = clip->x, yc = clip->y;
	int	wc = clip->width, hc = clip->height;
	int	ww = pb->win.width, hw = pb->win.height;
	int	x, y, xtmp1, xtmp2;

	if(xc < 0) {
		wc += xc;
		xc = 0;
	}
	if(yc < 0) {
		hc += yc;
		yc = 0;
	}
	if(xc + wc > ww)
		wc = ww - xc;
	if(wc <= 0) /* Nothing to do */
		return;
	if(yc + hc > hw)
		hc = hw - yc;

	for (y = yc; y < yc+hc; y++) {
		xtmp1=xc>>3;
		xtmp2=(xc+wc)>>3;
		base = pb->mask + y*96;
		if(xc != 0 || wc >= 8)
			*(base + xtmp1) &= (unsigned char)(0x00ff &
				(0xff00 >> (xc&7)));
		for (x = xtmp1 + 1; x < xtmp2; x++) {
			*(base + x) = 0;
		}
		if(xc < (ww & ~0x7))
			*(base + xtmp2) &= (unsigned char)(0x00ff >>
				((xc+wc) & 7));
	}

	return;
}

static void fill_cmd_buff(planb_t *pb)
{
	int restore = 0;
	volatile dbdma_command_t last;

	if(pb->overlay_last1 != pb->ch1_cmd) {
		restore = 1;
		last = *(pb->overlay_last1);
	}
	memset((void *)pb->ch1_cmd, 0, 2 * pb->tab_size
						* sizeof(dbdma_command_t));
	cmd_buff (pb);
	if(restore)
		*(pb->overlay_last1) = last;
	pb->cmd_buff_inited = 1;

	return;
}

static void cmd_buff(planb_t *pb)
{
	int		i, bpp, count, nlines, stepsize, interlace;
	unsigned long	base, jump, addr_com, addr_dep;
	volatile dbdma_command_t *c1 = pb->ch1_cmd;
	volatile dbdma_command_t *c2 = pb->ch2_cmd;

	interlace = pb->win.interlace;
	bpp = pb->win.bpp;
	count = (bpp * ((pb->win.x + pb->win.width > pb->win.swidth) ?
		(pb->win.swidth - pb->win.x) : pb->win.width));
	nlines = ((pb->win.y + pb->win.height > pb->win.sheight) ?
		(pb->win.sheight - pb->win.y) : pb->win.height);

	/* Do video in: */

	/* Preamble commands: */
	addr_com = kvtophys((vm_offset_t)c1);
	addr_dep = kvtophys((vm_offset_t)&c1->d_cmddep);
	tab_cmd_dbdma(c1++, DBDMA_NOP, 0);
	jump = kvtophys((vm_offset_t)(c1+16)); /* 14 by cmd_geo_setup() and 2 for padding */
	if((c1 = cmd_geo_setup(c1, pb->win.width, pb->win.height, interlace,
					bpp, 1, pb)) == NULL) {
		tab_cmd_dbdma(pb->ch1_cmd + 1, DBDMA_STOP, 0);
		tab_cmd_dbdma(pb->ch2_cmd + 1, DBDMA_STOP, 0);
		return;
	}
	tab_cmd_store(c1++, addr_com, (unsigned)(DBDMA_NOP | BR_ALWAYS) << 16);
	tab_cmd_store(c1++, addr_dep, jump);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->ch1.d_wait),
							PLANB_SET(FIELD_SYNC));
		/* (1) wait for field sync to be set */
	tab_cmd_dbdma(c1++, DBDMA_NOP | WAIT_IFCLR, 0);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->ch1.d_branch),
							PLANB_SET(ODD_FIELD));
		/* wait for field sync to be cleared */
	tab_cmd_dbdma(c1++, DBDMA_NOP | WAIT_IFSET, 0);
		/* if not odd field, wait until field sync is set again */
	tab_cmd_dbdma(c1, DBDMA_NOP | BR_IFSET, kvtophys((vm_offset_t)(c1-3)));
	c1++;
		/* assert ch_sync to ch2 */
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->ch2.d_control),
							PLANB_SET(CH_SYNC));
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->ch1.d_branch),
							PLANB_SET(DMA_ABORT));

	base = (pb->frame_buffer_phys + pb->win.y * (pb->win.bpl + pb->win.pad)
							+ pb->win.x * bpp);

	if (interlace) {
		stepsize = 2;
		jump = kvtophys((vm_offset_t)(c1 + (nlines + 1) / 2));
	} else {
		stepsize = 1;
		jump = kvtophys((vm_offset_t)(c1 + nlines));
	}

	/* even field data: */
	for (i=0; i < nlines; i += stepsize, c1++)
		tab_cmd_gen(c1, INPUT_MORE | KEY_STREAM0 | BR_IFSET,
			count, base + i * (pb->win.bpl + pb->win.pad), jump);

	/* For non-interlaced, we use even fields only */
	if (!interlace)
		goto cmd_tab_data_end;

	/* Resync to odd field */
		/* (2) wait for field sync to be set */
	tab_cmd_dbdma(c1++, DBDMA_NOP | WAIT_IFCLR, 0);
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->ch1.d_branch),
							PLANB_SET(ODD_FIELD));
		/* wait for field sync to be cleared */
	tab_cmd_dbdma(c1++, DBDMA_NOP | WAIT_IFSET, 0);
		/* if not odd field, wait until field sync is set again */
	tab_cmd_dbdma(c1, DBDMA_NOP | BR_IFCLR, kvtophys((vm_offset_t)(c1-3)));
	c1++;
		/* assert ch_sync to ch2 */
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->ch2.d_control),
							PLANB_SET(CH_SYNC));
	tab_cmd_store(c1++, (unsigned)(&pb->planb_base_phys->ch1.d_branch),
							PLANB_SET(DMA_ABORT));
	
	/* odd field data: */
	jump = kvtophys((vm_offset_t)(c1 + nlines / 2));
	for (i=1; i < nlines; i += stepsize, c1++)
		tab_cmd_gen(c1, INPUT_MORE | KEY_STREAM0 | BR_IFSET, count,
			base + i * (pb->win.bpl + pb->win.pad), jump);

	/* And jump back to the start */
cmd_tab_data_end:
	pb->overlay_last1 = c1;	/* keep a pointer to the last command */
	tab_cmd_dbdma(c1, DBDMA_NOP | BR_ALWAYS,
					kvtophys((vm_offset_t)pb->ch1_cmd));

	/* Clipmask command buffer */

	/* Preamble commands: */
	tab_cmd_dbdma(c2++, DBDMA_NOP, 0);
	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch2.d_wait),
							PLANB_SET(CH_SYNC));
		/* wait until ch1 asserts ch_sync */
	tab_cmd_dbdma(c2++, DBDMA_NOP | WAIT_IFCLR, 0);
		/* clear ch_sync asserted by ch1 */
	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch2.d_control),
							PLANB_CLR(CH_SYNC));
	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch2.d_wait),
							PLANB_SET(FIELD_SYNC));
	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch2.d_branch),
							PLANB_SET(ODD_FIELD));

	/* jump to end of even field if appropriate */
	/* this points to (interlace)? pos. C: pos. B */
	jump = (interlace) ? kvtophys((vm_offset_t)(c2 + (nlines + 1) / 2 + 2)):
				kvtophys((vm_offset_t)(c2 + nlines + 2));
		/* if odd field, skip over to odd field clipmasking */
	tab_cmd_dbdma(c2++, DBDMA_NOP | BR_IFSET, jump);

	/* even field mask: */
	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch2.d_branch),
							PLANB_SET(DMA_ABORT));
	/* this points to pos. B */
	jump = (interlace) ? kvtophys((vm_offset_t)(c2 + nlines + 1)):
					kvtophys((vm_offset_t)(c2 + nlines));
	base = kvtophys((vm_offset_t)(pb->mask));
	for (i=0; i < nlines; i += stepsize, c2++)
		tab_cmd_gen(c2, OUTPUT_MORE | KEY_STREAM0 | BR_IFSET, 96,
			base + i * 96, jump);

	/* For non-interlaced, we use only even fields */
	if(!interlace)
		goto cmd_tab_mask_end;

	/* odd field mask: */
/* C */	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch2.d_branch),
							PLANB_SET(DMA_ABORT));
	/* this points to pos. B */
	jump = kvtophys((vm_offset_t)(c2 + nlines / 2));
	base = kvtophys((vm_offset_t)(pb->mask));
	for (i=1; i < nlines; i += 2, c2++)     /* abort if set */
		tab_cmd_gen(c2, OUTPUT_MORE | KEY_STREAM0 | BR_IFSET, 96,
			base + i * 96, jump);

	/* Inform channel 1 and jump back to start */
cmd_tab_mask_end:
	/* ok, I just realized this is kind of flawed. */
	/* this part is reached only after odd field clipmasking. */
	/* wanna clean up? */
		/* wait for field sync to be set */
		/* corresponds to fsync (1) of ch1 */
/* B */	tab_cmd_dbdma(c2++, DBDMA_NOP | WAIT_IFCLR, 0);
		/* restart ch1, meant to clear any dead bit or something */
	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch1.d_control),
							PLANB_CLR(RUN));
	tab_cmd_store(c2++, (unsigned)(&pb->planb_base_phys->ch1.d_control),
							PLANB_SET(RUN));
	pb->overlay_last2 = c2;	/* keep a pointer to the last command */
		/* start over even field clipmasking */
	tab_cmd_dbdma(c2, DBDMA_NOP | BR_ALWAYS,
					kvtophys((vm_offset_t)(pb->ch2_cmd)));

	eieio();
	return;
}

/*
 * Mach functions
 */

int
planbprobe(caddr_t addr, void *ui)
{
	planb_t			*pb = &planbs[planb_num];
	pcicfgregs		*node;
	device_node_t		*planb;
	pci_reg_property_t	*pci_addrs;
	unsigned long		l, new_base;
	unsigned char		dev_fn, confreg, bus, slot, func;

	if ((powermac_info.machine != gestaltPowerMac7500)
	     && (powermac_info.machine != gestaltPowerMac8500))
		return 0;

	simple_lock_init(&pb->lock, ETAP_IO_CHIP);
	for (node = pci_devices; node; node = node->next) {
		if (strncmp("planb", node->devnode->name, 5) == 0)
			break;
	}
	if(node == NULL)
		goto err;
	planb = node->devnode;
	if (planb->n_addrs != 1) {
		printf("planb0: expecting 1 address (got %d)", planb->n_addrs);
		goto err;
	}
	if (planb->n_intrs == 0) {
		printf("planb0: no intrs for device %s\n", planb->name);
		goto err;
	} else {
		pb->irq = node->intline;
	}

	/* Initialize PlanB's PCI registers */

	/* There is a bug with the way OF assigns addresses
	   to the devices behind the chaos bridge.
	   control needs only 0x1000 of space, but decodes only
	   the upper 16 bits. It therefore occupies a full 64K.
	   OF assigns the planb controller memory within this space;
	   so we need to change that here in order to access planb. */

	/* We remap to 0xf1000000 in hope that nobody uses it ! */
	new_base = 0xf1000000;

	pci_addrs = (struct pci_reg_property *)
				get_property(planb, "assigned-addresses", &l);
	if(!(pci_addrs))
		goto err;
	bus = (pci_addrs[0].addr.a_hi >> 16) & 0xff;
	dev_fn = (pci_addrs[0].addr.a_hi >> 8) & 0xff;
	confreg = (pci_addrs[0].addr.a_hi) & 0xff;
	slot = (dev_fn >> 3) & 0x1f;
	func = dev_fn & 0x07;
	/* Set the new base address */
	pci_cfgwrite(node, confreg, new_base, 4);

	/* Enable response in memory space, bus mastering,
	   use memory write and invalidate */
	pci_cfgwrite(node, PCIR_COMMAND, node->cmdreg /*| PCIM_CMD_PORTEN */
			| PCIM_CMD_MEMEN | PCIM_CMD_BUSMASTEREN
			/* | PCIM_CMD_INVALIDATEEN */, 2);
	/* Set PCI Cache line size & latency timer */
	pci_cfgwrite(node, PCIR_CACHELNSZ, 0x8, 1);
	pci_cfgwrite(node, PCIR_LATTIMER, 0x40, 1);
	pb->planb_base = (volatile planb_regmap_t *)
			io_map((unsigned int)new_base, round_page(0x400));
	if(pb->planb_base == NULL)
		goto err;
	pb->planb_base_phys = (planb_regmap_t *)new_base;
	pb->dev = planb_num;
	planb_num++;

	/* Reset DMA controllers */
	planb_dbdma_stop(&pb->planb_base->ch2);
	planb_dbdma_stop(&pb->planb_base->ch1);

	/* clear interrupt mask */
	pb->intr_mask = PLANB_CLR_IRQ;

	/* clear interrupts */
	outl_le((unsigned long)&pb->planb_base->intr_stat, PLANB_CLR_IRQ);

	return 1;
err:
	return 0;
}

void
planbattach(struct bus_device *device)
{
	planb_t *pb = &planbs[0];	/* XXX one and only planb */

	memset((void *) &pb->win, 0, sizeof(planb_window_t));
	pb->win.norm = VIDEO_MODE_PAL;
	pb->win.mode = PLANB_TV_MODE;	/* TV mode */
	pb->win.interlace=1;
	pb->win.width=768;
	pb->win.height=576;
	pb->maxlines=576;
	pb->win.bpp=4;
	pb->win.depth=32;
	pb->win.color_fmt=PLANB_COLOUR32;
	pb->win.bpl=1024*pb->win.bpp;
	pb->win.swidth=1024;
	pb->win.sheight=768;
	pb->tab_size = PLANB_MAXLINES + 40;
	pb->suspend = 0;
	pb->ch1_cmd = 0;
	pb->ch2_cmd = 0;
	pb->mask = 0;
	pb->priv_space = 0;
	pb->overlay = 0;
	pb->cmd_buff_inited = 0;
	pb->frame_buffer_phys = 0;

	/* Initialize the SAA registers in memory and on chip */
	saa_init_regs(pb);

	pb->picture.brightness=0x90<<8;
	pb->picture.contrast = 0x70 << 8;
	pb->picture.colour = 0x70<<8;
	pb->picture.hue = 0x8000;
	pb->picture.whiteness = 0;
	pb->picture.depth = pb->win.depth;
}

static struct planb_info planb_template=
{
	PLANB_DEVICE_NAME,	/* name */
	VFL_TYPE_GRABBER,	/* vfl type given to videodev */
	VID_TYPE_OVERLAY,	/* vid type */
	VID_HARDWARE_PLANB	/* registered hardware name */
};

io_return_t
planbopen(dev_t dev, dev_mode_t flag, io_req_t ior)
{
//	int s;
	planb_t *pb = planb_dev_to_pb(dev);

	if(pb == NULL)
		return D_NO_SUCH_DEVICE;

//	s = splbio();
	simple_lock(&pb->lock);
	memcpy(&pb->info,&planb_template,sizeof(planb_template));
	simple_unlock(&pb->lock);
//	splx(s);

	return	D_SUCCESS;
}

void
planbclose(dev_t dev)
{
	release_planb();
	return;
}

io_return_t
planbread(dev_t dev, io_req_t ior)
{
	return	D_INVALID_OPERATION;
}

io_return_t
planbwrite(dev_t dev, io_req_t ior)
{
	return	D_INVALID_OPERATION;
}

boolean_t
planbportdeath(dev_t dev, ipc_port_t port)
{
	return	FALSE;
}


io_return_t
planbgetstatus(dev_t dev, dev_flavor_t flavor, dev_status_t data,
	mach_msg_type_number_t *status_count)
{
	planb_t *pb = planb_dev_to_pb(dev);
	kern_return_t kr;

	if(pb == NULL)
		return D_NO_SUCH_DEVICE;

	switch (flavor) {
		case VIDIOCGCAP:
		{
			struct video_capability b;

			strncpy (b.name, pb->info.name, 32);
			b.type = VID_TYPE_OVERLAY | VID_TYPE_CLIPPING |
				 VID_TYPE_FRAMERAM | VID_TYPE_SCALES |
				 VID_TYPE_CAPTURE;
			b.channels = 2;	/* composite & svhs */
			b.audios = 0;
			b.maxwidth = PLANB_MAXPIXELS;
			b.maxheight = PLANB_MAXLINES;
			b.minwidth = 32; /* wild guess */
			b.minheight = 32;
			memcpy((void *)data, &b, sizeof(b));
			*status_count = sizeof(b)/sizeof(int);
			break;
		}
		case VIDIOCGFBUF:
		{
                        struct video_buffer v;

			v.base = (void *)pb->frame_buffer_phys;
			v.height = pb->win.sheight;
			v.width = pb->win.swidth;
			v.depth = pb->win.depth;
			v.bytesperline = pb->win.bpl + pb->win.pad;
			memcpy((void *)data, &v, sizeof(v));
			*status_count = sizeof(v)/sizeof(int);
			break;
		}
		case VIDIOCGCHAN:
		{
			struct video_channel v;

			v.flags = 0;
			v.tuners = 0;
			v.type = VIDEO_TYPE_CAMERA;
			v.norm = pb->win.norm;
			switch(pb->misc.channel)
			{
			case 0:
				strcpy(v.name,"Composite");
				break;
			case 1:
				strcpy(v.name,"SVHS");
				break;
			default:
				return	D_INVALID_OPERATION;
			}
			v.channel = pb->misc.channel;
			memcpy((void *)data, &v, sizeof(v));
			*status_count = sizeof(v)/sizeof(int);
			break;
		}
		case VIDIOCGPICT:
		{
			struct video_picture vp = pb->picture;

			switch(pb->win.color_fmt) {
			case PLANB_GRAY:
				vp.palette = VIDEO_PALETTE_GREY;
			case PLANB_COLOUR15:
				vp.palette = VIDEO_PALETTE_RGB555;
				break;
			case PLANB_COLOUR32:
				vp.palette = VIDEO_PALETTE_RGB32;
				break;
			default:
				vp.palette = 0;
				break;
			}

			memcpy((void *)data, &vp, sizeof(vp));
			*status_count = sizeof(vp)/sizeof(int);
			break;
		}
		case VIDIOCGWIN:
		{
			struct video_window vw;

			vw.x=pb->win.x;
			vw.y=pb->win.y;
			vw.width=pb->win.width;
			vw.height=pb->win.height;
			vw.chromakey=0;
			vw.flags=0;
			if(pb->win.interlace)
				vw.flags|=VIDEO_WINDOW_INTERLACE;
			memcpy((void *)data, &vw, sizeof(vw));
			*status_count = sizeof(vw)/sizeof(int);
			break;
		}
		case PLANBIOCGSAAREGS:
		{
			struct planb_saa_regs preg;

			if(pb->misc.addr >= SAA7196_NUMREGS)
				return D_INVALID_OPERATION;
			preg.addr = pb->misc.addr;
			preg.val = saa_regs[pb->win.norm][preg.addr];
			memcpy((void *)data, (void *)&preg, sizeof(preg));
			*status_count = sizeof(preg) / sizeof(int);
			break;
		}
		case PLANBIOCGSTAT:
		{
			struct planb_stat_regs pstat;

			pstat.ch1_stat = inl_le((unsigned long)
						&pb->planb_base->ch1.d_status);
			pstat.ch2_stat = inl_le((unsigned long)
						&pb->planb_base->ch2.d_status);
			pstat.saa_stat0 = saa_status(0, pb);
			pstat.saa_stat1 = saa_status(1, pb);
			memcpy((void *)data, (void *)&pstat, sizeof(pstat));
			*status_count = sizeof(pstat) / sizeof(int);
			break;
		}
		case PLANBIOCGMODE: {
			int v=pb->win.mode;

			memcpy((void *)data, (void *)&v, sizeof(v));
			*status_count = 1;
			break;
		}
		case PLANB_IOC_QUERY:
		{
			memcpy((void *)data, (void *)&pb->info,
						sizeof(struct planb_info));
			*status_count = sizeof(struct planb_info)/sizeof(int);
			break;
		}
		case PLANB_IOC_OPEN:
		{
			kr = planb_prepare_open(pb);
			if(kr != D_SUCCESS)
				return kr;
			break;
		}
		case PLANB_IOC_CLOSE:
		{
			planb_prepare_close(pb);
			break;
		}
		default:
		{
			return	D_INVALID_OPERATION;
		}
	}

	return	D_SUCCESS;
}

io_return_t
planbsetstatus(dev_t dev, dev_flavor_t flavor, dev_status_t data,
	mach_msg_type_number_t status_count)
{
	planb_t *pb = planb_dev_to_pb(dev);

	switch (flavor) {
		case VIDIOCSFBUF:
		{
                        struct video_buffer v;
			unsigned short bpp;
			unsigned int fmt;

			if(status_count != sizeof(v)/sizeof(int))
				return	D_INVALID_OPERATION;
			memcpy((void *)&v, (void *)data, sizeof(v));
			switch(v.depth) {
			case 8:
				bpp = 1;
				fmt = PLANB_GRAY;
				break;
			case 15:
			case 16:
				bpp = 2;
				fmt = PLANB_COLOUR15;
				break;
			case 24:
			case 32:
				bpp = 4;
				fmt = PLANB_COLOUR32;
				break;
			default:
				return	D_INVALID_OPERATION;
			}
			if (bpp * v.width > v.bytesperline) {
				return	D_INVALID_OPERATION;
			}
			pb->win.bpp = bpp;
			pb->win.color_fmt = fmt;
			pb->frame_buffer_phys = (unsigned long) v.base;
			pb->win.sheight = v.height;
			pb->win.swidth = v.width;
			pb->picture.depth = pb->win.depth = v.depth;
			pb->win.bpl = pb->win.bpp * pb->win.swidth;
			pb->win.pad = v.bytesperline - pb->win.bpl;

			pb->cmd_buff_inited = 0;
			if(pb->overlay) {
				suspend_overlay(pb);
				fill_cmd_buff(pb);
				resume_overlay(pb);
			}
			break;
		}
		case VIDIOCCAPTURE:
		{
			int i;

			if(status_count != sizeof(i)/sizeof(int))
				return	D_INVALID_OPERATION;
			memcpy((void *)&i, (void *)data, sizeof(i));
			if(i==0) {
				if (!(pb->overlay))
					return 0;
				pb->overlay = 0;
				overlay_stop(pb);
			} else {
				if (pb->frame_buffer_phys == 0 ||
					  pb->win.width == 0 ||
					  pb->win.height == 0)
					return	D_INVALID_OPERATION;
				if (pb->overlay)
					break;
				pb->overlay = 1;
				if(!(pb->cmd_buff_inited))
					fill_cmd_buff(pb);
				overlay_start(pb);
			}
			break;
		}
		case VIDIOCGCHAN:
		{
			struct video_channel v;

			if(status_count != sizeof(v)/sizeof(int))
				return	D_INVALID_OPERATION;
			memcpy((void *)&v, (void *)data, sizeof(v));
			switch(v.channel)
			{
			case 0:
			case 1:
				pb->misc.channel = v.channel;
				break;
			default:
				return	D_INVALID_OPERATION;
			}
			break;
		}
		case VIDIOCSCHAN:
		{
			struct video_channel v;

			if(status_count != sizeof(v)/sizeof(int))
				return	D_INVALID_OPERATION;
			memcpy((void *)&v, (void *)data, sizeof(v));

			if (v.norm != pb->win.norm) {
				int i, maxlines;

				switch (v.norm)
				{
				case VIDEO_MODE_PAL:
				case VIDEO_MODE_SECAM:
					maxlines = PLANB_MAXLINES;
					break;
				case VIDEO_MODE_NTSC:
					maxlines = PLANB_NTSC_MAXLINES;
					break;
				default:
					return	D_INVALID_OPERATION;
				}
				pb->maxlines = maxlines;
				pb->win.norm = v.norm;
				/* Stop overlay if running */
				suspend_overlay(pb);
				/* I know it's an overkill, but.... */
				fill_cmd_buff(pb);
				/* ok, now init it accordingly */
				saa_init_regs (pb);
				/* restart overlay if it was running */
				resume_overlay(pb);
			}

			switch(v.channel)
			{
			case 0:	/* Composite	*/
				saa_set (SAA7196_IOCC,
					((saa_regs[pb->win.norm][SAA7196_IOCC] &
					  ~7) | 3), pb);
				break;
			case 1:	/* SVHS		*/
				saa_set (SAA7196_IOCC,
					((saa_regs[pb->win.norm][SAA7196_IOCC] &
					  ~7) | 4), pb);
				break;
			default:
				return	D_INVALID_OPERATION;
			}

			break;
		}
		case VIDIOCSPICT:
		{
			struct video_picture vp;

			if(status_count != sizeof(vp)/sizeof(int))
				return	D_INVALID_OPERATION;
			memcpy((void *)&vp, (void *)data, sizeof(vp));
			pb->picture = vp;
			/* Should we do sanity checks here? */
			saa_set (SAA7196_BRIG, (unsigned char)
			    ((pb->picture.brightness) >> 8), pb);
			saa_set (SAA7196_HUEC, (unsigned char)
			    ((pb->picture.hue) >> 8) ^ 0x80, pb);
			saa_set (SAA7196_CSAT, (unsigned char)
			    ((pb->picture.colour) >> 9), pb);
			saa_set (SAA7196_CONT, (unsigned char)
			    ((pb->picture.contrast) >> 9), pb);

			break;
		}
		case VIDIOCSWIN:
		{
			struct video_window	vw;
			struct video_clip	clip;
			int 			i;
			unsigned char		*cptr;

			/* ok, let's bite a bullet copying without a check */
			memcpy((void *)&vw, (void *)data, sizeof(vw));
			cptr = (unsigned char *)data + sizeof(vw);
			if(vw.clipcount >= 0) {
				if(status_count != (sizeof(vw)
						+ vw.clipcount * sizeof(clip))
						/ sizeof(int))
					return	D_INVALID_OPERATION;
			}
			/* Stop overlay if running */
			suspend_overlay(pb);
			pb->win.interlace = (vw.height > pb->maxlines/2)? 1: 0;
			if (pb->win.x != vw.x ||
			    pb->win.y != vw.y ||
			    pb->win.width != vw.width ||
			    pb->win.height != vw.height ||
			    !pb->cmd_buff_inited) {
				pb->win.x = vw.x;
				pb->win.y = vw.y;
				pb->win.width = vw.width;
				pb->win.height = vw.height;
				fill_cmd_buff(pb);
			}
			/* Reset clip mask */
			memset ((void *) pb->mask, 0xff, (pb->maxlines
					* ((PLANB_MAXPIXELS + 7) & ~7)) / 8);
			/* Add any clip rects */
			for (i = 0; i < vw.clipcount; i++) {
				memcpy((void *)&clip, (void *)((void*)cptr
					+ i * sizeof(clip)), sizeof(clip));
				add_clip(pb, &clip);
			}
			/* restart overlay if it was running */
			resume_overlay(pb);
			break;
		}
		case PLANBIOCGSAAREGS:
		{
			struct planb_saa_regs preg;

			if(status_count != sizeof(preg)/sizeof(int))
				return D_INVALID_OPERATION;
			memcpy((void *)&preg, (void *)data, sizeof(preg));
			if(preg.addr >= SAA7196_NUMREGS)
				return D_INVALID_OPERATION;
			pb->misc.addr = preg.addr;
			break;
		}
		case PLANBIOCSSAAREGS:
		{
			struct planb_saa_regs preg;

			if(status_count != sizeof(preg)/sizeof(int))
				return D_INVALID_OPERATION;
			memcpy((void *)&preg, (void *)data, sizeof(preg));
			if(preg.addr >= SAA7196_NUMREGS)
				return D_INVALID_OPERATION;
			saa_set (preg.addr, preg.val, pb);
			break;
		}
		case PLANBIOCSMODE:
		{
			int v;

			if(status_count != 1)
				return D_INVALID_OPERATION;
			memcpy((void *)&v, (void *)data, sizeof(v));
			switch(v)
			{
			case PLANB_TV_MODE:
				saa_set (SAA7196_STDC,
					(saa_regs[pb->win.norm][SAA7196_STDC] &
					  0x7f), pb);
				break;
			case PLANB_VTR_MODE:
				saa_set (SAA7196_STDC,
					(saa_regs[pb->win.norm][SAA7196_STDC] |
					  0x80), pb);
				break;
			default:
				return D_INVALID_OPERATION;
				break;
			}
			pb->win.mode = v;
			break;
		}
		default:
		{
			return D_INVALID_OPERATION;
		}
	}

	return D_SUCCESS;
}

