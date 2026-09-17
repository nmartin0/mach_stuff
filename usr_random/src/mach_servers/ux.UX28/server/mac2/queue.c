/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	queue.c,v $
 * Revision 2.2  91/02/05  17:28:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:16:22  mrt]
 * 
 * Revision 2.1  89/08/03  15:51:47  rwd
 * Created.
 * 
 * 17-Mar-87  David Golub (dbg) at Carnegie-Mellon University
 *	Created from routines written by David L. Black.
 *
 */ 

/*
 *	Routines to implement queue package.
 */

#include <mac2/queue.h>

#if	defined(lint) || !defined(vax)

/*
 *	Insert element at head of queue.
 */
void enqueue_head(que, elt)
	register queue_t	que;
	register queue_entry_t	elt;
{
	elt->next = que->next;
	elt->prev = que;
	elt->next->prev = elt;
	que->next = elt;
}

/*
 *	Insert element at tail of queue.
 */
void enqueue_tail(que,elt)
	register queue_t	que;
	register queue_entry_t	elt;
{
	elt->next = que;
	elt->prev = que->prev;
	elt->prev->next = elt;
	que->prev = elt;
}

/*
 *	Remove and return element at head of queue.
 */
queue_entry_t dequeue_head(que)
	register queue_t	que;
{
	register queue_entry_t	elt;

	if (que->next == que)
		return((queue_entry_t)0);

	elt = que->next;
	elt->next->prev = que;
	que->next = elt->next;
	return(elt);
}

/*
 *	Remove and return element at tail of queue.
 */
queue_entry_t dequeue_tail(que)
	register queue_t	que;
{
	register queue_entry_t	elt;

	if (que->prev == que)
		return((queue_entry_t)0);

	elt = que->prev;
	elt->prev->next = que;
	que->prev = elt->prev;
	return(elt);
}

/*
 *	Remove arbitrary element from queue.
 *	Does not check whether element is on queue - the world
 *	will go haywire if it isn't.
 */

/*ARGSUSED*/
void remqueue(que, elt)
	queue_t			que;
	register queue_entry_t	elt;
{
	elt->next->prev = elt->prev;
	elt->prev->next = elt->next;
}

#endif	defined(lint) || !defined(vax)

/*
 *	Routines to directly imitate the VAX hardware queue
 *	package.
 */
#if	!(defined(vax) || defined(sun))
insque(entry, pred)
	register struct queue_entry *entry, *pred;
{
	entry->next = pred->next;
	entry->prev = pred;
	(pred->next)->prev = entry;
	pred->next = entry;
}

remque(elt)
	register struct queue_entry *elt;
{
	(elt->next)->prev = elt->prev;
	(elt->prev)->next = elt->next;
	return((int)elt);
}
#endif	!(defined(vax) || defined(sun))
