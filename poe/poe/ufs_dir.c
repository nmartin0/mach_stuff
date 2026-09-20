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
 * $Log:	ufs_dir.c,v $
 * Revision 2.4  91/12/19  20:29:50  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:55:52  rwd
 * 	Fix name bug in denter.
 * 	[90/09/26            rwd]
 * 	Fix loop in dremove which could loop forever.
 * 	[90/09/21            rwd]
 * 	Use map_file_lock/unlock.
 * 	[90/09/11            rwd]
 * 
 * Revision 2.2  90/09/08  00:21:27  rwd
 * 	Convert fileio to mapped window.
 * 	[90/08/28            rwd]
 * 
 */
/*
 *	File:	./ufs_dir.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <ufs_fops.h>
#include <device/device_types.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DIRBLKSIZ       DEV_BSIZE
#define	MAXNAMLEN	255
#define D_RECLEN(d_namelen) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + ((d_namelen + 4) &~ 3))


struct	direct {
	u_long	d_ino;			/* inode number of entry */
	u_short	d_reclen;		/* length of this record */
	u_short	d_namlen;		/* length of string in d_name */
	char	d_name[MAXNAMLEN + 1];	/* name must be no longer than this */
};

/*
 * Look up a name in a directory.
 * Should do exactly one thing, quickly.
 * Except... should probably return location of match, for
 * dremove's convenience. Should it also keep hint pointer?
 * I cannot rememer whether ffs paper said this was a good idea.
 */
dlook(name, un, ino_p)
	char *name;
	register struct unode *un;
	ino_t *ino_p;
{
	register struct direct *dp;
	register vm_offset_t	loc;
	register int len, error;

	if (name == 0 || *name == '\0') {
		*ino_p = 0;
		return ENOENT;
	}
	if ((un->i_mode & IFMT) != IFDIR) {
		return ENOTDIR;
	}
	error = bsd_map(un);
	if (error) {
		return error;
	}
	len = strlen(name);
	loc = 0;
	map_file_lock(&un->un_fn.fn_map_info);
	map_file_remap(&un->un_fn.fn_map_info, 0, un->i_size);
	dp = (struct direct *) un->un_fn.fn_map_info.mi_address;
	while (loc < un->i_size) {
		if (dp->d_ino != 0
		    && dp->d_namlen == len
		    && !strcmp(name, dp->d_name)) {
			*ino_p = dp->d_ino;
			map_file_unlock(&un->un_fn.fn_map_info);
			return 0;
		}
		loc += dp->d_reclen;
		dp = (struct direct *) ((char *)dp + dp->d_reclen);
	}
	map_file_unlock(&un->un_fn.fn_map_info);
	return ENOENT;
}

/*
 *  Enter a name into a directory.
 *
 *  We should really do this in two stages.
 *  When existing only in memory, just tag stuff at the end
 *  (don't bother looking for holes).
 *  When writing to disk, be trickier.
 */
denter(name, un, ino)
	char *name;
	register struct unode *un;
	ino_t ino;
{
	register struct direct *dp;
	register vm_offset_t	loc;
	register int len, error;
	int d_reclen, d_namlen;

	if (name == 0 || *name == '\0') {
		return EINVAL;
	}
	if ((un->i_mode & IFMT) != IFDIR) {
		return ENOTDIR;
	}
	error = bsd_map(un);
	if (error) {
		return error;
	}
	map_file_lock(&un->un_fn.fn_map_info);
	map_file_remap(&un->un_fn.fn_map_info, 0, un->i_size);
	dp = (struct direct *) un->un_fn.fn_map_info.mi_address;
	d_namlen = strlen(name);
	d_reclen = D_RECLEN(d_namlen);
	loc = 0;
	while (loc < un->i_size) {
#if 0
		if (dp->d_ino == 0 || dp->d_reclen > D_RECLEN(dp->d_namlen)) {
#else
		if (dp->d_reclen >= d_reclen + D_RECLEN(dp->d_namlen)) {
#endif
			goto foundroom;
		}
/*		printf("[ino %d rec[name]=%d rec=%d]\n",
			dp->d_ino, D_RECLEN(dp->d_namlen), dp->d_reclen);
*/
		loc += dp->d_reclen;
		dp = (struct direct *) ((char *)dp + dp->d_reclen);
	}
	/* XXX extend directory! */
	map_file_unlock(&un->un_fn.fn_map_info);
	return ENOSPC;

foundroom:
	/* XXX mark buffer (??) dirty */
	d_reclen = dp->d_reclen - D_RECLEN(dp->d_namlen);
	dp->d_reclen = D_RECLEN(dp->d_namlen);
	dp = (struct direct *) ((char *)dp + dp->d_reclen);
	dp->d_ino = ino;
	dp->d_reclen = d_reclen;
	dp->d_namlen = d_namlen;
	bzero(dp->d_name, (d_namlen+4)&~3); /* XXX */
	bcopy(name, dp->d_name, d_namlen);
	map_file_unlock(&un->un_fn.fn_map_info);
	return 0;
}

/*
 *  Remove a name from a directory.
 *  Should probably be merged(~) with dlook.
 *
 *  Also should be done in two stages...
 */
dremove(name, un)
	char *name;
	register struct unode *un;
{
	register struct direct *dp, *dpprev = 0;
	register vm_offset_t	loc;
	register int len, error;

	if (name == 0 || *name == '\0') {
		return ENOENT;
	}
	if ((un->i_mode & IFMT) != IFDIR) {
		return ENOTDIR;
	}
	error = bsd_map(un);
	if (error) {
		return error;
	}
	map_file_lock(&un->un_fn.fn_map_info);
	map_file_remap(&un->un_fn.fn_map_info, 0, un->i_size);
	dp = (struct direct *) un->un_fn.fn_map_info.mi_address;
	len = strlen(name);
	loc = 0;
	while (loc < un->i_size) {
	    if (dp->d_ino != 0) {
		if (dp->d_namlen == len && !strcmp(name, dp->d_name)) {
			goto zappit;
		}
	    }
	    loc += dp->d_reclen;
	    dpprev = dp;
	    dp = (struct direct *) ((char *)dp + dp->d_reclen);
	}
	map_file_unlock(&un->un_fn.fn_map_info);
	return ENOENT;

zappit:
	if ((((int)dp) & ~DIRBLKSIZ) == 0) {
		/* first entry -- just zero ino */
		dp->d_ino = 0;
	} else {
		/* merge with previous entry in block */
		dpprev->d_reclen += dp->d_reclen;
	}
	map_file_unlock(&un->un_fn.fn_map_info);
	return 0;
}
