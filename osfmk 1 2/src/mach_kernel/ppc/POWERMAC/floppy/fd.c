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

/* CMU_HIST */
/* 
 * Revision 2.6.2.3  92/04/30  11:55:03  bernadat
 *      Initialized sector size in fd_getparms
 *      [92/04/28            bernadat]
 * 
 *      Program DMA registers for 32 bits addresses and 24 bits count
 *      on Compaq Systempro.
 *      [92/04/16            bernadat]
 * 
 * Revision 2.6.2.2  92/03/28  10:06:06  jeffreyh
 *      Adaptations for Corollary:
 *              Use of windows for memory above 16 Megs
 *              Switch to Master CPU for IO regs access
 *      [92/03/04            bernadat]
 *      Changes from TRUNK
 *      [92/03/10  13:21:43  jeffreyh]
 * 
 * Revision 2.8  92/02/23  22:42:57  elf
 *      Added (mandatory) DEV_GET_SIZE flavor of get_status.
 *      [92/02/22            af]
 * 
 * Revision 2.7  92/01/27  16:42:54  rpd
 *      Fixed fdgetstat and fdsetstat to return D_INVALID_OPERATION
 *      for unsupported flavors.
 *      [92/01/26            rpd]
 * 
 * Revision 2.6  91/11/12  11:09:18  rvb
 *      Amazing how hard getting the probe to work for all machines is.
 *      [91/10/16            rvb]
 * 
 * Revision 2.5  91/10/07  17:25:22  af
 *      Still better
 *      [91/10/07  16:29:57  rvb]
 * 
 *      From mg32: Better probe for multiple controllers now possible.
 *      [91/09/23            rvb]
 *      New chips/busses.[ch] nomenclature.
 *      [91/09/09  17:12:23  rvb]
 * 
 *      Added a reset in open to prevent "no such device" errors
 *      Added dlb's fddevinfo.
 *      Reworked to make 2.5/3.0 compatible
 *      [91/09/04  15:46:49  rvb]
 * 
 *      Major rewrite by mg32.
 *      [91/08/07            mg32]
 * 
 * Revision 2.4  91/08/24  11:57:32  af
 *      New MI autoconf.
 *      [91/08/02  02:53:26  af]
 * 
 * Revision 2.3  91/05/14  16:22:47  mrt
 *      Correcting copyright
 * 
 * Revision 2.2  91/02/14  14:42:23  mrt
 *      This file is the logical contatenation of the previous c765.c,
 *      m765knl.c and m765drv.c, in that order.  
 *      [91/01/15            rvb]
 * 
 * Revision 2.5  91/01/08  17:33:32  rpd
 *      Add some 3.0 get/set stat stuff.
 *      [91/01/04  12:21:06  rvb]
 * 
 * Revision 2.4  90/11/26  14:50:54  rvb
 *      jsb bet me to XMK34, sigh ...
 *      [90/11/26            rvb]
 *      Synched 2.5 & 3.0 at I386q (r1.6.1.6) & XMK35 (r2.4)
 *      [90/11/15            rvb]
 * 
 * Revision 1.6.1.6  90/11/27  13:44:55  rvb
 *      Synched 2.5 & 3.0 at I386q (r1.6.1.6) & XMK35 (r2.4)
 *      [90/11/15            rvb]
 * 
 * Revision 2.3  90/08/27  22:01:22  dbg
 *      Remove include of device/param.h (unnecessary).  Flush ushort.
 *      [90/07/17            dbg]
 * 
 * Revision 1.6.1.5  90/08/25  15:44:31  rvb
 *      Use take_<>_irq() vs direct manipulations of ivect and friends.
 *      [90/08/20            rvb]
 * 
 * Revision 1.6.1.4  90/07/27  11:26:53  rvb
 *      Fix Intel Copyright as per B. Davies authorization.
 *      [90/07/27            rvb]
 * 
 * Revision 1.6.1.3  90/07/10  11:45:11  rvb
 *      New style probe/attach.
 *      NOTE: the whole probe/slave/attach is a crock.  Someone
 *      who spends the time to understand the driver should do
 *      it right.
 *      [90/06/15            rvb]
 * 
 * Revision 2.2  90/05/03  15:45:37  dbg
 *      Convert for pure kernel.
 *      Optimized fd_disksort iff dp empty.
 *      [90/04/19            dbg]
 * 
 * Revision 1.6.1.2  90/01/08  13:30:14  rvb
 *      Add Intel copyright.
 *      [90/01/08            rvb]
 * 
 * Revision 1.6.1.1  89/10/22  11:34:51  rvb
 *      Received from Intel October 5, 1989.
 *      [89/10/13            rvb]
 * 
 * Revision 1.6  89/09/25  12:27:05  rvb
 *      vtoc.h -> disk.h
 *      [89/09/23            rvb]
 * 
 * Revision 1.5  89/09/09  15:23:15  rvb
 *      Have fd{read,write} use stragegy now that physio maps correctly.
 *      [89/09/06            rvb]
 * 
 * Revision 1.4  89/03/09  20:07:26  rpd
 *      More cleanup.
 * 
 * Revision 1.3  89/02/26  12:40:28  gm0w
 *      Changes for cleanup.
 *
 */
/* CMU_ENDHIST */

/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989 Carnegie Mellon University
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
 */

/*
   Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

   All Rights Reserved

   Permission to use, copy, modify, and distribute this software and
   its documentation for any purpose and without fee is hereby
   granted, provided that the above copyright notice appears in all
   copies and that both the copyright notice and this permission notice
   appear in supporting documentation, and that the name of Intel
   not be used in advertising or publicity pertaining to distribution
   of the software without specific, written prior permission.

   INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
   INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
   IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
   NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
   WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*      Copyright (c) 1987, 1988 TOSHIBA Corp.          */
/*              All Rights Reserved                     */

#include <fd.h>

#if NFD > 0

#include <platforms.h>
#include <cpus.h>
#if 0
#include <eisa.h>
#include <himem.h>
#endif

#include <types.h>
#include <kern/spl.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include <sys/ioctl.h>
#include "portdef.h"
#include "floppypriv.h"
#include <chips/busses.h>
#include "fdreg.h"
#include "fd_entries.h"
#include <ppc/misc_protos.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/device_tree.h>
#include <machine/disk.h>
#include <ppc/POWERMAC/dbdma.h>

#ifdef	DEBUG
#define D(x) x
#else				/* DEBUG */
#define D(x)
#endif				/* DEBUG */

#include <ppc/POWERMAC/interrupts.h>

/* Forward */

void HALISRHandler(int device, void *ssp);
void HALISR_DMA(int device, void *ssp);
char mem[NFD][20500];		// 40 sectors worth per device

extern int machine_has_gatwick;

extern
 BSIOStatus
 FloppyPluginIO(int unit, BSStorePtr ioStore,
		BSBlockListDescriptorRef blocks,
		MemListDescriptorRef memory,
		BSIORequestBlockPtr parentRequest,
		OptionBits options,
		struct BSErrorList **errors);

extern BSIOStatus FloppyPluginFlush(BSStorePtr ioStore,
				    BSIORequestBlockPtr parentRequest,
				    struct BSErrorList **errors);
extern OSStatus
 FloppyPluginGotoState(BSStorePtr theStore,
		       BSAccessibilityState gotoState);
extern OSStatus
 FloppyPluginEject(BSStorePtr theStore,
		   BSAccessibilityState gotoState);
extern OSStatus
 CheckDriveNumber(int unit, short DriveNumber,
		  DriveStatusType ** DriveStatusPtr);
extern int fdprobe(
		      int ctrl,
		      struct bus_ctlr *bc);
extern int fdslave(
		      struct bus_device *bd,
		      caddr_t xxx);
extern void fdattach(
			struct bus_device *bd);
extern void fdstrategy(
			  struct buf *bp);
extern void chkbusy(
		       struct fdcmd *cmdp);
extern void openchk(
		       struct fdcmd *cmdp);
extern void openfre(
		       struct fdcmd *cmdp);
extern void m765io(
		      struct unit_info *uip);
extern void m765iosub(
			 struct unit_info *uip);
extern void rwcmdset(
			struct unit_info *uip);
extern io_return_t fd_setparms(
				  int unit,
				  int cmdarg);
extern io_return_t fd_getparms(
				  dev_t dev,
				  int *cmdarg);
extern io_return_t fd_format(
				dev_t dev,
				int *cmdarg);
extern int makeidtbl(
			struct fmttbl *tbl,
			struct fddrtab *dr,
			unsigned short track,
			unsigned short intlv);
extern void m765sweep(
			 struct unit_info *uip,
			 struct fddrtab *cdr);
extern void rstout(
		      struct unit_info *uip);
extern void specify(
		       struct unit_info *uip);
extern int fdc_sts(
		      int mode,
		      struct unit_info *uip);

/*
 * Floppy Device-Table Definitions (drtabs)
 *
 *      Cyls,Sec,spc,part,Mtype,RWFpl,FGpl
 */
struct fddrtab m765f[] =
{				/* format table */
    80, 18, 1440, 9, 0x88, 0x2a, 0x50,	/* [0] 3.50" 720  Kb  */
    80, 36, 2880, 18, 0x08, 0x1b, 0x6c,		/* [1] 3.50" 1.44 Meg */
    40, 18, 720, 9, 0xa8, 0x2a, 0x50,	/* [2] 5.25" 360  Kb  */
    80, 30, 2400, 15, 0x08, 0x1b, 0x54	/* [3] 5.25" 1.20 Meg */
#if 0
    80, 10, 800, 15, 0x08, 0x1b, 0x54,	/* [4] 3.50" 400  Kb  */
    80, 20, 1600, 15, 0x08, 0x1b, 0x54	/* [5] 3.50" 800  Kb  */
#endif

};

/*
 * The following are static initialization variables
 * which are based on the configuration.
 */
struct ctrl_info ctrl_info[MAXUNIT >> 1] =
{				/* device data table */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

struct unit_info unit_info[MAXUNIT];	/* unit buffer headers  */

char *fderr = "FD Error on unit";
char *fdmsg[] =
{
    "?",
    "Missing data address mark",
    "Write protected",
    "Sector not found",
    "Data Overrun",		/* Over run error */
    "Uncorrectable data read error",	/* CRC Error */
    "FDC Error",
    "Illegal format type",
    "Drive not ready",
    "diskette not present - please insert",
    "Illegal interrupt type"
};

struct buf fdrbuf[MAXUNIT];	/* data transfer buffer structures */

caddr_t fd_std[NFD] =
{0};
struct bus_device *fd_dinfo[NFD * 2];
struct bus_ctlr *fd_minfo[NFD];
struct bus_driver fddriver =
{(probe_t) fdprobe, fdslave, fdattach, 0, fd_std, "fd", fd_dinfo,
 "fd", fd_minfo, 0};

extern int machine_has_gatwick;

int m765verify[MAXUNIT] =
{1, 1, 1, 1};			/* write after read flag */
						/* 0 != verify mode     */
						/* 0 == not verify mode */
#if 	CBUS || HIMEM
vm_offset_t fd_remap_addr;	/* address at which is remapped memory */
#ifdef	CBUS
int fd_dma_window;		/* DMA window to access above 16 Megs */
#endif				/* CBUS */
#if	HIMEM
hil_t fd_hil;			/* Used for himem convert/revert */
#endif				/* HIMEM */
#endif				/* CBUS || HIMEM */

#define trfrate(uip, type)   outb(VFOREG(uip->addr),(((type)&RATEMASK)>>6))
#define rbskrate(uip, type)  trfrate(uip,(type)&RAPID?RPSEEK:NMSEEK)
#define getparm(type)   ((type<0||type>3)?(struct fddrtab *)ERROR:&m765f[type])		// added 2 formats --dg
#define relative(s1,s2) ((s1)>(s2)?(s1)-(s2):(s2)-(s1))

#if 1
#define outb(a,b)
#define inb(a) 0

#endif

int fdprobe(
	       int port,
	       struct bus_ctlr *ctlr)
{
    int spot = STSREG((int) ctlr->address);
    struct ctrl_info *cip = &ctrl_info[ctlr->unit];
    int i, in;
    static int fdcount = 0;
    device_node_t *swims = NULL;

    /* Only currently supported on PCI class machines */
#if 0
    if (powermac_info.class != POWERMAC_CLASS_PCI)
	return 0;
#endif

#ifdef WINTEL_CODE
    outb(spot, DATAOK);
    for (i = 1000; i--;) {
	in = inb(spot);
	if ((in & DATAOK) == DATAOK && !(in & 0x0f)) {
	    //take_ctlr_irq(ctlr);
	    cip->b_cmd.c_rbmtr = 0;	/* recalibrate/moter flag */
	    cip->b_cmd.c_intr = CMDRST;		/* interrupt flag */
	    cip->b_unitf = 0;
	    cip->b_uip = 0;
	    cip->b_rwerr = cip->b_seekerr = cip->b_rberr = 0;
#if     CBUS
	    if (!is_eisa_bus)
		fd_dma_window = cbus_alloc_win(1);
#else				/* CBUS */
#if	HIMEM
	    himem_reserve(1);
#endif				/* HIMEM */
#endif				/* CBUS */
	    printf("%s%d: port = %x, spl = %d, pic = %d.\n", ctlr->name,
		 ctlr->unit, ctlr->address, ctlr->sysdep, ctlr->sysdep1);
	    return (1);
	}
    }
#endif // WINTEL_CODE

#if 1
    switch(powermac_info.class) {
	case POWERMAC_CLASS_PCI:
	    swims = find_devices("floppy");
	    for (i = 0; (i < fdcount) && (swims); swims = swims->next, i++);
	    if (!swims) {
		swims = find_devices("swim3");
		for (; (i < fdcount) && (swims); swims = swims->next, i++);
	    }
	    if (!swims) {
		return 0;
	    }

	    if (swims->n_intrs) {
		printf("Registering interrupt %d\n", swims->intrs[0]);
		pmac_register_ofint(swims->intrs[0], SPLBIO, HALISRHandler);
		if ((swims->n_intrs > 1) &&
		    (swims->intrs[0] != swims->intrs[1])) {
			printf("Registering DMA interrupt %d\n",
				swims->intrs[1]);
			pmac_register_ofint(swims->intrs[1], SPLBIO,
				HALISR_DMA);
			}
		break;
	    } else {
		printf("swim: no interrupts found!\n");
		return 0;
	    }
	case POWERMAC_CLASS_PDM:
	    if (fdcount) return 0; // never more than one SWIM
	    pmac_register_int(PMAC_DEV_FLOPPY, SPLBIO, HALISRHandler);
	    pmac_register_int(PMAC_DMA_FLOPPY, SPLBIO, HALISR_DMA);
	    break;
	case POWERMAC_CLASS_POWERBOOK:
	case POWERMAC_CLASS_PERFORMA:
	    return 0;
	}

#else

    switch (fdcount) {
    case 0:
	pmac_register_int(PMAC_DEV_FLOPPY, SPLBIO, HALISRHandler);
	switch (powermac_info.class) {
	    case POWERMAC_CLASS_PCI:
		if (find_devices("ohare") || find_devices("gc"))
		    pmac_register_int(PMAC_DMA_FLOPPY, SPLBIO, HALISR_DMA);
		break;
	    case POWERMAC_CLASS_PDM:
	        pmac_register_int(PMAC_DMA_FLOPPY, SPLBIO, HALISR_DMA);
	        break;
	    case POWERMAC_CLASS_POWERBOOK:
	    case POWERMAC_CLASS_PERFORMA:
	        return 0;
	    }
	break;
    case 1:
	if (machine_has_gatwick) {
	    pmac_register_int(PMAC_DEV_FLOPPY2, SPLBIO, HALISRHandler);
//                 pmac_register_int(PMAC_DMA_FLOPPY2, SPLBIO, HALISR_DMA);
	} else {
	    return 0;
	}
	break;
    default:
	printf("Unknown floppy device... could not install interrupt.\n");
	return 0;
    }
#endif

    fdcount++;
    return (1);			/* Naga */
}

int fdslave(
	       struct bus_device *dev,
	       caddr_t xxx)
{
    switch (powermac_info.class) {
	case POWERMAC_CLASS_PCI:
	case POWERMAC_CLASS_PDM:
	    return (1);			/* gross hack */
	default:
	    return (0);
	}
}

void fdattach(
		 struct bus_device *dev)
{
    struct unit_info *uip = &unit_info[dev->unit];
    struct ctrl_info *cip = &ctrl_info[dev->ctlr];

    switch (powermac_info.class) {
    case POWERMAC_CLASS_PDM:
    case POWERMAC_CLASS_PERFORMA:
    case POWERMAC_CLASS_POWERBOOK:
	dev->address = (caddr_t) PDM_FLOPPY_BASE_PHYS;
	// dev->address = (caddr_t) POWERMAC_IO(PDM_FLOPPY_BASE_PHYS);
	break;
    }

    uip->dev = dev;
    /*dev->address = dev->mi->address; *//*naga */
    uip->addr = (long) dev->address;
    uip->b_cmd = &cip->b_cmd;
    uip->b_seekaddr = 0;
    uip->av_forw = 0;
    uip->wakeme = 0;
#ifdef WINTEL_CODE
    if (cip->b_unitf) {
	uip->b_unitf = cip->b_unitf->b_unitf;
	cip->b_unitf->b_unitf = uip;
    } else {
	uip->b_unitf = uip;
	cip->b_unitf = uip;
    }
#endif
    uip->d_drtab.dr_type &= ~OKTYPE;

    printf(", port = %x, spl = %d, pic = %d.",
	   dev->address, dev->sysdep, dev->sysdep1);

    rstout(uip);
    specify(uip);
}

/*****************************************************************************
 *
 * TITLE:	fdopen
 *
 * ABSTRACT:	Open a unit. 
 *
 ****************************************************************************/

OSStatus FloppyPluginInit(int unit, BSStorePtr initStore);
io_return_t
fdopen(
	  dev_t dev,
	  dev_mode_t flag,	/* not used */
	  io_req_t ior)
{				/* not used */
    struct fddrtab *driv;
    struct buf *wbp;
    spl_t x = SPL();
    io_return_t error = D_SUCCESS;
    int unit = UNIT(dev);
    struct unit_info *uip = &unit_info[unit];
    int slave = uip->dev->slave;
    // struct ctrl_info *cip = &ctrl_info[uip->dev->ctlr];
    struct fdcmd *cmdp = uip->b_cmd;
    device_node_t *swims = NULL;
    int i, j;

#if	NCPUS > 1
    io_grab_master();
#endif				/* NCPUS > 1 */

    error = D_SUCCESS;

    if (unit && !machine_has_gatwick)
	return D_NO_SUCH_DEVICE;
    error = FloppyPluginInit(unit, (void *) 0);

#if MACH_DEBUG
    if (error == D_SUCCESS) {
	printf("FLOPPY OPEN\n");
	printf("Dev: %x\n", dev);
	printf("unit:%d\n", unit);
	if (powermac_info.class == POWERMAC_CLASS_PCI) {
	    for (i = 0, swims = find_devices("floppy"); swims; swims = swims->next, i++) {
		printf("device floppy:%d: base: %x %x\n", i, swims->addrs[0].address,
		       swims->addrs[1].address);
		printf("interrupts -- %d: ", swims->n_intrs);
		for (j = 0; j < swims->n_intrs; j++)
		    printf("%x ", swims->intrs[j]);
		printf("\n");
	    }
	    for (i = 0, swims = find_devices("swim3"); swims; swims = swims->next, i++) {
		printf("device swim3:%d: base: %x %x\n", i, swims->addrs[0].address,
		       swims->addrs[1].address);
		printf("interrupts -- %d: ", swims->n_intrs);
		for (j = 0; j < swims->n_intrs; j++)
		    printf("%x ", swims->intrs[j]);
		printf("\n");
	    }
	    printf("Hardcoded addresses: %x %x %x %x\n", (LogicalAddress) POWERMAC_IO(PCI_FLOPPY_BASE_PHYS),
		   (PhysicalAddress) PCI_FLOPPY_BASE_PHYS,
		   (LogicalAddress) DBDMA_REGMAP(DBDMA_FLOPPY),
		   (PhysicalAddress) ((PCI_DMA_BASE_PHYS) + (DBDMA_FLOPPY << 8)));
	}
    }
#endif

    return error;		// D_SUCCESS;

}
/*****************************************************************************
 *
 * TITLE:	fdclose
 *
 * ABSTRACT:	Close a unit.
 *
 *	Called on last close. mark the unit closed and not-ready.
 *
 * 	Unix doesn't actually "open" an inode for rootdev, swapdev or pipedev.
 *	If UNIT(swapdev) != UNIT(rootdev), then must add code in init() to 
 *	"open" swapdev.  These	devices should never be closed.
 *
 *****************************************************************************/

void fdclose(
		dev_t dev)
{				/* major, minor numbers */
    extern dev_t rootdev, swapdev;
    struct unit_info *uip = &unit_info[UNIT(dev)];
    spl_t s;

    FloppyPluginFlush(0, 0, 0);	/*Flush */
    FloppyPluginGotoState(0, kBSOnline);	/* Flush */
#if	NCPUS > 1
    io_grab_master();
#endif				/* NCPUS > 1 */

    /* Clear the bit.
     * If last close of drive insure drtab queue is empty before returning.
     */
    s = SPL();
    while (uip->av_forw != 0) {
	uip->wakeme = 1;
	splx(s);
	sleep((char *) uip, PRIBIO);
	s = SPL();
    }
    splx(s);
#if	NCPUS > 1
    io_release_master();
#endif				/* NCPUS > 1 */
}

/*****************************************************************************
 *
 * TITLE:	fdstrategy
 *
 * ABSTRACT:	Queue an I/O Request, and start it if not busy already.
 *
 *	Reject request if unit is not-ready.
 *
 *	Note:	check for not-ready done here ==> could get requests
 *		queued prior to unit going not-ready.
 *		not-ready status to those requests that are attempted
 *		before a new volume is inserted.  Once a new volume is
 *		inserted, would get good I/O's to wrong volume.
 *
 * CALLS:	iodone()
 *
 * CALLING ROUTINES:	fdread (indirectly, thru physio)
 *			fdwrite (indirectly, thru physio)
 *
 ****************************************************************************/
int busyflag = 0;
void fdstrategy(
		   struct buf *bp)
{				/* buffer header */
    unsigned bytes_left;
    daddr_t secno;
    struct unit_info *uip = &unit_info[UNIT(bp->b_dev)];
    struct fddrtab *dr = &uip->d_drtab;
    struct fddrtab *sdr;
    BSIOStatus retval;
    int unit = UNIT(bp->b_dev);

    uint_t Result;
    while (busyflag == 1) {
	timeout((timeout_fcn_t) wakeup, &busyflag, (10 * HZ + 999) / 1000);	//10 milliseconds

	sleep((char *) &busyflag, PZERO);
    }
    busyflag = 1;
    bp->b_error = 0;
    /* set b_resid to b_bcount because we haven't done anything yet */
    bp->b_resid = bp->b_bcount;
    if (bp->b_flags & B_READ) {
	retval = FloppyPluginIO(unit, &Result, bp->b_bcount, bp->b_un.b_addr, bp->b_blkno * 512, kBSRead, 0);
    } else {
	retval = FloppyPluginIO(unit, &Result, bp->b_bcount, bp->b_un.b_addr, bp->b_blkno * 512, kBSWrite, 0);
    }
    if (Result == bp->b_bcount) {
	bp->b_resid = 0;
    } else {
//         printf("partial write=%d ",Result);
	bp->b_resid = bp->b_bcount - Result;
	bp->b_flags |= B_ERROR;
    }
    biodone(bp);
    busyflag = 0;
    return;
}

/***************************************************************************
 *
 *	check io_busy routine
 *
 ***************************************************************************/

void chkbusy(
		struct fdcmd *cmdp)
{
    while (cmdp->c_devflag & STRCHK) {
	cmdp->c_devflag |= STRWAIT;
	sleep((char *) &cmdp->c_devflag, PZERO);
    }
}

/***************************************************************************
 *
 *	check fdopen() routine
 *
 ***************************************************************************/

void openchk(
		struct fdcmd *cmdp)
{
    while (cmdp->c_devflag & FDMCHK) {
	cmdp->c_devflag |= FDWAIT;
	sleep((char *) &cmdp->c_devflag, PZERO);
    }
}

/***************************************************************************
 *
 *	free fdopen() routine
 *
 ***************************************************************************/

void openfre(
		struct fdcmd *cmdp)
{
    cmdp->c_devflag &= ~FDMCHK;
    if (cmdp->c_devflag & FDWAIT) {
	cmdp->c_devflag &= ~FDWAIT;
	wakeup((char *) &cmdp->c_devflag);
    }
}

/*****************************************************************************
 *
 * TITLE:	m765io
 *
 * ABSTRACT:	Start handling an I/O request.
 *
 ****************************************************************************/

void m765io(
	       struct unit_info *uip)
{
    register struct buf *bp;
    struct ctrl_info *cip = &ctrl_info[uip->dev->ctlr];

    bp = uip->av_forw;		/*move bp to ctrl_info[ctrl].b_buf */
    cip->b_buf = bp;
    cip->b_uip = uip;
    cip->b_xferaddr = bp->b_un.b_addr;
    cip->b_xfercount = bp->b_bcount - bp->b_resid;
    cip->b_sector = bp->b_pfcent;
    uip->b_cmd->c_stsflag |= MTRFLAG;
    printf("m765io:calling timeout/m765iosub ");
//      if(!mtr_start(uip))
    //      timeout((timeout_fcn_t)m765iosub, uip, HZ);
    //      else
    m765iosub(uip);
}

/****************************************************************************
 *
 *	m765io subroutine
 *
 ****************************************************************************/
void m765iosub(
		  struct unit_info *uip)
{
    struct fddrtab *dr = &uip->d_drtab;
    int startsec;
    int slave = uip->dev->slave;
    int unit = uip->dev->unit;
    struct ctrl_info *cip = &ctrl_info[uip->dev->ctlr];
    struct fdcmd *cmdp = uip->b_cmd;
    printf("fd.c:m765iosub: ");
    rwcmdset(uip);
    if (cip->b_buf->b_flags & B_FORMAT)
	goto skipchk;
    startsec = (cmdp->c_rwdata[3] * dr->dr_nsec) + cmdp->c_rwdata[4];
    if (startsec + (cip->b_xfercount >> 9) - 1 > dr->dr_spc)
	cip->b_xferdma = (dr->dr_spc - startsec + 1) << 9;
    else
      skipchk:cip->b_xferdma = cip->b_xfercount;

    /* 
     * Need to remap or use intermediate memory if above 16 Megs
     * if it is not an EISA bus 
     */
    {
	int i;
	uint_t Result;
	for (i = 0; i < sizeof(mem[0]); ++i)
	    mem[unit][i] = 'A' + i % 26;	// @@@

	printf("blkno=%d,len=%d ", cip->b_buf->b_blkno, cip->b_xfercount);
	if (cip->b_buf->b_flags & B_READ) {
	    //FloppyPluginIO(unit, &Result,cip->b_xfercount,cip->b_xferaddr,cip->b_buf->b_blkno*512, kBSWrite,0); 
	    //FloppyPluginIO(unit, &Result,cip->b_xfercount,mem[unit],cip->b_buf->b_blkno*512, kBSRead,0); 
	    FloppyPluginIO(unit, &Result, 40 * 512, mem[unit], 0, kBSWrite, 0);
	    for (i = 0; i < sizeof(mem[0]); ++i)
		mem[unit][i] = 0;
	    FloppyPluginIO(unit, &Result, 40 * 512, mem[unit], 0, kBSRead, 0);
	    for (i = 0; i < sizeof(mem[0]); ++i) {
		if (mem[unit][i] != ('A' + i % 26)) {
		    printf("WRITE/READ mismatch at %d ", i);
		    break;
		}
	    }
	    if (i == sizeof(mem[unit]))
		printf("WRITE/READ works ");
//for(i=0;i<50;++i)printf("0x%x ",mem[unit][i]);
	} else
	    FloppyPluginIO(unit, &Result, cip->b_xfercount, cip->b_xferaddr, cip->b_buf->b_blkno * 512, kBSWrite, 0);
	cip->b_buf->b_resid = cip->b_xfercount - Result;
//for(i=0;i<5;++i)printf("0x%x ",mem[unit][i]);
	biodone(cip->b_buf);
    }
/* naga */
    return;
}
/***************************************************************************
 *
 *	read / write / format / verify command set to command table
 *
 ***************************************************************************/

void rwcmdset(
		 struct unit_info *uip)
{
    short resid;
    int slave = uip->dev->slave;
    struct ctrl_info *cip = &ctrl_info[uip->dev->ctlr];
    struct fdcmd *cmdp = uip->b_cmd;
    printf("rwcmdset: ");
    switch (cip->b_buf->b_flags & (B_FORMAT | B_VERIFY | B_READ | B_WRITE)) {
    case B_VERIFY | B_WRITE:	/* VERIFY after WRITE */
	cmdp->c_rwdata[0] = RDMV;
	break;
    case B_FORMAT:
	cmdp->c_dcount = FMTCNT;
	cmdp->c_rwdata[0] = FMTM;
	cmdp->c_saddr = cip->b_sector / uip->d_drtab.dr_spc;
	resid = cip->b_sector % uip->d_drtab.dr_spc;
	cmdp->c_rwdata[1] = slave | ((resid / uip->d_drtab.dr_nsec) << 2);
	cmdp->c_rwdata[2] =
	    ((struct fmttbl *) cip->b_buf->b_un.b_addr)->s_type;
	cmdp->c_rwdata[3] = uip->d_drtab.dr_nsec;
	cmdp->c_rwdata[4] = uip->d_drtab.dr_fgpl;
	cmdp->c_rwdata[5] = FMTDATA;
	break;
    case B_WRITE:
    case B_READ:
    case B_READ | B_VERIFY:
	printf("rwcmdset:BREAD ");
	cmdp->c_dcount = RWCNT;
	if (cip->b_buf->b_flags & B_READ)
	    if (cip->b_buf->b_flags & B_VERIFY)
		cmdp->c_rwdata[0] = RDMV;
	    else
		cmdp->c_rwdata[0] = RDM;
	else
	    cmdp->c_rwdata[0] = WTM;	/* format or write */
	resid = cip->b_sector % uip->d_drtab.dr_spc;
	cmdp->c_rwdata[3] = resid / uip->d_drtab.dr_nsec;
	cmdp->c_rwdata[1] = slave | (cmdp->c_rwdata[3] << 2);
	cmdp->c_rwdata[2] = cmdp->c_saddr =
	    cip->b_sector / uip->d_drtab.dr_spc;
	cmdp->c_rwdata[4] = (resid % uip->d_drtab.dr_nsec) + 1;
	cmdp->c_rwdata[5] = 2;
	cmdp->c_rwdata[6] = uip->d_drtab.dr_nsec;
	cmdp->c_rwdata[7] = uip->d_drtab.dr_rwgpl;
	cmdp->c_rwdata[8] = DTL;
	D(printf("SET %x %x C%x H%x S%x %x %x %x %x ",
		 cmdp->c_rwdata[0], cmdp->c_rwdata[1],
		 cmdp->c_rwdata[2], cmdp->c_rwdata[3],
		 cmdp->c_rwdata[4], cmdp->c_rwdata[5],
		 cmdp->c_rwdata[6], cmdp->c_rwdata[7],
		 cmdp->c_rwdata[8]));
	break;
    }
    printf("rwcmdset:outof it ");
}
/*****************************************************************************
 *
 * TITLE:	fdread
 *
 * ABSTRACT:	"Raw" read.  Use physio().
 *
 * CALLS:	m765breakup (indirectly, thru physio)
 *
 ****************************************************************************/

io_return_t
fdread(
	  dev_t dev,
	  io_req_t uio)
{
/*
   int unit = UNIT(dev);

   printf("fdread:got control ");
   mem[0]=mem[1]=mem[2]='Z';
   FloppyPluginIO(unit, 0,0,mem,0, kBSRead,0); 
   printf("\nmem=%c%c%c\n",mem[0],mem[1],mem[2]);
 */
    /* no need for page-size restriction */
    return (block_io(fdstrategy, minphys, uio));
}
/*****************************************************************************
 *
 * TITLE:	fdwrite
 *
 * ABSTRACT:	"Raw" write.  Use physio().
 *
 * CALLS:	m765breakup (indirectly, thru physio)
 *
 ****************************************************************************/

io_return_t
fdwrite(
	   dev_t dev,
	   io_req_t uio)
{
    DriveStatusType *DriveStatus;
    OSStatus IOErrorCode = noErr;

    /* @@@ write protect check HERE! @@@ */
    if ((IOErrorCode = CheckDriveNumber(UNIT(dev), 1, &DriveStatus)) == noErr) {

	if (DriveStatus->writeProt)
	    return wPrErr;	// D_INVALID_OPERATION? // D_IO_ERROR?

	/* no need for page-size restriction */
	return (block_io(fdstrategy, minphys, uio));
    } else {
	return IOErrorCode;
    }
}

/* IOC_OUT only and not IOC_INOUT */
io_return_t
fdgetstat(
	     dev_t dev,
	     dev_flavor_t flavor,
	     dev_status_t data,	/* pointer to OUT array */
	     natural_t * count)
{				/* OUT */
    struct disk_parms p;
    io_return_t ret;

//printf("fdgetstat:test from getstat ");
    switch (flavor) {

	/* Mandatory flavors */

    case DEV_GET_SIZE:
	ret = fd_getparms(dev, (int *) &p);
//naga          if (ret) return ret;
	data[DEV_GET_SIZE_DEVICE_SIZE] = 2880 * NBPSCTR;	// naga   p.dp_pnumsec * NBPSCTR;

	data[DEV_GET_SIZE_RECORD_SIZE] = NBPSCTR;
	*count = DEV_GET_SIZE_COUNT;
	return (D_SUCCESS);

	/* Extra flavors */

    case V_GETPARMS:
	if (*count < sizeof(struct disk_parms) / sizeof(int))
	     return (D_INVALID_OPERATION);
	*count = sizeof(struct disk_parms) / sizeof(int);
	return (fd_getparms(dev, data));
    default:
	return (D_INVALID_OPERATION);
    }
}
/* IOC_VOID or IOC_IN or IOC_INOUT */

io_return_t
fdsetstat(
	     dev_t dev,
	     dev_flavor_t flavor,
	     dev_status_t data,
	     natural_t count)
{
    int unit = UNIT(dev);
    struct unit_info *uip = &unit_info[UNIT(dev)];
    spl_t s;

#if MACH_DEBUG
    printf("fdsetstat:entered setstat ");
#endif
    switch (flavor) {
    case V_SETPARMS:		/* Caller wants reset_parameters */
	return (fd_setparms(unit, *(int *) data));
    case V_FORMAT:
	return (fd_format(dev, data));
    case V_VERIFY:		/* cmdarg : 0 == no verify, 0 != verify */
	m765verify[unit] = *(int *) data;
	return (D_SUCCESS);
    case V_EJECT:
#if MACH_DEBUG
	printf("FLOPPY EJECT\n");
#endif
	FloppyPluginFlush(0, 0, 0);	/*Flush */
	// fdclose(dev); /* @@@ dg @@@ */
	// FloppyPluginGotoState( 0, kBSOnline );   /* old Flush and Eject*/
#if 0
	{
	    int mycount = 0;
	    while ((mycount++ < 30) && (FloppyPluginEject(0, kBSOnline) != noErr));
	}
#else
	FloppyPluginEject(0, kBSOnline);
#endif
	fdclose(dev);		/* @@@ dg @@@ */
#if 1
	FloppyPluginFlush(0, 0, 0);	/*Flush */// added -- dg
#endif

#if	NCPUS > 1
	io_grab_master();
#endif				/* NCPUS > 1 */

	/* Clear the bit.
	   * If drive closed, insure drtab queue is empty before returning.
	 */
	s = SPL();
	while (uip->av_forw != 0) {
	    uip->wakeme = 1;
	    splx(s);
	    sleep((char *) uip, PRIBIO);
	    s = SPL();
	}
	splx(s);
#if	NCPUS > 1
	io_release_master();
#endif				/* NCPUS > 1 */
	/* TODO correct error codes */
	return (D_SUCCESS);
    default:
	return (D_INVALID_OPERATION);
    }
}

/*
 *      Get block size
 */
io_return_t
fddevinfo(
	     dev_t dev,
	     dev_flavor_t flavor,
	     char *info)
{
    register struct fddrtab *dr;
    register struct fdpart *p;
    register int result = D_SUCCESS;

    switch (flavor) {
    case D_INFO_BLOCK_SIZE:
	dr = &unit_info[UNIT(dev)].d_drtab;

	if (dr->dr_type & OKTYPE)
	    *((int *) info) = 512;
	else
	    result = D_INVALID_OPERATION;

	break;
    default:
	result = D_INVALID_OPERATION;
    }

    return (result);
}

/****************************************************************************
 *
 *	set fd parameters 
 *
 ****************************************************************************/

io_return_t
fd_setparms(
	       int unit,
	       int cmdarg)
{
    struct fddrtab *fdparm;
    spl_t x;
    struct unit_info *uip = &unit_info[unit];
    struct fdcmd *cmdp = uip->b_cmd;

    cmdp->c_rbmtr &= ~(1 << (RBSHIFT + uip->dev->slave));
    if ((fdparm = getparm(MEDIATYPE(cmdarg))) == (struct fddrtab *) ERROR)
	return (D_INVALID_SIZE);
    x = SPL();
    openchk(cmdp);
    cmdp->c_devflag |= FDMCHK;
    chkbusy(cmdp);
    m765sweep(uip, fdparm);
    uip->d_drtab.dr_type |= OKTYPE;
    openfre(cmdp);
    splx(x);
    return (0);
}
/****************************************************************************
 *
 *	get fd parameters 
 *
 ****************************************************************************/

io_return_t
fd_getparms(
	       dev_t dev,	/* major, minor numbers */
	       int *cmdarg)
{
    struct disk_parms *diskp = (struct disk_parms *) cmdarg;
    register struct fddrtab *dr = &unit_info[UNIT(dev)].d_drtab;

    if (dr->dr_type & OKTYPE) {
	diskp->dp_type = DPT_FLOPPY;
	diskp->dp_heads = 2;
	diskp->dp_sectors = dr->dr_nsec;
	diskp->dp_pstartsec = 0;
	diskp->dp_cyls = dr->dr_ncyl;
	diskp->dp_pnumsec = dr->p_nsec;
	diskp->dp_secsiz = 512;
	return (0);
    }
    return (D_NO_SUCH_DEVICE);
}
/****************************************************************************
 *
 *	format command
 *
 ****************************************************************************/

io_return_t
fd_format(
	     dev_t dev,		/* major, minor numbers */
	     int *cmdarg)
{
    register struct buf *bp;
    register daddr_t track;
    union io_arg *varg;
    u_short num_trks;
    register struct fddrtab *dr = &unit_info[UNIT(dev)].d_drtab;

    if (!(dr->dr_type & OKTYPE))
	return (D_INVALID_SIZE);
    varg = (union io_arg *) cmdarg;
    num_trks = varg->ia_fmt.num_trks;
    track = (daddr_t) (varg->ia_fmt.start_trk * dr->dr_nsec);
    if ((track + (num_trks * dr->dr_nsec)) > dr->p_nsec)
	return (D_INVALID_SIZE);
    bp = geteblk(BLKSIZE);	/* get struct buf area */
    while (num_trks > 0) {
	bp->b_flags &= ~B_DONE;
	bp->b_dev = dev;
	bp->b_error = 0;
	bp->b_resid = 0;
	bp->b_flags = B_FORMAT;
	bp->b_bcount = dr->dr_nsec * FMTID;
	bp->b_blkno = (daddr_t) ((track << 9) / NBPSCTR);
	if (makeidtbl((struct fmttbl *) bp->b_un.b_addr, dr,
		      varg->ia_fmt.start_trk++, varg->ia_fmt.intlv))
	    return (D_INVALID_SIZE);
	fdstrategy(bp);
	biowait(bp);
	if (bp->b_error)
	    if ((bp->b_error == (char) EBBHARD) ||
		(bp->b_error == (char) EBBSOFT))
		return (EIO);
	    else
		return (bp->b_error);
	num_trks--;
	track += dr->dr_nsec;
    }
    brelse(bp);
    return (D_SUCCESS);
}
/****************************************************************************
 *
 *	make id table for format
 *
 ****************************************************************************/

int makeidtbl(
		 struct fmttbl *tblpt,
		 struct fddrtab *dr,
		 unsigned short track,
		 unsigned short intlv)
{
    register int i, j, secno;

    if (intlv >= dr->dr_nsec)
	return (1);
    for (i = 0; i < dr->dr_nsec; i++)
	tblpt[i].sector = 0;
    for (i = 0, j = 0, secno = 1; i < dr->dr_nsec; i++) {
	tblpt[j].cyl = track >> 1;
	tblpt[j].head = track & 1;
	tblpt[j].sector = secno++;
	tblpt[j].s_type = 2;
	if ((j += intlv) < dr->dr_nsec)
	    continue;
	for (j -= dr->dr_nsec; j < dr->dr_nsec; j++)
	    if (!tblpt[j].sector)
		break;
    }
    return (0);
}
/*****************************************************************************
 *
 * TITLE:	m765sweep
 *
 * ABSTRACT:	Perform an initialization sweep.  
 *
 **************************************************************************/

void m765sweep(
		  struct unit_info *uip,
		  struct fddrtab *cdr)
{				/* device initialization data */
    register struct fddrtab *dr = &uip->d_drtab;

    dr->dr_ncyl = cdr->dr_ncyl;
    dr->dr_nsec = cdr->dr_nsec;
    dr->dr_spc = cdr->dr_spc;
    dr->p_nsec = cdr->p_nsec;
    dr->dr_type = cdr->dr_type;
    dr->dr_rwgpl = cdr->dr_rwgpl;
    dr->dr_fgpl = cdr->dr_fgpl;
}

/*****************************************************************************
 *
 *	fdc reset routine
 *
 *****************************************************************************/

void rstout(
	       struct unit_info *uip)
{
    register int outd;

    outd = ((uip->b_cmd->c_rbmtr & MTRMASK) << MTR_ON) | uip->dev->slave;
    outb(CTRLREG(uip->addr), outd);
    outd |= FDC_RST;
    outb(CTRLREG(uip->addr), outd);
    outd |= DMAREQ;
    outb(CTRLREG(uip->addr), outd);
}

/*****************************************************************************
 *
 *	specify command routine
 *
 *****************************************************************************/

void specify(
		struct unit_info *uip)
{
    /* status check */
    if (fdc_sts(FD_OSTS, uip))
	return;
    /* Specify command */
    outb(DATAREG(uip->addr), SPCCMD);
    /* status check */
    if (fdc_sts(FD_OSTS, uip))
	return;
    /* Step rate,Head unload time */
    outb(DATAREG(uip->addr), SRTHUT);
    /* status check */
    if (fdc_sts(FD_OSTS, uip))
	return;
    /* Head load time,Non DMA Mode */
    outb(DATAREG(uip->addr), HLTND);
    return;
}

/*****************************************************************************
 *
 *	fdc status get routine
 *
 *****************************************************************************/

int fdc_sts(
	       int mode,
	       struct unit_info *uip)
{
    register int ind;
    int cnt0 = STSCHKCNT;

    while (cnt0--)
	if (((ind = inb(STSREG(uip->addr))) & DATAOK) &&
	    ((ind & DTOCPU) == mode))
	    return (0);
    return (TIMEOUT);
}

#endif // NFD > 0
