/*
 * Copyright (c) 1991 Carnegie Mellon University
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
 *
 * Purpose:
 *	V86 BIOS Disk emulation
 *
 * HISTORY: 
 * $Log:	bios_disk.c,v $
 * Revision 2.8  92/07/01  14:24:37  grm
 * 	Changed the code in read_abs_hdisk so that it sets the zero
 * 	partition information to be correct.
 * 	[92/06/16            grm]
 * 	Fix bug in bios_read/write_sector code where the top two bits of
 * 	cylinder address were being ignored.
 * 	[92/06/02            grm]
 * 
 * Revision 2.7  92/05/22  15:59:07  grm
 * 	Minor bug fixes.
 * 	[92/05/21            grm]
 * 
 * Revision 2.6  92/02/14  17:44:29  grm
 * 	Conditionalized the device opens to use the DOSPREFIX with
 * 	OSF_SVR defined.
 * 	[92/02/14            grm]
 * 	Removed the fd paramter of read_abs_hdisk.  Removed the code
 * 	which accessed rhd0c in hd_init.
 * 	[92/02/11            grm]
 * 
 * Revision 2.5  92/02/03  14:24:33  rvb
 * 	Clean Up
 * 
 * Revision 2.4  92/02/02  23:02:13  rvb
 * 	Changed rhd0h to dosdisk.
 * 	[92/01/28            grm]
 * 	Fixed the starting sector relative offset problem.  Look in "hidden"
 * 	field of BPB to find it in DOS 5.0 (Could have been there previously).
 * 	Fixed the system identification procedure.  Got rid of the exit(0)'s.
 * 	[92/01/27            grm]
 * 
 * Revision 2.3  91/12/05  16:39:53  grm
 * 	Added support for Huge partitions which are used in versions of
 * 	DOS 4.0+.  The device initialization code open files on the LPATH
 * 	now.
 * 	[91/12/04            grm]
 * 	rfr changes.
 * 	[91/06/14  11:47:02  grm]
 * 
 * 	New Copyright.  Change line support changed.
 * 	[91/05/28  14:34:18  grm]
 * 
 * 	Attempt at getting /dev/floppy to behave.
 * 	[91/03/26  19:16:31  grm]
 * 
 * 	Mach3 changes.
 * 	[91/02/01  13:23:16  grm]
 * 
 * 	Added support for absolute referenced
 * 	hard drives (ie hd as a file).  Separated
 * 	code into bios_disk.h and .c.
 * 	[90/12/08  20:49:45  grm]
 * 
 * 	Put in checks for invalid partition writes.
 * 	[90/12/07  19:31:47  grm]
 * 
 * 	Checkin' in for the weekend.  Small changes.
 * 	[90/11/09  20:57:59  grm]
 * 
 * 	Modified hard disk partition code so that it 
 * 	works on different disk geometries.
 * 	[90/11/08  17:39:07  grm]
 * 
 * 	Removed pause from after hd write.
 * 	[90/10/18  17:55:21  grm]
 * 
 * 	Changed some fprintfs to debugs.
 * 	[90/10/04  20:58:53  grm]
 * 
 * 	Hard disk works for reads and writes now.  Made the partition
 * 	table reads generic.
 * 	[90/07/25  19:55:25  grm]
 * 
 * 	First version for Hard Drive reads.  Only one partition
 * 	works (special cases for drive=0x80).  No write ability
 * 	yet.  Special case for horse.mach in get_drive_params.
 * 	[90/07/25  18:33:19  grm]
 * 
 * 	Changed fprintfs to use dbg_fd instead of stderr.
 * 	[90/04/17  22:31:21  grm]
 * 
 * 	Support for Hard Drives added: fixed up int 0x41,
 * 	debugging added.  Support for 3.5" disk added.
 * 	[90/04/05  21:16:32  grm]
 * 
 * 	corrected error some error returns.
 * 	[90/03/29  17:17:48  grm]
 * 
 * 	Started editing as grm.  Moved to v86 branch.
 * 	[90/03/28  18:35:21  grm]
 * 
 * Revision 2.1.1.6  90/03/22  21:45:34  dorr
 * 	Changed validate drive so that it got the media info on open.
 * 	Also put code in validate drive that would try to open device
 * 	read only if rdwr failed.  Changed read and write sector to use
 * 	media specific values.  Changed some debug stuff to use Fprintfs.
 * 
 * Revision 2.1.1.5  90/03/19  17:38:55  orr
 * 	Added change line constants.  Made validate_drive into a general purpose
 * 	procedure.  Added disk_write_sector, and disk_change_line.  Replaced
 * 	DebugX's with fprintf's.
 * 
 * Revision 2.1.1.4  90/03/14  16:59:20  orr
 * 	first pass at generalizing device stuff.
 * 
 * Revision 2.1.1.3  90/03/13  15:28:03  orr
 * 	Version that gives an A:> prompt and does a DIR.
 * 	Replaced DEBUG with DebugX.  No major changes.
 * 
 * Revision 2.1.1.2  90/03/12  02:11:30  orr
 * 	First working version.  route all debug output
 * 	through dbg_fd.
 * 
 * 
 * Revision 2.1.1.1  90/03/11  20:02:37  orr
 * 
 * Revision 1.1  90/03/09  11:57:18  orr
 * Initial revision
 * 
 */
#include "bios_disk.h"

extern int errno;
extern char * lpath;
extern int exit_index;

#ifdef	OSF_SVR
extern char osfdir[];
extern int prefix_len;
#endif	OSF_SVR

int drive_error;
boolean_t first_reset = TRUE;
u_char change_line_status = 0x00;

/*
 *
 * Routines for reading the absolute first block of the hard drive
 * Taken from /afs/cs/project/mach-6/usr/src/etc/etc.i386/diskutil/io.c
 *
 */

#define V_RDABS		_IOW(v,10,struct absio)

/* from ../i386at/disk.h */
struct absio {
	long	abs_sec;		/* Absolute sector number (from 0) */
	char	*abs_buf;		/* Sector buffer */
};

int open_hdisk(name)
	char * name;
{
	int fd;

	if ((fd = open(name, O_RDONLY)) == -1) {
		fprintf(dbg_fd,"Can't open device %s\n", name);
		return -1;
	}
	return(fd);
}

int 
read_phys(fd, buf, cnt)
int fd;
char * buf;
int cnt;
{

	int i = cnt;
	int amount;
	char buff[4096];
	while (i > 0) {
		int ret;
		if (i > (4*1024)) amount = (4*1024); else amount = i;
		ret = read(fd, buff, amount);
		if (ret > 0) bcopy(buff, buf, ret);
		if (ret < amount) {
			printf("Disk error!! %d, %d\n",amount, ret);
			exit_dos();
		}
		i -= ret;
		buf += ret;
	}
	return(cnt);
}


int 
write_phys(fd, buf, cnt)
int fd;
char * buf;
int cnt;
{
	int i = cnt;
	int amount;
	char buff[4096];

	while (i > 0) {
		int ret;
		if (i > (4*1024)) amount = (4*1024); else amount = i;
		bcopy(buf, buff, amount);
		ret = write(fd, buff, amount);
		if (ret < amount) {
			printf("Disk error!! %d, %d\n",amount, ret);
			exit_dos();
		}
		i -= ret;
		buf += ret;
	}
	return(cnt);
}

read_abs_hdisk(sector, buf)
	int sector;
	char * buf;
{
	int	start_head;
	int	start_cylsec;
	int	partition_type;
	int	end_head;
	int	end_cylsec;
	int	startsec_rel;
	int	partition_length;
	int	spc, spt, heads, start, end;
	int	start_cyl, end_cyl;
	int	start_sec, end_sec;
	char    tmp[SEC_LENGTH];
        struct boot_sector * nbuf = (struct boot_sector *)&tmp;
	int i;
	int real_fd = open_hdisk("/dev/dosdisk");

	lseek(real_fd, 0, L_SET);
    	if ((i=read_phys(real_fd, nbuf, SEC_LENGTH)) != SEC_LENGTH) {
		Ddebug0((dbg_fd, "error w/boot_sector %x %x %x\n",
    	    			sizeof(struct boot_sector),i, errno));
		exit_index = 6;
		exit_dos();
    	}
	close(real_fd);

	/* Can't read the absolute zero sector, so we manufacture one. */

	partition_length = *((unsigned short *)&(nbuf->sects));

	partition_type = (partition_length/nbuf->spa > 4096) ? 0x4 : 0x1;
	/* Huge partitions Dos v4.0+ */
	if (partition_length == 0) {
		partition_length = *((unsigned long *)&(nbuf->SECTS));
		partition_type = 0x6;
	}
	startsec_rel = (!nbuf->hidden ? nbuf->sectrk : nbuf->hidden);

	spt = nbuf->sectrk;	/* sectors per track */
	heads = nbuf->heads;	/* heads */
	spc = spt * heads;	/* sectors per cylinder */
	start = startsec_rel;

	start_cyl = start / spc;
	start_head = (start - (start_cyl * spc)) / spt;
	start_sec = (start - (start_cyl * spc) - (start_head * spt)) + 1;

	start_cylsec = (((start_cyl & 0xff) << 8) |
			(((start_cyl & 0x300) >> 2) | start_sec));

	end = (partition_length + start);
	end_cyl = end / spc;
	end_head = (end - (end_cyl * spc)) / spt;
	end_sec = (end - (end_cyl * spc) - (end_head * spt)) + 1;

	end_cylsec = (((end_cyl & 0xff) << 8) |
		      (((end_cyl & 0x300) >> 2) | end_sec));

	bzero(buf, SEC_LENGTH);
	buf[0x1be] 			= 0x80;
	buf[0x1be+1] 			= start_head;
	*(u_short *)(buf + 0x1be + 2) 	= start_cylsec;
	buf[0x1be+4] 			= partition_type;
	buf[0x1be+5] 			= end_head;
	*(u_short *)(buf + 0x1be + 6) 	= end_cylsec;
	*(u_long *)(buf + 0x1be + 8) 	= startsec_rel;
	*(u_long *)(buf + 0x1be + 12) 	= partition_length;
	*(u_short *)(buf + 0x1fe) 	= 0xaa55;
}

bios_disk_init()
{
	register int i;
	
	for(i=0; i < MAX_DEVICES; i++) {
		devices[i].d_state = DEVICE_NOT_PRESENT;
	}

}

hd_init(drive)
	int drive;
{
	u_char buf[0x200];
	int pos;
	boolean_t ex = FALSE;
	u_char sys;

	if (strncmp(devices[drive].d_path,"/dev/dos",8) == 0) {
		Ddebug0((dbg_fd,"\rbios_disk: hdinit() /dev/dos* init\n"));
		/* get zero sector from local machine's hard drive */
		read_abs_hdisk(ZERO,buf);
		devices[drive].d_absability = FALSE;
	}else{
		/* otherwise read from absolute place in file */
		Ddebug0((dbg_fd,"\rbios_disk: hdinit() absdev init\n"));
		lseek(devices[drive].d_fd,0,L_SET);
		read_phys(devices[drive].d_fd,buf,0x200);
		devices[drive].d_absability = TRUE;
	}
	

	/*
	 * Just looks for info on the first dos partition.
	 * it finds. XXX
	 */
	for (pos = 0x01be; ((pos < 0x1fe) && !ex) ; pos += 0x10) {
		sys = buf[pos + 4];
		if ((sys == DOS_SYS1) || (sys == DOS_SYS2) ||
		    (sys == DOS_SYS3)) {
			devices[drive].start_head = *(u_char *)(buf + pos + 1);
			devices[drive].start_cylsec = *(u_short *)(buf + pos + 2);
			devices[drive].partition_type = *(u_char *)(buf + pos + 4);
			devices[drive].end_head = *(u_char *)(buf + pos + 5);
			devices[drive].end_cylsec = *(u_short *)(buf + pos + 6);
			devices[drive].start_sec_rel = *(u_long *)(buf + pos + 8);
			devices[drive].partition_len = *(u_long *)(buf + pos + 12);
			
			Ddebug0((dbg_fd,"Drive = 0x%x  part_type = 0x%x\n",
				 drive, devices[drive].partition_type));
			Ddebug0((dbg_fd,"\rstart_head 0x%x cylsec 0x%x\n",
				 devices[drive].start_head,devices[drive].start_cylsec));
			Ddebug0((dbg_fd,"\rend_head 0x%x cylsec 0x%x\n",
				 devices[drive].end_head,devices[drive].end_cylsec));
			Ddebug0((dbg_fd,"\rstarting sector: 0x%x\n",
				 devices[drive].start_sec_rel));
			Ddebug0((dbg_fd,"\rpartition_len = 0x%x\n",
				 devices[drive].partition_len));
			ex = TRUE;
		}
	}
	
	if (!ex)
		devices[drive].d_state = DEVICE_INITIALIZATION_FAILED;
}

/*
 *  validate_drive: verify that the drive indicates
 *  a valid and active device
 */

boolean_t validate_drive(drive)
	int drive;
{

	if (drive >= MAX_DEVICES )
		return(FALSE);

	drive_error = 0;

	switch (devices[drive].d_state) {
	    case DEVICE_NOT_PRESENT:
		Ddebug0((dbg_fd,"\rdevice not present (%x)\n",drive));
		devices[drive].d_state = DEVICE_NOT_PRESENT;
		return (FALSE);
 		break;
	    case DEVICE_NOT_INITIALIZED: {
		    int fd;
		    int i;
		    char tmp[SEC_LENGTH];
		    struct boot_sector * buf = (struct boot_sector *)&tmp;
		    char buffer[256];

		    Ddebug2((dbg_fd,"\rdevice not inted (%x)\n",drive));

#ifdef	OSF_SVR
		    fd = open(devices[drive].d_path,O_RDWR);
		    if (fd < 0) {
			    strcpy(osfdir+prefix_len, devices[drive].d_path);
			    fd = open(osfdir, O_RDWR);
			    if (fd < 0) {
				    Ddebug0((dbg_fd, "failed opening device %x for rdrw\n",drive));
				    devices[drive].d_state = DEVICE_INITIALIZATION_FAILED;
				    return(FALSE);
			    }
		    }
#else	/* BSD 4.3 UX Server */

		    fd = openp(lpath, devices[drive].d_path, buffer, O_RDWR);
		    Debug0((dbg_fd,"opened %s in %s\n",devices[drive].d_path,buffer));
		    if (fd < 0) {
			   Ddebug0((dbg_fd, "failed opening device %x for rdwr\n",drive));
			    fd = openp(lpath, devices[drive].d_path, buffer, O_RDONLY);
			    if (fd < 0) {
				    devices[drive].d_state = DEVICE_INITIALIZATION_FAILED;
				    Ddebug0((dbg_fd, "failed opening device %x:%x for rdonly\n",drive,errno));
				    return (FALSE);
			    }
		    }
#endif	OSF_SVR


		    devices[drive].d_state = DEVICE_INITIALIZED;
		    devices[drive].d_fd = fd;

		    if (drive < 0x80) {
			    devices[drive].start_cylsec = 0;
			    devices[drive].start_head = 0;
			    devices[drive].end_cylsec = 0;
			    devices[drive].end_head = 0;
			    devices[drive].start_sec_rel = 0;
			    devices[drive].partition_len = 0;
			    devices[drive].d_absability = FALSE;
		    }else{
			    hd_init(drive);
		    }
		    
		    if (devices[drive].d_absability && (drive > 0x7f)) {
			    i = devices[drive].start_sec_rel * SEC_LENGTH;
			    lseek(devices[drive].d_fd,i,L_SET);
		    }else{
			    i = 0;
			    lseek(devices[drive].d_fd,i,L_SET);
		    }

		    if ((i=read_phys(fd, buf, SEC_LENGTH)) != SEC_LENGTH) {
			    Ddebug0((dbg_fd, "validate drive error [0x%x] %x\n",
				    drive, errno));
			    if (drive < 0x80) {
				    devices[drive].d_state = DEVICE_NOT_INITIALIZED;
				    close(devices[drive].d_fd);
				    if (errno == EIO) {
					    drive_error = NO_DISK_PRESENT;
				    }else{
					    drive_error = DRIVE_ERROR;
				    }
				    return(FALSE);
			    }
			    exit_index = 7;
			    exit_dos();
		    }

		    devices[drive].media = buf->media;
		    devices[drive].bps[0] = buf->bps[0];
		    devices[drive].bps[1] = buf->bps[1];
		    devices[drive].spa = buf->spa;
		    devices[drive].sectrk = buf->sectrk;
		    devices[drive].heads = buf->heads;
		    devices[drive].drive = buf->drive;
		    devices[drive].sects = *((unsigned short *)&buf->sects);
		    devices[drive].SECTS = buf->SECTS;

		    Ddebug0((dbg_fd,"\rbps: %x %x spa: %x sectrk: %x heads: %x drive: %x\n",buf->bps[1],buf->bps[0],buf->spa,buf->sectrk,buf->heads,buf->drive));
		    Ddebug0((dbg_fd,"\rsects: %x SECTS: %x\n",devices[drive].sects,devices[drive].SECTS));

		    return (TRUE);
		    break;
	    }
	    case DEVICE_INITIALIZED:
		return (TRUE);
		break;
	    case DEVICE_INITIALIZATION_FAILED:
		Fprintf((dbg_fd,"\rmon_disk: device init failed (%x)\n",drive));	
		return (FALSE);
		break;
	    default:
		/* shouldn't get here */
		Fprintf((dbg_fd, "mon_disk: bad device validation.\n"));
		return (FALSE);
		break;
	}
}

int read_floppy_disk(pos, nms, address, drive)
	int pos;
	int nms;
	vm_address_t address;
	int drive;
{
	int i;

	if (lseek(devices[drive].d_fd, pos, L_SET) < 0)
		return(FALSE);
	Ddebug2((dbg_fd,"mon_disk: addr = 0x%x\n",address));
	if ((i = read_phys(devices[drive].d_fd, address, nms * SEC_LENGTH)) != nms * SEC_LENGTH) {
		Ddebug0((dbg_fd,"mon_disk: read failed i = %x\n", i));
		return(FALSE);
	}
	return(TRUE);
}

boolean_t read_partition_block(address,drive)
	vm_address_t address;
{
	int fdx;
	int pos;
	unsigned char * sys;
	unsigned char * bootable;
	boolean_t dos_boot = FALSE;
	
	if (devices[drive].d_absability) {
		/* get zero sector from abs place in file */
		lseek(devices[drive].d_fd,0,L_SET);
		if (read_phys(devices[drive].d_fd,address,0x200) != 0x200) {
			Ddebug0((dbg_fd,"\rbios_disk: rd_p_block file error. %d\n",
				 errno));
			return(FALSE);
		}
	}else{
		/* get zero sector from local machine's hard drive */
		read_abs_hdisk(ZERO,address);
	}

	/* 
	 * Simple minded scheme for making sure a dos partition is the bootable one.
	 * Doesn't account for multiple dos partitions.  Then it makes the first
	 * dos partition in the table the bootable one, even if another dos partition
	 * later in the list is already the bootable partition.
	 */
	for (pos = 0x01be; pos < 0x1fe ; pos += 0x10) {
		bootable = (unsigned char *)(address + pos);
		sys = (unsigned char *)(address + pos + 1);

		if (dos_boot) {
			*bootable = ZERO;
		}else if ((*sys == DOS_SYS1) || (*sys == DOS_SYS2)) {
			*bootable = BOOTABLE;
			dos_boot = TRUE;
		}else{
			*bootable = ZERO;
		}
	}
	
	return (TRUE);
}

boolean_t legal_disk_pos(pos, nms, drive)
int pos;
int nms;
int drive;
{
   return((pos >= devices[drive].start_sec_rel * SEC_LENGTH) &&
	  ((pos+(nms*SEC_LENGTH)) <=
	            ((devices[drive].start_sec_rel + 
		      devices[drive].partition_len) * SEC_LENGTH)));
}

xx_read_hard_disk(pos, nms, address, drive) 
	int pos;
	int nms;
	vm_address_t address;
	int drive;
{
	int i;
	
	/* check to see where to read the sector from */

	if ((pos == 0)&&(nms == 1)) {
		Ddebug0((dbg_fd, "\rReading block ZERO from hd, add = %x\n\r",address));
		read_partition_block(address, drive);
	}else{
		if (!legal_disk_pos(pos, nms, drive)) {
			fprintf(dbg_fd,"\rhdread: illegal position given 0x%x\n",pos);
		}else{
			if (devices[drive].d_absability) {
				lseek(devices[drive].d_fd,pos,L_SET);
			}else{
				pos -= devices[drive].start_sec_rel * SEC_LENGTH;
				lseek(devices[drive].d_fd, pos, L_SET);
			}
			Ddebug0((dbg_fd,"\rmon_disk: disk_read(%x) 0x%x 0x%x 0x%x\n",
				 drive,pos,address, nms));
			if ((i = read_phys(devices[drive].d_fd,address,nms*512))!=nms*SEC_LENGTH) {
				Ddebug0((dbg_fd,"mon_disk: read failed i = %x\n", i));
			}
		}
	}
}

boolean_t invalidate_read_buffer = FALSE;
#define	DISK_READ_SIZE 4096

read_hard_disk(pos, nms, address, drive) 
	int pos;
	int nms;
	vm_address_t address;
	int drive;
{
	static char buffer[DISK_READ_SIZE];
	static int last_pos = -1;
	static int last_drive = -1;
	int size = nms*SEC_LENGTH;
	if (invalidate_read_buffer) {
		last_pos = -1;
		last_drive = -1;
		invalidate_read_buffer = FALSE;
	} else 	{
		if ((pos >= last_pos) &&
		    ((pos+size) <= (last_pos + DISK_READ_SIZE)) &&
		    (drive == last_drive)) {
			bcopy(&buffer[pos-last_pos], address, size);
			Ddebug2((dbg_fd,"Cached disk read %x\n",pos));
			return;		
		}
	}
	if ((!legal_disk_pos(pos, DISK_READ_SIZE/SEC_LENGTH, drive)) ||
	    ((size >= DISK_READ_SIZE) || 
	    (pos == 0))) {
		xx_read_hard_disk(pos, nms, address, drive);
	} else {
		Ddebug2((dbg_fd,"Caching disk read %x\n",pos));
		xx_read_hard_disk(pos, DISK_READ_SIZE/SEC_LENGTH, &buffer[0], drive);
		last_pos = pos;
		last_drive = drive;
		invalidate_read_buffer = FALSE;
		return (read_hard_disk(pos, nms, address, drive));
	}
}

int write_floppy_disk(pos, nms, address, drive)
	int pos;
	int nms;
	vm_address_t address;
{
	int i;

	if (lseek(devices[drive].d_fd, pos, L_SET) < 0)
		return(FALSE);
	Ddebug2((dbg_fd,"mon_disk: addr = 0x%x\n",address));
	if ((i = write_phys(devices[drive].d_fd, address, nms * SEC_LENGTH)) != nms * SEC_LENGTH) {
		Ddebug0((dbg_fd,"mon_disk: write failed i = %x\n", i));
		return(FALSE);
	}
	return(TRUE);
}

void write_hard_disk(pos, nms, address, drive) 
	int pos;
	int nms;
	vm_address_t address;
	int drive;
{
	int fdx;
	int i;
	
	invalidate_read_buffer = TRUE;

	Ddebug2((dbg_fd, "hdwrite: sector number = %d, address = 0x%x\n",pos/SEC_LENGTH,
		 (unsigned int)address));
	if (!legal_disk_pos(pos, nms, drive)) {
		fprintf(dbg_fd,"\rhdwrite: illegal position given 0x%x\n",pos);
	}else{
		if (devices[drive].d_absability) {
			lseek(devices[drive].d_fd,pos,L_SET);
		}else{
			pos -= devices[drive].start_sec_rel * SEC_LENGTH;
			lseek(devices[drive].d_fd, pos, L_SET);
		}
		Ddebug0((dbg_fd,"\rmon_disk: disk_write(%x) 0x%x 0x%x 0x%x\n",
			 drive,pos, address, nms));
		if ((i = write_phys(devices[drive].d_fd,address,nms*SEC_LENGTH))!=nms*SEC_LENGTH) {
			Fprintf((dbg_fd,"\rmon_disk: write failed i = %x\n", i));
			fprintf(dbg_fd,"\rmon_disk: errno = %d\n",errno);
		}
	}
}

/*
 * The main dispatching routine for Bios disk calls
 *
 */
boolean_t bios_disk_fn(state)
	state_t *state;
{
	switch (HIGH(state->eax)) {
	    case DISK_RESET: {
		    int drive = LOW(state->edx);
		    Ddebug2((dbg_fd,"mon_disk: bios_reset_disk [%x]\n", drive));

		    if (first_reset && (drive == 0)) {
			    first_reset = FALSE;
			    return(TRUE);
		    }

		    if (!validate_drive(drive)) {
			    if ((drive  < 0x80)&&(devices[drive].d_state == 
						 DEVICE_NOT_INITIALIZED)) {
				    SETHIGH(&(state->eax), ERR_FLOPPY_DISK_REMOVED);
			    }else{
				    SETHIGH(&(state->eax), 0x80);
			    }

			    return (FALSE);
		    }
	
		    /* clear diskette status word */
		    *((u_long *)0x043e) = 0;

		    break;
	    }
	    case DISK_STATUS: {
		    int drive = LOW(state->edx) ;

		    Ddebug2((dbg_fd,"mon_disk: bios_disk_status [%x]\n", drive));

		    if (!validate_drive(drive)) {
			    return FALSE;
		    }
		    SETLOW(&(state->eax), ERR_NONE);
		    break;

	    }
	    case DISK_READ_SECTOR: {
		    int	drive = LOW(state->edx);
		    Ddebug2((dbg_fd,"mon_disk: bios_read_sector [%x]\n", drive));
		    if (!validate_drive(drive)) {
			    if (drive < 0x80)
				    SETHIGH(&(state->eax), ERR_FLOPPY_DISK_REMOVED);
			    return (FALSE);
		    } else {
			    boolean_t done;
			    int i;
			    int pos;
			    int cly, head, sec, nms, nspt, nspc;
			    vm_address_t address;
			    int ret;
			    
			    /*
			     * Hard disks have the top 2 bits of the
			     * cylinder in the sector register (cl).
			     */
			    cly = ((drive < 0x80) ? HIGH(state->ecx) :
				   (((LOW(state->ecx) & 0xc0)<<2) |
				       HIGH(state->ecx)));

			    head = HIGH(state->edx);

			    sec = ((drive < 0x80) ? LOW(state->ecx) :
				   (LOW(state->ecx) & 0x3f));

			    nms = LOW(state->eax);
			    nspt = devices[drive].sectrk;
			    nspc = devices[drive].heads * nspt;

			    pos = ((cly * nspc)+((head-0) * nspt)+sec-1) * SEC_LENGTH;

			    address = Addr(state, es, ebx);

			    Ddebug2((dbg_fd,"mon_disk: cly: 0x%x head: 0x%x sec: 0x%x pos: 0x%x nms: 0x%x\n",	cly, head, sec, pos, nms));

			    if (drive < 0x80) {
				    ret = read_floppy_disk(pos, nms, address, drive);
					    
			    }else{
				    read_hard_disk(pos, nms, address, drive);
				    ret = TRUE;
			    }

			    if (ret) {
				    SETHIGH(&(state->eax), 0x00);
				    SETLOW(&(state->eax), nms);
			    }else{
				    close(devices[drive].d_fd);
				    devices[drive].d_state = DEVICE_NOT_INITIALIZED;
				    SETHIGH(&(state->eax), ERR_FLOPPY_DISK_REMOVED);
				    return(FALSE);
			    }
		    }
		    break;
	    }
	    case DISK_WRITE_SECTOR:{
		    int	drive = LOW(state->edx);
		    Ddebug2((dbg_fd,"mon_disk: bios_write_sector [%x]\n", drive));
		    if (!validate_drive(drive)) {
			    if (drive < 0x80) 
				    SETHIGH(&(state->eax), ERR_FLOPPY_DISK_REMOVED);
			    return FALSE;
		    } else {
			    int i;
			    int pos;
			    int cly, head, sec, nms, nspt, nspc;
			    vm_address_t address;
			    int ret;
			    
			    /*
			     * Hard disks have the top 2 bits of the
			     * cylinder in the sector register (cl).
			     */
			    cly = ((drive < 0x80) ? HIGH(state->ecx) :
				   (((LOW(state->ecx) & 0xc0)<<2) |
				       HIGH(state->ecx)));

			    head = HIGH(state->edx);

			    sec = ((drive < 0x80) ? LOW(state->ecx) :
				   (LOW(state->ecx) & 0x3f));

			    nms = LOW(state->eax);
			    nspt = devices[drive].sectrk;
			    nspc = devices[drive].heads * nspt;

			    pos = ((cly * nspc)+((head-0) * nspt)+sec-1) * SEC_LENGTH;

			    address = Addr(state, es, ebx);

			    Ddebug2((dbg_fd,"mon_disk: write cly: 0x%x head: 0x%x sec: 0x%x pos: 0x%x nms: 0x%x\n", cly, head, sec, pos, nms));

			    if (drive < 0x80) {
				    ret = write_floppy_disk(pos, nms, address, drive);
			    }
			    else{
				    write_hard_disk(pos, nms, address, drive);
				    ret = TRUE;
			    }

			    if (ret) {
				    SETHIGH(&(state->eax), 0x00);
				    SETLOW(&(state->eax), nms);
			    }else{
				    close(devices[drive].d_fd);
				    devices[drive].d_state = DEVICE_NOT_INITIALIZED;
				    SETHIGH(&(state->eax), ERR_FLOPPY_DISK_REMOVED);
				    return(FALSE);
			    }
		    }
		    break;
	    }
	    case DISK_VERIFY_SECTOR:{
		    int	drive = LOW(state->edx);
		    Fprintf((dbg_fd,"mon_disk: bios_verify_sector [%x]\n", drive));
		    return (FALSE);
		    break;
	    }
	    case DISK_FORMAT_TRACK:{
		    int	drive = LOW(state->edx);
		    Fprintf((dbg_fd,"mon_disk: bios_format_track [%x]\n", drive));
		    return (FALSE);
		    break;
	    }
	    case DISK_FORMAT_BAD_TRACK:{
		    int	drive = LOW(state->edx);
		    Fprintf((dbg_fd,"mon_disk: bios_format_bad_track [%x]\n", drive));
		    return (FALSE);
		    break;
	    }
	    case DISK_FORMAT_DRIVE:{
		    int	drive = LOW(state->edx);
		    Fprintf((dbg_fd,"mon_disk: bios_format_drive [%x]\n", drive));
		    return (FALSE);
		    break;
	    }
	    case DISK_DRIVE_PARAMETERS:{
		    int	drive = LOW(state->edx);
		    Ddebug2((dbg_fd,"mon_disk: bios_get_drive_params [%x]\n", drive));
		    if (!validate_drive(drive)) {
			    return FALSE;
		    }
		    if (drive < 0x80) {
			    /* floppy */
			    /* drive type (floppy) */
			    switch(devices[drive].media) {
				case MICRO_FLOPPY_0:
				    SETLOW(&(state->ebx), FD_3_H);
				    break;
				case BIG_FLOPPY:
				    SETLOW(&(state->ebx), FD_5_H);
				    break;
				default:
				    SETLOW(&(state->ebx), FD_5_H);
				    break;
			    }
			    SETLOW(&(state->ecx), devices[drive].sectrk);
		    } else {
			    int max_cyl;
			    u_char temp;

			    /* Fixed disk */
			    SETLOW(&(state->ebx), 0);
			    max_cyl = (devices[drive].sects/(devices[drive].heads * 
							     devices[drive].sectrk));
			    SETHIGH(&(state->ecx), (max_cyl & 0xff));
			    SETLOW(&(state->ecx),
				   (((0x300 & max_cyl)>>2) | devices[drive].sectrk));
			    SETHIGH(&(state->edx), devices[drive].heads-1);

			    if (devices[0x81].d_state != DEVICE_NOT_PRESENT) {
				    SETLOW(&(state->edx),2);
			    }else{
				    SETLOW(&(state->edx), 1);
			    }
			    /*
			     * XXX don't know what to do with es:di
			     * if f000:e641 then sector read error.
			     * bios changes it, but I don't know where to.
			     * seems to do all right with no mod. hmmmm....
			     */
			    SETHIGH(&(state->eax), ERR_NONE);
			    
			    break;
		    }
		    
		    SETHIGH(&(state->edx), devices[drive].heads);
		    /* number of phys drives present */
		    SETLOW(&(state->edx), 1);
		    SETHIGH(&(state->eax), ERR_NONE);
		    break;
	    }
	    case DISK_GET_TYPE: {
		    int drive;

		    drive = LOW(state->edx);
		    if (!validate_drive(drive)) {
			    Ddebug2((dbg_fd,"mon_disk: disk_get_type no disk 0x%x\n",drive));
			    SETHIGH(&(state->eax), DRIVE_NOT_PRESENT);
			    return (FALSE);
		    }

		    Ddebug2((dbg_fd,"\rmon_disk: bios_disk_get_type [%x]\n",drive));
		    
		    if (drive < 0x80) {
			    Ddebug2((dbg_fd,"mon_disk: disk_get_type floppy\n"));
			    SETHIGH(&(state->eax), DRIVE_FLOPPY_WITH_CNGLN);
		    } else {
			    Ddebug2((dbg_fd,"mon_disk: disk_get_type no disk 0x%x\n",drive));
			    SETHIGH(&(state->eax), DRIVE_FIXED);
		    }
		    SETLOW(&(state->eax), ERR_NONE);
		    break;
	    }
	    case DISK_CHANGE_LINE: {
		    int drive;
		    int ret;
		    drive = LOW(state->edx);
		    if (!validate_drive(drive)) {
			    Ddebug2((dbg_fd,"mon_disk: disk_change_line 0x%x\n",drive));
			    SETHIGH(&(state->eax), DRIVE_NOT_PRESENT);
			    return (FALSE);
		    }
		    /* always return change line active (may have changed) */
		    if (strncmp(devices[drive].d_path,"/dev/rfloppy",
				strlen("/dev/rfloppy")) == 0) {
			    SETHIGH(&(state->eax), change_line_status);
			    ret = (change_line_status == 0 ? TRUE : FALSE);
			    change_line_status = 0x00;
		    }else{
			    SETHIGH(&(state->eax), 0x06);
			    ret = FALSE;
		    }			    
		    return (ret);
		    break;
	    }		
	    default:
		Fprintf((dbg_fd,"mon_disk: DISK_fn, XXX unknown ah=0x%x\n",
			HIGH(state->eax)));
		return (FALSE);
	}
	return (TRUE);
}
