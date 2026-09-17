/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

#include "prio.h"
#include "ether_hash.h"

#include <mac2os/Types.h>

static hash_entry_ptr_t	ether_hash_entry_alloc();

boolean_t
ether_proto_hash_add(protocols, proto, handler)
hash_bucket_t	protocols[];
unsigned short	proto;
unsigned long	handler;
{
    register hash_entry_ptr_t	entry;

    lookup_proto_hash_entry(proto, entry);
    if (entry != HASH_ENTRY_NULL)
	return (FALSE);

    entry = ether_hash_entry_alloc();
    if (entry == HASH_ENTRY_NULL)
	return (FALSE);

    entry->proto = proto;
    entry->handler = handler;
    {
	register	prio = prio_set(PRIO_MAX);

	insert_hash_entry(entry);

	prio_set(prio);
    }
	
    return (TRUE);
}

static void ether_hash_entry_dealloc(hash_entry_ptr_t entry)
{
  DisposPtr((Ptr)entry);
}

void
ether_proto_hash_delete(protocols, proto)
hash_bucket_t	protocols[];
unsigned short	proto;
{
    register hash_entry_ptr_t	entry;

    lookup_proto_hash_entry(proto, entry);

    if (entry == HASH_ENTRY_NULL)
	return;

    {
	register	prio = prio_set(PRIO_MAX);

	remove_hash_entry(entry);

	prio_set(prio);
    }
	
    ether_hash_entry_dealloc(entry);
}

unsigned long
ether_proto_hash_lookup(protocols, proto)
hash_bucket_t	protocols[];
unsigned short	proto;
{
    register hash_entry_ptr_t	entry;

    lookup_proto_hash_entry(proto, entry);

    if (entry == HASH_ENTRY_NULL)
	return (-1);

    return (entry->handler);
}

boolean_t
ether_proto_hash_insert_port(protocols, proto, port)
hash_bucket_t	protocols[];
unsigned short	proto;
mach_port_t	port;
{
    register hash_entry_ptr_t	entry;

    lookup_proto_hash_entry(proto, entry);

    if (entry == HASH_ENTRY_NULL)
	return (FALSE);

    entry->recv_port = port;

    return (TRUE);
}

mach_port_t
ether_proto_hash_return_port(protocols, proto)
hash_bucket_t	protocols[];
unsigned short	proto;
{
    register hash_entry_ptr_t	entry;

    lookup_proto_hash_entry(proto, entry);

    if (entry == HASH_ENTRY_NULL)
	return (MACH_PORT_NULL);

    return (entry->recv_port);
}

static
hash_entry_ptr_t
ether_hash_entry_alloc()
{
    hash_entry_ptr_t	entry;

    entry = (hash_entry_ptr_t)NewPtrSysClear(sizeof (*entry));
	
    return (entry);
}
