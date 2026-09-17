/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mac2/queue.h>
#include "lock.h"

typedef struct {
    queue_chain_t	hash_link;
    unsigned		loc;
    unsigned short	inst;
    int			index;
    unsigned		count;
} hash_entry_t, *hash_entry_ptr_t;

#define HASH_ENTRY_NULL	((hash_entry_ptr_t)0)

typedef struct {
    queue_head_t	head;
    lock_t		lock;
} hash_bucket_t;

#define HASH_SIZE	(1 << 12)

#define HASH_FUNCT(n)	((n) & (HASH_SIZE - 1))

#define hash_bucket(loc)	(&inst_hash_table[HASH_FUNCT(loc)])
#define hash_chain(loc)		(&inst_hash_table[HASH_FUNCT(loc)].head)

#define insert_hash_entry(entry)	\
{								\
    queue_enter(hash_chain((entry)->loc),			\
		(entry),					\
		hash_entry_ptr_t,			       	\
		hash_link);					\
}

#define remove_hash_entry(entry)	\
{								\
    queue_remove(hash_chain((entry)->loc),			\
		 (entry),					\
		 hash_entry_ptr_t,				\
		 hash_link);					\
}

#define lookup_hash_entry(loc_, entry)				\
{								\
    register queue_t		q;				\
    register hash_entry_ptr_t	h;				\
\
    (entry) = HASH_ENTRY_NULL;					\
    q = hash_chain(loc_);					\
    h = (hash_entry_ptr_t)queue_first(q);			\
    while (!queue_end(q, (queue_entry_t)h)) {			\
	if (h->loc == (loc_)) {					\
	    (entry) = h;					\
	    break;						\
	}							\
	h = (hash_entry_ptr_t)queue_next(&h->hash_link);	\
    }								\
}
