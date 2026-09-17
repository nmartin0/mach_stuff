/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

#include "inst.h"
#include "access.h"
#include "inst_hash.h"

static hash_bucket_t	inst_hash_table[HASH_SIZE];

static
inline
hash_entry_ptr_t
alloc_hash_entry()
{
    return ((hash_entry_ptr_t) malloc(sizeof (hash_entry_t)));
}

void
inst_patch_init()
{
    register	i;

    for (i = 0; i < HASH_SIZE; i++)
	queue_init(hash_chain(i));
}

#define TRAP1_INST	0x4e41
void
patch_inst(inst, index, state)
unsigned short		inst;
int			index;
register inst_state_t	*state;
{
    register hash_entry_ptr_t	entry;
    register unsigned		loc = state->frame->f_normal.f_pc;

    lookup_hash_entry(loc, entry);

    if (entry == HASH_ENTRY_NULL) {
	entry = alloc_hash_entry();
	entry->loc = loc;
	entry->inst = inst;
	entry->index = index;
	insert_hash_entry(entry);
    }

    inst = TRAP1_INST;
    STORE(loc - sizeof (inst), &inst, inst);

    entry->count++;
}

unsigned short
patch_inst_lookup(loc, index)
register unsigned	loc;
register int		*index;
{
    register hash_entry_ptr_t	entry;

    lookup_hash_entry(loc, entry);
    if (entry == HASH_ENTRY_NULL)
	return (0);

    entry->count++;

    *index = entry->index;

    return (entry->inst);
}
