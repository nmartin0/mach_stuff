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
 * $Log:	bsd_lookup.c,v $
 * Revision 2.6  94/03/25  18:21:54  mrt
 * 	Added support for CMU style FASTLINKS.
 * 	[94/02/19            mrt]
 * 
 * Revision 2.5  91/12/19  20:27:26  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  91/03/09  14:32:10  rpd
 * 	Fixed path_split to return writeable static strings.
 * 	[91/02/18            rpd]
 * 
 * Revision 2.3  90/09/27  13:54:02  rwd
 * 	Remove extraneous printf in bsd_lookup.
 * 	[90/09/26            rwd]
 * 
 * Revision 2.2  90/09/08  00:07:28  rwd
 * 	First checkin
 * 	[90/08/31  13:32:02  rwd]
 * 
 */
/*
 *	File:	./bsd_lookup.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <ufs_fops.h>
#include <ufs_disk.h>
#include <ux_param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/*
 *  Split a pathname into a directory name and a base name, both
 *  null terminated. We use this instead of a parent-returning
 *  option to bsd_lookup.
 */
path_split(pathname, dir, base)
	char *pathname;
	char **dir;
	char **base;
{
	static char dot[] = ".";	/* must be writeable */
	static char slash[] = "/";	/* must be writeable */
	char *s;

	/*
	 *  Look for the last slash in the pathname.
	 *  If we find a slash and it is not the first character,
	 *  replace the slash with a null and return the two halves.
	 */
	for (s = &pathname[strlen(pathname)-1]; s > pathname; s--) {
		if (*s == '/') {
			*s = '\0';
			*dir = pathname;
			*base = &s[1];
			return;
		}
	}
	/*
	 *  If there found no slashes after the first character, then
	 *  we do not have room to insert a null. Instead, return
	 *  an artificially constructed directory string.
	 */
	if (pathname[0] == '/') {
		*dir = slash;
		*base = &pathname[1];
	} else {
		*dir = dot;
		*base = pathname;
	}
}

static int
bsd_follow(fn, fnsp)
	struct fnode *fn;
	struct fnode **fnsp;
{
	int error, resid, out_len;
	char*	fstlnk_str;
	char pathname[MAXPATHLEN];
	struct fnode *fns = *fnsp;
	struct unode *ufn;

	error = fn_checktype(fns, S_IFLNK, -1);
	if (error) {
		return (error == -1 ? 0 : error);
	}
#if 0
	if (??_nlinks++ == MAXSYMLINKS) {
		return ELOOP;
	}
#endif
	ufn = (struct unode *) fns;
#if DEBUG
	printf ("lookup: start #%d\n", ufn->un_ino);
#endif
	if ( ufn ->i_ic.ic_flags & IC_FASTLINK != 0 ) {
	    out_len = (long)ufn->i_size;
	    if (out_len > MAXPATHLEN) out_len = MAXPATHLEN;
	    resid = MAXPATHLEN - out_len;
	    fstlnk_str = ufn->i_ic.ic_Mun.ic_Msymlink;
#if DEBUG
	    printf ("lookup: #%d , len=%d out_len=%d resid=%d str='%s'\n",
		ufn->un_ino,  MAXPATHLEN, out_len, resid, fstlnk_str);
#endif

	    if (resid > 0 ) {
		bcopy(fstlnk_str, pathname, out_len);
	    }
	}
	else {
	    error = FOP_READ(fns, 0, pathname, MAXPATHLEN, &resid);
	    if (error) {
		return error;
	    }
	}
	if (resid == 0) {
		return ENAMETOOLONG;
	}
	pathname[MAXPATHLEN - resid] = '\0';
	error = bsd_lookup(fn, pathname, fnsp, TRUE);
	if (error) {
		return error;
	}
	FOP_DECR(fns);
	return 0;
}

/*
 * >>> Do other folks cache non-existant pathnames??????
 * >>> Would it be a good idea?
 *
 * Where do we put the lookup cache? Hopefully we don't really need two!
 */
bsd_lookup(fn, pathname, fnp, follow)
	struct fnode *fn;	/* start from */
	char *pathname;
	struct fnode **fnp;	/* eventual fnode */
	boolean_t follow;
{
	kern_return_t kr;
	int error = 0;
#if 1
	extern struct fnode *root_fn;
#endif
	struct fnode *fnc;
	char *elt, *null_spot, null_save;
	struct stat st;

	while (pathname[0] == '/') {
#if 1
		fn = root_fn;
#else
		fn = uu->u_rdir;
#endif
		pathname++;
	}
	FOP_INCR(fn);
	for (;;) {
		if (fn->fn_mounted) {
			fn = fn->fn_mounted->fs_root;
		}
		if (pathname[0] == '\0') {
			*fnp = fn;
			return 0;
		}
		error = fn_checktype(fn, S_IFDIR, ENOTDIR);
		if (error) {
			break;
		}
		elt = pathname;
		while (pathname[0] != '\0' && pathname[0] != '/') {
			pathname++;
		}
		null_spot = pathname;
		null_save = *null_spot;
		while (pathname[0] == '/') {
			pathname++;
		}
		*null_spot = '\0';
		if (elt[1] == '.' && elt[2] == '\0' && elt[0] == '.') {
			if (fn == fn->fn_fs->fs_root) {
				fn = fn->fn_fs->fs_mountpoint;
			}
		}
		error = FOP_LOOKUP(fn, elt, &fnc);
		*null_spot = null_save;
		if (error) {
			break;
		}
		if (follow || pathname[0] != '\0') {
			error = bsd_follow(fn, &fnc);
			if (error) {
				FOP_DECR(fnc);
				break;
			}
		}
		FOP_DECR(fn);
		fn = fnc;
	}
	FOP_DECR(fn);
	return error;
}
