/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: disk.c,v $
 * Revision 1.1.3.3  1996/02/19  13:24:15  bernadat
 * 	Fixed SCSI detection.
 * 	[96/02/09            bernadat]
 *
 * Revision 1.1.3.2  1996/01/25  11:22:34  bernadat
 * 	If head count > 15 assume scsi.
 * 	[95/11/27            bernadat]
 * 
 * Revision 1.1.3.1  1995/08/21  19:28:34  devrcs
 * 	Renamed devopen() to disk_open().
 * 	net_open() is now called by mach_boot.c::devopen().
 * 	[95/07/04            yp]
 * 
 * 	Replaced Bread/devread with disk_read: handles mutliple sectors
 * 	and copy the directly to desired location.
 * 
 * 	handle DOS primary and extended partitions.
 * 	Keep old vtoc compatibility.
 * 	[95/03/20            bernadat]
 * 
 * 	Copied from src/stand/AT386
 * 	[95/03/20            bernadat]
 * 
 * Revision 3.0.12.4  1995/04/07  18:41:02  barbou
 * 	Detect SCSI disks.
 * 	Device type is now passed in environment.
 * 	This allows to bootstrap on machines with both SCSI and IDE
 * 	disks.
 * 	[94/10/13            bernadat]
 * 	Probe floppy real density (2.8 MB floppy drive support)
 * 	[94/01/19            bernadat]
 * 	[95/03/10            barbou]
 * 
 * Revision 3.0.12.3  1995/01/06  21:32:07  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 
 * 	BEGIN OSC1_3 HISTORY
 * 	Revision 3.0.2.4  1993/10/20  19:28:11  gm
 * 	CR9704: Added prototypes of all functions.  Removed knowledge
 * 	of kernel major device names and numbers.  Simplified Bread()
 * 	calling signature.
 * 	[1993/10/13  17:39:50  gm]
 * 
 * 	END OSC1_3 HISTORY
 * 	Revision 3.0.12.1  1994/09/10  22:05:49  bolinger
 * 	Merge up to NMK17.3
 * 	[1994/09/08  20:11:49  bolinger]
 * 
 * Revision 3.0.10.1  1994/07/06  07:58:11  bernadat
 * 	Merged NMK16 changes:
 * 	Probe floppy real density (2.8 MB floppy drive support)
 * 	[94/01/19            bernadat]
 * 	[94/06/30            bernadat]
 * 
 * Revision 3.0.2.3  1993/09/20  17:38:02  jph
 * 	CR9385: Updated to use exported headers.
 * 	[1993/09/18  16:47:09  gm]
 * 
 * Revision 3.0.2.2  1993/06/07  19:04:16  gm
 * 	Moved here from mach3 /boot.
 * 	[1993/06/07  18:58:58  gm]
 * 
 * Revision 1.3  1993/04/19  15:04:44  devrcs
 * 	Moved here from /mach_kernel/boot.
 * 	[1993/03/16  11:24:56  gm]
 * 
 * 	Allow user to interrupt disk IOs with <CTRL>C
 * 	[93/01/25            bernadat]
 * 
 * 	Return status for Bread() call (success or failure).
 * 	[92/12/18            bernadat]
 * 
 * Revision 1.2  1992/12/16  10:21:39  devrcs
 * 	Pick up the one and only boot.h from the right place !
 * 	[1992/12/11  21:54:16  david]
 * 
 * Revision 1.1  1992/09/30  02:03:30  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.1.2.1  92/04/30  11:53:17  bernadat
 * 	Fixed code to use mk includes only (not sys/inode.h ....).
 * 	Copied from main line
 * 	[92/03/19            bernadat]
 * 
 * Revision 2.2  92/04/04  11:35:49  rpd
 * 	Fabricated from 3.0 bootstrap and 2.5 boot disk.c:
 * 	with support for scsi
 * 	[92/03/30            mg32]
 * 
 */
/* CMU_ENDHIST */
/*
 * Mach Operating System
 * Copyright (c) 1992, 1991 Carnegie Mellon University
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
 */

#include "boot.h"
#include "bios.h"
#include <i386/disk.h>

#define BPS		512
#define	SPT(di)		((di)&0xff)
#define	HEADS(di)	((((di)>>8)&0xff)+1)
#define	BDEV(dev,unit)	(((dev)==DEV_FLOPPY ? BIOS_DEV_FLOPPY:BIOS_DEV_WIN)+(unit))

static struct alt_info alt_info;
static int spt, spc;

struct fs 	*fs;
int 		unit;
int 		part;
int 		boff, poff, bnum, cnt;
char 		dev;

vtoc_part_t	partitions[V_NUMPAR];
int    		npartitions;
int		active_partition;

void
show_part(struct ipart *, int, int);

int badsect(int);

int part_id;
boolean_t use_badsect;

int
disk_open(void)
{
	int di;

	di = get_diskinfo(BDEV(dev,unit));

#ifdef	DEBUG
	if(debug) {
		printf("disk_open()\n");
		printf("dev %x\n", dev);
		printf("diskinfo %x\n", di);
		printf("HS %d %d\n", HEADS(di) ,SPT(di));
	}
#endif
	spc = (spt = SPT(di)) * HEADS(di);
	use_badsect = FALSE;
	if (dev == DEV_FLOPPY) {
		boff = 0;
		part = (spt == 15 ? 3 : 1);
		return 0;
	}
	else {
#ifndef ykpark
	    	/* I'm using IDE Disk !  Why default disk is SCSI ? */
#else /* ykpark */
	  	/*
		 * Usually H=32 S=64 for SCSI
		 * But sometimes we also have H=33 & S=62.
		 * For IDE Apparently H<64 
		 */
		if (spt >= 64 || HEADS(di) > 32) /* scsi */
			dev = DEV_SD;
#endif /* ykpark */
		if (read_partitions())
			return(1);
#if	DEBUG
	if(debug)
		printf("%d partitions, active is %d\n", 
		       npartitions, active_partition);
#endif
		if (part > npartitions)
			return(1);
		boff = partitions[part].p_start;
	}
	return 0;
}

int
read_partitions(void)
{
	char 		*altptr;
	struct ipart 	*iptr, *active, *extended;
	int 		i, sector;
	int 		base =0;
	int		ext_base = 0;
	struct evtoc 	*evp;
	int 		di;

	di  = get_diskinfo(BDEV(dev,unit));
	spc = (spt = SPT(di)) * HEADS(di);
	use_badsect = FALSE;

	npartitions = 0;
	active_partition = 0;
	active = (struct ipart *)0;
	extended = (struct ipart *)0;

	disk_read(0, BPS, (vm_offset_t)0);
	iptr = (struct ipart *)((struct mboot *)0)->parts;
	for (i = 0; i < FD_NUMPART; i++, iptr++) {
#if	DEBUG
	  	if (debug)
			show_part(iptr, i, 0);
#endif
		if (iptr->bootid == ACTIVE) {
			active = iptr;
			active_partition = i;
		}
		if (iptr->systid == EXT_PART)
			extended = iptr;
		partitions[i].p_start = iptr->relsect;
		partitions[i].p_size = iptr->numsect;
	}
	if (active && active->systid == UNIXOS) {
	  	sector = active->relsect + HDPDLOC;
		evp = (struct evtoc *)0;
		if (disk_read(sector++, BPS, (vm_offset_t)0) == 0 &&
		    evp->sanity == VTOC_SANE) {	/* compatibility */
			bcopy((char *)evp->part,
			      (char *)partitions, sizeof(partitions));
			npartitions = evp->nparts;
			altptr = (char *)(&alt_info);
			for (i = 0; i++ < 4; altptr += BPS, sector++) {
			    if (disk_read(sector, BPS, (vm_offset_t)altptr))
				return 1;
			}
			if (alt_info.alt_sanity != ALT_SANITY) {
				printf("Bad alt_sanity\n");
				return 1;
			}
			active_partition = 0;
			use_badsect = TRUE;
			return 0;
		} 
	}
	if (extended) {
		/* look for extended partitions */
		base = extended->relsect;
		ext_base = base;
		while (1) {
			if (disk_read(base, BPS, (vm_offset_t)0))
		        	return 1;	
			iptr = (struct ipart *)((struct mboot *)0)->parts;
#if	DEBUG
			if (debug)
				show_part(iptr, i, base);
#endif
			partitions[i].p_start = base + iptr->relsect;
			partitions[i].p_size = iptr->numsect;
			i++;
			iptr++;
#if 	DEBUG
			if (debug)
				show_part(iptr, i, base);
#endif
			if (iptr->systid == EXT_PART) {
		        	extended = iptr;
				base = extended->relsect + ext_base;
			} else
				break;
		}
	}
	npartitions = i;
	return 0;
}

#if	DEBUG
void
show_part(struct ipart *iptr, int index, int sector)
{
	int offset = iptr->relsect;
	int length = iptr->numsect;

	printf("part %d sys %x boot %x off %d+%d length %d\n",
	       index, iptr->systid, iptr->bootid,
	       sector, offset, length);
}
#endif


int disk_read(int sector, int nbytes, vm_offset_t addr)
{
	int retry;
	int cyl, head, secnum;
	int saved_cyl = -1;
	int saved_head =  -1;
	int saved_secnum =  -1;
	int nsect;
	int rc = 0;
	int size;
	int sect;
	int interrupted = 0;
	
        nsect = 0;
	size = nbytes;

#ifdef	DEBUG
	if(debug) {
		printf("disk_read(%x, %x, %x)\n", sector, nbytes, addr);
		printf("dev %d, unit %d, spc %d, spt %d\n", dev, unit, spc, spt);
	}
#endif
	while(nbytes > 0) {
	    sect = badsect(sector);
	    cyl = sect/spc;
	    head = (sect%spc)/spt;
	    secnum = sect%spt;
	    if (cyl != saved_cyl || head != saved_head || size <=0) {
	        if (nsect) {
		    retry = 3;
		    while(retry-- && !interrupted) {
		  	interrupted = is_intr_char();
#ifdef	DEBUG
			if(debug)
		       	    printf("@%x: biosread(%x, %x, %x, %x, %x, %x)\n",
				   (int)&retry,
				   BDEV(dev,unit),
				   saved_cyl,
				   saved_head,
				   saved_secnum,
				   nsect,
				   addr);
#endif
			rc = biosread(BDEV(dev,unit),
						  saved_cyl,
						  saved_head,
						  saved_secnum,
						  nsect,
						  addr);
			if (rc) {
			     if (nsect > 1) { /* try 1 sector at a time */
			         int j;
#ifdef	DEBUG
				 if(debug)
				     printf("Retry: C:%d H:%d S:%d (%d sects @ 0x%x) error code 0x%x\n",
					    saved_cyl,saved_head,
					    saved_secnum, nsect, addr, rc);
#endif
				 for (j = 0; j < nsect; j++)
				     if (disk_read(sector-nsect+j,
						   BPS, addr+j*BPS))
					return(1);
				 break;
			     } else
				 printf("Error %x on C:%d H:%d S:%d @%x\n",
					rc, saved_cyl,saved_head,
					saved_secnum, addr);
		    	} else
			  	break;
		    }
		    if (rc || interrupted)
			return(1);
		    nbytes -= nsect*BPS;
		    addr += nsect*BPS;
		}
		saved_cyl = cyl;
		saved_head = head;
		saved_secnum = secnum;
		nsect = 0;
	    }
	    sector++;
	    nsect++;
	    size -= BPS;
	}
#ifdef	DEBUG
	if(debug)
		printf("disk_read done\n");
#endif
	
	return(0);
}

int
badsect(int sector)
{
	int i;
	if (use_badsect) {
		for (i = 0; i < alt_info.alt_trk.alt_used; i++)
			if (sector/spt == alt_info.alt_trk.alt_bad[i])
				return (alt_info.alt_trk.alt_base + 
					i*spt + sector%spt);
		for (i = 0; i < alt_info.alt_sec.alt_used; i++)
			if (sector == alt_info.alt_sec.alt_bad[i])
				return (alt_info.alt_sec.alt_base + i);
	}
	return sector;
}

