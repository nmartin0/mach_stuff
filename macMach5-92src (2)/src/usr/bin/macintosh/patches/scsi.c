/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

#include <mach.h>

#include <device/device_types.h>

#include <emul.h>
#include <prio.h>

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Files.h>
#include <mac2os/Disks.h>

#include <mac2dev/sdisk.h>

#include <OSUtils.h>
#include <Devices.h>

typedef struct scsiVars {
    DrvQElPtr	drv;
    device_t	device;
} *scsiVars_t;

static unsigned char ICN[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7F, 0xFF, 0xFF, 0xFE, 0x80, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x01, 0x88, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01,
    0x7F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7F, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x7F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 'S', 'C', 'S', 'I'
};

extern unsigned short	scsiHdr[];

extern mach_port_t	master_device_port;

void
scsiInstall(d, unit, drvq, ndrv)
register DCtlPtr	d;
register 		unit;
register DrvQElPtr	drvq;
register		ndrv;
{
    register			i, parts;
    register kern_return_t	result;
    register scsiVars_t		v;
    device_t			disk;
    sd_part_info_t		info;
    unsigned			info_count;
    static unsigned char	name[5] = { 's', 'd' };

    d->dCtlDriver = (Ptr)scsiHdr;
    d->dCtlFlags = scsiHdr[0] | 0x20;

    if (ndrv > 0) {
	name[2] = '0' + unit;
	name[3] = 0;
	result = device_open(master_device_port,
			     D_READ,
			     name,
			     &disk);
	if (result == KERN_SUCCESS) {
	    info_count = SD_PART_INFO_COUNT;
	    result = device_get_status(disk,
				       SD_PART_INFO,
				       &info, &info_count);
	    if (result != KERN_SUCCESS) {
		(void) device_close(disk);
		(void) mach_port_deallocate(mach_task_self(), disk);
	    }
	}

	if (result == KERN_SUCCESS) {
	    (scsiVars_t)d->dCtlStorage =
		v = (scsiVars_t)malloc(ndrv * sizeof (struct scsiVars));

	    parts = info.parts;
	    for (i = 0; i < parts; i++) {
		name[3] = 'b' + i;
		result = device_open(master_device_port,
				     D_READ|D_WRITE,
				     name,
				     &v->device);
		if (result == KERN_SUCCESS) {
		    info_count = SD_PART_INFO_COUNT;
		    result = device_get_status(v->device,
					       SD_PART_INFO,
					       &info, &info_count);
		    if (result != KERN_SUCCESS) {
			(void) device_close(v->device);
			(void) mach_port_deallocate(mach_task_self(),
						    v->device);
		    }
		}
		if (result == KERN_SUCCESS) {
		    if (!strncmp("Apple_HFS",
				 info.type,
				 sizeof (info.type))) {
			v->drv = drvq;
			drvq = drvq->qLink;
			v++;
			if (--ndrv <= 0)
			    break;
		    }
		    else {
			(void) device_close(v->device);
			(void) mach_port_deallocate(mach_task_self(),
						   v->device);
		    }
		}
	    }
	    (void) device_close(disk);
	    (void) mach_port_deallocate(mach_task_self(), disk);
	}
    }
    else
	d->dCtlStorage = 0;
}

#define drvrActive	0x0080
#define Immed		0x0200
#define Async		0x0400

static inline
void
call_complete(pb)
register IOParam	*pb;
{
    asm volatile("movw %0,d0; movl %1,a0; jsr %2@"
	:
	: "rm" (pb->ioResult), "rm" (pb), "a" (pb->ioCompletion)
	: "d0", "d1", "d2", "d3", "a0", "a1", "a2", "a3");
}

void
scsiCall(regs)
os_reg_t	regs;
{
    register IOParam	*pb, *tpb;
    register DCtlPtr	dce;
    register		prio;

    pb = regs.a_0;
    dce = regs.a_1;

    if (pb->ioTrap&Immed)
	scsi_do(pb, dce);
    else {
	prio = prio_set(PRIO_MAX);
	do {
	    tpb = (IOParam *)dce->dCtlQHdr.qHead;
	    if ((dce->dCtlQHdr.qHead = tpb->qLink) == 0)
		dce->dCtlQHdr.qTail = 0;
	    prio_set(prio);

	    scsi_do(tpb, dce);
	    
	    if ((tpb->ioTrap&Async) && tpb->ioCompletion) {
		prio = prio_set(PRIO_HIGH);
		call_complete(tpb);
		prio_set(prio);
	    }
	    
	    prio = prio_set(PRIO_MAX);
	} while (dce->dCtlQHdr.qHead != 0);
	dce->dCtlFlags &= ~drvrActive;
	prio_set(prio);
    }

    regs.d_0 = pb->ioResult;
}

static
void
scsi_do(pb, dce)
register IOParam	*pb;
register DCtlPtr	dce;
{
    register scsiVars_t	v = (scsiVars_t)dce->dCtlStorage;

    v = &v[pb->ioVRefNum - v->drv->dQDrive];

    switch ((unsigned)pb->ioTrap & ~(Immed|Async)) {
      case 0xa002:
	{
	    io_buf_ptr_t	buffer;

	    if (pb->ioPosMode != fsFromStart)
		stop(pb, dce);

	    if (pb->ioReqCount == 0)
		pb->ioResult = noErr;
	    else
	    if (device_read(v->device,
			    0,
			    (pb->ioPosOffset >> SD_REC_SIZE_LOG2),
			    pb->ioReqCount,
			    &buffer,
			    &pb->ioActCount) != D_SUCCESS)
		pb->ioResult = ioErr;
	    else {
		bcopy(buffer, pb->ioBuffer, pb->ioActCount);
		dce->dCtlPosition += pb->ioActCount;
		(void) vm_deallocate(mach_task_self(),
				     buffer, pb->ioActCount);
		pb->ioResult = noErr;
	    }
	}	
	break;

      case 0xa003:
	{
	    if (pb->ioPosMode != fsFromStart)
		stop(pb, dce);

	    if (pb->ioReqCount == 0)
		pb->ioResult = noErr;
	    else
	    if (device_write(v->device,
			     0,
			     (pb->ioPosOffset >> SD_REC_SIZE_LOG2),
			     pb->ioBuffer,
			     pb->ioReqCount,
			     &pb->ioActCount) != D_SUCCESS)
		pb->ioResult = ioErr;
	    else {
		dce->dCtlPosition += pb->ioActCount;
		pb->ioResult = noErr;
	    }
	}	
	break;

      case 0xa004:
	{
	    register CntrlParam		*cpb = (CntrlParam *)pb;

	    switch (cpb->csCode) {
	      case 21:
	      case 22:
		*(unsigned char **)cpb->csParam = ICN;
		pb->ioResult = noErr;
		break;

	      case 23:
		cpb->csParam[0] = 0x201;
		pb->ioResult = noErr;
		break;

	      default:
		stop(pb, dce);
		pb->ioResult = ioErr;
		break;
	    }
	}
	break;

      case 0xa005:
	{
	    register CntrlParam		*cpb = (CntrlParam *)pb;

	    switch (cpb->csCode) {
	      case 8:
		{
		    register DrvSts	*st = (DrvSts *)cpb->csParam;

		    st->track = 0;
		    bcopy(((unsigned)v->drv) - 4,
			  &st->writeProt,
			  sizeof (DrvQEl) + 4);
		    pb->ioResult = noErr;
		}
		break;

	      default:
		stop(pb, dce);
		pb->ioResult = ioErr;
		break;
	    }
	}
	break;

      default:
	stop(pb, dce);
	pb->ioResult = ioErr;
	break;
    }
}
