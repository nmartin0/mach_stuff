/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	rock_ridge.c,v $
 * Revision 2.3  93/09/15  16:06:43  mrt
 * 	Lint.
 * 	[93/08/26  16:55:22  af]
 * 
 * Revision 2.2  93/08/07  16:56:26  mrt
 * 	Revised, based on my reading of the Rock Ridge specifications
 * 	Version 1, Rev 1.09, Aug 14, 1991.
 * 	Added implementation of HOST finame components (@sys for us).
 * 	Added first pass at mapping special device numbers (empty).
 * 	Added rock_ridge_isa_symlink(), symlinks actually work now.
 * 	Relocated directories still to be tested [could not find a
 * 	real CD with them :-(( ].
 * 	[93/07/04  21:56:06  af]
 * 
 * 	Taken from Linux source and heavily pounded upon.
 * 	[93/06/29            af]
 * 
 */
/*
 *  linux/fs/isofs/rock.c
 *
 *  (C) 1992  Eric Youngdale
 *
 *  Rock Ridge Extensions to iso9660
 *
 * This file is part of Linux.
 *
 * It is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * It is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAS; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/macro_help.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>


#include <isofs/isofs.h>
#include <isofs/isofs_node.h>
#include <isofs/rock_ridge.h>

#define	atsys()		"@sys"
#define	atsys_len()	4

int rock_debug = 0;

/* These functions are designed to read the system areas of a directory record
 * and extract relevant information.  There are different functions provided
 * depending upon what information we need at the time.  One function fills
 * out an inode structure, a second one extracts a filename, a third one
 * returns a symbolic link name, and a fourth one returns the extent number
 * for the file. */

#define signature(A,B) ((A << 8) | B)


/* This is a way of ensuring that we have something in the system
   use fields that is compatible with Rock Ridge */
#define isa_SP(_rr_)	       			\
	(((_rr_)->x.SP.magic[0] == 0xbe) &&	\
	 (rr->x.SP.magic[1] == 0xef))

/* These use-once macros hide some recurring tasks,
   to keep the main flow of these functions obvious */

#define setup_rock_ridge(_de_,_chr_,_len_)	      		      	\
MACRO_BEGIN								\
  (_len_) = sizeof(struct iso_directory_record) + (_de_)->name_len[0];	\
  if ((_len_) & 1) (_len_)++;						\
  (_chr_) = ((unsigned char *) (_de_)) + (_len_);			\
  (_len_) = *((unsigned char *) (_de_)) - (_len_);			\
MACRO_END

#define maybe_continue(_label_,_ip_)					\
MACRO_BEGIN								\
  if (cont_extent){ 							\
    daddr_t block;							\
    int offset, bsize;							\
									\
    bsize = (_ip_)->i_mnt->im_bsize;					\
    offset = cont_offset; 						\
    if (offset >= bsize) cont_extent++;					\
    offset &= (bsize-1);						\
    block = iso_map_extent(_ip_,cont_extent);				\
    if (bp) brelse(bp);							\
    bp = bread((_ip_)->o_devvp, block, bsize);				\
    chr = (unsigned char *)bp->b_un.b_addr;				\
    len = cont_size; 							\
    cont_extent = 0; 							\
    cont_size = 0; 							\
    cont_offset = 0; 							\
    goto _label_; 							\
  }									\
MACRO_END


/*
 * Given a directory entry, returns the disk extent
 * (block number) for the file, possibly following
 * any relocation SUSP record.
 */
public int
rock_ridge_find_relocation(
	struct iso_directory_record * de, 
	struct iso_node * ip)
{
	int             dotdot;
	int             len;
	int             retval;
	unsigned char  *chr;
	int             cont_extent = 0, cont_offset = 0, cont_size = 0;
	struct buf	*bp = NULL;

	/*
	 * If this is a '..' then we are looking for the parent, otherwise we
	 * are looking for the child 
	 */
	dotdot = 0;
	if (de->name[0] == 1 && de->name_len[0] == 1)
		dotdot = 1;

	/*
	 * Return value if we do not find appropriate record. 
	 */
	retval = isonum_733(de->extent);

	if (ip->i_mnt->im_flags & ISOFSMNT_NORR)
		return retval;

	setup_rock_ridge(de, chr, len);

repeat:
	{
		int             rrflag, sig;
		struct rock_ridge *rr;

		/*
		 * There may be one byte for padding somewhere 
		 */
		while (len > 1) {
			rr = (struct rock_ridge *) chr;
			/* sanity check */
			if (rr->len == 0)
				goto out;

			sig = signature(chr[0], chr[1]);
			chr += rr->len;
			len -= rr->len;

			switch (sig) {
			    case signature('R','R'):
				rrflag = rr->x.RR.flags[0];
				if (dotdot && !(rrflag & RR_RR_PL))
					goto out;
				if (!dotdot && !(rrflag & RR_RR_CL))
					goto out;
				break;
			    case signature('S','P'):
				if (!isa_SP(rr))
					goto out;
				break;
			    case signature('C','L'):
printf("RR: CL\n");
				if (dotdot == 0) {
					retval = isonum_733(rr->x.CL.location);
					goto out;
				};
				break;
			    case signature('P','L'):
printf("RR: PL\n");
				if (dotdot != 0) {
					retval = isonum_733(rr->x.PL.location);
					goto out;
				};
				break;
			    case signature('C','E'):
				/* Is there a continuation record */
				cont_extent = isonum_733(rr->x.CE.extent);
				cont_offset = isonum_733(rr->x.CE.offset);
				cont_size = isonum_733(rr->x.CE.size);
				break;
			    default:
				break;
			}
		};
	};
	maybe_continue(repeat, ip);
out:
	if (bp)
		brelse(bp);
	return retval;
}

/*
 * Given a directory entry, checks if it is followed
 * by any SUSP record, e.g. a continuation that points
 * to a longer/non-standard filename.
 * Returns such a string in newly malloc-ed memory.
 */
public int
rock_ridge_get_filename(
	struct iso_directory_record * de,
	char		**name,
	int		*namlen,
	struct iso_node *ip)
{
	int             len;
	unsigned char  *chr;
	int             cont_extent = 0, cont_offset = 0, cont_size = 0;
	struct buf     *bp = NULL;
	char           *retname = NULL;
	int             retnamlen = 0;
	int		truncate = 0;
	int		retval = 0;

	if (ip->i_mnt->im_flags & ISOFSMNT_NORR)
		return 0;

if (rock_debug){
int i = isonum_711(de->name_len); char *p = de->name;
printf("rr-fn %x: ", ip->o_number);
while (i--) printf("%c", *p++);
printf(" --> ");
}
	setup_rock_ridge(de, chr, len);
repeat:
	{
		struct rock_ridge *rr;
		int             sig;

		/*
		 * There may be one byte for padding somewhere 
		 */
		while (len > 1) {
			rr = (struct rock_ridge *) chr;
			if (rr->len == 0)	/* sanity */
				goto out;

trace(rock_debug,("[rr %c%c %d]", chr[0], chr[1], rr->len));

			sig = signature(chr[0], chr[1]);
			chr += rr->len;
			len -= rr->len;

			switch (sig) {
			    case signature('R','R'):
trace(rock_debug,("[%x]", rr->x.RR.flags[0]));
				if ((rr->x.RR.flags[0] & RR_RR_NM) == 0)
					goto out;
				break;
			    case signature('S','P'):
trace(rock_debug,("[%x%x]", rr->x.SP.magic[0], rr->x.SP.magic[1]));
				if (!isa_SP(rr))
					goto out;
				break;
			    case signature('C','E'):
				/* Is there a continuation record */
				cont_extent = isonum_733(rr->x.CE.extent);
				cont_offset = isonum_733(rr->x.CE.offset);
				cont_size = isonum_733(rr->x.CE.size);
trace(rock_debug,("[x%x x%x x%x]", cont_extent, cont_offset, cont_size));
				break;

			    case signature('N','M'):
trace(rock_debug,("[%x]", rr->x.NM.flags));
				if (truncate)
					break;
				if (!retname) {
					retname = (char *) malloc(255);
					/*
					 * This may be a waste, but we only
					 * need this for a moment.  The layers
					 * that call this function should
					 * deallocate the mem fairly soon
					 * after control is returned 
					 */

					*retname = 0;	/* Zero length string */
					retnamlen = 0;
				} else
				if ((retnamlen + rr->len - 5) >= 254) {
					truncate = 1;
					break;
				};
				switch (rr->x.NM.flags & ~RR_NM_CONT) {
				    case 0:
					/* append now */
					bcopy(	rr->x.NM.name,
						retname + retnamlen,
						rr->len - 5);
					retnamlen += rr->len - 5;
					break;
				    case RR_NM_DOTDOT:
					retname[retnamlen++] = '.';
					/* fall through */
				    case RR_NM_DOT:
					retname[retnamlen++] = '.';
					break;
				    case RR_NM_HOST:
					bcopy(	atsys(),
						retname + retnamlen,
						atsys_len());
					retnamlen += atsys_len();
					break;
				    default:
					printf("RR: Unsupported NM flag settings (x%x)\n", rr->x.NM.flags);
					break;
				}
				retname[retnamlen] = 0;
				break;


			    case signature('R','E'):
trace(rock_debug,("[%x]", ip->o_number));
				retval = -1;
				goto out;
			    case signature('E','R'):
trace((rock_debug>1),("[%s %s %s]\n", rr->x.ER.data,
			&rr->x.ER.data[rr->x.ER.len_id],
			&rr->x.ER.data[rr->x.ER.len_id+rr->x.ER.len_des]));
			    default:
				break;
			}
		};
	}
	maybe_continue(repeat, ip);
	if (retname) {
		*name = retname;
		*namlen = retnamlen;
		retname = 0; /* dont free */
		retval = 1;
	} else
		retval = 0;		/* This file did not have a NM field */
out:
trace(rock_debug,(".\n"));
	if (bp)
		brelse(bp);
	if (retname)
		free(retname);
	return retval;
}


/*
 * Handle unix-oriented extensions, such as
 *  PX: records mode, ownership, and link counts
 *  PN: special device inum
 *  TF: file times
 *  SL: symbolic links
 *  CL: all of the above..
 */
public int
rock_ridge_parse_vnode(
	struct iso_directory_record * de,
	struct iso_node * ip)
{
	int             len;
	unsigned char  *chr;
	int             cont_extent = 0, cont_offset = 0, cont_size = 0;
	struct buf     *bp = NULL;

	if (ip->i_mnt->im_flags & ISOFSMNT_NORR)
		return 0;

trace(rock_debug,("rr-prse %x ", ip->o_number));
	setup_rock_ridge(de, chr, len);
repeat:
	{
		int             cnt, sig;
		struct iso_node   *reloc;
		struct rock_ridge *rr;
		int             slen;
		struct SL_component *slp;

		/*
		 * There may be one byte for padding somewhere 
		 */
		while (len > 1) {
			rr = (struct rock_ridge *) chr;
			if (rr->len == 0)	/* sanity */
				goto out;

			sig = signature(chr[0], chr[1]);
			chr += rr->len;
			len -= rr->len;

			switch (sig) {
			    case signature('R','R'):
				if ((rr->x.RR.flags[0] &
				     (RR_RR_PX | RR_RR_PN | RR_RR_TF | RR_RR_SL | RR_RR_CL)) == 0)
					goto out;
				break;
			    case signature('S','P'):
				if (!isa_SP(rr))
					goto out;
				break;
			    case signature('C','E'):
				/* Is there a continuation record */
				cont_extent = isonum_733(rr->x.CE.extent);
				cont_offset = isonum_733(rr->x.CE.offset);
				cont_size = isonum_733(rr->x.CE.size);
				break;
			    case signature('E','R'):
#if 0
				printf("ISO9660 Extensions: ");
				{
					int             p;
					for (p = 0; p < rr->x.ER.len_id; p++)
						printf("%c", rr->x.ER.data[p]);
				};
				printf("\n");
#endif
				break;
			    case signature('P','X'):
				ip->o_mode = isonum_733(rr->x.PX.mode) & RR_PX_MMASK;
				ip->o_nlink = isonum_733(rr->x.PX.n_links);
				ip->o_uid = isonum_733(rr->x.PX.uid);
				ip->o_gid = isonum_733(rr->x.PX.gid);
trace(rock_debug,("PX %x %d ", ip->o_mode, ip->o_nlink));
				break;
			    case signature('P','N'):
				{
					int             high, low;

					high = isonum_733(rr->x.PN.dev_high);
					low = isonum_733(rr->x.PN.dev_low);
					ip->o_dev = rock_ridge_mapdev(ip->i_mnt,high,low);
				};
				break;
			    case signature('T','F'):
				cnt = 0;
				/* xxx LONG_FORM xxx */
				if (rr->x.TF.flags & RR_TF_CREATE)
					ip->o_ctime = iso_date(rr->x.TF.times[cnt++].time, 0);
				if (rr->x.TF.flags & RR_TF_MODIFY)
					ip->o_mtime = iso_date(rr->x.TF.times[cnt++].time, 0);
				if (rr->x.TF.flags & RR_TF_ACCESS)
					ip->o_atime = iso_date(rr->x.TF.times[cnt++].time, 0);
				/* we do not git nothing else */
				break;
			    case signature('S','L'):

trace(rock_debug,("SL %d ", ip->i_size));
				slen = rr->len - 5;
				slp = &rr->x.SL.link;
				while (slen > 1) {
					switch (slp->flags & ~RR_SL_CONT) {
					    case 0:
						ip->i_size += slp->len;
trace(rock_debug,("%.32s ", slp->text));
						break;
					    case RR_SL_DOT:
						ip->i_size += 1;
						break;
					    case RR_SL_DOTDOT:
						ip->i_size += 2;
						break;
					    case RR_SL_ROOT:
						ip->i_size += 1;
						break;
					    case RR_SL_HOST:
						ip->i_size += atsys_len();
						break;
					    case RR_SL_VOLROOT:
					    default:
						printf("RR: Symlink component flag not implemented\n");
					};
					slen -= slp->len + 2;
					slp = (struct SL_component *) (((char *) slp) + slp->len + 2);

					if (slen < 2)
						break;
					ip->i_size += 1;	/* '/' */
				};
trace(rock_debug,("-> %d ", ip->i_size));
				break;
			    case signature('R','E'):
				printf("RR: Attempt to read vnode for relocated directory\n");
				goto out;
			    case signature('C','L'):
printf("RR CL (%x)\n", ip->o_number);
				cnt = isonum_733(rr->x.CL.location);
				if (iso_iget(ip, cnt, &reloc, de, ip->iso_diroff) != 0)
					goto out;
				ip->iso_extent = cnt;
				ip->o_mode = reloc->o_mode;
				ip->o_nlink = reloc->o_nlink;
				ip->o_uid = reloc->o_uid;
				ip->o_gid = reloc->o_gid;
				ip->o_dev = reloc->o_dev;
				ip->i_size = reloc->i_size;
				ip->o_atime = reloc->o_atime;
				ip->o_ctime = reloc->o_ctime;
				ip->o_mtime = reloc->o_mtime;
				iso_iput(reloc);
				break;
			    default:
				break;
			}
		};
	}
	maybe_continue(repeat, ip);
out:
	if (bp)
		brelse(bp);
	return 0;
}


/*
 * Say if this is a symlink or not
 */
public int
rock_ridge_isa_symlink(
	struct iso_directory_record *de,
	struct iso_node * ip)
{
	int             len;
	unsigned char  *chr;
	int             cont_extent = 0, cont_offset = 0, cont_size = 0;
	struct buf     *bp = NULL;
	int		retval = 0;

	if (ip->i_mnt->im_flags & ISOFSMNT_NORR)
		return 0;

trace(rock_debug,("rr-isal %x ", de));
	setup_rock_ridge(de, chr, len);
repeat:
	{
		int             cnt, sig;
		struct iso_node   *reloc;
		struct rock_ridge *rr;
		int             slen;
		struct SL_component *slp;

		/*
		 * There may be one byte for padding somewhere 
		 */
		while (len > 1) {
			rr = (struct rock_ridge *) chr;
			if (rr->len == 0)	/* sanity */
				goto out;

			sig = signature(chr[0], chr[1]);
			chr += rr->len;
			len -= rr->len;

			switch (sig) {
			    case signature('R','R'):
				retval = (rr->x.RR.flags[0] & RR_RR_SL);
				goto out;
				break;
			    case signature('S','P'):
				if (!isa_SP(rr))
					goto out;
				break;
			    case signature('C','E'):
				/* Is there a continuation record */
				cont_extent = isonum_733(rr->x.CE.extent);
				cont_offset = isonum_733(rr->x.CE.offset);
				cont_size = isonum_733(rr->x.CE.size);
				break;
			    case signature('S','L'):
				retval = 1;
				goto out;
			    case signature('R','E'):
			    case signature('C','L'):
				goto out;
			    default:
				break;
			}
		};
	}
	maybe_continue(repeat, ip);
out:
	if (bp)
		brelse(bp);
	return retval;
}

/*
 * If this vnode is a symlink, return the
 * filename it is symlinked to (else NULL).
 */
public char *
rock_ridge_get_symlink(struct iso_node * ip)
{
	char           *rpnt = 0;
	struct iso_directory_record *de;
	int             cont_extent = 0, cont_offset = 0, cont_size = 0;
	struct buf     *bp = NULL;
	daddr_t         block;
	int             sig;
	int             len;
	unsigned char  *chr;
	struct rock_ridge *rr;
	struct iso_mnt  *isomp = ip->i_mnt;

	if (ip->i_mnt->im_flags & ISOFSMNT_NORR)
		return NULL;

	/*
	 * Find direntry for inode
	 */
	block = iso_map_extent(ip,
		ip->iso_dirextent + iso_lblkno(isomp, ip->iso_diroff));
	len = isomp->im_bsize;
	if (!(bp = bread(ip->o_devvp, block, len))) {
		printf("RR: unable to read i-node block");
		return NULL;
	}

	len = ip->iso_diroff & (len - 1);
	de = (struct iso_directory_record *)((char *) bp->b_un.b_addr + len);

	/*
	 * Now look for the SL records and parse them.
	 */

	setup_rock_ridge(de, chr, len);
	rpnt = 0;
repeat:
	/*
	 * There may be one byte for padding somewhere 
	 */
	while (len > 1) {
		int             slen, clen;
		struct SL_component *slp;

		if (rpnt)
			break;
		rr = (struct rock_ridge *) chr;
		if (rr->len == 0)	/* sanity */
			goto out;

		sig = signature(chr[0], chr[1]);
		chr += rr->len;
		len -= rr->len;

		switch (sig) {
		    case signature('R','R'):
			if ((rr->x.RR.flags[0] & RR_RR_SL) == 0)
				goto out;
			break;
		    case signature('S','P'):
			if (!isa_SP(rr))
				goto out;
			break;
		    case signature('S','L'):

			slen = rr->len - 5;
			slp = &rr->x.SL.link;
			while (slen > 1) {
				if (!rpnt) {
					rpnt = (char *) malloc(ip->i_size + 1);
					*rpnt = 0;
					clen = 0;
				};
				switch (slp->flags & ~RR_SL_CONT) {
				    case 0:
					bcopy(slp->text, rpnt + clen, slp->len);
					clen += slp->len;
					break;
				    case RR_SL_DOTDOT:
					rpnt[clen++] = '.';
					/* fall through */
				    case RR_SL_DOT:
					rpnt[clen++] = '.';
					break;
				    case RR_SL_ROOT:
					rpnt[clen++] = '/';
					break;
				    case RR_SL_HOST:
					bcopy(atsys(), rpnt + clen, atsys_len());
					clen += atsys_len();
					break;
				    case RR_SL_VOLROOT:
				    default:
					printf("RR: Symlink component flag not implemented (%d)\n", slen);
				};
				slen -= slp->len + 2;
				slp = (struct SL_component *) (((char *) slp) + slp->len + 2);

				if (slen < 2)
					break;
				rpnt[clen++] = '/';
			};
			if (rpnt) rpnt[clen] = 0;
			break;
		    default:
			break;
		};
	};
	maybe_continue(repeat, ip);
out:
	if (bp)
		brelse(bp);
	return rpnt;
}


/*
 * Map special device encoding to local names
 */
public dev_t
rock_ridge_mapdev(
	struct iso_mnt	*isomp,
	int		high,
	int		low)
{
	/* This will need the map table code as per specs */
	/* for now.. */
	return makedev(high,(low&0xff));
}
