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

#include <OSUtils.h>
#include <Devices.h>

typedef struct sonyVars {
    DrvQElPtr	drv;
    device_t	device;
} *sonyVars_t;

extern unsigned short	sonyHdr[];

extern mach_port_t	master_device_port;

void
sonyInstall(d, drvq, ndrv)
register DCtlPtr	d;
register DrvQElPtr	drvq;
register		ndrv;
{
    register			i;
    register kern_return_t	result;
    register sonyVars_t		v;
    static unsigned char	name[6] = { 's', 'o', 'n', 'y' };

    d->dCtlDriver = (Ptr)sonyHdr;
    d->dCtlFlags = sonyHdr[0] | 0x20;

    if (ndrv > 0) {
	(sonyVars_t)d->dCtlStorage =
	    v = (sonyVars_t)malloc(ndrv * sizeof (struct sonyVars));
	for (i = 0; i < ndrv; i++) {
	    name[4] = '0' + drvq->dQDrive;
	    result = device_open(master_device_port,
				 D_READ|D_WRITE,
				 name,
				 &v->device);
	    if (result == KERN_SUCCESS)
		v->drv = drvq;

	    drvq = drvq->qLink;
	    v++;
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
sonyCall(regs)
os_reg_t	regs;
{
    register IOParam	*pb, *tpb;
    register DCtlPtr	dce;
    register		prio;

    pb = regs.a_0;
    dce = regs.a_1;

    if (pb->ioTrap&Immed)
	sony_do(pb, dce);
    else {
	prio = prio_set(PRIO_MAX);
	do {
	    tpb = (IOParam *)dce->dCtlQHdr.qHead;
	    if ((dce->dCtlQHdr.qHead = tpb->qLink) == 0)
		dce->dCtlQHdr.qTail = 0;
	    prio_set(prio);
		
	    sony_do(tpb, dce);

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
sony_do(pb, dce)
register IOParam	*pb;
register DCtlPtr	dce;
{
    register sonyVars_t	v = (sonyVars_t)dce->dCtlStorage;

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
			    (pb->ioPosOffset >> 9),
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
			     (pb->ioPosOffset >> 9),
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
	      case 7:
		(void) device_set_status(v->device,
					 7,
					 0, 0);
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
		    unsigned		count;

		    count = 1;
		    stop(v->drv, st);
		    (void) device_get_status(v->device,
					     8,
					     ((unsigned)v->drv) - 4, &count);
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
