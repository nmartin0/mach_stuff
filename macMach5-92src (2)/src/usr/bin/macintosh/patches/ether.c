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

#include <device/device_types.h>
#include <device/net_status.h>

#include <mac2if/if_en.h>

#include <exit.h>
#include <ether_hash.h>
#include <emul.h>
#include <prio.h>
#include <th.h>

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Ethernet.h>

#include <OSUtils.h>
#include <Devices.h>

#define is802_3(proto)	((proto) > 0 && (proto) <= 1500)
static filter_t		ether_802_3_filter[] = {
    (NETF_PUSHWORD + 1)	| NETF_NOP,
    NETF_PUSHLIT	| NETF_LE,
    1500,
    NETF_PUSHLIT	| NETF_CAND,
    TRUE,
    (NETF_PUSHWORD + 1)	| NETF_NOP,
    NETF_PUSHZERO	| NETF_GT,
};

typedef struct etherVars {
    device_t		device;
    mach_port_t		recv_ports;
    ether_address_t	eaddr;
    boolean_t		active802_3;
    hash_bucket_t	protocols[HASH_SIZE];
    unsigned char	packet[EMaxPacketSz];
} *etherVars_t;

extern unsigned short	etherHdr[];

extern mach_port_t	master_device_port;

void
etherInstall(d, unit)
register DCtlPtr	d;
register		unit;
{
    register kern_return_t	result;
    register etherVars_t	v;
    device_t			device;
    static unsigned char	name[4] = { 'e', 'n' };
    void			etherinput();

    d->dCtlDriver = (Ptr)etherHdr;
    d->dCtlFlags = etherHdr[0] | 0x20;

    name[2] = '0' + unit;
    result = device_open(master_device_port,
			 D_READ|D_WRITE,
			 name,
			 &device);
    if (result == KERN_SUCCESS) {
	register		i;
	en_address_status_t	address;
	unsigned		address_count;

	(etherVars_t)d->dCtlStorage =
	    v = (etherVars_t)malloc(sizeof (struct etherVars));
	v->device = device;
	address_count = EN_ADDRESS_STATUS_COUNT;
	(void) device_get_status(device,
				 NET_ADDRESS,
				 &address, &address_count);
	bcopy(address, v->eaddr, sizeof (ether_address_t));
	for (i = 0; i < HASH_SIZE; i++)
	    queue_init(&v->protocols[i].head);
	result = mach_port_allocate(mach_task_self(),
				    MACH_PORT_RIGHT_PORT_SET,
				    &v->recv_ports);
	if (result != KERN_SUCCESS)
	    return;

	(void) th_alloc(etherinput, (int)v);
    }
    else
	d->dCtlStorage = 0;
}

#define drvrActive	0x0080

static inline
void
call_complete(pb)
register Eiopb		*pb;
{
    asm volatile("movw %0,d0; movl %1,a0; jsr %2@"
	:
	: "rm" (pb->ioResult), "rm" (pb), "a" (pb->ioCompletion)
	: "d0", "d1", "d2", "d3", "a0", "a1", "a2", "a3");
}

void
etherCall(regs)
os_reg_t	regs;
{
    register Eiopb	*pb, *tpb;
    register DCtlPtr	dce;
    register		prio;

    pb = regs.a_0;
    dce = regs.a_1;

    if (pb->ioTrap&Immed)
	ether_do(pb, dce);
    else {
	prio = prio_set(PRIO_MAX);
	do {
	    tpb = (Eiopb *)dce->dCtlQHdr.qHead;
	    if ((dce->dCtlQHdr.qHead = tpb->qLink) == 0)
		dce->dCtlQHdr.qTail = 0;
	    prio_set(prio);

	    ether_do(tpb, dce);
			
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
ether_do(pb, dce)
register Eiopb		*pb;
register DCtlPtr	dce;
{
    etherVars_t		v = dce->dCtlStorage;

    switch ((unsigned)pb->ioTrap & ~(Immed|Async)) {
      case 0xa004:
	pb->ioResult = ether_ctl_calls(v, pb);
	break;
	
      default:
	stop(pb, dce);
	pb->ioResult = ioErr;
	break;
    }
}

void
etherinput(v)
register etherVars_t	v;
{
    register msg_return_t	result;
    register net_rcv_msg_t	msg;

    (unsigned)msg = malloc(sizeof (struct net_rcv_msg));

    sched_set_prio(PRIO_HIGH);
    while (TRUE) {
	msg->msg_hdr.msgh_local_port = v->recv_ports;
	msg->msg_hdr.msgh_size = sizeof (*msg);
	result = mach_msg_receive(msg);
	if (result != MACH_MSG_SUCCESS)
	    exit();

	etherrecv(v, msg->header, msg->packet);
    }
}

static
inline
call_dispatch(handler, data, length)
{
    asm volatile("movl %0,a1; movl %1,a0; movl %2,d1; moveml d2-d3/a2-a5,sp@-; movl a0,a3; lea etherRPRR,a4; jsr a1@; moveml sp@+,d2-d3/a2-a5"
	:
	: "rm" (handler), "rm" (data), "rm" (length)
	: "d0", "d1", "a0", "a1");
}

void
etherrecv(v, hdr, pkt)
etherVars_t			v;
unsigned char			*hdr;
register struct packet_header	*pkt;
{
    register			prio;
    register unsigned		length;
    register unsigned short	proto;
    register unsigned		handler;
    unsigned char		*bdy;

    length = pkt->length - sizeof (struct packet_header);
    proto = pkt->type;
    if (is802_3(proto) && v->active802_3) {
	length = proto;
	proto = 0;
    }
			
    handler = ether_proto_hash_lookup(v->protocols, proto);
    switch (handler) {
      case -1:
	break;
			
      case 0:
	/* ERead XXX */
	break;
			
      default:
	{
	    unsigned char	*p = *(unsigned long *)0x824;

	    *p ^= 0xc0;
	}
	/*
	 * Copy ethernet header to
	 * front of packet body.
	 */
	bdy = (unsigned char *)(pkt + 1) - sizeof (ether_header_t);
	*((unsigned *)bdy)++ = *((unsigned *)hdr)++;
	*((unsigned *)bdy)++ = *((unsigned *)hdr)++;
	*((unsigned *)bdy)++ = *((unsigned *)hdr)++;
	*((unsigned short *)bdy)++ = *(unsigned short *)hdr;

	prio = prio_enter(PRIO_HIGH);
	call_dispatch(handler, bdy, length);
	prio_exit(prio);
	break;
    }
}

OSErr
ewrite(v, pb)
etherVars_t	v;
Eiopb		*pb;
{
    register wdsPtr		wds = pb->p.write.EWdsPointer;
    register unsigned char	*p = v->packet;
    register unsigned int	wlen, length = 0;
    OSErr			ret = noErr;
	
    while (wlen = wds->length) {
	if ((length + wlen) > EMaxPacketSz)
	    wlen = EMaxPacketSz - length;

	if (wlen == 0)
	    break;

	(void) bcopy(wds->ptr, p, wlen);
	p += wlen; length += wlen;
	wds++;
    }

    if (length < EHdrSize)
	ret = eLenErr;
    else {
	{
	    unsigned char	*p = *(unsigned long *)0x824;

	    *p ^= 0x18;
	}

	if (length > sizeof (io_buf_ptr_inband_t)) {
	    if (device_write_request(v->device,
				     MACH_PORT_NULL,
				     0, 0,		/* mode, recnum */
				     v->packet, length) != D_SUCCESS)
		ret = ioErr;
	}
	else {
	    if (device_write_request_inband(v->device,
					    MACH_PORT_NULL,
					    0, 0,	/* mode, recnum */
					    v->packet, length) != D_SUCCESS)
		ret = ioErr;
	}
    }
	
    return (ret);
}

OSErr
eattachPH(v, pb)
etherVars_t	v;
Eiopb		*pb;
{
    filter_t			filter_data[NET_MAX_FILTER];
    register filter_array_t	filter;
    register unsigned		filter_count;
    unsigned			proto = pb->p.proto.EProtType;
    mach_port_t			port;

    if (ether_proto_hash_add(v->protocols,
			     proto, pb->p.proto.EHandler) != TRUE)
	return (lapProtErr);

    if (mach_port_allocate(mach_task_self(),
			   MACH_PORT_RIGHT_RECEIVE,
			   &port) != KERN_SUCCESS)
	return (lapProtErr);

    (void) mach_port_set_qlimit(mach_task_self(),
				port, MACH_PORT_QLIMIT_MAX);

    (void) mach_port_move_member(mach_task_self(),
				 port, v->recv_ports);

    (void) ether_proto_hash_insert_port(v->protocols, proto, port);

    if (proto == 0) {
	filter = ether_802_3_filter;
	filter_count = sizeof (ether_802_3_filter) / sizeof (filter_t);
    }
    else {
#define filter_enter(x)	\
	filter[filter_count++] = (x)

	filter = filter_data;
	filter_count = 0;

	filter_enter(NETF_PUSHWORD + 1		| NETF_NOP);
	filter_enter(NETF_PUSHLIT		| NETF_EQ);
	filter_enter(proto);

#undef filter_enter
    }

    if (device_set_filter(v->device,
			  port,
			  MACH_MSG_TYPE_MAKE_SEND,
			  10,	/* priority */
			  filter,
			  filter_count) != KERN_SUCCESS)
	return (lapProtErr);

    (void) device_set_status(v->device,
			     EN_ENBL_PROTO,
			     &proto, 1);

    if (proto == 0)
	v->active802_3 = TRUE;

    return (noErr);
}

OSErr
edetachPH(v, pb)
etherVars_t	v;
Eiopb		*pb;
{
    register mach_port_t	port;
    register unsigned short	proto = pb->p.proto.EProtType;

    port = ether_proto_hash_return_port(v->protocols, proto);

    (void) mach_port_destroy(mach_task_self(), port);

    if (ether_proto_hash_delete(v->protocols, proto) != TRUE)
	return (lapProtErr);

    if (proto == 0)
	v->active802_3 = FALSE;
		
    return (noErr);
}

OSErr
eaddmulti(v, pb)
etherVars_t	v;
Eiopb		*pb;
{
    en_address_status_t		address;

    bcopy(pb->p.multi.EMultiAddr, &address, sizeof (ether_address_t));

    if (device_set_status(v->device,
			  EN_ADD_MULTI,
			  &address, EN_ADDRESS_STATUS_COUNT) != D_SUCCESS)
	return (eMultiErr);
		
    return (noErr);
}

OSErr
edelmulti(v, pb)
etherVars_t	v;
Eiopb		*pb;
{
    en_address_status_t		address;

    bcopy(pb->p.multi.EMultiAddr, &address, sizeof (ether_address_t));

    if (device_set_status(v->device,
			  EN_DEL_MULTI,
			  &address, EN_ADDRESS_STATUS_COUNT) != D_SUCCESS)
	return (eMultiErr);
		
    return (noErr);
}

OSErr
ether_ctl_calls(v, pb)
etherVars_t	v;
Eiopb		*pb;
{
    OSErr	ret;

    switch (pb->csCode) {
      case EWrite:
	ret = ewrite(v, pb);
	break;
		
      case EAttachPH:
	ret = eattachPH(v, pb);
	break;
		
      case EDetachPH:
	ret = edetachPH(v, pb);
	break;
		
      case EGetInfo:
	if (pb->p.info.EBuffSize >= 18)	/* XXX */
	    bzero(pb->p.info.EBuffPtr, 18);

	if (pb->p.info.EBuffSize >= sizeof (ether_address_t))
	    bcopy(v->eaddr, pb->p.info.EBuffPtr, sizeof (ether_address_t));
			
	ret = noErr;
	break;

      case EAddMulti:
	ret = eaddmulti(v, pb);
	break;
		
      case EDelMulti:
	ret = edelmulti(v, pb);
	break;

      case 255:
      case ESetGeneral:
	ret = noErr;
	break;
	  
      default:
	stop(7, pb);
	ret = ioErr;
	break;
    }

    return (ret);
}
