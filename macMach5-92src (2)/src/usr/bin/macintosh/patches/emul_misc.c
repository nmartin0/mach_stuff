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

#include <mach/message.h>

#include <emul.h>
#include <prio.h>
#include <access.h>

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Files.h>

#include <OSUtils.h>
#include <Devices.h>

#define SCSIPoll		(*(unsigned char *)0xc2f)
#define DrvQHdr			(*(QHdrPtr)0x308)
#define UTableBase		(*(unsigned **)0x11c)
#define UnitNtryCnt		(*(unsigned short *)0x1d2)

void
emul_patch_init()
{
#define trap1_vector		(*(unsigned *)0x84)
    extern void			_trap1();

    trap1_vector = _trap1;
    initFLUSH_ICACHE();
}

void
emul_device_init()
{
    initVBL();
    initRTC();
    initSOUND();
    initSONY();
    initSCSI();
    initTIMER();
    initETHER();
}

initMEMORY()
{
    unsigned		old;
    extern void		LogicalPageSize(), LogicalRAMSize(), VMAttr();

    (void) ReplaceGestalt(*(unsigned *)"pgsz", LogicalPageSize, &old);
    (void) ReplaceGestalt(*(unsigned *)"lram", LogicalRAMSize, &old);
    (void) ReplaceGestalt(*(unsigned *)"vm  ", VMAttr, &old);
}

static struct {
    mach_msg_header_t		head;
    mach_msg_type_t		addressType;
    vm_address_t		address;
    mach_msg_type_t		sizeType;
    vm_size_t			size;
    mach_msg_type_t		attributeType;
    vm_machine_attribute_t	attribute;
    mach_msg_type_t		valueType;
    vm_machine_attribute_val_t	value;
} flush_icache_msg;

initFLUSH_ICACHE()
{
#define flush_icache_vector	((void (*)())(*(unsigned *)0x6f4))
    extern void			flush_icache();

    static mach_msg_type_t addressType = {
	/* msgt_name = */		MACH_MSG_TYPE_INTEGER_32,
	/* msgt_size = */		32,
	/* msgt_number = */		1,
	/* msgt_inline = */		TRUE,
	/* msgt_longform = */		FALSE,
	/* msgt_deallocate = */		FALSE,
	/* msgt_unused = */		0
    };
    static mach_msg_type_t sizeType = {
	/* msgt_name = */		MACH_MSG_TYPE_INTEGER_32,
	/* msgt_size = */		32,
	/* msgt_number = */		1,
	/* msgt_inline = */		TRUE,
	/* msgt_longform = */		FALSE,
	/* msgt_deallocate = */		FALSE,
	/* msgt_unused = */		0
    };
    static mach_msg_type_t attributeType = {
	/* msgt_name = */		MACH_MSG_TYPE_INTEGER_32,
	/* msgt_size = */		32,
	/* msgt_number = */		1,
	/* msgt_inline = */		TRUE,
	/* msgt_longform = */		FALSE,
	/* msgt_deallocate = */		FALSE,
	/* msgt_unused = */		0
    };
    static mach_msg_type_t valueType = {
	/* msgt_name = */		MACH_MSG_TYPE_INTEGER_32,
	/* msgt_size = */		32,
	/* msgt_number = */		1,
	/* msgt_inline = */		TRUE,
	/* msgt_longform = */		FALSE,
	/* msgt_deallocate = */		FALSE,
	/* msgt_unused = */		0
    };

    flush_icache_msg.addressType = addressType;
    flush_icache_msg.address = 0;

    flush_icache_msg.sizeType = sizeType;
    flush_icache_msg.size = vm_page_size;
    
    flush_icache_msg.attributeType = attributeType;
    flush_icache_msg.attribute = 0;

    flush_icache_msg.valueType = valueType;
    flush_icache_msg.value = MATTR_VAL_ICACHE_FLUSH;

    flush_icache_msg.head.msgh_bits =
	MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);

    flush_icache_msg.head.msgh_remote_port = mach_task_self();
    flush_icache_msg.head.msgh_local_port = MACH_PORT_NULL;
    flush_icache_msg.head.msgh_kind = MACH_MSGH_KIND_NORMAL;
    flush_icache_msg.head.msgh_id = 2099;

    flush_icache_vector = flush_icache;
}

void
FlushICache()
{
    (void) mach_msg_trap(&flush_icache_msg,
			 MACH_SEND_MSG,
			 sizeof (flush_icache_msg),
			 0,
			 MACH_PORT_NULL,
			 MACH_MSG_TIMEOUT_NONE,
			 MACH_PORT_NULL);
}

void
MemoryDispatch(regs)
os_reg_t	regs;
{
    stop(&regs, 0);
}

#define POP(x)		FETCH_INCR(regs.a_7, &(x), typeof (x))

#define RET(x)		STORE(regs.a_7, &(x), typeof (x))

#define P_BEGIN \
{				\
    unsigned	ret_addr;	\
\
    POP(ret_addr);		\
    {

#define P_END \
    }					\
    regs.a_7 -= sizeof (ret_addr);	\
    STORE(regs.a_7, &ret_addr, long);	\
}

void
SoundDispatch(regs)
tbox_reg_t	regs;
{
    P_BEGIN

    switch (regs.d_0) {
      case 0x00000010:
      case 0x00000014:
      case 0x000c0008:
	{
	    long	version = 0;

	    RET(version);
	}
	break;
	
      default:
	stop(&regs, 0);
    }

    P_END
}

void
SndDisposeChannel(regs)
tbox_reg_t	regs;
{
    stop(&regs, 1);
}

void
SndAddModifier(regs)
tbox_reg_t	regs;
{
    stop(&regs, 2);
}

void
SndDoCommand(regs)
tbox_reg_t	regs;
{
    stop(&regs, 3);
}

void
SndDoImmediate(regs)
tbox_reg_t	regs;
{
    stop(&regs, 4);
}

void
SndPlay(regs)
tbox_reg_t	regs;
{
    stop(&regs, 5);
}

void
SndControl(regs)
tbox_reg_t	regs;
{
    stop(&regs, 6);
}

void
SndNewChannel(regs)
tbox_reg_t	regs;
{
    P_BEGIN

    OSErr	err = noHardware;
    Ptr		chan, userRoutine;
    long	init;
    short	synth;

    POP(userRoutine);
    POP(init);
    POP(synth);
    POP(chan);

    RET(err);

    P_END
}

void
Enqueue(regs)
tbox_reg_t	regs;
{
    (void) enqueue(regs.a_0, regs.a_1);
}

void
Dequeue(regs)
tbox_reg_t	regs;
{
    regs.d_0 = dequeue(regs.a_0, regs.a_1);
}

void
enqueue(entry, queue)
register QElemPtr	entry;
register QHdrPtr	queue;
{
    int		prio;

    prio = prio_set(PRIO_MAX);

    if (queue->qTail == 0)
	queue->qHead = queue->qTail = entry;
    else {
	queue->qTail->qLink = entry;
	queue->qTail = entry;
    }

    entry->qLink = 0;

    prio_set(prio);
}

dequeue(entry, queue)
register QElemPtr	entry;
register QHdrPtr	queue;
{
    register QElemPtr		qp, *ppl;
    int				prio, result = qErr;

    prio = prio_set(PRIO_MAX);

    if (queue->qHead == queue->qTail) {
	/*
	 * <= 1 entry in queue
	 */
	if (entry == queue->qHead) {
	    queue->qHead = queue->qTail = 0;
	    result = noErr;
	}
    }
    else {
	/*
	 * > 1 entry in queue
	 */
	ppl = &queue->qHead;

	while (qp = *ppl) {
	    if (qp == entry) {
		*ppl = entry->qLink;
		if (entry == queue->qTail)
		    queue->qTail = (QElemPtr)ppl;

		result = noErr;
		break;
	    }

	    ppl = &qp->qLink;
	}
    }
    
    prio_set(prio);

    return (result);
}

static
void
initVBL()
{
    QHdrPtr	q = (QHdrPtr)0x160;

    q->qHead = q->qTail = 0; q->qFlags = 0;
}

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strncmp.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * Compare strings (at most n bytes):  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

strncmp(s1, s2, n)
register char *s1, *s2;
register n;
{

	while (--n >= 0 && *s1 == *s2++)
		if (*s1++ == '\0')
			return(0);
	return(n<0 ? 0 : *s1 - *--s2);
}

typedef struct {
    unsigned short	drvrFlags;
    unsigned short	drvrDelay;
    unsigned short	drvrEMask;
    unsigned short	drvrMenu;
    unsigned short	drvrOpen;
    unsigned short	drvrPrime;
    unsigned short	drvrCtl;
    unsigned short	drvrStatus;
    unsigned short	drvrClose;
    String(0)		drvrName;
} *DrvrHdr;    

static
void
initSCSI()
{
    register DrvQElPtr	drv;
    register DCtlHandle	dctl;
    register		i;
    static int		ndrv[7];
    static DrvQElPtr	drvq[7];

    SCSIPoll |= 0x80;

    for (drv = DrvQHdr.qHead; drv; drv = (DrvQElPtr)drv->qLink) {
	i = ~drv->dQRefNum - 0x20;
	if (i >= 0 && i <= 6) {
	    if (ndrv[i]++ == 0)
		drvq[i] = drv;
	}
    }
	    
    for (i = 0; i <= 6; i++) {
	if (dctl = UTableBase[0x20 + i])
	    scsiInstall(*dctl, i, drvq[i], ndrv[i]);
    }
}

static
void
initSONY()
{
    register DrvQElPtr	drv;
    register DCtlHandle	dctl;
    static int		ndrv;
    static DrvQElPtr	drvq;

    for (drv = DrvQHdr.qHead; drv; drv = (DrvQElPtr)drv->qLink) {
	if (drv->dQRefNum == -5 && ndrv++ == 0)
	    drvq = drv;
    }

    if (dctl = UTableBase[0x4])
	sonyInstall(*dctl, drvq, ndrv);
}

static		nen;

static
void
initETHER()
{
    register			i, e;
    register DCtlHandle		dctl;
    register DrvrHdr		d;

    e = UnitNtryCnt;

    for (i = 1; i < e; i++) {
	if (dctl = UTableBase[i]) {
	    if ((*dctl)->dCtlFlags & 0x40)
		/* RAM based driver */
		d = *(DrvrHdr *)(*dctl)->dCtlDriver;
	    else
		/* ROM based driver */
		d = (DrvrHdr)(*dctl)->dCtlDriver;

	    if (d->drvrName.length == 5
		&& !strncmp(".ENET", d->drvrName.text, 5)) {

		etherInstall(*dctl, nen++);
	    }
	}
    }
}

static
void
initSOUND()
{
    register DCtlHandle		dctl;
    extern unsigned short	soundHdr[];

    if (dctl = UTableBase[0x3]) {
	(*dctl)->dCtlDriver = (Ptr)soundHdr;
	(*dctl)->dCtlFlags = soundHdr[0] | 0x20;
    }
}
