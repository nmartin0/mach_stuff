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
 * $Log:	dosmkfs.c,v $
 * Revision 2.1.1.2  93/12/01  11:23:23  af
 * 	Read in smallish chunks.
 * 
 * Revision 2.1.1.1  93/08/27  13:20:49  af
 * 	Created.
 * 	[93/07/26            af]
 * 
 */

/*
 * Yet Another Mkfs, for DOS filesystems.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fstab.h>
#include <signal.h>

#include <cthreads.h>
#include <dosfs/dosfs.h>

#define DOS_ROOTINO 0

struct dosfs_directory_record root_dirtemplate[3] =
{ { "IO      ", "SYS", DOS_ATTR_SYS|DOS_ATTR_HIDDEN|DOS_ATTR_RONLY, },
  { "MSDOS   ", "SYS", DOS_ATTR_SYS|DOS_ATTR_HIDDEN|DOS_ATTR_RONLY, },
  { "MS-DOS_5", "   ", DOS_ATTR_ARCHIVE|DOS_ATTR_LABEL, } };

struct dosfs_bootsector volume_template = {
	{ 0xeb, 0x3c, 0x90},
	"MSDOS5.0",
	{0,0}, 4, {1,0}, 2, {0,0}, {0,0}, 0xf8,
	{0,0}, {0,0}, {0,0}, {0,0,0,0}, {0,0,0,0}, 0,
	0, 0x29, {0,0,0,0}, "NOT MOUNTED", "FAT16   ",
	{
	    0x33fa, 0x8ec0, 0xbcd0, 0x7c00, 0x0716, 0x78bb, 0x3600, 0x37c5,
	    0x561e, 0x5316, 0x3ebf, 0xb97c, 0x000b, 0xf3fc, 0x06a4, 0xc61f,
	    0xfe45, 0x8b0f, 0x180e, 0x887c, 0xf94d, 0x4789, 0xc702, 0x3e07,
	    0xfb7c, 0x13cd, 0x7972, 0xc033, 0x0639, 0x7c13, 0x0874, 0x0e8b,
	    0x7c13, 0x0e89, 0x7c20, 0x10a0, 0xf77c, 0x1626, 0x037c, 0x1c06,
	    0x137c, 0x1e16, 0x037c, 0x0e06, 0x837c, 0x00d2, 0x50a3, 0x897c,
	    0x5216, 0xa37c, 0x7c49, 0x1689, 0x7c4b, 0x20b8, 0xf700, 0x1126,
	    0x8b7c, 0x0b1e, 0x037c, 0x48c3, 0xf3f7, 0x0601, 0x7c49, 0x1683,
	    0x7c4b, 0xbb00, 0x0500, 0x168b, 0x7c52, 0x50a1, 0xe87c, 0x0092,
	    0x1d72, 0x01b0, 0xace8, 0x7200, 0x8b16, 0xb9fb, 0x000b, 0xe6be,
	    0xf37d, 0x75a6, 0x8d0a, 0x207f, 0x0bb9, 0xf300, 0x74a6, 0xbe18,
	    0x7d9e, 0x5fe8, 0x3300, 0xcdc0, 0x5e16, 0x8f1f, 0x8f04, 0x0244,
	    0x19cd, 0x5858, 0xeb58, 0x8be8, 0x1a47, 0x4848, 0x1e8a, 0x7c0d,
	    0xff32, 0xe3f7, 0x0603, 0x7c49, 0x1613, 0x7c4b, 0x00bb, 0xb907,
	    0x0003, 0x5250, 0xe851, 0x003a, 0xd872, 0x01b0, 0x54e8, 0x5900,
	    0x585a, 0xbb72, 0x0105, 0x8300, 0x00d2, 0x1e03, 0x7c0b, 0xe2e2,
	    0x2e8a, 0x7c15, 0x168a, 0x7c24, 0x1e8b, 0x7c49, 0x4ba1, 0xea7c,
	    0x0000, 0x0070, 0x0aac, 0x74c0, 0xb429, 0xbb0e, 0x0007, 0x10cd,
	    0xf2eb, 0x163b, 0x7c18, 0x1973, 0x36f7, 0x7c18, 0xc2fe, 0x1688,
	    0x7c4f, 0xd233, 0x36f7, 0x7c1a, 0x1688, 0x7c25, 0x4da3, 0xf87c,
	    0xf9c3, 0xb4c3, 0x8b02, 0x4d16, 0xb17c, 0xd206, 0x0ae6, 0x4f36,
	    0x8b7c, 0x86ca, 0x8ae9, 0x2416, 0x8a7c, 0x2536, 0xcd7c, 0xc313,
	    0x0a0d, 0x6f4e, 0x2d6e, 0x7953, 0x7473, 0x6d65, 0x6420, 0x7369,
	    0x206b, 0x726f, 0x6420, 0x7369, 0x206b, 0x7265, 0x6f72, 0x0d72 },
	{{
	    0x520a, 0x7065, 0x616c, 0x6563, 0x6120, 0x646e, 0x7020, 0x6572,
	    0x7373, 0x6120, 0x796e, 0x6b20, 0x7965, 0x7720, 0x6568, 0x206e,
	    0x6572, 0x6461, 0x0d79, 0x000a, 0x4249, 0x424d, 0x4f49, 0x2020,
	    0x4f43, 0x494d, 0x4d42, 0x4f44, 0x2053, 0x4320, 0x4d4f, 0x0000 },
	{ 0x55, 0xaa}}
};

#include <setjmp.h>
jmp_buf	io_error;

#define	DEFAULT_ROOTDIREN	128

char	*program;
int	readonly;
unsigned int	fssize;
unsigned int	secsize = 512, blksize = 4192;
int	spt, tpc;
int	medium = 0xf8; /* fixed disk */
int	bootsecs = 1;
int	root_direntries = DEFAULT_ROOTDIREN;
int	nfat = 2;

char	BootSmall[] = "Needs more than %d boot sectors\n";
char	RootSmall[] = "Root must be a multiple of the cluster size, will use %d root entries\n";
char	BlkBig[] = "Block size %d seems excessive\n";
char	WarnUnall[] = "Warning: %d sector(s) in last cylinder unallocated\n";
char	Stat1[] = "%s:\t%d sectors in %d cylinders of %d tracks, %d sectors\n";
char	Stat2[] = "\t%.1fMb total, %d blocks free, blocksize %d secsize %d\n";
char	Stat3[] = "%d FAT table(s) of %d sectors each at sectors:\n";
char	RootDirs[] = "\n%d Root directory entries, at sector %d\n";

off_t
dolseek(int fi, off_t where, int how);
void
doread(int fi, char *buf, unsigned int len);
void
dowrite(int fi, char *buf, unsigned int len);

/*void *malloc();*/


void usage()
{
	printf("Usage: %s %s %s\n",
		program,
		"[-N] special size [spt [tpc [blksize [secsize [media_code",
		 "[bootsecs [root_entries [nFAT]]]]]]]]");
	exit(1);
}

main(
	int	argc,
	char	**argv)
{
	char	*fname;

	program = argv[0];
	if (argc < 3) usage();

	if (strcmp(argv[1],"-N") == 0) {
		readonly = 1;
		if (argc < 4) usage();
		argc--, argv++;
	}

	fname = argv[1];
	fssize = atoi(argv[2]);

	if (argc > 3)
		spt = atoi(argv[3]);
	if (argc > 4)
		tpc = atoi(argv[4]);
	if (argc > 5)
		blksize = atoi(argv[5]);
	if (argc > 6)
		secsize = atoi(argv[6]);
	if (argc > 7)
		medium = medium_type(argv[7]);
	if (argc > 8)
		bootsecs = atoi(argv[8]);
	if (argc > 9)
		root_direntries = atoi(argv[9]);
	if (argc > 10)
		nfat = atoi(argv[10]);

	return mkfs(fname);
}

mkfs(
	char	*filename)
{
	int	fi;
	char	*buf;
	unsigned short *fat;
	int	overhead, fatso, fbits, fatlen, nblocks;
	int	i;

	fi = open(filename, readonly ? O_RDONLY : O_RDWR, 0);
	if (fi < 0)
		goto badexit;

	if (setjmp(io_error))
		goto badexit;

	/*
	 * Validate arguments: size
	 */
	if (secsize < 512 || nfat < 1 || fssize < 1)
		goto badargs;

	dolseek(fi, (fssize - 1) * secsize, 0);
	buf = (char *)malloc(secsize);
	if (buf == NULL) {
		errno = ENOMEM;
		goto badexit;
	}
	doread(fi, buf, secsize);

	if (bootsecs < 1) {
		printf(BootSmall, bootsecs);
		return (2);
	}

	blksize = (blksize / secsize) * secsize;

	/*
	 * In order not to buffer-conflict between root and
	 * first cluster (causing kernel deadlocks) we must
	 * assure root is a multiple of the cluster size.
	 */
	{
		unsigned long rootsize;
		int	      warnuser;

		warnuser = root_direntries != DEFAULT_ROOTDIREN;
		rootsize = root_direntries * sizeof(struct dosfs_directory_record);
		if (rootsize < blksize ||
		    ((rootsize % blksize) != 0))
			rootsize = ((rootsize + blksize - 1) / blksize) * blksize;
		root_direntries = rootsize / sizeof(struct dosfs_directory_record);
		if (warnuser)
			printf(RootSmall, root_direntries);
	}

	/*
	 * find FAT size
	 */
	overhead = ((root_direntries * sizeof(struct dosfs_directory_record)) +
			secsize - 1) / secsize;
	overhead += bootsecs;
	if (fssize <= overhead)
		goto badargs;
	if (blksize < secsize ||
	    blksize > (secsize * 255))
		goto badargs;

	fatlen = 0;
	do {
		fatso = fatlen * nfat;
		nblocks = (fssize - (overhead + fatso)) / (blksize / secsize);
		fbits = (nblocks > 4093) ? 16 : 12;
		fatlen = (((fbits * nblocks) / 8/*NBBY*/) + secsize - 1) / secsize;
	} while (fatso != (fatlen * nfat));

	/*
	 * Validate overhead viz size and blksize
	 */
	overhead += fatso;
	if (fssize <= overhead || (blksize / secsize) > (fssize - overhead)) {
		printf(BlkBig, blksize);
		return(2);
	}

	/*
	 * Say something
	 */
	nblocks = (fssize - overhead) / (blksize / secsize);
	i = (fssize - overhead) - (nblocks * (blksize / secsize));
	if (i)
		printf(WarnUnall, i);

	i = fssize / (spt * tpc);
	if ((i * (spt * tpc)) != fssize)
		i++;
	printf(Stat1, filename, fssize, i, tpc, spt);
	/* same as old mkfs, "Mb" has strange meanings */
	printf(Stat2, (float)(fssize * secsize) / (1000000),
		nblocks, blksize, secsize);
	printf(Stat3, nfat, fatlen);

	/*
	 * Set volume descriptor fields
	 */
	volume_template.descr = medium;
	mkdosfsnum_16(volume_template.secsiz, secsize);
	volume_template.clsiz = blksize / secsize;
	mkdosfsnum_16(volume_template.nsect, spt);
	mkdosfsnum_16(volume_template.nheads, tpc);
	if (fssize < 0x10000)
		mkdosfsnum_16(volume_template.psect, fssize);
	else
		mkdosfsnum_32(volume_template.bigsect, fssize);
	mkdosfsnum_16(volume_template.nrsvsect, bootsecs);
	mkdosfsnum_16(volume_template.dirents, root_direntries);
	volume_template.nfat = nfat;
	mkdosfsnum_16(volume_template.fatlen, fatlen);
	{
		time_t now = time(0);
		mkdosfsnum_32(volume_template.volid, now);
	}
	volume_template.res1[4] = (fbits == 12) ? '2' : '6';

	/*
	 * Write volume descriptor
	 */
	bzero(buf, secsize);
	dolseek(fi, 0, 0);
	bcopy(&volume_template, buf, sizeof volume_template);
	dowrite(fi, buf, secsize);

	/*
	 * Skip if we should
	 */
	dolseek(fi, (bootsecs - 1) * secsize, L_INCR);

	/*
	 * Prepare and write FATs
	 */
	fat = (unsigned short *) malloc(fatlen * secsize);
	bzero(fat, fatlen * secsize);
	fat[0] = 0xfff0;
	if (fbits == 16) {
		fat[1] = 0xffff;
	} else {
		fat[1] = 0xff;
	}
	i = 0;
	while (nfat-- > 0) {
		dowrite(fi, (char *)fat, fatlen * secsize);
		printf(" %d,", bootsecs);
		bootsecs += fatlen;
		if (++i == 10) {
			i = 0;
			printf("\n");
		}
	}

	/*
	 * Root directories
	 */
	i = root_direntries;
	bzero(buf, secsize);
	root_dirtemplate[0].name[0] = DOS_NAME_DELETED;
	root_dirtemplate[1].name[0] = DOS_NAME_DELETED;
	bcopy(root_dirtemplate, buf, sizeof root_dirtemplate);
	dowrite(fi, buf, secsize);
	fatso = secsize / sizeof(struct dosfs_directory_record);
	root_direntries -= fatso;
	if (root_direntries > 0) {
		bzero(buf, secsize);
		do {
			dowrite(fi, buf, secsize);
			root_direntries -= fatso;
		} while (root_direntries > 0);
	}
	printf(RootDirs, i, bootsecs);
	return (0);
badargs:
	errno = EINVAL;
badexit:
	perror(filename);
	close(fi);
	return(2);
}

/*
 * Medium codes
 * Nicknames are like "<n> inX <sectors> X <sides>"
 */
struct mtable {
	char	*nickname;
	int	media_code;
} media_names[] = {
	{ "3.5inX18X2",0xf0 },
	{ "3.5in",0xf0 },

	{ "disk", 0xf8 },

	{ "5inX15X2",0xf9 },
	{ "3.5inX9X2",0xf9 },

	{ "5inX9",0xfc },

	{ "5inX9X2",0xfd },
	{ "8inXs",0xfd },

	{ "8in",0xfe },
	{ "5inX8",0xfe },

	{ "5inX8X2",0xff },
};

int
medium_type(
	char	*name)
{
	struct mtable *tb;

	for (tb = media_names; tb->nickname; tb++)
		if (strcmp(name, tb->nickname) == 0)
			return tb->media_code;
	return atoh(name);
}

/*
 * Disk I/O
 */
void
dowrite(
	int	fi,
	char	*buf,
	unsigned int len)
{
	int ret;

	if (readonly)
		return;
	ret = write(fi,buf,len);
	if (ret != len) {
		if (ret != -1) errno = EIO;
		longjmp(io_error,1);
	}
}

void
doread(
	int	fi,
	char	*buf,
	unsigned int len)
{
	int ret, rlen;

	/* Cover for system bug on large reads
	   of raw devices by chunking up */
	while (len > 0) {
#define _64K (64*1024)
		rlen = (len > _64K) ? _64K : len;
		ret = read(fi,buf,rlen);
		if (ret != rlen) {
			if (ret != -1) errno = EIO;
			longjmp(io_error,1);
		}
		len -= rlen;
		buf += rlen;
	}
}

off_t
dolseek(
	int	fi,
	off_t	where,
	int	how)
{
	off_t ret;
	ret = lseek(fi, where, how);
	if (ret == -1)
		longjmp(io_error,1);

	return ret;
}
