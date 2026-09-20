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
 * $Log:	dosfs_bmap.c,v $
 * Revision 2.2  93/09/15  13:29:07  mrt
 * 	Added dosfs_bmap_seq() for large sequential operations.
 * 	Fixed various bugs in extend/truncate code.
 * 	Code seems good enough to pass all filesystem tests I have.
 * 	[93/09/14  00:05:33  af]
 * 
 * 	Macro versions of set/get entry for speed.
 * 	Record fat has been modified everywhere we do so.
 * 	Fixed file_len to treat root specially.
 * 	[93/08/26  16:23:09  af]
 * 
 * 	First version that can write to the filesystem.
 * 	[93/07/30  00:10:33  af]
 * 
 * 	Created.
 * 	[93/07/12            af]
 * 
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>

#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

private unsigned short
fat_get_entry(
	unsigned short	*fat,
	unsigned int	fbits,
	unsigned int	entry);
#define	FAT_GET_ENTRY(f,b,e) (((b) == 16) ? (f)[e] : fat_get_entry(f,b,e))

private int
fat_set_entry(
	unsigned short	*fat,
	unsigned int	fbits,
	unsigned int	entry,
	unsigned short	value);
#define	FAT_SET_ENTRY(f,b,e,v) {if ((b) == 16) (f)[e]=v; else fat_set_entry(f,b,e,v);}

private unsigned short
fat_alloc_entry(
	struct dosfs_mount	*imp,
	unsigned int		start);

/*
 * Map the logical block LBLKNO of inode IP
 * to a disk's physical block number
 */
int dosfs_bmap_cache = 1;

public daddr_t
dosfs_bmap(
	struct dosfs_node	*ip,
	daddr_t			lblkno)
{
	daddr_t			ret, l = lblkno;
	register struct dosfs_mount *dosfsmp = ip->i_mnt;
	
	/* Root is special, sigh */
	if (ip->o_number == DOS_ROOTINO) {
		ret = dosfsmp->rootdir_start + (lblkno * dosfsmp->im_clsiz);
	} else {
		register daddr_t	last = ip->last_lbn;

if (dosfs_bmap_cache) {
		if (lblkno >= last && last != -1) {
			ret = ip->last_cln;
			lblkno -= last;
		} else
			ret = ip->extent;
} else
ret = ip->extent;

		while (lblkno > 0) {
			/*
			 * Which one is the next block in the file ?
			 */
			ret = (daddr_t) FAT_GET_ENTRY(dosfsmp->fat, dosfsmp->fat_bits, ret);

			if ((ret == 0) || ((ret & 0xfff0) == 0xfff0)) {
trace(dosfs_debug,("\n?Dbmap x%x+%x -> %x %x", ip->extent, l, ret, lblkno));
				return -1;
			}
			lblkno--;
		}
		ip->last_lbn = l;
		ip->last_cln = ret;

		/* first cluster is no. 2 */
#define	dosfs_cltodb(_m_,_c_) ((((_c_) - 2) * (_m_)->im_clsiz) + (_m_)->clusters_start)
		ret = dosfs_cltodb(dosfsmp,ret);
	}

trace(dosfs_debug,("Dbmap x%x+%x -> %x ", ip->extent, l, ret));
	return (ret);
}

/*
 * Same, but also returns where the
 * contiguous chunk of disk space
 * that contains lblkno starts.
 */
public daddr_t
dosfs_bmap_seq(
	struct dosfs_node	*ip,
	daddr_t			lblkno,
	daddr_t			*seq_startp,
	unsigned int		*ssiz)
{
	daddr_t			ret, n, s, max = lblkno, m;
	unsigned int		rsiz;
	register struct dosfs_mount *dosfsmp = ip->i_mnt;
	
	/* Root is special, sigh */
	if (ip->o_number == DOS_ROOTINO) {
		s = dosfsmp->rootdir_start;
		ret = s + (lblkno * dosfsmp->im_clsiz);
		rsiz = dosfsmp->rootdir_entries * MDOS_DIR_SIZE;
		goto out;
	}

	ret = s = ip->extent;
	rsiz = dosfsmp->im_bsize;
	while (lblkno > 0) {
		n = (daddr_t) FAT_GET_ENTRY(dosfsmp->fat, dosfsmp->fat_bits, ret);
		if ((n == 0) || ((n & 0xfff0) == 0xfff0)) {
trace(dosfs_debug,("\n?DbmapS x%x+%x -> %x %x", ip->extent, max, n, lblkno));
			return -1;
		}
		lblkno--;
		if (n == ret + 1) {
			rsiz += dosfsmp->im_bsize;
		} else {
			s = n;
			rsiz = dosfsmp->im_bsize;
		}
		ret = n;
	}
	/* how far into the future (including read-ahead) */
	max = dosfsmp->chunk_size;
	max = dosfs_lblkno(dosfsmp,(max<<1)) - 1;
	m = ret;
	do {
		n = (daddr_t) FAT_GET_ENTRY(dosfsmp->fat, dosfsmp->fat_bits, m);
		if (n == m + 1) {
			rsiz += dosfsmp->im_bsize;
		} else
			break;
		m = n;
		lblkno++;
	} while (lblkno < max && n != 0xffff);
	ret = ((ret - 2) * dosfsmp->im_clsiz) + dosfsmp->clusters_start;
	s = ((s - 2) * dosfsmp->im_clsiz) + dosfsmp->clusters_start;
    out:
	if (seq_startp) {
		*seq_startp = s;
		*ssiz = rsiz;
	}
	return (ret);
}


/*
 * Extend an inode. ALLOCED is the (last) block number we
 * assume already allocated to the file.  This function
 * might have to zero between ip->i_size and alloced to
 * give a consistent picture of the file content.
 */
public int
dosfs_iextend(
	struct dosfs_node	*ip,
	off_t			length,
	int			alloced)
{
	int			nblks;
	unsigned int		i, j;
	register struct dosfs_mount *dosfsmp = ip->i_mnt;

trace(dosfs_debug,("Diextend %x(%x) -> %x ", ip->o_number, alloced, length));

	/* Sanity. Includes length==0 */
	if (length <= ip->i_size)
		return dosfs_itrunc(ip, length);

	if (ip->o_number == DOS_ROOTINO)
		return ENOSPC;

	/*
	 * How many blocks do we need, where do we start from.
	 * Note we must always have/keep at least one block,
	 * to guarantee unique inode numbers.
	 */
	nblks = dosfs_lblkno(dosfsmp, length-1) + 1;

	if ((i = ip->extent) != 0)	/* chain head, account for it */
	for (nblks--; nblks > 0; nblks--) {
		j = FAT_GET_ENTRY(dosfsmp->fat, dosfsmp->fat_bits, i);
		if (j >= 0xfff8) break;
		i = j;
	}

	/*
	 *  Allocate all the blocks. Zero the necessary ones.
	 */
	if (ip->i_size > 0)
		alloced -= dosfs_lblkno(dosfsmp, ip->i_size-1);
	while (nblks > 0) {
		i = fat_alloc_entry(dosfsmp, i);
trace(dosfs_debug>1,("fallo %d: %x ", nblks, i));
		if (i == 0xffff) break; /* overflow */
		if (ip->extent == 0) {
			ip->extent = i;
			if (alloced <= 0 && ((ip->i_vnode.v_mode&VFMT) == IFREG))
				alloced = 1;
		}
		if (alloced > 0) {
			register struct buf *bp;
			register int bn = dosfs_cltodb(dosfsmp,i);
			bp = getblk(ip->o_devvp, bn, dosfsmp->im_bsize);
trace(dosfs_debug>1,("clr %d: %x %x ", alloced, i, bn));
			clrbuf(bp);
			bdwrite(bp);
			alloced--;
		}
		nblks--;
	}

	/*
	 * Adjust size appropriately
	 */
	imark(ip, (ICHG|IUPD));
	ip->i_size = length - dosfs_lblktosize(dosfsmp, nblks);
	return (nblks) ? ENOSPC : 0;
}

/*
 * Contrary-wise..
 */
public int
dosfs_itrunc(
	struct dosfs_node	*ip,
	off_t			length)
{
	int			bno;
	unsigned int		i, j, osize, offset;
	register struct dosfs_mount *imp = ip->i_mnt;

trace(dosfs_debug,("Ditrunc %x(%x) -> %x ", ip->o_number, ip->i_size, length));

	imark(ip, (ICHG|IUPD));

	/* once again, this is what ufs does */
	if (ip->i_size <= length)
		return 0;

	/*
	 * Update the size of the file. Beware of partials in last block.
	 * Also, if we zero the file we must zero the first block because
	 * we are not going to reallocate it.
	 */
	offset = dosfs_blkoff(imp, length);
	osize = ip->i_size;
	if ((length == 0) || (offset != 0)) {
		int lbn, bn, size;
		struct buf *bp;

		lbn = dosfs_lblkno(imp, length);
		bn = dosfs_bmap(ip, lbn);
		if (bn < 0) return EIO; /* ?? */
		ip->i_size = length;
		if (ip->i_header.pager != MEMORY_OBJECT_NULL)
			inode_uncache(ip);
		size = dosfs_blksize(imp, ip, lbn);
		bp = bread(ip->o_devvp, bn, size);
		if (bp->b_error || bp->b_flags & B_ERROR) {
			ip->i_size = osize;
			brelse(bp);
			return EIO;
		}
		bzero(bp->b_un.b_addr + offset, (unsigned)(size - offset));
		bdwrite(bp);
	}
	ip->i_size = length;

	/*
	 * Truncate chain, but we cannot lose the first block.
	 */
	bno = (length > 0) ? dosfs_lblkno(imp, length-1) : 0;
	ip->last_lbn = -1;

	i = ip->extent;
	for (; bno > 0; bno--) {
		j = FAT_GET_ENTRY(imp->fat, imp->fat_bits, i);
		if (j >= 0xfff8) break;
		i = j;
	}
	j = FAT_GET_ENTRY(imp->fat, imp->fat_bits, i);
	FAT_SET_ENTRY(imp->fat, imp->fat_bits, i, 0xffff);
	ip->last_lbn = -1;


	/*
	 *  De-Allocate all the remaining blocks
	 */
	{
		register int hint = 0;

		while (j && j < 0xfff8) {
			/* next one up */
			i = FAT_GET_ENTRY(imp->fat, imp->fat_bits, j);
			/* free current one */
trace(dosfs_debug>1,("free %x ", j));
			FAT_SET_ENTRY(imp->fat, imp->fat_bits, j, 0);
			/* save hint to head of chain */
			if (hint++ == 0) imp->ffree = j;
			imp->im_free++; imp->im_alloc--;
			/* move on */
			j = i;
		}
	}
	imp->im_dirty = 2;

	return 0;
}


public void
dosfs_igone(
	struct dosfs_node	*ip)
{
	register struct dosfs_mount *imp = ip->i_mnt;
	register unsigned int		i, j;

	j = ip->extent;
trace(dosfs_debug,("Digone %x %x ", ip->o_number, j));
	while (j && j < 0xfff8) {
		i = FAT_GET_ENTRY(imp->fat, imp->fat_bits, j);
		FAT_SET_ENTRY(imp->fat, imp->fat_bits, j, 0);
		imp->im_free++; imp->im_alloc--;
		j = i;
	}
	ip->extent = 0;
	ip->i_size = 0;
	imp->im_dirty = 2;
}

/*
 * 	File allocation table (FAT) utils
 */
private unsigned short
fat_get_entry(
	unsigned short	*fat,
	unsigned int	fbits,
	unsigned int	entry)
{
	if (entry < 2)
		return 0xffff;

	if (fbits == 16)
		return fat[entry];

	else if (fbits != 12) {
		printf("Bad fatbits %d!\n");
		return 0xffff;
	}

	fbits = (entry * 3) / 4;/* 12/16 == 3/4 */

	switch (entry & 0x3) {
	case 0:
		fbits = fat[fbits];
		break;
	case 1:
		fbits = (fat[fbits] >> 12) | (fat[fbits + 1] << 4);
		break;
	case 2:
		fbits = (fat[fbits] >>  8) | (fat[fbits + 1] << 8);
		break;
	case 3:
		fbits = fat[fbits] >> 4;
		break;
	}

	fbits &= 0xfff;
	if ((fbits & 0xff0) == 0xff0)
		fbits |= 0xf000;/* make specials always 16 bits */

	return fbits;
}

private int
fat_set_entry(
	unsigned short	*fat,
	unsigned int	fbits,
	unsigned int	entry,
	unsigned short	value)
{
	if (entry < 2)
		return EINVAL;

	if (fbits == 16) {
		fat[entry] = value;
		return 0;
	} else if (fbits != 12) {
		printf("Bad fatbits %d!\n");
		return EINVAL;
	}

	value &= 0xfff;
	fbits = (entry * 3) / 4;/* 12/16 == 3/4 */

	switch (entry & 0x3) {
	case 0:
		fat[fbits] = (fat[fbits] & 0xf000) | value;
		break;
	case 1:
		fat[fbits] = (fat[fbits] & 0x0fff) | (value << 12);
		fbits++;
		fat[fbits] = (fat[fbits] & 0xff00) | (value >>  4);
		break;
	case 2:
		fat[fbits] = (fat[fbits] & 0x00ff) | (value <<  8);
		fbits++;
		fat[fbits] = (fat[fbits] & 0xfff0) | (value >>  8);
		break;
	case 3:
		fat[fbits] = (fat[fbits] & 0x000f) | (value <<  4);
		break;
	}

	return 0;
}

/*
 * Count entries in a chain
 */
int fat_lo3;

public int
dosfs_file_len(
	struct dosfs_mount *dosfsmp,
	unsigned int	   extent)
{
	int	len = 0;

	if (extent > DOS_ROOTINO) {
		do {
			extent = FAT_GET_ENTRY(dosfsmp->fat, dosfsmp->fat_bits, extent);
			len++;
fat_lo3++;
		} while (extent && (extent < 0xfff8));
		return (len * dosfsmp->im_bsize);
	}
	/* Wants len of root */
	return (dosfsmp->clusters_start - dosfsmp->rootdir_start);
}

/*
 * Add an entry to a chain
 */
int fat_l0, fat_l1, fat_l2;

private unsigned short
fat_alloc_entry(
	struct dosfs_mount	*imp,
	unsigned int		start)
{
	unsigned int	i, j, s;

	/* From anywhere ? */
	if ((s = start) == 0)
		s = imp->ffree;

	mutex_lock(&imp->fat_lock);

fat_l0++;
	for (i = s; i < imp->fat_entries; i++) {
		j = FAT_GET_ENTRY(imp->fat, imp->fat_bits, i);
		if (j == 0) goto found;
fat_l1++;
	}
	for (i = s - 1; i >= 2; i--) {
		j = FAT_GET_ENTRY(imp->fat, imp->fat_bits, i);
		if (j == 0) goto found;
fat_l2++;
	}

	/* Disk full */
	mutex_unlock(&imp->fat_lock);
	return 0xffff;

found:
	/* extend/truncate chain */
	FAT_SET_ENTRY(imp->fat, imp->fat_bits, i, 0xffff);
	imp->ffree = i+1;
	imp->im_free--;
	imp->im_alloc++;
	if (start != 0)
		FAT_SET_ENTRY(imp->fat, imp->fat_bits, start, i);
	imp->im_dirty = 2;

	mutex_unlock(&imp->fat_lock);

	return i;
}

/*
 * Collect the allocation statistics,
 * called at mount time.
 */
dosfs_stat_fat( struct dosfs_mount *dosfsmp)
{
	int             i, max;
	register int    nalloc, nfree;
	register int    fe;

	max = dosfsmp->im_size - (dosfsmp->clusters_start / dosfsmp->im_clsiz);
	fe = (dosfsmp->fat_len * MDOS_SECTOR_SIZE * NBBY) / dosfsmp->fat_bits;
	dosfsmp->fat_entries = (fe > max) ? max : fe;

	if (max > fe)
		max = fe;	/* sanity */

	nalloc = nfree = 0;
	for (i = 0; i < max; i++)
		if (FAT_GET_ENTRY(dosfsmp->fat, dosfsmp->fat_bits, i) == 0) {
			if (nfree++ == 0)
				dosfsmp->ffree = i;
		} else
			nalloc++;

	fe = dosfsmp->clusters_start / dosfsmp->im_clsiz;
	dosfsmp->im_free = nfree;
	dosfsmp->im_alloc = nalloc + fe;
trace(dosfs_debug, ("stat_fat: %d %d\n", dosfsmp->im_free, dosfsmp->im_alloc));
}

