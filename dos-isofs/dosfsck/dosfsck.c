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
 * $Log:	dosfsck.c,v $
 * Revision 2.1.1.4  93/11/30  12:45:35  af
 * 	Added sanity check in has_chain() against bogus inums.
 * 	Added check that there be only one dot and dotdot.
 * 	[93/10/10            af]
 * 
 * Revision 2.1.1.3  93/09/14  13:17:03  af
 * 	Fixed error returns (fsck-compat).
 * 	Do not use perror, go for stderr instead.
 * 	MangledDir was ... mangled.
 * 
 * Revision 2.1.1.2  93/09/01  22:22:46  af
 * 	No longer print fat (use dosdumpfs).
 * 	Fixed file_len to know about root.
 * 	Compile with new dosfs.h header file.
 * 
 * Revision 2.1.1.1  93/08/27  11:50:48  af
 * 	Created.
 * 	[93/07/26            af]
 * 
 */

/*
  Things this program does in the various phases:

	0- verify FAT coherency (if more than one)
	1- reconstruct all block allocation chains
	2- check and repair all directories
	3- check file lenghts against chains,
	   compact directories
	4- free unreachable, allocated blocks
	5- update permanent storage

*/

#ifndef	__STDC__
#define	__STDC__ 1
#endif

#include <stdarg.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fstab.h>
#include <signal.h>

#include <cthreads.h>
#include <dosfs/dosfs.h>

#define DOS_ROOTINO 0
#define CHAIN_HEAD  1

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
 * Alternate/work fat
 */
unsigned short *afat;

/*
 * All the directories we found
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

struct dosfs_directory_record dosfs_mastertemplate[2] =
{ { ".       ", "   ", DOS_ATTR_DIR, },
  { "..      ", "   ", DOS_ATTR_DIR, } };

/*
 * User control switches
 */
int	readonly = 0;		/* check only, no writes */
int	primary_fat = 0;	/* use an alternate FAT */
int	user_says = 0;		/* default all questions */
int	preen = 0;		/* do them all, in parallel */
int	pedantic = 0;		/* check reserved bits and fields */

/*
 * Other globals
 */
int	rewrite_fats = 0;	/* at end, if modified */
int	rewrite_dirs = 0;	/* if we changed any */
int	nfiles;			/* statistics */
int	nfrags;
time_t	time_now;		/* now */
time_t	cutoff_time;		/* some close future time */
int	back_to_single_user;

/*
 * Messages and other strings
 */
char	*program;		/* how do they call us ? */
char	*disk;			/* what we are checking */
char	Remove[] = "REMOVE";	/* upcase cuz old fsck */
char	Removed[] = "REMOVED";
char	Delete[] = "DELETE";
char	Deleted[] = "DELETED";
char	Fix[] = "FIX";
char	Fixed[] = "FIXED";
char	BadOption[] = "%s: illegal option -- %s\n";
char	AltFAT[] = "Alternate FAT table: %d\n";
char	NoFat[] = "NO FAT TABLE\n";
char	BadBitsize[] = "BAD BITSIZE %d IN FAT\n";
char	NoAlternate[] = "Disk has no alternate FATs\n";
char	ErrAlternate[] = "Alternate FAT no %d differs from primary no %d";
char	NoChain[] = "NO CHAIN FOR %s '%11.11s', INODE %d (x%x)\n";
char	ChainLoop[] = "CHAIN LOOP AT %d";
char	Danger[] = "TOO DANGEROUS TO CONTINUE\n";
char	MangledDir[] = "INODE %d: CORRUPTED DIRECTORY HAS NO '%s' ENTRY";
char	PtrRoot[] = "ENTRY %d POINTS BACK TO ROOT";
char	SelfRef[] = "Self-referencing directory at entry %d";
char	ParRef[] = "Parent-referencing directory at entry %d";
char	MultiDot[] = "Illegal name '.' for entry %d";
char	MultiDotDot[] = "Illegal name '..' for entry %d";
char	BadEntry[] = "CORRUPTED ENTRY(%d) '%11.11s' IN DIRECTORY '%11.11s'\n";
char	UnalInode[] = " INODE %d HAS NOT BEEN ALLOCATED";
char	FileSize[] = "inode %d file '%11.11s' owns %d blocks, not %d";
char	OrphanFile[] = "UNREF FILE I=%d SIZE=%d BLOCKS";
char	BadInum[] = "INVALID INODE NUMBER %d, FILE '%11.11s'";
char	DupChain[] = "DUP ALLOC BLOCK %d (REF FROM %d), CLEARED.\n";
char	ResAttribute[] = "Non-zero reserved bits in attribute byte %x, file '%11.11s'";
char	ResBytes[] = "Non-zero reserved bytes in directory entry for file '%11.11s'";
char	BadDate[] = "Preposterous date, file '%11.11s' %s";

char	LastMount[] = "** Last Mounted on %11.11s\n";
char	ToSingle[] = "returning to single-user after filesystem check\n";
char	Phase1[] = "** Phase 1 - Check File Allocation Table\n";
char	Phase2[] = "** Phase 2 - Check Connectivity\n";
char	Phase3[] = "** Phase 3 - Check and Cleanup Directories\n";
char	Phase4[] = "** Phase 4 - Check Orphaned Files\n";
char	Phase5[] = "** Phase 5 - Updating disk\n";
char	Stat1[] = "%d files, %d used, %d free\n";
char	Stat2[] = "    (%d frags, %.1f%% fragmentation)\n";

/*
 * Debugging
 */
int dosfs_debug = 0;
#ifndef trace
#define trace(c,w)	if (c) w
#endif

/*
 * Forward decls
 */
unsigned short
fat_get_entry(unsigned short *fat, unsigned int fbits, unsigned int entry);
int
fat_set_entry(unsigned short *fat, unsigned int fbits, unsigned int entry,
		unsigned short value);
#define	FAT_GET_ENTRY(f,b,e) (((b) == 16) ? (f)[e] : fat_get_entry(f,b,e))

off_t
dolseek(int fi, off_t where, int how);
void
doread(int fi, char *buf, unsigned int len);
void
dowrite(int fi, char *buf, unsigned int len);

unsigned int
check_file( ino_t inum, unsigned int size, unsigned char *name);

void
dosfs_dosdate(unsigned int unix_seconds, unsigned char *dtime, unsigned char *ddate);
time_t
dosfs_date( unsigned char *date, unsigned char *time);

int
checkdosfs(struct fstab *fsp);
int
fsck( char *filename );

int
tell_user(FILE *stream, char *fmt, ...);

void sigint_handler(), sigquit_handler();

/*
 * Command line parsing and exec
 */
int
main(
	int	argc,
	char	**argv)
{
	int status = 0, didsome = 0;

	program = argv[0];

	/* take a month from now */
	time_now = time(0);
	cutoff_time = time_now + (30*24*60*60);

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT, sigint_handler);
 

	for (; argc > 1; argc--, argv++) {

		if (argv[1][0] == '-') {
			switch (argv[1][1]) {
			case 'b':
				primary_fat = atoi(argv[2]);
				printf(AltFAT, primary_fat);
				argc--, argv++;
				break;
			case 'd':
				dosfs_debug++;
				break;
			case 'n':
				user_says = 'n';
				readonly = 1;
				break;
			case 'p':
				preen = 1;
				break;
			case 'r':
				pedantic = 1;
				break;
			case 'y':
				user_says = 'y';
				readonly = 0;
				break;
			default:
				fprintf(stderr,BadOption,
					program, &argv[1][1]);
				exit(8);
			}

			continue;
		}

		status |= fsck(argv[1]);
		didsome++;
	}
	if (preen || !didsome) {
		if (preen)
			(void) signal( SIGQUIT, sigquit_handler);
		status = checkfstab(preen, 10, checkdosfs, fsck);
	}
	if (back_to_single_user)
		return (2);
	return (status);
}

/*
 * Signal handlers
 */
void
sigquit_handler()
{
	/* old fsck's familiar message */
	printf(ToSingle);
	back_to_single_user = 1;
	(void) signal(SIGQUIT, SIG_DFL);
}

void
sigint_handler()
{
	exit(12); /* old fsck did so */
}

/*
 * See if we can do this filesystem
 */
int
checkdosfs(struct fstab *fsp)
{
	if (strcmp(fsp->fs_vfstype, "dosfs") ||
	    (strcmp(fsp->fs_type, FSTAB_RW) &&
	     strcmp(fsp->fs_type, FSTAB_RO)) ||
#if 0
	    fsp->fs_passno == 0)	/* should fix the ufs fsck !! */
#else
	    fsp->fs_passno == 99)
#endif
		return (0);
	return (1);
}

#include <setjmp.h>
jmp_buf	io_error;

/*
 * Check one filesystem
 */
fsck(
	char	*filename)
{
	int fi, ret = 8;

	rewrite_fats = rewrite_dirs = 0;
	nfiles = nfrags = 0;

	disk = filename;
	fi = open(filename, readonly ? O_RDONLY : O_RDWR, 0);
	if (fi < 0)
		goto out;

	if (setjmp(io_error))
		goto out;

	if (preen == 0)
	tell_user(stdout,"** %s%s\n", filename, readonly ? " (NO WRITE)" : "");

	dolseek(fi, MDOS_LABELSECTOR * MDOS_SECTOR_SIZE, 0);
	doread(fi, bsec, MDOS_SECTOR_SIZE);

	if (errno = mnt(fi))
		goto out;

	check_fat(fi);
	scan_fat();
	find_dirs(fi);
	check_dirs(fi);
	check_chains(fi);
	writeback(fi);

	{
		int tot_sectors;
		tell_user(stdout,Stat1,
			nfiles, imp->im_alloc, imp->im_free);
		tot_sectors = imp->im_size * imp->im_clsiz;
		tell_user(stdout,Stat2,
			nfrags, (float)(nfrags * 100) / (tot_sectors));
	}

	ret = 0;	/* all seems well */
out:
	close(fi);
	if (afat) {
		free(afat);
		afat = NULL;
	}
	if (imp->fat) {
		free(imp->fat);
		imp->fat = NULL;
	}
	free_dirs();
	if (ret)
		tell_user(stderr, "%s: %s\n", filename, errmsg(-1));
	return(ret);
}

/*
 * Read the volume information and primary FAT
 */
mnt( int fi)
{
	int logical_block_size, i;

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

	if (primary_fat >= imp->n_fat) {
		tell_user(stderr,NoFat);
		return EINVAL;
	}

	/* Read all of the FAT in */
	imp->fat = (u_short *)malloc(imp->fat_len * MDOS_SECTOR_SIZE);

	dolseek(fi,
		(imp->fat_start * MDOS_SECTOR_SIZE) +
		  (primary_fat * imp->fat_len * MDOS_SECTOR_SIZE),
		0);
	doread(fi, (char *)imp->fat, imp->fat_len * MDOS_SECTOR_SIZE);

	if (/*vdp->volume_label[0] == '/' &&*/ preen == 0)
		tell_user(stdout, LastMount, vdp->volume_label);

	dosfs_stat_fat(imp);

	imp->im_flags = 0;

	return (0);
}

/*
 * Scavenge a FAT
 */
scan_fat()
{
	scan_chains(imp->fat, imp->fat_bits, imp->fat_entries);
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
		} else {
			nalloc++;
			fe = i;
		}
	/* this speeds things up, but it is no longer the true fat size ! */
	dosfsmp->fat_entries = fe + 1;

	fe = dosfsmp->clusters_start / dosfsmp->im_clsiz;
	dosfsmp->im_free = nfree;
	dosfsmp->im_alloc = nalloc + fe;
	trace(dosfs_debug, ("stat_fat: %d %d\n", dosfsmp->im_free, dosfsmp->im_alloc));
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
		tell_user(stderr, BadBitsize, fbits);
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
		tell_user(stderr, BadBitsize, fbits);
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
 * Verify primary FAT against duplicates.
 * [User can use -b switch to select primary]
 */
check_fat( int fi)
{
	int fatno;
	unsigned int fatlen;

	fatlen = imp->fat_len * MDOS_SECTOR_SIZE;

	if (afat == 0)
		afat = (u_short *)malloc(fatlen);

	if (imp->n_fat < 2) {
		tell_user(stdout,NoAlternate);
		return 1;
	}

	for (fatno = 0; fatno < imp->n_fat; fatno++) {
		if (fatno == primary_fat)
			continue;
		dolseek(fi,
			(imp->fat_start * MDOS_SECTOR_SIZE) + (fatno * fatlen),
			0);
		doread(fi, (char *)afat, fatlen);
		if (bcmp(afat, imp->fat, fatlen) == 0)
			continue;
		tell_user(stdout,ErrAlternate, fatno, primary_fat);
		if (askuser(Fix,1,Fixed)) {
			rewrite_fats++;
		}
	}
	return 0;
}

/*
 * Scavenge a FAT.
 * Finds all chains, removes duplicates and loops.
 * At end, "afat" contains entries for all chain
 * heads, marked CHAIN_HEAD.  All other entries
 * except bad blocks are zeroed.
 */
scan_chains(
	unsigned short	*fat,
	int		bits,
	int		nent)
{
	int		i, j, k, l;
	int             p = 0, np;

	if (preen == 0)
	tell_user(stdout,Phase1);

	l = (nent * bits) / NBBY;
	if (afat == 0)
		afat = (unsigned short *)malloc( l );
	bcopy(fat, afat, l);

	trace(dosfs_debug,("*** Chains ***\n"));
	for (i = 2; i < nent; i++) {
		j = FAT_GET_ENTRY(afat,bits,i);
		if (j < 2 || ((j >= 0xfff8) && (j != 0xffff)))
			continue;

		/* Check against loops */
		if (j == i) {
			tell_user(stdout,ChainLoop, i);
			if (! askuser(Fix,0,Fixed)) {
too_risky:
				tell_user(stderr,Danger);
				exit(8);
			}
			fat_set_entry(afat,bits,i,0xffff);
			fat_set_entry(imp->fat,bits,i,0xffff);
			rewrite_fats = 1;
		}

		/* lookup i's predecessor, if any */
		p = i;
again:
		/*
		 * We have to scan the whole thing at each pass
		 * because we are also checking against duplicates.
		 * [This is where most of the elapsed time goes].
		 */
		np = p;
		for (k = i + 1; k < nent; k++) {
			l = FAT_GET_ENTRY(afat,bits,k);
			if (l == p) {
				/* A duplicate predecessor ? */
				if (np != p) {
					tell_user(stdout,DupChain, p, k);
					/* Too expensive to keep DUPs around.
					 * We will have to rid of them eventually
					 * and doing it at the last minute
					 * in check_chains() [see note there]
					 * is way more expensive than doing it
					 * here.  But we cannot give any saying
					 * to the user, we just have to do it.
					 */
					if (! askuser(Fix,0,Fixed))
						goto too_risky;
					fat_set_entry(afat,bits,k,0xffff);
					fat_set_entry(imp->fat,bits,k,0xffff);
					rewrite_fats++;
				} else
					np = k;
			}
		}
		if (np != p){
			p = np;
			goto again;
		}


		/* print and zero this chain */
		trace(dosfs_debug,("\n%x:", p));
		j = FAT_GET_ENTRY(afat,bits,p);
		fat_set_entry(afat,bits,p,CHAIN_HEAD);	/* head marker */
		while (1) {
			trace(dosfs_debug,(" %x", j));
			if ((j == 0) || (j >= 0xfff8))
				break;
			p = j;
			j = FAT_GET_ENTRY(afat,bits,p);
			fat_set_entry(afat,bits,p,0);
		}
	}

	if (dosfs_debug) {
		trace(dosfs_debug,("\n"));

		trace(dosfs_debug,("*** Reservations ***\n"));
		p = 0;
		for (i = 2; i < nent; i++) {
			j = FAT_GET_ENTRY(afat,bits,i);
#if 0
			if (j >= 0xfff8)
				trace(dosfs_debug,("%x: %x\n", i, j));
#else
			if (j < 0xfff8) {
				if (p == 2)
					trace(dosfs_debug,("%x\n", i - 1));
				p = 0;
			} else
			if (p == 0) {
				trace(dosfs_debug,("%x ", i));
				p = 1;
			} else
			if (p == 1) {
				trace(dosfs_debug,(".. "));
				p = 2;
			}
#endif
		}
	}

	trace(dosfs_debug,("\n"));

}

/*
 * Is an inode the head of some chain,
 * as it should be ?
 */
has_chain(
	daddr_t		inum)
{
	int i;
	if (inum > imp->fat_entries) return 0;
	i = FAT_GET_ENTRY(afat,imp->fat_bits,inum);
	return (i == CHAIN_HEAD);
}

/*
 * Done with a chain
 */
int
unmark_chain(
	daddr_t		head,
	char		*type,
	unsigned char	*name)
{
	int i;

	i = FAT_GET_ENTRY(afat,imp->fat_bits,head);
	if (i != CHAIN_HEAD) {
		tell_user(stderr, NoChain, type, name, head, i);
		return 1;
	} else {
		fat_set_entry(afat,imp->fat_bits,head,0);
		return 0;
	}
}

/*
 * Scan a disk for all directories.
 * Result is a list with head ALL_DIRS.
 */
find_dirs( int fi)
{
	if (preen == 0)
	tell_user(stdout,Phase2);

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
 * Core routine, checks and fixes,
 * recurse for all subdirectories.
 */
scan_dir(
	int		fi,
	struct dirinfo *dir)
{
	int size;
	struct dirinfo *sdir, *h = 0;
	struct dosfs_directory_record *dosdir;
	int mangled = 0;
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
	 * Check . and ..
	 */
	if (dir->inum != DOS_ROOTINO) {
		int fixit = 0, j = 1;

		/* dot */
		if ((dosfsnum_16(dosdir[0].start) != dir->inum) ||
		    (dosdir[0].name[0] != '.')) {

			mangled |= 1;

			tell_user(stdout,MangledDir, dir->inum, ".");
			if (askuser(Fix,1,Fixed)) {
			    /*
			     * Find a free entry, if any, to swap
			     * If none available, just clobber it
			     */
			    for (j = 0; j < dir->nentries; j++)
				if ((dosdir[j].name[0] == DOS_NAME_DELETED) ||
				    (dosdir[j].name[0] == DOS_NAME_EMPTY))
				    break;
			    if (j == dir->nentries)
				j = 0;
			    dosdir[j] = dosdir[0];
			    dosdir[0] = dosfs_mastertemplate[0];
			    mkdosfsnum_16(dosdir[0].start,dir->inum);
			    /* DOS keeps size zero, sigh */
			}
		}

		/* dotdot */
		if ((dosfsnum_16(dosdir[1].start) != dir->dotdot) ||
		    (dosdir[1].name[0] != '.') ||
		    (dosdir[1].name[1] != '.')) {

			mangled |= 2;

			tell_user(stdout,MangledDir, dir->inum, "..");
			if (askuser(Fix,1,Fixed)) {
			    /*
			     * Ditto, start where we left above
			     */
			    for (; j < dir->nentries; j++)
				if ((dosdir[j].name[0] == DOS_NAME_DELETED) ||
				    (dosdir[j].name[0] == DOS_NAME_EMPTY))
				    break;
			    if (j == dir->nentries)
				j = 1;
			    dosdir[j] = dosdir[1];
			    dosdir[1] = dosfs_mastertemplate[1];
			    mkdosfsnum_16(dosdir[1].start,dir->dotdot);
			    /* DOS keeps size zero, sigh */
			}
		}
	}

	/*
	 * Check all entries
	 */
	for (i = 0; i < dir->nentries; i++) {
		ino_t	ino;
		register unsigned int c;

		if ((dosdir[i].name[0] == DOS_NAME_DELETED) ||
		    (dosdir[i].name[0] == DOS_NAME_EMPTY))
			continue;

		ino = dosfsnum_16(dosdir[i].start);

		if (dosfs_debug)
			printf(" %5.5d: %11.11s %x\n",
				ino, dosdir[i].name, dosdir[i].attr);

		if (dosdir[i].attr & DOS_ATTR_LABEL)
			continue;

		/*
		 * There can only be one '.' and '..'
		 */
		c = dosdir[i].name[0];
		if (i > 1 && c == '.') {
		  c = dosdir[i].name[1];
		  if (c == ' ' || c == '.') {
		    /* dot ? */
		    if (c == ' ' && dosdir[i].ext[0] == ' ') {
			tell_user(stdout,MultiDot, i);
			goto maybe_delete_it;
		    }
		    /* dot-dot ? */
		    if (c == '.' &&
		        dosdir[i].name[2] == ' ' &&
		        dosdir[i].ext[0] == ' ') {
			tell_user(stdout,MultiDotDot, i);
			goto maybe_delete_it;
		    }
		  }
		}

		/*
		 * Check inode actually does own at least one block
		 */
		if ((ino != DOS_ROOTINO) && ! has_chain(ino)) {
			tell_user(stdout,BadEntry, i, dosdir[i].name, dir->name);
			tell_user(stdout,UnalInode, ino);
maybe_delete_it:
			if (askuser(Delete,1,Deleted)) {
				dosdir[i].name[0] = DOS_NAME_DELETED;
				dir->modified = 1;
				rewrite_dirs = 1;
			}
			continue;
		}

		/*
		 * Perform more stringest checks if required,
		 * Check reserved fields and time stamps.
		 */
		if (pedantic) {
			time_t dtime;

			if (dosdir[i].attr & DOS_ATTR_xxx) {
				tell_user(stdout,ResAttribute,
					  dosdir[i].attr, dosdir[i].name);
				if (askuser(Fix,1,Fixed)) {
					dosdir[i].attr &= ~DOS_ATTR_xxx;
					dir->modified = 1;
					rewrite_dirs = 1;
				}
			}
			if (bcmp(dosdir[i].res_ext.reserved,
				 dosfs_mastertemplate[0].res_ext.reserved,
				 sizeof dosfs_mastertemplate[0].res_ext.reserved) != 0) {
				tell_user(stdout,ResBytes, dosdir[i].name);
				if (askuser(Fix,1,Fixed)) {
					bzero(dosdir[i].res_ext.reserved,
					      sizeof dosdir[i].res_ext.reserved);
					dir->modified = 1;
					rewrite_dirs = 1;
				}
			}
			dtime = dosfs_date(dosdir[i].date, dosdir[i].time);
			if (dtime > cutoff_time) {
				char *adate, *ctime();
				adate = ctime(dtime);
				adate[strlen(adate)-2] = 0;/* kill nl */
				tell_user(stdout,BadDate, dosdir[i].name, adate);
				if (askuser(Fix,1,Fixed)) {
					dosfs_dosdate(time_now,
						dosdir[i].date, dosdir[i].time);
					dir->modified = 1;
					rewrite_dirs = 1;
				}
			}
		}

		/*
		 * Check if file
		 */
		if ((dosdir[i].attr & DOS_ATTR_DIR) == 0) {
			unsigned int fsize, nsize;

			if ((ino < 2) || (ino >= 0xfff8)) {
				tell_user(stdout, BadInum, ino, dosdir[i].name);
				goto maybe_delete_it;
			}

			fsize = dosfsnum_32(dosdir[i].size);
			nsize = check_file( ino, fsize, dosdir[i].name);
			if ((fsize != nsize) && askuser(Fix,1,Fixed)) {
				mkdosfsnum_32(dosdir[i].size,nsize);
				dir->modified = 1;
				rewrite_dirs = 1;
			}
			/* stats on fragmentation */
			if ((fsize = dosfs_blkoff(imp,nsize)) != 0) {
				fsize = imp->im_bsize - fsize;
				nfrags += (fsize + MDOS_SECTOR_SIZE - 1) / MDOS_SECTOR_SIZE;
			}
			nfiles++;
			continue;
		}

		/*
		 * Directory, check if it makes sense
		 */
		if (ino == DOS_ROOTINO) {
			if ((i != 1) || (dir->dotdot != DOS_ROOTINO)) {
				tell_user(stdout,PtrRoot, i);
				goto maybe_delete_it;
			}
			continue;
		}

		/*
		 * DOS takes only one of . or .. entries
		 */
		if (ino == dir->inum) {
			if (i == 0)
				continue;
			tell_user(stdout,SelfRef, i);
			goto maybe_delete_it;
		}

		if (ino == dir->dotdot) {
			if (i == 1)
				continue;
			tell_user(stdout,ParRef, i);
			goto maybe_delete_it;
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

		trace(dosfs_debug,("dir(%d,%d) %11.11s\n",
			sdir->inum, sdir->dotdot, dosdir[i].name));
		scan_dir(fi, sdir);

		unmark_chain(sdir->inum, "directory", sdir->name);
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
		ret = (daddr_t) FAT_GET_ENTRY(imp->fat, imp->fat_bits, ret);
			if ((ret == 0) || ((ret & 0xfff0) == 0xfff0)) {
				tell_user(stderr, "? bmap x%x+%x -> %x %x\n",
					inum, l, ret, lbl);
				return -1;
			}
		lbl--;
	}
	return ((ret - 2) * imp->im_clsiz) + imp->clusters_start;
}

/*
 * Check file size.
 * File must have a chain.
 */
unsigned int
check_file(
	ino_t		inum,
	unsigned int	size,
	unsigned char	*name)
{
	int i = dosfs_file_len(imp, inum);
	register int bsize;

	bsize = MDOS_SECTOR_SIZE * imp->im_clsiz;
	bsize = (size + bsize - 1) / bsize;
	/* Must keep at least 1 block even if size is zero */
	if ((bsize != i) && !(bsize == 0 && i == 1)) {
		tell_user(stdout,FileSize, inum, name, i, bsize);
		size = MDOS_SECTOR_SIZE * (imp->im_clsiz * i);
	}
	unmark_chain(inum, "file", name);
	return size;
}

/*
 * Directory compaction
 */
check_dirs( int fi)
{
	if (preen == 0)
	tell_user(stdout,Phase3);
	/* here code to cleanup and compact directories */
}

/*
 * Reclaim unreachable storage
 */
check_chains( int fi )
{
	int		i, j, k;
	int             p = 0;

	if (preen == 0)
	tell_user(stdout,Phase4);

	k = imp->fat_entries;
	for (i = 2; i < k; i++) {

		j = FAT_GET_ENTRY(afat,imp->fat_bits,i);
		if (j != CHAIN_HEAD)
			continue;

		tell_user(stdout,OrphanFile, i, dosfs_file_len(imp, i));
		if (! askuser(Remove,1,Removed))
			continue;

		rewrite_fats++;

		/*
		 * We have already checked in scan_chains() for
		 * common sub-chains (illegal, DOS has no links).
		 * That is faster than the following alternative.
		 * We would need another FAT table, initialize it
		 * with the primary, mark all reachable blocks
		 * from it (good chains) using the directory list,
		 * then follow the orphaned files (bad chains)
		 * and truncate them when they merge with a good
		 * chain.  What is left can be freed.
		 */

		/* print and zero this chain */
		p = i;
		trace(dosfs_debug,("\n%x:", p));
		while (1) {
			j = FAT_GET_ENTRY(imp->fat,imp->fat_bits,p);
			fat_set_entry(imp->fat,imp->fat_bits,p,0);
			imp->im_free++, imp->im_alloc--;

			trace(dosfs_debug,(" %x", j));
			if ((j == 0) || (j >= 0xfff8))
				break;

			p = j;
		}
	}
}

/*
 * This is the one and only place where
 * the disk is actually modified.
 * The FAT table and all changed directories
 * are written back to disk.
 */
writeback(
	int	fi)
{
	register int fatno;
	struct dirinfo *dir;
	int omask;

	if (!rewrite_dirs && !rewrite_fats)
		return;

	if (preen == 0)
	tell_user(stdout,Phase5);

	/*
	 * Dont stop in the middle
	 */
	omask = sigblock( sigmask(SIGINT));

	if (rewrite_dirs)
	for (dir = &all_dirs; dir; dir = dir->next)
		rewrite_dir(dir, fi);

	if (rewrite_fats)
	for (fatno = 0; fatno < imp->n_fat; fatno++) {
		dolseek(fi,
			(imp->fat_start * MDOS_SECTOR_SIZE) +
			(fatno * imp->fat_len * MDOS_SECTOR_SIZE),
			0);
		dowrite(fi, (char *)imp->fat, imp->fat_len * MDOS_SECTOR_SIZE);
	}

	(void) sigsetmask(omask);
}

/*
 * Write a directory back to disk.
 */
rewrite_dir(
	register struct dirinfo	*dir,
	int			fi)
{
	int size;

	if (!dir->modified)
		return;

	trace(dosfs_debug,("Updating directory '%11.11s'\n", dir->name));

	size = dir->nentries * sizeof(struct dosfs_directory_record);
	size = (size + MDOS_SECTOR_SIZE - 1) & ~(MDOS_SECTOR_SIZE - 1);

	if (dir->startsec == imp->rootdir_entries)
		dowrite(fi, (char *)dir->dosdir, size);
	else {
		char *buf = (char *)dir->dosdir;
		register int clsiz = MDOS_SECTOR_SIZE * imp->im_clsiz, bl = 0;

		while (size > 0) {
			dolseek(fi, bmap(dir->inum,bl) * MDOS_SECTOR_SIZE, 0);
			bl++;
			dowrite(fi, buf, clsiz);
			buf += clsiz;
			size -= clsiz;
		}
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
 * Parse from dos to unix date rep
 */
static int days_per_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

time_t
dosfs_date(
	unsigned char	*date,
	unsigned char	*time)
{
	unsigned int year, month, day, hour, minute, second;
	int crtime, days, i;

	year = (date[1] >> 1) + 10;	/* was '80 based */
	month = ((date[0] >> 5) & 0x7) | ((date[1] & 1) << 3);
	day = date[0] & 0x1f;
	hour = time[1] >> 3;
	minute = ((time[0] >> 5) & 0x7) | ((time[1] & 0x7) << 3);
	second = (time[0] & 0x1f) << 1;

	days = year * 365;
	days += (year+2) / 4;
	for (i = 1; i < month; i++)
		days += days_per_month[i-1];
	if (((year+2) % 4) == 0 && month > 2)
		days++;
	days += day - 1;
	crtime = ((((days * 24) + hour) * 60 + minute) * 60)
			+ second;
	return crtime;
}

/*
 * The other way around
 */
void
dosfs_dosdate(
	unsigned int	unix_seconds,
	unsigned char	*dtime,
	unsigned char	*ddate)
{
	register unsigned years, months, days, hours, minutes, seconds;

#define	SECMIN	((unsigned)60)			/* seconds per minute */
#define	SECHOUR	((unsigned)(60*SECMIN))		/* seconds per hour */
#define	SECDAY	((unsigned)(24*SECHOUR))	/* seconds per day */
#define	SECYR	((unsigned)(365*SECDAY))	/* sec per reg year */

#define	YRREF		1970
#define	LEAPYEAR(x)	(((x) % 4) == 0)

	years = YRREF;
	while (1) {
		seconds = SECYR;
		if (LEAPYEAR(years))
			seconds += SECDAY;
		if (unix_seconds < seconds)
			break;
		unix_seconds -= seconds;
		years++;
	}

	months = 0;
	while (1) {
		seconds = days_per_month[months++] * SECDAY;
		if (months == 2 /* February */ && LEAPYEAR(years))
			seconds += SECDAY;
		if (unix_seconds < seconds)
			break;
		unix_seconds -= seconds;
	}

	days = unix_seconds / SECDAY;
	unix_seconds -= SECDAY * days++;

	hours = unix_seconds / SECHOUR;
	unix_seconds -= SECHOUR * hours;

	minutes = unix_seconds / SECMIN;
	unix_seconds -= SECMIN * minutes;

	seconds = unix_seconds;

	/*
	 * On to dos now
	 */

	dtime[0] = ((seconds >> 1) & 0x1f) |
		   ((minutes & 0x7) << 5);
	dtime[1] = ((minutes >> 3) & 0x7) |
		   ((hours << 3));
	ddate[0] = (days & 0x1f) |
		   (months << 5);
	ddate[1] = ((months >> 3) & 1) |
		   ((years - 1980) << 1);

}

/*
 * Prompt user for action.
 * Takes yes/no defaults,
 * and automatic-mode.
 */
int
askuser(
	unsigned char	*action,
	int		preen_answer,
	char		*preen_message)
{
	/* Statically allocated to repeat last answer if eof */
	static char	buf[128];
	int		gotsome;

	if (preen) {
		printf(" (%s%s)\n", (readonly) ? "NOT " : "", preen_message);
		return preen_answer;
	}
	printf("\n%s? [yn] ", action);
	(void) fflush(stdout);
	if (user_says == 'n') {
		printf("no\n");
		return 0;
	}
	if (user_says == 'y') {
		printf("yes\n");
		return 1;
	}
	gotsome = fgets(buf, sizeof buf, stdin) != NULL;
	if (!gotsome)
		printf(buf);
	if (*buf == 'n' || *buf == 'N')
		return 0;
	return 1;
}

/*
 * Say something and who sez so
 */
int
tell_user(
	FILE	*stream,
	char	*fmt,
	...)
{
	char	line[1024];
	va_list	args;
	int	ret;

	va_start(args, fmt);
	(void) vsnprintf(line, sizeof line, fmt, args);
	va_end(args);

	if (preen)
		ret = fprintf(stream, "%s: %s", disk, line);
	else
		ret = fprintf(stream, "%s", line);
	return ret;
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
