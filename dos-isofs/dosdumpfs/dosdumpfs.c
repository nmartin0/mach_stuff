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
 * $Log:	dosdumpfs.c,v $
 * Revision 2.1.1.4  93/12/01  11:25:41  af
 * 	Added -hH switches to show where free blocks are.
 * 	[93/12/01            af]
 * 
 * Revision 2.1.1.3  93/11/30  12:48:27  af
 * 	Added "-l" switch to only print the label.
 * 	Fixed printing of file's blocks.
 * 	Read 64k most at once.
 * 
 * Revision 2.1.1.2  93/09/14  13:14:58  af
 * 	Use ranges when printing a file's block list.
 * 
 * Revision 2.1.1.1  93/09/01  22:20:45  af
 * 	Created.
 * 	[93/09/01            af]
 * 
 */

/*
 * Yet Another Dumpfs, for DOS filesystems.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
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
boolean_t label_only = FALSE;
boolean_t print_holes = FALSE;
boolean_t count_holes = FALSE;

/*
 * Messages and other strings
 */
char	*program;

char	BadBitsize[] = "BAD BITSIZE %d IN FAT\n";


/*
 * I/O
 */
#include <setjmp.h>
jmp_buf	io_error;

off_t
dolseek(int fi, off_t where, int how);
void
doread(int fi, char *buf, unsigned int len);

void *malloc();


void usage()
{
	printf("Usage: %s [-l|-h|-H] filesys|device\n", program);
	exit(1);
}

main(
	int	argc,
	char	**argv)
{
	char	*fname;

	program = argv[0];
	if (argc < 2) usage();

	if (strcmp(argv[1], "-l") == 0) {
		argc--,argv++;
		label_only = TRUE;

	} else if (strcmp(argv[1], "-h") == 0) {
		argc--,argv++;
		print_holes = TRUE;

	} else if (strcmp(argv[1], "-H") == 0) {
		argc--,argv++;
		print_holes = TRUE;
		count_holes = TRUE;
	}
	if (argc < 2) usage();

	fname = argv[1];

	return dumpfs(fname);
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

	if (label_only)
		return(0);

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
 * Tell 'em
 */
dumpfs(
	char	*filename)
{
	int	fi;
	char	*buf;
	unsigned short *fat;
	int	overhead, fatso, fbits, fatlen, nblocks;
	int	i, secsize;

	fi = open(filename, O_RDONLY, 0);
	if (fi < 0)
		goto badexit;

	if (setjmp(io_error))
		goto badexit;

	if (errno = mnt(fi))
		goto badexit;

	print_bootsector(vdp);

	if (label_only)
		return (0);

	if (print_holes) {
		print_fat_holes(imp->fat, imp->fat_bits, imp->fat_entries);
		return (0);
	} else
		print_fat(imp->fat, imp->fat_bits, imp->fat_entries);

	print_all_files(fi);

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

char *
medium_type(
	int	media_code)
{
	struct mtable *tb;
	static char unknown[12];

	for (tb = media_names; tb->nickname; tb++)
		if (media_code == tb->media_code)
			return tb->nickname;
	sprintf(unknown, "x%x", media_code);
	return unknown;
}

/*
 * Print content of boot sector
 */
print_bootsector(
	struct dosfs_bootsector	*vdp)
{
	static char *indent = "      ";
	printf("Contents of the boot sector:\n");
	printf("%sbanner %c%c%c%c%c%c%c%c", indent,
		vdp->banner[0], vdp->banner[1], vdp->banner[2], vdp->banner[3],
		vdp->banner[4], vdp->banner[5], vdp->banner[6], vdp->banner[7]);
	printf(" secsiz %4.4d blksiz  %6.6d fats   %2.2d bootsecs %3.3d\n",
		dosfsnum_16(vdp->secsiz),
		dosfsnum_16(vdp->secsiz) * vdp->clsiz,
		vdp->nfat,
		dosfsnum_16(vdp->nrsvsect)  );
	printf("%srootents %6.6d psect %5.5d bigsect %6.6d hidsec %2.2d fatlen   %3.3d\n",
		indent,
		dosfsnum_16(vdp->dirents),
		dosfsnum_16(vdp->psect),
		dosfsnum_32(vdp->bigsect),
		dosfsnum_32(vdp->nhs),
		dosfsnum_16(vdp->fatlen)  );
	printf("%sdrive %9.9d spt    %4.4d tpc     %6.6d medium %s\n", indent,
		vdp->driveno,
		dosfsnum_16(vdp->nsect),
		dosfsnum_16(vdp->nheads),
		medium_type(vdp->descr));
	printf("\n");
}

/*
 * Print FAT table
 */
print_fat(
	unsigned short	*fat,
	int		bits,
	int		nent)
{
	register int    i, j;
	int             p = 0;

	printf("Contents of the File Allocation Table:\n");

	for (i = 2; i < nent; i++) {
		j = fat_get_entry(fat, bits, i);
		if (j == 0) {
			if (p) {
				p = 0;
				printf("\n");
			}
		}
		else {
			if (p == 0)
				printf("%4.4x: ", i);
			if (p++ == 16) {
				p = 1;
				printf("\n      ");
			}
			printf("%4.4x ", j);
		}
	}
	printf("\n");
}

print_fat_holes(
	unsigned short	*fat,
	int		bits,
	int		nent)
{
	int i = 16, j, head, cur, prev;

	printf("Free Clusters in the File Allocation Table:\n");

#if 0
	for (cur = 2; cur < nent; cur++) {
		j = fat_get_entry(fat, bits, cur);

		if (i >= 16) {
			i = 0;
			printf("\n%s", ""/*free: */);
		}
		if (j == 0) {
			printf(" %4.4d", cur); i++;
		}
	}
#else
	head = 1;
	prev = 1;
	for (cur = 2; cur < nent; cur++) {

		if (i >= 16) {
			i = 0;
			printf("\n%s", ""/*free: */);
		}

		j = fat_get_entry(fat, bits, cur);

/*		if (j != 0 && prev != 0) skip; */
/*		if (j == 0 && prev == 0) skip; */

		if (j == 0 && prev != 0) {
			/* print head */
			printf(" %4.4d", cur); i++;
			head = cur;
		}
		if (j != 0 && prev == 0) {
			/* print tail */
			if (cur - 1 != head) {
				printf("..%4.4d", cur - 1); i++;
				if (count_holes) {
					printf("\t%d\n", cur - head);
					i = 0;
				}
			}
		}

		prev = j;

	}
	if (j == 0 && prev == 0) {
		printf("..%4.4d", cur - 1);
		if (count_holes)
			printf("\t%d", cur - head);
	}
#endif
	printf("\n");
}

/*
 * Find sector number for logical block
 * LBL in file/directory INUM.
 */
bmap(
	ino_t	inum,
	daddr_t	lbl)
{
	daddr_t	ret, l = lbl;

	if (inum == DOS_ROOTINO)
		return imp->rootdir_start + (lbl * imp->im_clsiz);
	ret = inum;
	while (lbl > 0) {
		ret = (daddr_t) fat_get_entry(imp->fat, imp->fat_bits, ret);
			if ((ret == 0) || ((ret & 0xfff0) == 0xfff0)) {
				fprintf(stderr, "? bmap x%x+%x -> %x %x\n",
					inum, l, ret, lbl);
				longjmp(io_error, 1);
				return -1;
			}
		lbl--;
	}
	return ((ret - 2) * imp->im_clsiz) + imp->clusters_start;
}

/*
 * Count the clusters in a chain
 */
int
dosfs_file_len(
	struct dosfs_mount *dosfsmp,
	unsigned int	   extent)
{
	int	len = 0;
	if (extent > DOS_ROOTINO) {
		do {
			extent = fat_get_entry(dosfsmp->fat, dosfsmp->fat_bits, extent);
			len++;
		} while (extent && (extent < 0xfff8));
		return len;
	}
	/* Wants len of root */
	return (dosfsmp->clusters_start - dosfsmp->rootdir_start);
}

/*
 * All the directories we find
 */
struct dirinfo {
	struct dirinfo	*next;
	unsigned char	*name;
	struct dosfs_directory_record *dosdir;
	int	nentries;
	ino_t	inum;
	ino_t	dotdot;
	int	startsec;
	int	modified;
} all_dirs;

/*
 * Scan a disk for all directories.
 * Result is a list with head ALL_DIRS.
 */
find_dirs( int fi)
{
	/* Read root directory */
	all_dirs.next = NULL;
	all_dirs.nentries = imp->rootdir_entries;
	all_dirs.startsec = imp->rootdir_start;
	all_dirs.inum = DOS_ROOTINO;
	all_dirs.dotdot = 0;
	all_dirs.dosdir = 0;
	all_dirs.name = (unsigned char *)"root";
	all_dirs.modified = 0;

	scan_dir(fi, &all_dirs);

}

/*
 * Readin and scan a directory.
 * Recurse for all subdirectories.
 */
scan_dir(
	int		fi,
	struct dirinfo *dir)
{
	int size;
	struct dirinfo *sdir, *h = 0;
	struct dosfs_directory_record *dosdir;
	register int i;

	/*
	 * Readin directory from disk
	 */
	size = dir->nentries * sizeof(struct dosfs_directory_record);
	size = (size + MDOS_SECTOR_SIZE - 1) & ~(MDOS_SECTOR_SIZE - 1);

	if (dir->dosdir == 0)
		dir->dosdir = (struct dosfs_directory_record *)	malloc(size);
	dosdir = dir->dosdir;

	dolseek(fi, dir->startsec * MDOS_SECTOR_SIZE, 0);
	if (dir->startsec == imp->rootdir_entries)
		doread(fi, (char *)dosdir, size);
	else {
		char *buf = (char *)dosdir;
		register int clsiz = MDOS_SECTOR_SIZE * imp->im_clsiz, bl = 1;
		while (1) {
			doread(fi, buf, clsiz);
			buf += clsiz;
			size -= clsiz;
			if (size <= 0) break;
			dolseek(fi, bmap(dir->inum,bl) * MDOS_SECTOR_SIZE, 0);
			bl++;
		}
	}

	/*
	 * Scan all entries, except . and ..
	 */
	i = (dir->inum == DOS_ROOTINO) ? 0 : 2;

	for (; i < dir->nentries; i++) {
		ino_t	ino;

		if ((dosdir[i].name[0] == DOS_NAME_DELETED) ||
		    (dosdir[i].name[0] == DOS_NAME_EMPTY))
			continue;

		ino = dosfsnum_16(dosdir[i].start);

		if (dosdir[i].attr & DOS_ATTR_LABEL)
			continue;

		if ((dosdir[i].attr & DOS_ATTR_DIR) == 0) {
			continue;
		}

		/*
		 * Directory, check if it makes sense
		 */
		if (ino == DOS_ROOTINO) {
			continue;
		}

		/*
		 * DOS takes only one of . or .. entries
		 */
		if (ino == dir->inum) {
			continue;
		}

		if (ino == dir->dotdot) {
			continue;
		}

		/*
		 * Commit this one.
		 */
		sdir = (struct dirinfo *)malloc(sizeof(*sdir));
		sdir->next = h;
		h = sdir;

		sdir->dosdir = 0;
		sdir->inum = ino;
		sdir->dotdot = dir->inum;
		/* how many entries */
		size = dosfs_file_len(imp, sdir->inum);
		size = (size * imp->im_clsiz) * MDOS_SECTOR_SIZE;
		sdir->nentries = size / sizeof(struct dosfs_directory_record);
		sdir->startsec = bmap(sdir->inum,0);
		sdir->name = dosdir[i].name;
		sdir->modified = 0;

		scan_dir(fi, sdir);

	}

	/* Add what we parsed to dir list. */
	sdir = dir->next;
	if (sdir == NULL)
		dir->next = h;
	else {
		while (sdir->next != NULL)
			sdir = sdir->next;
		sdir->next = h;
	}
}

/*
 * Free in-memory data structures
 */
free_dirs()
{
	register struct dirinfo	*dir, *n;

	for (dir = all_dirs.next; dir; dir = n) {
		n = dir->next;
		free(dir->dosdir);
		free(dir);
	}
	if (all_dirs.dosdir)
		free(all_dirs.dosdir);
	bzero(&all_dirs, sizeof all_dirs);
}

/*
 * Print all the blocks in a chain
 */
print_chain(
	unsigned int	head,
	char		*indent)
{
#if 0
	int i = 16;

	do {
		if (i++ == 16) {
			i = 1;
			printf("\n%s", indent);
		}
		printf(" %4.4d", head);
		head = fat_get_entry(imp->fat, imp->fat_bits, head);
	} while (head > 1 && head < 0xfff0);
#else
	int i = 16, prev, cur;

	prev = cur = head;
	do {
		if (i >= 16) {
			i = 1;
			printf("\n%s", indent);
		}

		if (cur != prev + 1) {
			if (prev == head) {
				printf(" %4.4d", cur); i++;
			} else {
				printf("..%4.4d %4.4d", prev, cur); i += 2;
			}
			head = cur;
		}

		prev = cur;
		cur = fat_get_entry(imp->fat, imp->fat_bits, cur);

	} while (cur > 1 && cur < 0xfff0);
	if (prev != head)
		printf("..%4.4d", prev);
#endif
}

/*
 * List all directories and their blocks
 */
print_dirs(
	register struct dirinfo *dir)
{
	printf("\nDirectory: %11.11s",  dir->name);

	if (dir->inum == DOS_ROOTINO)
		printf("\n  sectors: %4.4d .. %4.4d",
			imp->rootdir_start, imp->clusters_start - 1);
	else
		print_chain(dir->inum, "   blocks:");

	if (dir->next)
		print_dirs(dir->next);
}

/*
 * List all files and their blocks
 */
print_files(
	register struct dirinfo *dir)
{
	register struct dosfs_directory_record *dosdir;
	int i;
	unsigned int ino;

	i = (dir->inum == DOS_ROOTINO) ? 0 : 2;
	dosdir = dir->dosdir;
	for ( ; i < dir->nentries; i++) {
		if ((dosdir[i].name[0] == DOS_NAME_DELETED) ||
		    (dosdir[i].name[0] == DOS_NAME_EMPTY))
			continue;

		if (dosdir[i].attr & (DOS_ATTR_LABEL|DOS_ATTR_DIR))
			continue;

		ino = dosfsnum_16(dosdir[i].start);

		if ((ino == DOS_ROOTINO) ||
		    (ino == dir->inum) ||
		    (ino == dir->dotdot))
			continue;

		printf("\n  File: %11.11s", dosdir[i].name);
		print_chain(ino, "blocks:");
	}
	if (dir->next)
		print_files(dir->next);
}

/*
 * Say many many words...
 */
print_all_files(
	int	fi)
{
	find_dirs(fi);
	print_dirs(&all_dirs);
	print_files(&all_dirs);
	printf("\n");
	free_dirs();
}

/*
 * Disk I/O
 */
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
