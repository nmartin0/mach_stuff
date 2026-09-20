/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	subr_cbuf.c,v $
 * Revision 2.4  91/12/19  20:29:07  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  91/06/18  13:06:57  jjc
 * 	Picked up line discipline support from Adam Richter at 
 * 	Berkeley, adding cbuf_expunge().
 * 	[91/05/30            jjc]
 * 
 * Revision 2.2  90/09/08  00:20:11  rwd
 * 	First checkin
 * 	[90/08/31  13:55:16  rwd]
 * 
 */
/*
 *	File:	./subr_cbuf.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

/*
 * Circular buffer package
 * (I am clearly stretching my programming abilities here, sadly enough)
 */

struct cbuf {
	int		cb_start;
	int		cb_count;
	int		cb_size;
	char		*cb_buf;
};

struct cbuf *
cbuf_alloc(size)
	int size;
{
	struct cbuf *cb;

	cb = (struct cbuf *) malloc(size + sizeof(*cb));
	cb->cb_start = 0;
	cb->cb_count = 0;
	cb->cb_size  = size;
	cb->cb_buf   = (char *) &cb[1];
	return cb;
}

cbuf_free(cb)
	struct cbuf *cb;
{
	free(cb);
}

cbuf_read(cb, buffer, size)
	struct cbuf *cb;
	char *buffer;
	int size;
{
	int count, count1;

	count = ((cb->cb_count < size) ? cb->cb_count : size);
	if (count <= 0) {
		return 0;
	}
	if (cb->cb_start + count > cb->cb_size) {
		count1 = cb->cb_size - cb->cb_start;
		bcopy(&cb->cb_buf[cb->cb_start], buffer, count1);
		bcopy(cb->cb_buf, buffer+count1, count - count1);
	} else {
		bcopy(&cb->cb_buf[cb->cb_start], buffer, count);
	}
	cb->cb_count -= count;
	cb->cb_start = (cb->cb_start + count) % cb->cb_size;
	return count;
}

cbuf_write(cb, buffer, size)
	struct cbuf *cb;
	char *buffer;
	int size;
{
	int count, count1;
	int end; /* where we start writing */

	count = cb->cb_size - cb->cb_count;
	if (count > size) {
		count = size;
	}
	if (count <= 0) {
		return 0;
	}
	end = (cb->cb_start + cb->cb_count) % cb->cb_size;
	if (end + count > cb->cb_size) {
		count1 = cb->cb_size - end;
		bcopy(buffer, &cb->cb_buf[end], count1);
		bcopy(buffer+count1, cb->cb_buf, count-count1);
	} else {
		bcopy(buffer, &cb->cb_buf[end], count);
	}
	cb->cb_count += count;
	return count;
}

void
cbuf_expunge( cb )
     struct cbuf *cb;
{
   cb->cb_start = cb->cb_count = 0;
}
