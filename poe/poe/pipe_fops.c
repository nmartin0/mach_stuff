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
 * $Log:	pipe_fops.c,v $
 * Revision 2.4  91/12/19  20:29:02  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  91/06/18  13:06:52  jjc
 * 	Fixed pipe_close() to signal any waiting readers if closing
 * 	the writer and signal any waiting writers if closing the reader.
 * 	The routines, pipe_read() and pipe_write(), can deadlock otherwise.
 * 	[91/05/07            jjc]
 * 
 * Revision 2.2  90/09/08  00:20:01  rwd
 * 	Remove sleeps and add pipe condition.
 * 	[90/09/06            rwd]
 * 	First checkin
 * 	[90/08/31  13:54:53  rwd]
 * 
 */
/*
 *	File:	./pipe_fops.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <ux_user.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	PIPE_BSIZE	4096	/* sez the man page */
extern struct cbuf *	cbuf_alloc();

extern int null_open();
extern int pipe_close();
extern int pipe_read();
extern int pipe_write();
extern int pipe_getstat();
extern int pipe_setstat();
extern int pipe_select();
extern int nil_ioctl();
extern int nil_getpager();
extern int nil_lookup();
extern int nil_create();
extern int nil_link();
extern int nil_unlink();

struct fops pipe_fops = {
	null_open,
	pipe_close,
	pipe_read,
	pipe_write,
	pipe_getstat,
	pipe_setstat,
	pipe_select,
	nil_ioctl,
	nil_getpager,
	nil_lookup,
	nil_create,
	nil_link,
	nil_unlink,
};

struct fnfs pipe_fs = {
	0,		/* fs_mountpoint */
	0,		/* fs_root */
	&pipe_fops,	/* fs_fops */
	FALSE,		/* fs_mayseek */
	FALSE,		/* fs_maymap */
};

/*
 *  Currently, we rely on master lock for locking.
 *  Should instead add a lock to the pipe struct.
 */

static int pino = 0;
extern struct ux_task *ut_self();
extern struct mutex *master_lock;

struct pipe {
	struct cbuf *	pi_cb;
	boolean_t	pi_noreader;
	boolean_t	pi_nowriter;
	struct condition pi_condition;
};

struct pnode {
	struct fnode	pn_fn;
	int		pn_ino;
	boolean_t	pn_iswriter;
	struct pipe	*pn_pi;
};

int
pipe_open(pnrp, pnwp)
	struct fnode **pnrp;
	struct fnode **pnwp;
{
	struct pnode *pnr, *pnw;
	struct pipe *pi;

	pi			= (struct pipe *) malloc(sizeof(*pi));
	pi->pi_cb		= cbuf_alloc(PIPE_BSIZE);
	pi->pi_noreader		= FALSE;
	pi->pi_nowriter		= FALSE;
	condition_init(&pi->pi_condition);

	pnr			= (struct pnode *) malloc(sizeof(*pnr));
	pnr->pn_fn.fn_fs	= &pipe_fs;
	pnr->pn_fn.fn_mounted	= 0;
	pnr->pn_fn.fn_map_info.mi_pager	= 0;
	pnr->pn_fn.fn_refcount	= 1;

	pnr->pn_ino		= pino++;
	pnr->pn_iswriter	= FALSE;
	pnr->pn_pi		= pi;

	pnw			= (struct pnode *) malloc(sizeof(*pnw));
	*pnw			= *pnr;
	pnw->pn_ino		= pino++;
	pnw->pn_iswriter	= TRUE;

	*pnrp			= (struct fnode *) pnr;
	*pnwp			= (struct fnode *) pnw;
	return 0;
}

pipe_read(pn, offset, buffer, size, resid)
	struct pnode *pn;
	vm_offset_t offset;
	vm_offset_t buffer;
	long size;
	vm_size_t *resid;
{
	struct pipe *pi = pn->pn_pi;
	int count;

	if (pn->pn_iswriter) {
		return EBADF;
	}
	for (;;) {
		count = cbuf_read(pi->pi_cb, buffer, size);
		if (count > 0) {
			condition_signal(&pi->pi_condition);
			*resid = size - count;
			return 0;
		}
		if (pi->pi_nowriter) {
			*resid = size;
			return 0;
		}
		server_cthread_busy();
		condition_wait(&pi->pi_condition, master_lock);
		server_cthread_active();
	}
}

pipe_write(pn, offset, buffer, size, resid)
	struct pnode *pn;
	vm_offset_t offset;
	vm_offset_t buffer;
	vm_size_t size;
	vm_size_t *resid;
{
	struct pipe *pi = pn->pn_pi;
	int count, end, size0 = size;

	if (! pn->pn_iswriter) {
		return EBADF;
	}
	for (;;) {
		if (pi->pi_noreader) {
			*resid = size0 - size;
			return EPIPE;
		}
		count = cbuf_write(pi->pi_cb, buffer, size);
		if (count)
			condition_signal(&pi->pi_condition);
		buffer += count;
		size -= count;
		if (size == 0) {
			break;
		}
		server_cthread_busy();
		condition_wait(&pi->pi_condition, master_lock);
		server_cthread_active();
	}
	*resid = 0;
	return 0;
}

pipe_close(pn)
	struct pnode *pn;
{
	struct pipe *pi = pn->pn_pi;
	int count;

	if (pn->pn_iswriter) {
		pi->pi_nowriter = TRUE;
		/* kick any waiting readers */
		condition_signal(&pi->pi_condition);
	} else {
		pi->pi_noreader = TRUE;
		/* kick any waiting writers */
		condition_signal(&pi->pi_condition);
	}
	free(pn);
	if (pi->pi_noreader && pi->pi_nowriter) {
		cbuf_free(pi->pi_cb);
		free(pi);
	}
	return 0;
}

pipe_getstat(pn, stp, mask)
	struct pnode *pn;
	struct stat *stp;
	unsigned long mask;
{
	struct stat st;

	bzero(&st, sizeof(st));
	st.st_dev = 0xffffffff;
	st.st_ino = pn->pn_ino;
	st.st_blksize = PIPE_BSIZE;
	st_copy(&st, stp, mask);
	return 0;
}

pipe_setstat(pn, stp, mask)
	struct pnode *pn;
	struct stat *stp;
	unsigned long mask;
{
	return 0;	/* XXX */
}

pipe_select()
{
}

