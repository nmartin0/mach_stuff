/*
 * Mach Operating System
 * Copyright (c) 1993,1992-1987 Carnegie Mellon University
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
 * HISTORY
 * $Log: envprocs.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:47  sclawson
 * New files.
 *
 * Revision 2.3  93/04/14  11:25:30  mrt
 * 	Changed queue.h to be a local file, since Mach kernel shouldn't
 * 	export it any more.
 * 	[92/12/10            mrt]
 * 
 * Revision 2.2  92/04/01  19:09:27  rpd
 * 	Created.
 * 	[92/02/21            jtp]
 * 
 *
 */
/*
 *  File: envmgrmain.c
 *      Environment Manager procedures
 *
 *  Author: Jukka Partanen, Helsinki University of Technology 1992
 *      based on the Mach 2.5 version by Mary Thompson
 *
 */

#include <mach.h>
#include <mach/notify.h>
#include <mach/message.h>
#include <mach_init.h>
#include <mach_error.h>
#include <servers/service.h>
#include <servers/emdefs.h>
#include "queue.h"
#include <stdio.h>

#define HASHTABLESIZE 	32	/* must be a power of 2 */
#define HASHTABLEMASK 	(HASHTABLESIZE -1)
#define PORT_HASH_TABLE_SIZE	500

extern int debug;
extern char *progname;
extern mach_port_t service;	/* our service port */
extern mach_port_t pset;	/* port set we are listening to */
extern mach_port_t notify;	/* dead name notifications are sent here */

typedef enum env_t { envstr, envport} env_t;

typedef union env_var_t {
  env_str_t	str;
  mach_port_t	port;
} env_var_t;

typedef struct hash_entry_t *pHashEntry;

typedef struct hash_entry_t {
  env_name_t	name;  
  int		context;
  env_t		vartype;
  env_var_t	envval;
  pHashEntry	next;
} hash_entry_t;

/* hash table head, one per environment */

typedef struct hash_head_t	*pHashHead;

typedef struct hash_head_t {
  int		refcnt;		/* number of environments using this */
  pHashEntry	 head[HASHTABLESIZE];
} hash_head_t;

typedef struct environment_t {
  union {
    pHashHead			head;
    struct environment_t	*next;
  } list;
} environment_t;

typedef struct hash_port {
    queue_head_t queue;
} hash_port_t;

typedef struct hash_port_entry {
    queue_chain_t chain;
    mach_port_t port;
    queue_head_t env_queue;
    mach_port_urefs_t refs;
} *hash_port_entry_t;

typedef struct env_list {
    queue_chain_t chain;
    environment_t *env;
    char *key;
} *env_list_t;

#define DELTA (int)('A') - (int)('a')

extern char *malloc();
extern void free();

static pHashEntry		HashEntryPool;
static mach_port_array_t	tmp_ports;

static hash_port_t		*hash_port_table = NULL;
static int 			hash_port_table_size;
static environment_t		*free_list = NULL, *bad_list = NULL;

kern_return_t mod_urefs(port, delta)
    mach_port_t port;
    int delta;
{
    kern_return_t kr;

    kr = mach_port_mod_refs(mach_task_self(), port,
			    MACH_PORT_RIGHT_SEND, delta);
    if (kr == KERN_INVALID_RIGHT)
	kr = mach_port_mod_refs(mach_task_self(), port,
				MACH_PORT_RIGHT_DEAD_NAME, delta);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mod_urefs(0x%x, %d): %s\n", progname, port, delta,
	     mach_error_string(kr));
    return kr;
}

kern_return_t port_destroy(port)
    mach_port_t port;
{
    kern_return_t kr;

    kr = mach_port_destroy(mach_task_self(), port);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_port_destroy(0x%x): %s\n",
	     progname, port, mach_error_string(kr));
    return kr;
}

/* Get a pointer to a new environment. If free_list is NULL, this will
   allocate a new page and add all entries into the free list. */

kern_return_t env_get_new_env(env)
    environment_t **env;
{
    kern_return_t kr;
    
    if (!free_list) {
	environment_t *tmp;
	int envs;
	
	kr = vm_allocate(mach_task_self(), (vm_address_t *)&free_list,
			 vm_page_size, TRUE);
	if (kr != KERN_SUCCESS) {
	    if (debug)
		fprintf(stderr, "%s: env_get_new_env: vm_allocate: %s\n",
			progname, mach_error_string(kr));
	    return kr;
	}
	envs = vm_page_size / sizeof(environment_t);
	if (debug)
	    fprintf(stderr,
		    "%s: env_get_new_env: new free list at 0x%x, %d entries\n", 
		    progname, (long)free_list, envs);
	for (tmp = free_list, envs--;
	     envs;
	     envs--, tmp = tmp->list.next) {
	    tmp->list.next = (environment_t *)((char *)tmp +
					       sizeof(environment_t));
	}
	tmp->list.next = NULL;
    }
    *env = free_list;
    free_list = free_list->list.next;
    if (debug)
	fprintf(stderr,
		"%s: env_get_new_env: got env at 0x%x, free_list at 0x%x\n",
		progname, *env, free_list);
    return KERN_SUCCESS;
}

/* Add an environment to the 'bad' memory list */

void env_put_bad_env(env)
    environment_t *env;
{
    if (debug)
	fprintf(stderr, "%s: put_bad_env %x\n", progname, (int)env);
    env->list.next = bad_list;
    bad_list = env;
}

/* Add an environment to the free environment list ('good' memory list). */

void env_put_free_env(env)
    environment_t *env;
{
    env->list.next = free_list;
    free_list = env;
}

environment_t *env_from_port(port, restrict)
    mach_port_t port;
    boolean_t *restrict;
{
    environment_t *env;
    
    *restrict = port & 0x1;
    env = (environment_t *)(port & ~0x1);
    return env;
}

kern_return_t env_allocate(new_env)
    environment_t **new_env;
{
    mach_port_t env_rw_port, env_ro_port;
    kern_return_t kr;
    
    while (1) {
	kr = env_get_new_env(new_env);
	if (kr == KERN_SUCCESS) {
	    if (debug)
		fprintf(stderr, "%s: env_allocate: new env at 0x%x\n",
			progname, *new_env);
	    env_rw_port = (mach_port_t)*new_env;
	    env_ro_port = (mach_port_t)*new_env + 1;
	    kr = mach_port_allocate_name(mach_task_self(),
					 MACH_PORT_RIGHT_RECEIVE,
					 env_rw_port);
	    if (kr != KERN_SUCCESS) {
		if (debug)
		    fprintf(stderr,
			    "%s: env_allocate: mach_port_allocate_name(0x%x): %s\n",
			    progname, env_rw_port,
			    mach_error_string(kr));
		env_put_bad_env(*new_env);
		continue;
	    }
	    kr = mach_port_insert_right(mach_task_self(),
					env_rw_port, env_rw_port,
					MACH_MSG_TYPE_MAKE_SEND);
	    if (kr != KERN_SUCCESS) {
		if (debug)
		    fprintf(stderr, "%s: mach_port_insert_right(0x%x): %s.\n",
			    progname, env_rw_port,
			    mach_error_string(kr));
		port_destroy(env_rw_port);
		env_put_bad_env(*new_env);
		continue;
	    }
	    kr = mach_port_allocate_name(mach_task_self(),
					 MACH_PORT_RIGHT_RECEIVE,
					 env_ro_port);
	    if (kr != KERN_SUCCESS) {
		if (debug)
		    fprintf(stderr,
		    "%s: env_allocate: mach_port_allocate_name(0x%x): %s\n",
			    progname, env_ro_port,
			    mach_error_string(kr));
		port_destroy(env_rw_port);
		env_put_bad_env(*new_env);
		continue;
	    }
	    kr = mach_port_insert_right(mach_task_self(),
					env_ro_port, env_ro_port,
					MACH_MSG_TYPE_MAKE_SEND);
	    if (kr != KERN_SUCCESS) {
		if (debug)
		    fprintf(stderr,
		    "%s: env_allocate: mach_port_insert_right(0x%x): %s.\n",
			    progname, env_ro_port,
			    mach_error_string(kr));
		env_deallocate(*new_env);
		continue;
	    }
	    break;	/* ok, no need to continue */
	}
    }
    return kr;
}

kern_return_t env_deallocate(env)
    environment_t *env;
{
    env_put_bad_env(env);
    (void)mach_port_destroy(mach_task_self(), (mach_port_t)env);
    (void)mach_port_destroy(mach_task_self(), (mach_port_t)env + 1);
    return KERN_SUCCESS;
}

int hash_port(port)
    mach_port_t port;
{
    return (((int)port >> 8) + ((int)port & 0xff)) % hash_port_table_size;
}

#define HASH_PORT(port) \
    (((int)(port) >> 8) + ((int)(port) & 0xff) % hash_port_table_size)

hash_port_entry_t hash_port_lookup(port)
    mach_port_t port;
{
    register hash_port_entry_t pe;
    register hash_port_t *b;
    
    b = &hash_port_table[HASH_PORT(port)];
    for (pe = (hash_port_entry_t)queue_first(&b->queue);
	 !queue_end(&b->queue, (queue_entry_t)pe);
	 pe = (hash_port_entry_t)queue_next(&pe->chain))
    {
	if (pe->port == port)
	    return pe;
    }
    return NULL;
}

kern_return_t hash_port_add(port, env, key)
    mach_port_t port;
    environment_t *env;
    char *key;
{
    register hash_port_entry_t pe;
    register env_list_t entry;
    
    pe = hash_port_lookup(port);
    if (pe == NULL) {
	register hash_port_t *b;
	
	if (debug)
	    fprintf(stderr,
		    "%s: hash_port_add: adding env/key 0x%x/%s to new port 0x%x\n",
		    progname, env, key, port);
	pe = (hash_port_entry_t)malloc(sizeof(struct hash_port_entry));
	if (pe == NULL) {
	    return KERN_RESOURCE_SHORTAGE;
	}
	queue_init(&pe->env_queue);
	pe->port = port;
	pe->refs = 0;
	entry = (env_list_t)malloc(sizeof(struct env_list));
	if (entry == NULL) {
	    free(pe);
	    return KERN_RESOURCE_SHORTAGE;
	}
	entry->env = env;
	entry->key = key;
	queue_enter(&pe->env_queue, entry, env_list_t, chain);
	b = &hash_port_table[HASH_PORT(port)];
	queue_enter(&b->queue, pe, hash_port_entry_t, chain);
    } else {
	if (debug)
	    fprintf(stderr,
		    "%s: hash_port_add: adding env/key 0x%x/%s to port 0x%x\n",
		    progname, env, key, port);
	entry = (env_list_t)malloc(sizeof(struct env_list));
	if (entry == NULL) {
	    return KERN_RESOURCE_SHORTAGE;
	}
	entry->env = env;
	entry->key = key;
	queue_enter(&pe->env_queue, entry, env_list_t, chain);
    }
    pe->refs++;
    return KERN_SUCCESS;
}

kern_return_t hash_port_del(port, env, key)
    mach_port_t port;
    environment_t *env;
    char *key;
{
    register hash_port_entry_t pe, p;
    register env_list_t el;
    
    pe = hash_port_lookup(port);
    if (pe == NULL) {
	return KERN_FAILURE;
    }
    if (debug)
	fprintf(stderr, "%s: hash_port_del(0x%x, 0x%x, %s)\n",
		progname, port, env, key);
    for (el = (env_list_t)queue_first(&pe->env_queue);
	 !queue_end(&pe->env_queue, (queue_entry_t)el);
	 el = (env_list_t)queue_next(&el->chain)) {
	if (debug)
	    fprintf(stderr, "%s: hash_port_del, comparing to 0x%x/%s\n",
		    progname, el->env, el->key);
	if (env == el->env && key == el->key) {
	    if (debug)
		fprintf(stderr,
		"%s: hash_port_del: removing env/key 0x%x/%s from port 0x%x\n",
			progname, env, key, port);
	    queue_remove(&pe->env_queue, el, env_list_t, chain);
	    free(el);
	    pe->refs--;
	    if (pe->refs == 0) {
		hash_port_t *b;
		
		if (debug)
		    fprintf(stderr,
"%s: hash_port_del: no env/key list for 0x%x, removing from hash chains\n",
			    progname, port);
		b = &hash_port_table[HASH_PORT(port)];
		queue_remove(&b->queue, pe,
			     hash_port_entry_t, chain);
		free(pe);
	    }
	    return KERN_SUCCESS;
	}
    }
    return KERN_FAILURE;
}

/***********************************************************
 * function StrEqCase (S1,S2) 
 *
 * Abstract:
 *   Compare an Upper cased string (S1) with a mixed case string (S2)
 *   case INsensitively.
 *
 * Design:
 *   Since we don't modify our arguments, make them var.  This also
 *   allows our locals to be referenced more quickly.  It is actually
 *   a lot slower if they aren't vars.  -dwp
 ************************************************************/

boolean_t StrEqCase (S1, S2)
    char  *S1, *S2;
{
    int  i;
    char tc;
    
    /* do-it-yourself strlen in case user hands in garbage */
    for (i = 0; i < env_name_size; i++ )
	if (S1[i] == '\0')
	    break;
    
    if (strlen(S2) != i)
	return FALSE;
    
    if (i > 0)
	do {
	    tc = S2[i];
	    if (tc >= 'a' && tc <= 'z')
		tc = (char)((int)(tc) + DELTA);
	    if (S1[i] != tc)
		return FALSE;
	    i--;
	} while (i);
    
    return TRUE;
}


/***********************************************************
 * ConvUpper (S) 
 *
 * Abstract:
 *   Convert a string to all upper-case letters
 *
 ************************************************************/

void ConvUpper(S)
    char  *S;
{
    int  i;
    char tc;
    
    for (i = 0; S[i] != '\0'; i++) {
	tc = S[i];
	if (tc >= 'a' && tc <= 'z') {
	    tc = (char)((int)tc + DELTA);
	    S[i] = tc;
	}
    } 
}


/****************************************************************
 * function NewHashEntry  
 *
 * Abstract:
 *   Allocate a new HashEntry.
 * Returns:
 *   Pointer to a new HashEntry record.
 ****************************************************************/

pHashEntry NewHashEntry ()
{
    pHashEntry result;
    
    if (HashEntryPool == (pHashEntry)(NULL))
	result = (pHashEntry)malloc(sizeof(hash_entry_t));
    else {
	result = HashEntryPool;
	HashEntryPool = HashEntryPool->next;
    }
    return result;
}

/****************************************************************
 * procedure FreeHashEntry (elem) 
 *
 * Abstract:
 *   Frees a HashEntry record.  Returns it to the pool of free HashEntry
 *   records.
 * Parameters:
 *   elem        pointer to the record to be freed.
 ****************************************************************/

FreeHashEntry (elem)
    pHashEntry elem;
{
    elem->next = HashEntryPool;
    HashEntryPool = elem;
}

/****************************************************************
 * function Hash (s) 
 *
 * Abstract:
 *   Hash function for storing env var
 * Parameters:
 *   S:   name of the env var to hash
 * Returns:
 *   Hash value
 ****************************************************************/

int Hash (S)
    char  *S;
{
    int i, n;
    
    n = 0;
    for ( i = 0; S[i] != '\0'; i++ )
	n = n + (int)(S[i]);
    
    /* Hash := n mod HASHTABLESIZE */
    
    return (n % HASHTABLEMASK);
}

/****************************************************************
 * function FindHashEntry (s, env) 
 *
 * Abstract:
 *   Finds hash entry
 * Parameters:
 *   S:   name of the env var to find
 *   env: environment
 * Returns:
 *   Pointer to hash entry for name, or NULL if not found
 ****************************************************************/
pHashEntry FindHashEntry (_S, env)
    env_name_t  _S;
    environment_t *env;
{
    env_name_t S;
    pHashEntry entry;
    int N;
    
    strcpy(S,_S);
    ConvUpper(S);
    
    N = Hash(S);
    
    if (debug)
	fprintf(stderr, "%s: FindHashEntry, string hashes to %d [%s].\n",
		progname, N, S);
    
    entry = env->list.head->head[N]; /* head of list */
    while (entry != (pHashEntry)(NULL)) {
	if (debug)
	    fprintf(stderr,
		    "%s: FindHashEntry: comparing to 0x%x  [%s].\n",
		    progname, entry, entry->name);
	if (StrEqCase(S,entry->name))
	    return entry;
	entry = entry->next;
    }
    return entry;  /* NULL */
}

/****************************************************************
 * function CreateHashEntry (s, env) 
 *
 * Abstract:
 *   Creates internal env_var record.
 * Parameters:
 *   S:   name of the env_var to create
 *   env: environment
 * Returns:
 *   Pointer to new hash entry for env var
 * Side Effects:
 *   New entry added to hash table for environment env.
 ****************************************************************/

pHashEntry CreateHashEntry (_S, env)
    env_name_t  _S;
    environment_t *env;
{
    env_name_t S;
    pHashEntry Entry;
    int N;
    
    strcpy(S, _S);
    ConvUpper(S);
    
    N = Hash(S);
    
    Entry = NewHashEntry();
    Entry->next = env->list.head->head[N];
    env->list.head->head[N] = Entry;
    return Entry;
}

/***************************************************************
 * function DeleteHashEntry (s, env, vt) 
 *
 * Abstract:
 *   Deletes internal env var record.
 * Parameters:
 *   s:   name of the env var record to delete
 *   env: pointer to the environment
 *   vt;  type of variable to be deleted
 * Side Effects:
 *   Hash entry deleted, if entry is for a string, the string is freed.
 ***************************************************************/

int DeleteHashEntry (_S, env, vt)
    env_name_t  _S;
    environment_t *env;
    env_t vt;
{
    env_name_t S;
    pHashEntry entry, prev;
    int N;
    kern_return_t kr;
    
    strcpy(S, _S);
    ConvUpper(S);
    
    N = Hash(S);
    entry = env->list.head->head[N]; /* head of list */
    if (entry == (pHashEntry)NULL)
	return ENV_VAR_NOT_FOUND;
    if (StrEqCase(S, entry->name)) {
	if (entry->vartype == vt)
	    env->list.head->head[N] = entry->next;
	else
	    return ENV_WRONG_VAR_TYPE;
    } else {
	do {
	    prev = entry;
	    entry = entry->next;
	    if (entry == (pHashEntry)(NULL))
		return ENV_VAR_NOT_FOUND; /* entry not there */
	} while (!(StrEqCase(S, entry->name)));
	if (entry->vartype != vt)
	    return ENV_WRONG_VAR_TYPE;
	prev->next = entry->next;
    }
    
    /* Entry found and removed from hash table chains */
    if (entry->vartype == envstr && entry->envval.str) {
	if (debug)
	    fprintf(stderr, "%s: DeleteHashEntry: freeing %d bytes.\n",
		    progname, strlen(entry->envval.str) + 1);
	free(entry->envval.str);
    }
    FreeHashEntry(entry);
    return ENV_SUCCESS;
}

/***************************************************************
 * procedure CopyHashTable (env) 
 *
 * Abstract:
 *   Copies all entries (and the hash table itself) for a  context from the
 *   hash table it was using to a new hash table.
 * Parameters:
 *   env   environment whose hash table is to be copied.
 * Design:
 *   This copy is done when a set_env_var is about to occur on
 *   a context whose refcnt is greater than 1. This is a lazy
 *   evaluation of env_copy_conn.
 ***************************************************************/

CopyHashTable (env)
    environment_t *env;
{
    int i;
    pHashEntry Entry, NewEntry;
    pHashEntry PrevEntry = (pHashEntry)(NULL);
    pHashHead OldHead;
    
    if (debug)
	fprintf(stderr, "%s: copying hash table for env 0x%x\n",
		progname, env);
    
    OldHead = env->list.head;
    OldHead->refcnt--;
    
    env->list.head = (pHashHead)malloc(sizeof(hash_head_t));
    
    env->list.head->refcnt = 1;
    for (i = 0; i < HASHTABLESIZE; i++) {
	/* copy old variables */
	Entry = OldHead->head[i];
	env->list.head->head[i] = (pHashEntry)(NULL);
	while (Entry != (pHashEntry)(NULL)) {
	    NewEntry = NewHashEntry();
	    if (env->list.head->head[i] == (pHashEntry)(NULL))
		env->list.head->head[i] = NewEntry;
	    else 
		PrevEntry->next = NewEntry;
	    *NewEntry = *Entry;
	    if (Entry->vartype == envport) {
		hash_port_add(NewEntry->envval.port,
			      env, NewEntry->name);
		mod_urefs(Entry->envval.port, 1);
	    } else {
		if (debug)
		    fprintf(stderr, "%s: CopyHashTable: malloc(%d)\n",
			    progname, strlen(Entry->envval.str) + 1);
		NewEntry->envval.str = malloc(strlen(Entry->envval.str) + 1);
		strcpy(NewEntry->envval.str, Entry->envval.str);
	    }
	    NewEntry->next = (pHashEntry)(NULL);
	    PrevEntry = NewEntry;
	    Entry = Entry->next;
	}
    }
}

/******************************************************
 * Abstract:
 *	Asks the Service server for the environment_port
 *	Creates a new empty context and associates with
 * 	this port. If Service server won't give us the
 *	enviroment_port, it creates a new port and context.
 *  Parameters:
 *	KnownEnvPort	returns a port to the new copy of the context
 *  Returns:
 *	ENV_SUCCESS
 ******************************************************/
kern_return_t initial_conn(known_env_port)
    mach_port_t *known_env_port;
{
#define ENV_RETRYCNT	5
    int		i;
    pHashHead	HashHead;
    kern_return_t kr;
    mach_port_t	new_env_port, ro_port;
    
    /* First, ask the service server if there is
       already an established port. */
    if ((service_checkin(service_port, environment_port,
			 &new_env_port) == KERN_SUCCESS) &&
	(environment_port == new_env_port)) {
	environment_t *global_env;
	
	for (i = 0; i < ENV_RETRYCNT; i++) {
	    kr = env_get_new_env(&global_env);
	    if (kr != KERN_SUCCESS)
		return kr;
	    kr = mach_port_rename(mach_task_self(), new_env_port,
				  (mach_port_t)global_env);
	    if (kr != KERN_SUCCESS) {
		fprintf(stderr,
			"%s: initial_conn: mach_port_rename: %s, retrying.\n",
			progname, mach_error_string(kr));
		env_put_bad_env(global_env);
		continue;
	    }
	    new_env_port = (mach_port_t)global_env;
	    ro_port = new_env_port + 1;
	    kr = mach_port_allocate_name(mach_task_self(),
					 MACH_PORT_RIGHT_RECEIVE,
					 ro_port);
	    if (kr != KERN_SUCCESS) {
		if (debug)
		    fprintf(stderr,
		    "%s: initial_conn: mach_port_allocate_name: %s, retrying.\n",
			    progname, mach_error_string(kr));
		env_put_bad_env(global_env);
		continue;
	    }
	    kr = mach_port_insert_right(mach_task_self(),
					ro_port, ro_port,
					MACH_MSG_TYPE_MAKE_SEND);
	    if (kr != KERN_SUCCESS) {
		if (debug)
		    fprintf(stderr,
		    "%s: initial_conn: mach_port_insert_right(0x%x): %s.\n",
			    progname, ro_port,
			    mach_error_string(kr));
		port_destroy(ro_port);
		env_put_bad_env(global_env);
		continue;
	    }
	    break;	/* everything is ok, don't retry */
	}
	if (i == ENV_RETRYCNT) {
	    fprintf(stderr,
		    "%s: initial_conn: couldn't allocate global env.\n",
		    progname);
	    return KERN_FAILURE;
	}
	if (debug)
	    fprintf(stderr, "%s: initial_conn: global env at 0x%x\n",
		    progname, global_env);
	*known_env_port = new_env_port;
	
	/* create new, empty hash table */
	HashHead =  (pHashHead)malloc(sizeof(hash_head_t));
	if (HashHead == NULL) {
	    quit(1, "malloc(HashHead): out of memory\n");
	}
	HashHead->refcnt = 1;
	for (i = 0; i < HASHTABLESIZE; i++)
	    HashHead->head[i] = NULL;
	global_env->list.head = HashHead;
    } else {
	kr = KERN_FAILURE;
	*known_env_port = MACH_PORT_NULL;
    }
    return kr;
}

/************************************************************
 *	procedure  env_init (knownenvport) 
 *
 *  Abstract
 *	initializes the env_mgr tables, creates an empty
 *	environment, and returns the port to it.
 ************************************************************/
void env_init (known_env_port, hashtablesize)
    mach_port_t *known_env_port;
    int hashtablesize;
{
    int i;
    kern_return_t kr; 

    HashEntryPool = (pHashEntry)(NULL);
    
    kr = vm_allocate(mach_task_self(), (vm_address_t *)&hash_port_table,
		     hashtablesize * sizeof(*hash_port_table), TRUE);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: env_init: vm_allocate(port_hash_table): %s\n",
	     progname, mach_error_string(kr));
    
    hash_port_table_size = hashtablesize;
    
    for (i = 0; i < hash_port_table_size; i++)
	queue_init(&hash_port_table[i].queue);
#if 0
    /* put the standard ports in the port table so that they
       wont't be deallocated if some client removes them */
    /* XXX */
    j = add_port(environment_port);
    j = add_port(name_server_port);
    j = add_port(service_port);
#endif

    kr = initial_conn(known_env_port);
}

/******************************************************
 *  Abstract:
 *	Define a value for a string environment variable
 *	in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the environment to use
 *	env_name	name of env variable to be set
 *	env_val		value of env_name
 *  Returns:
 *	ENV_SUCCESS
 *	ENV_READ_ONLY	serv_port only allows reading of
 *			the environment.
 ******************************************************/
kern_return_t do_env_set_string (serv_port, env_name, env_val)
    mach_port_t	serv_port;
    env_name_t	env_name;
    env_str_val_t env_val;
{
    pHashEntry	entry;
    boolean_t	restrict;
    environment_t *env;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    
    if (restrict)
	return ENV_READ_ONLY;
    
    if (env->list.head->refcnt > 1)
	CopyHashTable(env);
    
    entry = FindHashEntry(env_name, env);
    if (entry != NULL) {
	if (entry->vartype != envstr)
	    return ENV_WRONG_VAR_TYPE;
	free(entry->envval.str);
    } else {
	entry = CreateHashEntry(env_name, env);
	strcpy(entry->name, env_name);
	entry->vartype = envstr;
    }
    if (debug)
	fprintf(stderr, "%s: do_env_set_string: malloc(%d)\n",
	       progname, strlen(env_val) + 1);
    entry->envval.str = malloc(strlen(env_val) + 1);
    if (entry->envval.str == NULL) {
	DeleteHashEntry(env_name, env);
	return KERN_RESOURCE_SHORTAGE;
    }
    strcpy(entry->envval.str, env_val);
    return ENV_SUCCESS;
}

/******************************************************
 *  Abstract:
 *	Return the value of a string environment variable
 *	in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the context to use
 *	env_name	name of env variable to be found
 *	env_val		returned pointing to value of env_name
 *  Returns:
 *	ENV_SUCCESS
 *	WrongEnvVarType	 env_name was found but was 
 *			 not of type env_str.
 *	EnvVariableNotFound  env_name not found.
 ******************************************************/
kern_return_t	do_env_get_string (serv_port, env_name, env_val)
    mach_port_t	serv_port;
    env_name_t	env_name;
    env_str_val_t	env_val;
{
    environment_t *env;
    pHashEntry	entry;
    boolean_t	restrict;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    
    strcpy(env_val, "");   /* in case of error */
    entry = FindHashEntry(env_name, env);
    if (entry == NULL)
	return ENV_VAR_NOT_FOUND;
    
    if (entry->vartype != envstr)
	return ENV_WRONG_VAR_TYPE;
    
    strcpy(env_val, entry->envval.str);
    return ENV_SUCCESS;
}

/******************************************************
 *  Abstract:
 *	Remove a string variable from the context specified
 *	by serv_port
 *  Parameters:
 * 	serv_port	port identifying the context to use
 *	env_name	name of env variable to be deleted
 *  Returns:
 *	ENV_SUCCESS
 *	WrongEnvVarType	 env_name was found but was 
 *			 not of type env_str.
 *	EnvVariableNotFound  env_name not found.
 *	ENV_READ_ONLY	serv_port only allows reading of
 *			the environment.
 ******************************************************/
kern_return_t	do_env_del_string (serv_port, env_name)
    mach_port_t	serv_port;
    env_name_t	env_name;
{
    environment_t *env;
    boolean_t	restrict;
    int		retcode;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    if (restrict)
	return ENV_READ_ONLY;
    if (env->list.head->refcnt > 1)
	CopyHashTable(env);
    
    retcode = DeleteHashEntry(env_name, env, envstr);
    
    return (kern_return_t)retcode;
}

/******************************************************
 *  Abstract:
 *	Define a value for a port environment variable
 *	in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the context to use
 *	env_name	name of env variable to be set
 *	env_val		value of env_name
 *  Returns:
 *	ENV_SUCCESS
 *	ENV_READ_ONLY	serv_port only allows reading of
 *			the environment.
 *	ENV_PORT_NULL 	Attempting to enter a null port.
 ******************************************************/
kern_return_t	do_env_set_port (serv_port, env_name, env_val)
    mach_port_t	serv_port;
    env_name_t	env_name;
    mach_port_t	env_val;
{
    environment_t *env;
    int bucket;
    pHashEntry entry;
    boolean_t restrict;
    kern_return_t kr;
    mach_port_t	previous;
    hash_port_entry_t pe;
    
    if (env_val == MACH_PORT_NULL)
	return ENV_PORT_NULL;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    if (restrict)
	return ENV_READ_ONLY;
    
    if (env->list.head->refcnt > 1)
	CopyHashTable(env);
    
    entry = FindHashEntry(env_name, env);
    
    if (entry != NULL) {
	/* an entry already exists */
	if (debug)
	    fprintf(stderr, 
		    "%s: do_env_set_port: a hash entry already exists.\n",
		    progname);
	if (entry->vartype != envport)
	    return ENV_WRONG_VAR_TYPE;
	if (entry->envval.port == env_val) {
	    /* consume the extra ref */
	    mod_urefs(env_val, -1);
	    return ENV_SUCCESS;
	}
	
	kr = hash_port_del(entry->envval.port, env, entry->name);
	if (kr != KERN_SUCCESS)
	    return kr;
	mod_urefs(entry->envval.port, -1);
	entry->envval.port = env_val;
    } else {
	/* no previous port */
	if (debug)
	    fprintf(stderr,
		    "%s: env_set_port: creating new hash entry.\n",
		    progname);
	entry = CreateHashEntry(env_name, env);
	strcpy(entry->name, env_name);
	entry->vartype = envport;
	entry->envval.port = env_val;
    }

    pe = hash_port_lookup(env_val);
    /* if the port is not in the hash chains already 
       ie. a notification has not already been requested */
    if (!pe) {
	if (debug)
	    fprintf(stderr,
		    "%s: do_env_set_port: requesting notification (0x%x)\n", 
		    progname, env_val);
	kr = mach_port_request_notification(mach_task_self(), env_val,
					    MACH_NOTIFY_DEAD_NAME, TRUE,
					    notify,
					    MACH_MSG_TYPE_MAKE_SEND_ONCE,
					    &previous);
	if ((kr != KERN_SUCCESS) || (previous != MACH_PORT_NULL))
	    quit(1,
		 "do_env_set_port: mach_port_request_notification: %s\n", 
		 mach_error_string(kr));
    }
    kr = hash_port_add(env_val, env, entry->name);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: do_env_set_port: hash_port_add: %s\n",
	     progname, mach_error_string(kr));
    return ENV_SUCCESS;
}

/******************************************************
 *  Abstract:
 *	Return the value of a port environment variable
 *	in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the context to use
 *	env_name	name of env variable to be found
 *	env_val		returned pointing to value of env_name
 *  Returns:
 *	ENV_SUCCESS
 *	WrongEnvVarType	 env_name was found but was 
 *			 not of type mach_port_t
 *	EnvVariableNotFound  env_name not found.
 ******************************************************/
kern_return_t	do_env_get_port (serv_port, env_name, env_val)
    mach_port_t	serv_port;
    env_name_t	env_name;
    mach_port_t	*env_val;
{
    environment_t *env;
    pHashEntry	entry;
    boolean_t	restrict;
    kern_return_t	kr;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    
    *env_val = MACH_PORT_NULL;   /* in case of error */
    entry = FindHashEntry(env_name, env);
    if (entry == NULL)
	return ENV_VAR_NOT_FOUND;
    
    if (entry->vartype != envport)
	return ENV_WRONG_VAR_TYPE;
    
    *env_val = entry->envval.port;
    
    return ENV_SUCCESS;
}

/******************************************************
 *  Abstract:
 *	Remove a port variable from the context specified
 *	by serv_port
 *  Parameters:
 * 	serv_port	port identifying the context to use
 *	env_name	name of env variable to be deleted
 *  Returns:
 *	ENV_SUCCESS
 *	WrongEnvVarType	 env_name was found but was 
 *			 not of type mach_port_t
 *	EnvVariableNotFound  env_name not found.
 *	ENV_READ_ONLY	serv_port only allows reading of
 *			the environment.
 ******************************************************/
kern_return_t	do_env_del_port (serv_port, env_name)
    mach_port_t	serv_port;
    env_name_t	env_name;
{
    environment_t *env;
    boolean_t restrict;
    kern_return_t kr;
    hash_entry_t *entry;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL) {
	if (debug)
	    fprintf(stderr, "%s: do_env_del_port: 0x%x is unknown!\n",
		    progname, serv_port);
	return ENV_UNKNOWN_PORT;
    }
    
    if (restrict) {
	if (debug)
	    fprintf(stderr, "%s: do_env_del_port: 0x%x is read only\n",
		    progname, serv_port);
	return ENV_READ_ONLY;
    }
    
    entry = FindHashEntry(env_name, env);
    if (entry == NULL)
	return ENV_VAR_NOT_FOUND;
	
    if (entry->vartype != envport)
	return ENV_WRONG_VAR_TYPE;
    
    if (env->list.head->refcnt > 1)
	CopyHashTable(env);
    
    hash_port_del(entry->envval.port, env, entry->name);
    mod_urefs(entry->envval.port, -1);
    DeleteHashEntry(env_name, env, envport);
    
    return KERN_SUCCESS;
}

/******************************************************
 *  Abstract:
 *	Return the names and values of all the String
 *	variables in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the context to use
 *	env_names	returned pointing to a list of env variable names
 *	env_names_Cnt	number of names returned
 *	env_string_val	values of the names returned
 *	env_string_vals_Cnt	same as env_names_Cnt
 *  Returns;
 *	ENV_SUCCESS
 *
 ******************************************************/
kern_return_t	do_env_list_strings (serv_port, env_names, env_names_Cnt,
				     env_string_vals, env_string_vals_Cnt)
    mach_port_t		serv_port;
    env_name_list	*env_names;
    int			*env_names_Cnt;
    env_str_list	*env_string_vals;
    int			*env_string_vals_Cnt;
{
    environment_t	*env;
    pHashEntry		entry;
    boolean_t		restrict;
    int			hi, vc;
    kern_return_t	kr;
    env_name_list	tmp_env_names; /* pointer to names */
    env_str_list	tmp_env_strings; /* pointer to strings */
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    
    kr = ENV_SUCCESS;
    vc = 0;			/* count the number of string vars */
    for (hi = 0; hi < HASHTABLESIZE; hi++) {
	entry = env->list.head->head[hi];
	while (entry != NULL) {
	    if (entry->vartype == envstr)
		vc++;
	    entry = entry->next;
	}
    }			/* end counting loop */
    
    tmp_env_names = NULL;
    tmp_env_strings = NULL;
    
    if (vc == 0)
	goto ex1;
    
    kr = vm_allocate(mach_task_self(), (vm_address_t *)&tmp_env_names,
		     vc*sizeof(env_name_t), TRUE);
    if (kr != KERN_SUCCESS) {
	if (debug)
	    fprintf(stderr, "%s: vm_allocate(names): %s\n", progname,
		    mach_error_string(kr));
	vc = 0;
	goto ex1;
    }
    
    kr = vm_allocate(mach_task_self(), (vm_address_t *)&tmp_env_strings,
		     vc*sizeof(env_str_val_t), TRUE);
    if (kr != KERN_SUCCESS) {
	if (debug)
	    fprintf(stderr, "%s: vm_allocate(strings): %s\n", progname,
		    mach_error_string(kr));
	vc = 0;
	goto ex1;
    }
    vc = 0;
    
    for (hi = 0; hi < HASHTABLESIZE; hi++) {
	entry = env->list.head->head[hi];
	while (entry != NULL) {
	    if (entry->vartype == envstr) {
		strcpy(tmp_env_names[vc], entry->name);
		strcpy(tmp_env_strings[vc], entry->envval.str);
		vc++;
	    }
	    entry = entry->next;
	}
    }			/* loop over all hash table entries */
 ex1:
    
    *env_names = tmp_env_names;
    *env_string_vals = tmp_env_strings;
    *env_names_Cnt = vc;
    *env_string_vals_Cnt = vc;
    
    return kr;
}

/******************************************************
 *  Abstract:
 *	Return the names and values of all the port
 *	variables in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the context to use
 *	env_names	returned pointing to a list of env variable names
 *	env_names_Cnt	number of names returned
 *	env_port_vals	values of the names returned
 *	env_port_vals_Cnt	same as env_names_Cnt
 *  Returns;
 *	ENV_SUCCESS
 *
 *
 ******************************************************/
kern_return_t	do_env_list_ports (serv_port, env_names, env_names_Cnt,
				   env_port_vals, env_port_vals_Cnt)
    mach_port_t	serv_port;
    env_name_list	*env_names;
    int		*env_names_Cnt;
    mach_port_array_t	*env_port_vals;
    int		*env_port_vals_Cnt;
{
    environment_t	*env;
    pHashEntry	entry, nextentry;
    boolean_t	restrict;
    int		hi, vc;
    kern_return_t	kr;
    env_name_list	tmp_env_names; /* pointer to names */
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    
    kr = ENV_SUCCESS;
    vc = 0;			/* count the number of port vars */
    for (hi = 0; hi < HASHTABLESIZE; hi++) {
	entry = env->list.head->head[hi];
	while (entry != NULL) {
	    if (entry->vartype == envport) {
		vc++;
	    }
	    entry = entry->next;
	}
    }
    
    tmp_env_names = NULL;
    if (vc == 0) {
	*env_port_vals = NULL;
	goto ex2;
    }
    
    kr = vm_allocate(mach_task_self(), (vm_address_t *)&tmp_ports,
		     vc*sizeof(mach_port_t), TRUE);
    if (kr != KERN_SUCCESS) {
	if (debug)
	    fprintf(stderr, "%s: env_list_ports: vm_allocate: %s\n",
		    progname, mach_error_string(kr));
	vc = 0;
	goto ex2;
    }
    
    kr = vm_allocate(mach_task_self(), (vm_address_t *)&tmp_env_names,
		     vc*sizeof(env_name_t), TRUE);
    if (kr != KERN_SUCCESS)	{
	if (debug)
	    fprintf(stderr, "%s: env_list_ports: vm_allocate: %s\n",
		    progname, mach_error_string(kr));
	vc = 0;
	goto ex2;
    }
    
    vc = 0;
    for (hi = 0; hi < HASHTABLESIZE; hi++) {
	entry = env->list.head->head[hi];
	while (entry != NULL) {
	    if (entry->vartype == envport) {
		strcpy(tmp_env_names[vc], entry->name);
		tmp_ports[vc] = entry->envval.port;
		vc++;
	    }
	    entry = entry->next;
	}
    }
    *env_port_vals = tmp_ports;
    
 ex2:
    *env_names = tmp_env_names;
    *env_names_Cnt = vc;
    *env_port_vals_Cnt = vc;
    
    return kr;
}

/******************************************************
 *  Abstract:
 *	Enters a set of string environment
 *	variables in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the context to use
 *	env_names	a list of env variable names
 *	env_names_Cnt	number of names
 *	env_string_val	values of the names
 *	env_string_vals_Cnt	same as env_names_Cnt
 *  Returns;
 *	ENV_SUCCESS
 *	ENV_READ_ONLY	serv_port only allows reading of
 *			the environment.
 *  Side effects;
 *	vm_deallocates the list of name and string values
 ******************************************************/
kern_return_t	do_env_set_stlist (serv_port, env_names, env_names_Cnt,
				   env_string_vals, env_string_vals_Cnt)
    mach_port_t	serv_port;
    env_name_list env_names;
    int		env_names_Cnt;
    env_str_list env_string_vals;
    int		env_string_vals_Cnt;
{
    int vc;
    kern_return_t kr;
    
    for (vc = 0; vc < env_names_Cnt; vc++) { 
	kr = do_env_set_string(serv_port, env_names[vc],
			       env_string_vals[vc]);
	if (kr != ENV_SUCCESS)
	    break;
    }
    
    (void)vm_deallocate(mach_task_self(), (vm_address_t)env_names,
			env_names_Cnt * sizeof(env_name_t));
    (void)vm_deallocate(mach_task_self(), (vm_address_t)env_string_vals,
			env_string_vals_Cnt * sizeof(env_str_val_t));
    return kr;
}

/******************************************************
 *  Abstract:
 *	Return the names and values of all the port
 *	variables in the context specified by serv_port
 *  Parameters:
 *	serv_port	port identifying the context to use
 *	env_names	a list of env variable names
 *	env_names_Cnt	number of names
 *	env_port_vals	values of the names 
 *	env_port_vals_Cnt	same as env_names_Cnt
 *  Returns;
 *	ENV_SUCCESS
 *	ENV_READ_ONLY	serv_port only allows reading of
 *			the environment.
 ******************************************************/
kern_return_t	do_env_set_ptlist (serv_port, env_names, env_names_Cnt,
				   env_port_vals, env_port_vals_Cnt)
    mach_port_t	serv_port;
    env_name_list	env_names;
    int		env_names_Cnt;
    mach_port_array_t	env_port_vals;
    int		env_port_vals_Cnt;
{
    int vc;
    kern_return_t kr;
    
    for (vc = 0; vc < env_names_Cnt; vc++) { 
	kr = do_env_set_port(serv_port, env_names[vc],
			     env_port_vals[vc]);
	if (kr != ENV_SUCCESS)
	    break;
    }
    (void)vm_deallocate(mach_task_self(), (vm_address_t)env_names,
			env_names_Cnt*sizeof(env_name_t));
    (void)vm_deallocate(mach_task_self(), (vm_address_t)env_port_vals,
			env_port_vals_Cnt*sizeof(mach_port_t));
    return kr;
}


/******************************************************
 * Abstract:
 *	Copies the context specified by serv_port and
 * 	allocates a  new port which will be used to
 * 	reference the new context.
 *  Parameters:
 *	serv_port	port identifying the context to copy
 *	new_env_port	returns a port to the new copy of the context
 *  Returns:
 *	ENV_SUCCESS
 *  Design:
 *	The context is marked as having been copied, but
 *	the actual copying is done only when a variable
 *	in one of the two logical copies is changed.
 *	This is important since the shell will normally
 *	pass a copy of its environment to any task it
 *	starts and normally the child task will never
 *	modify its environment.
 ******************************************************/
kern_return_t	do_env_copy_conn (serv_port, new_env_port)
    mach_port_t	serv_port;
    mach_port_t	*new_env_port;
{
    environment_t	*env, *new_env;
    boolean_t	restrict;
    kern_return_t	kr;
    mach_port_t	previous;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    
    kr = env_allocate(&new_env);
    if (kr != KERN_SUCCESS)
	return kr;
    
    *new_env_port = (mach_port_t)new_env;
    kr = mach_port_request_notification(mach_task_self(),
					*new_env_port,
					MACH_NOTIFY_NO_SENDERS,
					0,
					*new_env_port,
					MACH_MSG_TYPE_MAKE_SEND_ONCE,
					&previous);
    if ((kr != KERN_SUCCESS) || (previous != MACH_PORT_NULL)) {
	if (debug)
	    fprintf(stderr, "%s: mach_port_request_notification: %s\n",
		    progname, mach_error_string(kr));
	env_deallocate(new_env);
	return KERN_FAILURE;
    }
    
    kr = mach_port_move_member(mach_task_self(), *new_env_port, pset);
    if (kr != KERN_SUCCESS)	{
	env_deallocate(new_env);
	return kr;
    }
    
    if (debug) {
	fprintf(stderr, "%s: env_copy_conn: new ports 0x%x 0x%x\n",
		progname, (int)*new_env_port, (int)*new_env_port + 1);
    }
    
    env->list.head->refcnt++;
    new_env->list.head = env->list.head;
    
    return ENV_SUCCESS;
}

/******************************************************
 * Abstract:
 *	Creates a new empty context
 * 	allocates a  new port which will be used to
 * 	reference the new context.
 *  Parameters:
 *	serv_port	not used
 *	new_env_port	returns a port to the new copy of the context
 *  Returns:
 *	ENV_SUCCESS
 ******************************************************/
kern_return_t	do_env_new_conn (serv_port, new_env_port)
    mach_port_t	serv_port;
    mach_port_t	*new_env_port;
{
    environment_t	*env;
    int		i;
    pHashHead	HashHead;
    kern_return_t	kr;
    mach_port_t	previous;
    
    kr = env_allocate(&env);
    if (kr != KERN_SUCCESS) {
	if (debug)
	    fprintf(stderr, "%s: do_env_new_conn: env_allocate: %s\n",
		    progname, mach_error_string(kr));
	return kr;
    }
    *new_env_port = (mach_port_t)env;
    
    kr = mach_port_request_notification(mach_task_self(),
					*new_env_port,
					MACH_NOTIFY_NO_SENDERS,
					0,
					*new_env_port,
					MACH_MSG_TYPE_MAKE_SEND_ONCE,
					&previous);
    if ((kr != KERN_SUCCESS) || (previous != MACH_PORT_NULL)) {
	if (debug)
	    fprintf(stderr, "%s: mach_port_request_notification: %s\n",
		    progname, mach_error_string(kr));
	env_deallocate(env);
	return KERN_FAILURE;
    }
    
    /* create new, empty hash table */
    
    HashHead =  (pHashHead)malloc(sizeof(hash_head_t));
    if (HashHead == NULL) {
	if (debug)
	    fprintf(stderr,
		    "%s: env_new_conn: malloc: out of memory\n",
		    progname);
	env_deallocate(env);
	return KERN_RESOURCE_SHORTAGE;
    }
    
    kr = mach_port_move_member(mach_task_self(), *new_env_port, pset);
    if (kr != KERN_SUCCESS) {
	env_deallocate(env);
	return kr;
    }
    
    HashHead->refcnt = 1;
    for (i = 0; i < HASHTABLESIZE; i++)
	HashHead->head[i] = NULL;
    
    if (debug) {
	fprintf(stderr, 
		"%s: do_env_new_conn: new port %x\n",
		progname, (int)*new_env_port);
    }
    env->list.head = HashHead;
    
    return ENV_SUCCESS;
}

/******************************************************
 * Abstract:
 * 	Allocates a  new port through which the current
 *	environment variables may be read but not modified.
 *	A read-only port allows one task to allow a child
 *	task to read, but not modify its environment.
 *  Parameters:
 *	serv_port	port identifying the current context
 *	new_env_port	returns a read only port to the the context
 *  Returns:
 *	ENV_SUCCESS
 *  Design:
 *	There is only one read/write and zero or one read-only port
 *	to each context. If a read only port has already
 *	been created for this context it is returned.
 ******************************************************/
kern_return_t	do_env_restrict_conn (serv_port, new_env_port)
    mach_port_t	serv_port;
    mach_port_t	*new_env_port;
{
    environment_t *env;
    boolean_t	restrict;
    kern_return_t kr;
    mach_port_t	port;
    mach_port_array_t ports;
    mach_msg_type_number_t count, i;
    
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    *new_env_port = (mach_port_t)(serv_port + 1);
    
    kr = mach_port_get_set_status(mach_task_self(), pset, &ports, &count);
    if (kr != KERN_SUCCESS)	{
	if (debug)
	    fprintf(stderr, "%s: mach_port_set_get_status: %s.\n",
		    progname, mach_error_string(kr));
	return kr;
    }
    for (i = 0; i < count; i++)
	if (*new_env_port == ports[i])
	    break;
    if (i == count) {
	if (debug)
	    fprintf(stderr,
		    "%s: do_env_restrict_conn: moving port 0x%x to pset 0x%x\n",
		    progname, *new_env_port, pset);
	kr = mach_port_move_member(mach_task_self(), *new_env_port, pset);
	if (kr != KERN_SUCCESS)	{
	    if (debug)
		fprintf(stderr,
			"%s: mach_port_move_member(0x%x -> 0x%x): %s\n",
			progname, *new_env_port, pset,
			mach_error_string(kr));
	    return kr;
	}
    }
    if (debug) {
	fprintf(stderr, "%s: do_env_restrict_conn: handing out port 0x%x\n",
		progname, (int)*new_env_port);
    }
    return ENV_SUCCESS;
}

/******************************************************
 *  Abstract:
 *	Destroys the environment for serv_port
 *  Parameters:
 *	serv_port	specified the context to be destroyed.
 *  Returns:
 *	ENV_SUCCESS
 *      ENV_READ_ONLY	serv_port only allowed reading of the context
 *  SideEffects:
 *	Deallocates both the read-only and read/write ports
 *	for this environment.
 ******************************************************/
kern_return_t	do_env_disconnect (serv_port)
    mach_port_t	serv_port;
{
    environment_t	*env;
    int		i;
    boolean_t	restrict;
    kern_return_t	kr;
    pHashEntry	entry, tempentry;
    pHashHead	Head;
    
    if (debug)
	fprintf(stderr, "%s: do_env_disconnect(0x%x)\n",
		progname, serv_port);
    env = env_from_port(serv_port, &restrict);
    if (env == NULL)
	return ENV_UNKNOWN_PORT;
    
    if (restrict) {
	return ENV_SUCCESS;
    }
    
    Head = env->list.head;
    Head->refcnt--;
    if (Head->refcnt == 0 )	{ /* dispose of the hash table */
	for (i = 0; i < HASHTABLESIZE; i++) {
	    entry = Head->head[i];
	    while (entry != NULL) {
		if (entry->vartype == envport) {
		    hash_port_del(entry->envval.port,
				  env, entry->name);
		    mod_urefs(entry->envval.port, -1);
		} else {
		    free(entry->envval.str);
		}
		tempentry = entry->next;
		FreeHashEntry(entry);
		entry = tempentry;
	    }
	}
	free((char *)Head);
    }
    
    if (debug)
	fprintf(stderr, "%s: do_env_disconnect: destroying rwport 0x%x\n",
		progname, (int)serv_port);
    port_destroy(serv_port);
    if (debug)
	fprintf(stderr, "%s: do_env_disconnect: destroying roport 0x%x\n",
		progname, (int)serv_port + 1);
    port_destroy(serv_port + 1);

    env_put_free_env(env);
    return ENV_SUCCESS;
}

kern_return_t do_mach_notify_port_deleted(notify_port, name)
    mach_port_t notify_port;
    mach_port_t name;
{
    if (debug)
	fprintf(stderr, "%s: do_mach_notify_port_deleted\n", progname);
    return KERN_SUCCESS;
}

kern_return_t do_mach_notify_msg_accepted(notify_port, name)
    mach_port_t notify_port;
    mach_port_t name;
{
    quit(1, "%s: do_mach_notify_msg_accepted\n", progname);
    return KERN_FAILURE;
}

kern_return_t do_mach_notify_port_destroyed(notify_port, port)
    mach_port_t notify_port;
    mach_port_t port;
{
    quit(1, "%s: do_mach_notify_port_destroyed\n", progname);
    return KERN_FAILURE;
}

kern_return_t do_mach_notify_no_senders(notify_port, mscount)
    mach_port_t notify_port;
    mach_port_mscount_t mscount;
{
    if (debug)
	fprintf(stderr, 
		"%s: do_mach_notify_no_senders(0x%x, %d)\n",
		progname, (int)notify_port, mscount);
    do_env_disconnect(notify_port);
    return KERN_SUCCESS;
}

kern_return_t do_mach_notify_send_once(notify_port)
    mach_port_t notify_port;
{
    quit(1, "%s: do_mach_notify_send_once\n", progname);
    return KERN_FAILURE;
}

kern_return_t do_mach_notify_dead_name(notify_port, name)
    mach_port_t notify_port;
    mach_port_t name;
{
    kern_return_t kr;
    hash_port_entry_t p;
    
    if (debug)
	fprintf(stderr, "%s: do_mach_notify_dead_name: 0x%x\n",
		progname, name);
    
    p = hash_port_lookup(name);
    if (p) {
	env_list_t el;
	
	for (el = (env_list_t)queue_first(&p->env_queue);
	     !queue_end(&p->env_queue, (queue_entry_t)el);
	     el = (env_list_t)queue_next(&el->chain)) {
	    if (debug)
		fprintf(stderr,
		"%s: do_mach_notify_dead_name: deleting env/name pair 0x%x/%s\n",
			progname, el->env, el->key);
	    kr = do_env_del_port(el->env, el->key);
	    if (kr != KERN_SUCCESS)
		quit(1, "%s: do_mach_notify_dead_name: %s\n",
		     progname, mach_error_string(kr));
	}
    } else if (debug) {
	fprintf(stderr, 
	"%s: do_mach_notify_dead_name: port 0x%x not in hash_port_table.\n",
		progname, name);
    }
    port_destroy(name);
    return kr;
}
