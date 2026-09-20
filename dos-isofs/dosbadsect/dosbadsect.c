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
 * $Log:	dosbadsect.c,v $
 * Revision 2.1.1.3  93/11/29  17:56:20  af
 * 	Added force and marker options.
 * 
 * Revision 2.1.1.2  93/09/01  22:23:21  af
 * 	De-lint.
 * 	[93/09/01            af]
 * 
 * Revision 2.1.1.1  93/09/01  14:15:50  af
 * 	Created.
 * 	[93/09/01            af]
 * 
 */

/*
 * Mark bad blocks in a DOS filesystem
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fstab.h>
#include <signal.h>

#include <mach.h>
#include <dosfs/dosfs.h>

#define DOS_ROOTINO 0

/*
 * Volume label
 */
char	bsec[MDOS_SECTOR_SIZE];
struct dosfs_bootsector *vdp = (struct dosfs_bootsector *)bsec;

/*
 * Mount info
 */
struct dosfs_mount dosfsmp;
struct dosfs_mount *imp = &dosfsmp;

/*
 * Other globals
 */
typedef enum { MARK, UNMARK } act_t;

int	readonly;
act_t	action = MARK;
int	bbmark = 0xfffe;
int	force = 0;

/*
 * Messages and other strings
 */
char	*program;

char	BadBitsize[] = "BAD BITSIZE %d IN FAT\n";
char	NoBad[] = "No bad blocks\n";
char	BlkRange[] = "Block %d is out of range\n";
char	BlkAlloced[] = "Block %d is in use, remove the file first\n";
char	BlkNotBad[] = "Block %d is marked x%x. I do not think that is a bad block.\n";
char	BlkMarked[] = "Block %d is marked x%x. I think that is a bad block already.\n";

/*
 * I/O
 */
#include <setjmp.h>
jmp_buf	io_error;

off_t
dolseek(int fi, off_t where, int how);
void
doread(int fi, char *buf, unsigned int len);
void
dowrite(int fi, char *buf, unsigned int len);

void *malloc();


void usage()
{
	printf("Usage: %s %s\n",
		program,
		"[-u] special [blockno ...]");
	exit(1);
}

int main(
	int	argc,
	char	**argv)
{
	int	fi, ret;

	program = argv[0];
swtches:
	if (argc < 2) usage();
	if (strcmp(argv[1],"-u") == 0) {
		action = UNMARK;
		argc--, argv++;
		goto swtches;
	}
	if (strcmp(argv[1],"-f") == 0) {
		force = 1;
		argc--, argv++;
		goto swtches;
	}
	if (strcmp(argv[1],"-m") == 0) {
		argc--, argv++;
		if (argc < 3) usage();
		bbmark = atoh(argv[1]);
		argc--, argv++;
		goto swtches;
	}

	readonly = (argc == 2);

	fi = open(argv[1], readonly ? O_RDONLY : O_RDWR, 0);
	if (fi < 0)
		goto badexit;

	if (setjmp(io_error))
		goto badexit;

	if (errno = mnt(fi))
		goto badexit;

	if (readonly)
		ret = printbad(fi);
	else
		ret = badsect(fi, action, argv+2);
	close(fi);
	return ret;
badexit:
	perror(argv[1]);
	close(fi);
	return(2);
}

/*
 * Read the volume information and primary FAT
 */
mnt( int fi)
{
	int logical_block_size, i;

	dolseek(fi, MDOS_LABELSECTOR * MDOS_SECTOR_SIZE, 0);
	doread(fi, bsec, MDOS_SECTOR_SIZE);

	if (dosfsnum_16(vdp->label.magic) != BIOS_LABEL_MAGIC)
		return EINVAL;

	bzero(imp, sizeof(*imp));

	/*
	 * XXXX add code to mistrust and fix the volume info
	 */

	imp->im_clsiz = vdp->clsiz;
	logical_block_size = dosfsnum_16(vdp->secsiz) * vdp->clsiz;

	imp->im_bsize = logical_block_size;
	imp->im_bmask = ~(imp->im_bsize - 1);
	imp->im_bshift = 0;
	while ((1 << imp->im_bshift) < imp->im_bsize)
		imp->im_bshift++;

	imp->fat_start = dosfsnum_16(vdp->nrsvsect);
	imp->fat_len = dosfsnum_16(vdp->fatlen);
	imp->n_fat = vdp->nfat;
	i = dosfsnum_16(vdp->psect);
	if (i == 0)
		i = dosfsnum_32(vdp->bigsect);
	imp->im_size = i / vdp->clsiz;
	imp->fat_bits =  (i > 4087) ? 16 : 12;

	imp->rootdir_entries = dosfsnum_16(vdp->dirents);
	imp->rootdir_start = imp->fat_start +
				 (imp->n_fat * imp->fat_len);

	i = imp->rootdir_entries * MDOS_DIR_SIZE;
	i = (i + MDOS_SECTOR_SIZE - 1) / (unsigned) MDOS_SECTOR_SIZE;
	imp->clusters_start = imp->rootdir_start + i;

	/* Read all of the FAT in */
	imp->fat = (u_short *)malloc(imp->fat_len * MDOS_SECTOR_SIZE);

	dolseek(fi, (imp->fat_start * MDOS_SECTOR_SIZE), 0);
	doread(fi, (char *)imp->fat, imp->fat_len * MDOS_SECTOR_SIZE);

	dosfs_stat_fat(imp);

	imp->im_flags = 0;

	return (0);
}

/*
 * 	File allocation table (FAT) utils
 */
unsigned short
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
		fprintf(stderr, BadBitsize, fbits);
		errno = EINVAL;
		longjmp(io_error,1);
		return 0xffff;	/* compiler happy */
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

int
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
		fprintf(stderr, BadBitsize, fbits);
		errno = EINVAL;
		longjmp(io_error,1);
		return EINVAL;	/* compiler happy */
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
		if (fat_get_entry(dosfsmp->fat, dosfsmp->fat_bits, i) == 0) {
			if (nfree++ == 0)
				dosfsmp->ffree = i;
		} else {
			nalloc++;
			fe = i;
		}
	fe = dosfsmp->clusters_start / dosfsmp->im_clsiz;
	dosfsmp->im_free = nfree;
	dosfsmp->im_alloc = nalloc + fe;
}

/*
 * Print all bad blocks
 */
printbad(
	int	fi)
{
	register int    i, n = 0;
	unsigned short	*fat;
	int		bits;
	int		nent;
	register unsigned int j;

	fat = imp->fat;
	bits = imp->fat_bits;
	nent = imp->fat_entries;

	for (i = 2; i < nent; i++) {

		j = fat_get_entry(fat, bits, i);
		if (j < 0xfff0 || j == 0xffff)
			continue;

		/* why use two switches when one would do.. */
		if ((action == UNMARK) || (j != bbmark))
			printf("x%x at ", j);
		printf("%d\n", i);
		n++;
	}
	if (n == 0 && action == UNMARK)
		printf(NoBad);
	return 0;
}

/*
 * Mark/unmark bad blocks
 */
badsect(
	int	fi,
	act_t	action,
	char	**bblist)
{
	char	*bl;
	int	i, mods = 0;
	unsigned int j;

	while ((bl = *bblist++) != (char *)0) {

		i = atoi(bl);
		if (i < 2 || i >= imp->fat_entries) {
			fprintf(stderr, BlkRange, i);
			continue;
		}

		j = fat_get_entry(imp->fat, imp->fat_bits, i);

		if (action == MARK) {
			if (j != 0 && !force) {
				if (j < 0xfff0 || j == 0xffff)
					fprintf(stderr, BlkAlloced, i);
				else
					fprintf(stderr, BlkMarked, i, j);
				continue;
			}
			(void) fat_set_entry(imp->fat, imp->fat_bits, i, bbmark);
			mods++;
		}
		if (action == UNMARK) {
			if (((j < 0xfff0) || (j > 0xfffe)) && !force) {
				fprintf(stderr, BlkNotBad, i, j);
				continue;
			}
			(void) fat_set_entry(imp->fat, imp->fat_bits, i, 0);
			mods++;
		}
	}
	if (mods)
	    for (i = 0; i < imp->n_fat; i++) {
		dolseek(fi,
			(imp->fat_start * MDOS_SECTOR_SIZE) +
			(i * imp->fat_len * MDOS_SECTOR_SIZE),
			0);
		dowrite(fi, (char *)imp->fat, imp->fat_len * MDOS_SECTOR_SIZE);
	}
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
	int ret;

	ret = read(fi,buf,len);
	if (ret != len) {
		if (ret != -1) errno = EIO;
		longjmp(io_error,1);
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
