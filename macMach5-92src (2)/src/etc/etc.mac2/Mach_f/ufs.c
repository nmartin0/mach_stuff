/* read-only access to a Unix file system */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <Memory.h>

#include "types.h"
#include "param.h"
#include "fs.h"
#include "inode.h"
#include "ufs.h"
#include "scsi.h"
#include "disklabel.h"
#include "string.h"

struct dirstuff {
	long loc;
};

static void zero(ptr, size)
register char *ptr;
register long size;
{
	while (size-- > 0) *ptr++ = 0;
}

/* read disklabel, return non-zero if error */
static int read_disklabel(io)
register struct iob *io;
{
	struct disklabel *dl;
	struct partition *p;
	
	io->i_ma = io->i_buf;
	io->i_cc = DEV_BSIZE;
	io->i_bn = io->i_boff + LABELSECTOR;
	if (scsi_disk_read(io)) return -1;
	dl = (struct disklabel *)&io->i_buf[LABELOFFSET];
	if (dl->d_magic != DISKMAGIC) return E_BAD_LABEL_MAGIC;
	if (dl->d_type != DTYPE_SCSI && dl->d_type != 0) return E_BAD_DRIVE_TYPE;
	p = &dl->d_partitions[scsipart(io->i_dev)];
	if (p->p_fstype != FS_BSDFFS) return E_BAD_FS_TYPE;
	io->i_boff += p->p_offset;
	return 0;
}

/* read inode, return non-zero if error */
static int openi(io, n)
register struct iob *io;
register long n;
{
	register struct dinode *dp;

	io->i_offset = 0;
	io->i_bn = fsbtodb(&io->i_fs, itod(&io->i_fs, n)) + io->i_boff;
	io->i_cc = io->i_fs.fs_bsize;
	io->i_ma = io->i_buf;
	if (scsi_disk_read(io)) return -1;
	dp = (struct dinode *)io->i_buf;
	io->i_ino.i_ic = dp[itoo(&io->i_fs, n)].di_ic;
	return 0;
}

/* read sbmap, return zero if error */
static daddr_t sbmap(io, bn)
register struct iob *io;
daddr_t bn;
{
	register struct inode *ip;
	long i, j, sh;
	daddr_t nb, *bap;

	ip = &io->i_ino;
	if (bn < 0)	return (daddr_t)0;
	/* blocks 0..NDADDR are direct blocks */
	if(bn < NDADDR) {
		nb = ip->i_db[bn];
		return nb;
	}
	/*
	 * addresses NIADDR have single and double indirect blocks.
	 * the first step is to determine how many levels of indirection.
	 */
	sh = 1;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(&io->i_fs);
		if (bn < sh) break;
		bn -= sh;
	}
	if (j == 0) return (daddr_t)0;
	/* fetch the first indirect block address from the inode */
	if ((nb = ip->i_ib[NIADDR - j]) == 0) return (daddr_t)0;
	/* fetch through the indirect blocks */
	for (; j <= NIADDR; j++) {
		if (io->i_blknos[j] != nb) {
			io->i_bn = fsbtodb(&io->i_fs, nb) + io->i_boff;
			if (io->i_b[j] == 0) {
				if (!(io->i_b[j] = (char *)StripAddress(NewPtr(MAXBSIZE)))) return (daddr_t)0;
				zero(io->i_b[j], MAXBSIZE);
			}
			io->i_ma = io->i_b[j];
			io->i_cc = io->i_fs.fs_bsize;
			if (scsi_disk_read(io)) return (daddr_t)0;
			io->i_blknos[j] = nb;
		}
		bap = (daddr_t *)io->i_b[j];
		sh /= NINDIR(&io->i_fs);
		i = (bn / sh) % NINDIR(&io->i_fs);
		nb = bap[i];
		if (nb == 0) return (daddr_t)0;
	}
	return nb;
}

/* read directory entry */
static struct direct *readdir(io, dirp)
register struct iob *io;
register struct dirstuff *dirp;
{
	register struct direct *dp;
	daddr_t lbn, d;
	long off;

	for (;;) {
		if (dirp->loc >= io->i_ino.i_size) return NULL;
		off = blkoff(&io->i_fs, dirp->loc);
		if (off == 0) {
			lbn = lblkno(&io->i_fs, dirp->loc);
			d = sbmap(io, lbn);
			if (d == 0) return NULL;
			io->i_bn = fsbtodb(&io->i_fs, d) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = blksize(&io->i_fs, &io->i_ino, lbn);
			if (scsi_disk_read(io)) return NULL;
		}
		dp = (struct direct *)(io->i_buf + off);
		dirp->loc += dp->d_reclen;
		if (dp->d_ino == 0) continue;
		return dp;
	}
}

/* lookup name in directory, return < 0 if error */
static ino_t dlook(io, name)
register struct iob *io;
register char *name;
{
	register struct direct *dp;
	register struct inode *ip;
	register len;
	struct dirstuff dirp;

	ip = &io->i_ino;
	if ((ip->i_mode&IFMT) != IFDIR) return E_NOT_DIR;
	if (ip->i_size == 0) return E_EMPTY_DIR;
	len = strlen(name);
	dirp.loc = 0;
	for (dp = readdir(io, &dirp); dp != NULL; dp = readdir(io, &dirp)) {
		if(dp->d_ino == 0) continue;
		if (dp->d_namlen == len && !strcmp(name, dp->d_name)) return dp->d_ino;
	}
	return E_NAME_NOT_FOUND;
}

/* lookup file, return < 0 if error */
static ino_t find(io, path)
register struct iob *io;
register char *path;
{
	register char *q;
	register ino_t n;
	char c;
	int result;

	if (path == NULL || *path == 0) return E_NULL_PATH;
	if (result = openi(io, ROOTINO)) return result;
	while (*path) {
		while (*path == '/') path++;
		q = path;
		while(*q != '/' && *q != '\0') q++;
		c = *q;
		*q = '\0';
		if (q == path) path = ((char *)getstr(STR_DOT)); /* "/" means "/." */
		if ((n = dlook(io, path)) < 0) return n;
		if (c == '\0') break;
		if (result = openi(io, n)) return result;
		*q = c;
		path = q;
	}
	return n;
}

#undef getc

/* get character from file, return -1 if EOF */
static long getc(io)
register struct iob *io;
{
	register struct fs *fs;
	register char *p;
	long c, lbn, off, size, diff;

	p = io->i_ma;
	if (io->i_cc <= 0) {
		diff = io->i_ino.i_size - io->i_offset;
		if (diff <= 0) return -1;
		fs = &io->i_fs;
		lbn = lblkno(fs, io->i_offset);
		io->i_bn = fsbtodb(fs, sbmap(io, lbn)) + io->i_boff;
		off = blkoff(fs, io->i_offset);
		size = blksize(fs, &io->i_ino, lbn);
		io->i_ma = io->i_buf;
		io->i_cc = size;
		if (scsi_disk_read(io)) return -1;
		if (io->i_offset - off + size >= io->i_ino.i_size)
			io->i_cc = diff + off;
		io->i_cc -= off;
		p = &io->i_buf[off];
	}
	io->i_cc--;
	io->i_offset++;
	c = (unsigned)*p++;
	io->i_ma = p;
	return c;
}

/* read from file, return count */
long UFSread(io, buf, count)
register struct iob *io;
char *buf;
long count;
{
	register i, size;
	register struct fs *fs;
	long lbn, off;

	if (io->i_offset+count > io->i_ino.i_size)
		count = io->i_ino.i_size - io->i_offset;

	if ((i = count) <= 0) return 0;

	/*
	 * While reading full blocks, do I/O into user buffer.
	 * Anything else uses getc().
	 */
	fs = &io->i_fs;
	while (i) {
		off = blkoff(fs, io->i_offset);
		lbn = lblkno(fs, io->i_offset);
		size = blksize(fs, &io->i_ino, lbn);
		if (off == 0 && size <= i) {
			io->i_bn = fsbtodb(fs, sbmap(io, lbn)) + io->i_boff;
			io->i_cc = size;
			io->i_ma = buf;
			if (scsi_disk_read(io)) return -1;
			io->i_offset += size;
			io->i_cc = 0;
			buf += size;
			i -= size;
		}
		else {
			size -= off;
			if (size > i)
				size = i;
			i -= size;
			do { *buf++ = getc(io); } while (--size);
		}
	}
	return count;
}

/* open file, return non-zero if error */
int UFSopen(io, path, dev)
register struct iob *io;
register char *path;
dev_t dev;
{
	int inum;
	int result;

	zero((char *)io, sizeof(*io));
	io->i_dev = dev;	
	if (result = scsi_disk_open(io)) return result;
	if (result = read_disklabel(io)) return result;
	io->i_ma = (char *)(&io->i_fs);
	io->i_cc = SBSIZE;
	io->i_bn = SBLOCK + io->i_boff;
	io->i_offset = 0;
	if (scsi_disk_read(io)) return -1;
	if (io->i_fs.fs_magic != FS_MAGIC) return E_BAD_FS;
	if ((inum = find(io, path)) < 0) return inum;
	if (result = openi(io, inum)) return result;
	io->i_offset = 0;
	io->i_cc = 0;
	return 0;
}

