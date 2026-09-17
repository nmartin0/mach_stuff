/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * pmk1.1
 */

#include <ata.h>
#include <cpus.h>
#include <chained_ios.h>
#include <device/param.h>

#include <string.h>
#include <types.h>
#define PRIBIO 20
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/subrs.h>
#include <device/misc_protos.h>
#include <device/ds_routines.h>
#include <device/param.h>
#include <device/driver_lock.h>
#include <sys/ioctl.h>
#include <kern/spl.h>
#include <kern/misc_protos.h>
#include <machine/disk.h>
#include <chips/busses.h>
#include <ppc/proc_reg.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/atareg.h>
#include <ppc/POWERMAC/ata_entries.h>
#include <ppc/POWERMAC/interrupts.h>
#include <ppc/POWERMAC/device_tree.h>
#include <ppc/POWERMAC/dbdma.h>
#include <vm/vm_kern.h>

#define b_cylin		b_resid
#define PAGESIZ 4096

// don't really have a place for this to go right now.
// the definition is in mac_label.c
extern io_return_t
read_ata_mac_label(
	       dev_t		dev,
	       int		sector,
	       int		ext_base,
	       int		*part_name,
	       struct disklabel	*label,
	       io_return_t	(*rw_abs)(
					  dev_t		dev,
					  dev_status_t	data,
					  int		rw,
					  int		sec,
					  int		count));
/* Forward */

extern int		ataprobe(
				int			port,
				struct bus_ctlr		* ctlr);
extern int		ataslave(
				struct bus_device	* bd,
				caddr_t			xxx);
extern void		ataattach(
				struct bus_device	*dev);
extern void		atastrategy(
				struct buf		* bp);
extern void		ataminphys(
				struct buf		* bp);
extern void		atastart(
				int			ctrl);
extern void		ata_dump_registers(
				int			addr);
extern void		ataerror(
				struct buf		*bp,
				int 			ctrl);
extern boolean_t	getvtoc(
				int			unit);
extern void		set_controller(
				int			unit);
extern void		waitcontroller(
				int			unit);
extern void		start_rw(
				int			read,
				int			ctrl);
extern int		badblock_mapping(
				int			ctrl);
extern int		xfermode(
				int			ctrl);

extern void		ata_read_id(
				   struct bus_device	*dev,
				   int			ctrl);

extern 	void ata_sync_request(io_req_t ior);

/*
 * Per controller info
 */

struct ata_ctrl {
	int			address; /* I/O location */
	struct	hh		state;
	int 			need_set_controller;

	dbdma_regmap_t		*dma_chan;
	dbdma_command_t		*dma_cmds;

	driver_lock_decl(,ata_lock)
};

typedef struct ata_ctrl *ata_ctrl_t;


#define setcontroller(unit) \
	(ata_ctrl[unit>>1]->need_set_controller |= (1<<(unit&1)))

ata_ctrl_t       ata_ctrl[(NATA+1)/2];
boolean_t        ata_ctrl_initted = FALSE;

/*
 * Per drive info
 */

typedef struct {
	unsigned short	ncyl;
	unsigned short	nheads;
	unsigned short	precomp;
	unsigned short	nsec;
} hdisk_t;

struct ata_drive {
	int			open_count;
	hdisk_t			cmos_parm;
	struct io_req 		io_queue;
	struct disklabel	label;
	struct alt_info		*alt_info;
};

typedef struct label_partition *partition_t;
typedef struct ata_drive *ata_drive_t;

void	ata_build_dma(ata_ctrl_t cntrl_p, ata_drive_t drive_p);
void	ata_intr_dev0(int device, void *ssp);
void	ata_intr_dev1(int device, void *ssp);
void	ata_intr(int ctrl, ata_ctrl_t cntrl_p);


struct ata_drive *ata_drive[NATA];

/*
 * metric variables
 */
int	ata_read_count = 0;
int	ata_write_count = 0;
int	ata_queue_length = 0;


#define	DISKTEST
#ifdef	DISKTEST
#define	NULLPARTITION(z)	((z) == 0xf)	/* last partition */
#endif	/* DISKTEST */

void (*ata_int_handlers[(NATA+1)/2])(int device, void *ssp) = {
	ata_intr_dev0,
	ata_intr_dev1
};

caddr_t	ata_std[NATA] = { 0 };
struct	bus_device *ata_dinfo[NATA*NDRIVES];
struct	bus_ctlr *ata_minfo[NATA];
struct	bus_driver	ata_driver = {
	(probe_t)ataprobe, ataslave, ataattach, 0, ata_std, "hd", ata_dinfo,
		"ata", ata_minfo, 0};


void outb(unsigned long addr, UInt8 value);
void
outb(unsigned long addr, UInt8 value)
{
     *(volatile UInt8*)addr = value; eieio();
}

UInt8 inb(unsigned long addr);
UInt8 inb(unsigned long addr)
{
  UInt8     ret;
 
  ret = *(volatile UInt8*)addr;
  eieio();
  return ret;
}


void outw(int addr, UInt16 raw, boolean_t swap);
void
outw(int addr, UInt16 raw, boolean_t swap)
{
     UInt16         val;

     if (swap)
       val = (((raw >> 8) & 0x00FF) | ((raw << 8) & 0xFF00));
     else
       val = raw;
     *(volatile UInt16*)addr = val; eieio();
}


UInt16 inw(int addr, boolean_t swap);
UInt16 inw(int addr, boolean_t swap)
{
      UInt16   ret;
      
      ret = *(volatile UInt16*)addr;
      eieio();
      if (swap)
	return (((ret >> 8) & 0x00FF) | ((ret << 8) & 0xFF00));
      else
	return ret;
}


int
ataprobe(int port, struct bus_ctlr *bus)
{
	int 		i, dev,  ctrl = bus->unit, addr;
	ata_ctrl_t	ctrl_p;	
	device_node_t	*atadev, *mediabay = NULL, *mediadev, *startdev = NULL,
			*list, *next;

	if (ctrl >= (NATA+1)/2) {
		printf("** ATA - Control %d out of range\n", ctrl);
		return(0);
	}

	/* The first probe will probe for the other controllers .. */

	if (ata_ctrl_initted) 
		return ata_ctrl[bus->unit] ? TRUE : FALSE;

	ata_ctrl_initted = TRUE;

	if ((atadev = find_devices("ata")) == NULL) 
		return 0;

	/*
	 * Do some reorig for 3400's.. 
	 */

	for (; atadev; atadev = next) {
		next = atadev->next;

		if (strcmp(atadev->parent->name, "media-bay") == 0) {
			if (mediabay == NULL) {
				mediadev = mediabay = atadev;
			} else {
				mediadev->next = atadev;
				mediadev = atadev;
			}
		} else {
			if (startdev == NULL) 
				list = startdev = atadev;
			else {
				list->next = atadev;
				list = atadev;
			}
		}

		atadev->next = NULL;
	}

	if (startdev == NULL)
		startdev = mediabay;
	else
		list->next = mediabay;
				
	atadev = startdev;

	for (ctrl = 0; ctrl < ((NATA+1)/2) && atadev != NULL; ctrl++, atadev = atadev->next) {
		ata_ctrl[ctrl] = NULL;

		addr = POWERMAC_IO(atadev->addrs[0].address);

		outb(PORT_DRIVE_HEADREGISTER(addr),0);
		outb(PORT_COMMAND(addr),CMD_RESTORE);

		if (inb(PORT_STATUS(addr))&(STAT_ECC|STAT_WRITEFAULT|STAT_ERROR)) 
			continue;

		for (i=200; i && inb(PORT_STATUS(addr))&STAT_BUSY; i--)
		  	delay(10000);

		if (i == 0 || (inb(PORT_STATUS(addr))&(STAT_READY|STAT_ERROR)) != STAT_READY)  {
			continue;
		}

		ctrl_p = (ata_ctrl_t) kalloc(sizeof(struct ata_ctrl));
		if (!ctrl_p) {
		  	printf("ata_probe: kalloc failed\n");
			continue;
		}
		bzero((char *) ctrl_p, sizeof(struct ata_ctrl));
		ata_ctrl[ctrl] = ctrl_p;
		ctrl_p->address = addr;
		ctrl_p->state.curdrive = ctrl<<1;
#if DEBUG
		printf("%s%d: port = %x, spl = %d, interrupt = %d.\n",
		       bus->name, bus->unit, atadev->addrs[0].address,
		       bus->sysdep, atadev->intrs[0]);
#endif
#if	0
		/* may be necesary for two controllers */
		outb(FIXED_DISK_REG(ctrl), 4);
		for(i = 0; i < 10000; i++);
		outb(FIXED_DISK_REG(ctrl), 0);
#endif
		driver_lock_init(&ctrl_p->ata_lock,
				 ctrl,
				 atastart,
				 (void (*)(int)) ata_intr,	/* XXX */
				 DRIVER_LOCK_NOFUNC,
				 DRIVER_LOCK_NOFUNC);

		ctrl_p->dma_cmds = dbdma_alloc(((SECLIMIT*512)/1024)+2);
		ctrl_p->dma_chan = (dbdma_regmap_t*) POWERMAC_IO(atadev->addrs[1].address);

	        pmac_register_ofint(atadev->intrs[0],
					SPLBIO, ata_int_handlers[ctrl]);
	}

	return ata_ctrl[bus->unit] ? TRUE : FALSE;
}

/*
 * ataslave:
 *
 *	Actually should be thought of as a slave probe.
 *
 */

int
ataslave(
	struct bus_device	*dev,
	caddr_t			xxx)
{
	int	i;
	int	addr;
	spl_t	s;

	if (ata_ctrl[dev->ctlr] == NULL) 
		return 0;

	addr = ata_ctrl[dev->ctlr]->address;

	if (dev->slave)
	{
#if DEBUG
	     printf("ataslave: don't support device 1 yet\n");
#endif
	     return 0;                // don't support multiple devices for now
        }

	s = splbio();
	outb(PORT_DRIVE_HEADREGISTER(addr),dev->slave<<4);
	outb(PORT_COMMAND(addr),CMD_RESTORE);

	for (i=100; i > 0; i--) {
             if ((inb(PORT_STATUS(addr)) & STAT_BUSY) == 0)
			break;
		delay(100);
	}

	splx(s);

#if 0
	if (i == 0) 
        {
		outb(FIXED_DISK_REG(dev->ctlr), 4);
		for(i = 0; i < 10000; i++);
		outb(FIXED_DISK_REG(dev->ctlr), 0);
		setcontroller(dev->slave);
                printf("ataslave: no slave");
		return 0;
	}
#endif
#if DEBUG
        printf("ataslave: addr(%08x), STAT (%02x), i (%d)\n", addr, inb(PORT_STATUS(addr)), i);
#endif
        if (i && (inb(PORT_STATUS(addr)) & STAT_READY)) 
             return 1;
        return 0;
}

/*
 * ataattach:
 *
 *	Attach the drive unit that has been successfully probed.  For the
 *	AT ESDI drives we will initialize all driver specific structures
 *	and complete the controller attach of the drive.
 *
 */

void
ataattach(
	struct bus_device	*dev)
{
	int 		unit = dev->unit;
	dev_ops_t 	ata_dev_ops;
	int		tmpunit;
	ata_drive_t	drive;

	drive =(ata_drive_t) kalloc(sizeof(struct ata_drive));
	if (!drive) {
		printf("ata_attach: kalloc failed\n");
		return;
	}

	ata_drive[unit] = drive;
	bzero((char *)drive, sizeof(struct ata_drive));
	simple_lock_init(&drive->io_queue.io_req_lock, ETAP_IO_REQ);
	if (dev_name_lookup("hd", &ata_dev_ops, &tmpunit)) {
		char devname[16];
		extern int disk_indirect_count;

		strcpy(devname, "disk");
		itoa(disk_indirect_count, devname + 4);
		dev_set_indirection(devname,
				    ata_dev_ops,
				    unit * ata_dev_ops->d_subdev);
		disk_indirect_count++;
	}
	ata_read_id(dev, unit);
	setcontroller(unit);
	return;
}

io_return_t
ataopen(
	dev_t		dev,
	dev_mode_t	flag,
	io_req_t	ior)
{
	u_char	unit = UNIT(dev),
		part = PARTITION(dev);

	ata_drive_t	drive;
	struct label_partition *part_p;

	if (!(drive = ata_drive[UNIT(dev)]))
		return(D_NO_SUCH_DEVICE);

	if (part >= V_NUMPAR)
		return(D_NO_SUCH_DEVICE);
#ifdef	DISKTEST
	if (NULLPARTITION(part)) 
		return(D_SUCCESS);
#endif	/* DISKTEST */
	if (drive->open_count == 0)
		if (!getvtoc(dev))
			return(D_NO_SUCH_DEVICE);

	part_p = &drive->label.d_partitions[part];
	if (part_p->p_size <= 0)
		return(D_NO_SUCH_DEVICE);
	drive->open_count++;
	return(D_SUCCESS);
}

void
ataclose(
	dev_t		dev)
{
	ata_drive[UNIT(dev)]->open_count--;
}

void
ataminphys(
	struct buf	*bp)
{
	if (bp->b_bcount > SECLIMIT*SECSIZE)
		bp->b_bcount = SECLIMIT*SECSIZE;
}

io_return_t
ataread(
	dev_t		dev,
	io_req_t 	ior)
{
	ata_read_count++;
	return (block_io(atastrategy, ataminphys, ior));
}

io_return_t
atawrite(
	dev_t		dev,
	io_req_t	ior)
{
	ata_write_count++;
	return (block_io(atastrategy, ataminphys, ior));
}

int abs_sec   = -1;
int abs_count = -1;

/* IOC_OUT only and not IOC_INOUT */
io_return_t
atagetstat(
	dev_t		dev,
	dev_flavor_t	flavor,
	dev_status_t	data,	/* pointer to OUT array */
	natural_t	*count)	/* OUT */
{
	int		unit = UNIT(dev);
	struct buf	*bp1;
	int		i;
	ata_drive_t	drive = ata_drive[unit];
	partition_t	part_p = &drive->label.d_partitions[PARTITION(dev)];

	switch (flavor) {

	/* Mandatory flavors */

	case DEV_GET_SIZE: {
		int 		part = PARTITION(dev);
		int 		size;
		
		data[DEV_GET_SIZE_DEVICE_SIZE] = part_p->p_size * SECSIZE;
		data[DEV_GET_SIZE_RECORD_SIZE] = SECSIZE;
		*count = DEV_GET_SIZE_COUNT;
		break;
	}

	/* Extra flavors */

	case V_GETPARMS: {
		struct disk_parms 	*dp;
		int 			part = PARTITION(dev);
		hdisk_t 		*parm;

		if (*count < sizeof (struct disk_parms)/sizeof(int))
			return (D_INVALID_OPERATION);
		dp = (struct disk_parms *) data;
		dp->dp_type = DPT_WINI;
		dp->dp_heads = ata_drive[unit]->label.d_ntracks;
		dp->dp_cyls = ata_drive[unit]->label.d_ncylinders;
		dp->dp_sectors  = ata_drive[unit]->label.d_nsectors;
  		dp->dp_dosheads = ata_drive[unit]->cmos_parm.nheads;
		dp->dp_doscyls = ata_drive[unit]->cmos_parm.ncyl;
		dp->dp_dossectors  = ata_drive[unit]->cmos_parm.nsec;
		dp->dp_secsiz = SECSIZE;
		dp->dp_ptag = 0;
		dp->dp_pflag = 0;
		dp->dp_pstartsec = part_p->p_offset;
		dp->dp_pnumsec = part_p->p_size;
		*count = sizeof(struct disk_parms)/sizeof(int);
		break;
	}
	case V_RDABS: {
		/* V_RDABS is relative to head 0, sector 0, cylinder 0 */
		if (*count < SECSIZE/sizeof (int)) {
			printf("ata%d: RDABS bad size %x", unit, count);
			return (D_INVALID_OPERATION);
		}
		if (ata_rw_abs(dev, data, IO_READ, 
				abs_sec, SECSIZE) != D_SUCCESS)
			return(D_INVALID_OPERATION);
		*count = SECSIZE/sizeof(int);
		break;
	}
	case V_VERIFY: {
		int cnt = abs_count * SECSIZE;
		int sec = abs_sec;
                int bsize = PAGE_SIZE;
		char *verify_buf;

		(void) kmem_alloc(kernel_map,
				  (vm_offset_t *)&verify_buf,
				  bsize);

		*data = 0;
		while (cnt > 0) {
			int xcount = (cnt < bsize) ? cnt : bsize;
			if (ata_rw_abs(dev, (dev_status_t)verify_buf,
					IO_READ, sec, xcount) != D_SUCCESS) {
				*data = BAD_BLK;
				break;
			} else {
				cnt -= xcount;
				sec += xcount / SECSIZE;
			}
	        }

		(void) kmem_free(kernel_map, (vm_offset_t)verify_buf, bsize);
		*count = 1;
		break;
	}
	default:
		return(D_INVALID_OPERATION);
	}
	return D_SUCCESS;
}

/* IOC_VOID or IOC_IN or IOC_INOUT */
io_return_t
atasetstat(
	dev_t		dev,
	dev_flavor_t	flavor,
	dev_status_t	data,
	natural_t	count)
{
	struct buf	*bp1;
	io_return_t	errcode = D_SUCCESS;
	int		unit = UNIT(dev);
	ata_drive_t	drive = ata_drive[unit];
	partition_t	part_p = &drive->label.d_partitions[PARTITION(dev)];

	switch (flavor) {
	case V_REMOUNT:
		(void) getvtoc(dev);
		break;
	case V_ABS:
		abs_sec = *(int *)data;
		if (count == 2)
			abs_count = data[1];
		break;
	case V_WRABS:
		/* V_WRABS is relative to head 0, sector 0, cylinder 0 */
		if (count < SECSIZE/sizeof (int)) {
			printf("ata%d: WRABS bad size %x", unit, count);
			return (D_INVALID_OPERATION);
		}
		if (ata_rw_abs(dev, data, IO_WRITE, 
			       abs_sec, SECSIZE) != D_SUCCESS)
			return(D_INVALID_OPERATION);
		break;
	default:
		return (D_INVALID_OPERATION);
	}
	return (errcode);
}

#if CHAINED_IOS

#define IO_OP(op) (op & (IO_READ|IO_WRITE))

/* 
 * can_chain_io_reqs(a, b)
 * Can we chain two IOs ?
 * Check that:
 *	OPs (Read, write) are identical,
 *	Record numbers are consecutive,
 *	SECLIMIT is not reached.
 */

#define can_chain_io_reqs(a, b)						\
  	(IO_OP(a->io_op) == IO_OP(b->io_op) &&				\
	 (a->io_unit == b->io_unit) &&					\
	 (a->io_recnum + ((a->io_count + 511) >> 9) == b->io_recnum ||	\
	  b->io_recnum + ((b->io_count + 511) >> 9) == a->io_recnum) && \
	 (a->io_count + b->io_count <= SECLIMIT*SECSIZE))
	 
#endif /* CHAINED_IOS */

void
atastrategy(
	struct buf	*bp)
{
	dev_t		dev = bp->b_dev;
	u_char		unit = UNIT(dev),
			ctrl = unit>>1,
			part = PARTITION(dev);
	u_int		opri;
	hdisk_t		*parm;
	ata_drive_t	drive = ata_drive[unit];
	partition_t	part_p;
	int 		size;
	int		start;

#ifdef	DISKTEST
	if (NULLPARTITION(PARTITION(dev))) {
		bp->b_resid = 0;
		goto done;
	}
#endif	/* DISKTEST */

	
	if (!bp->b_bcount)
		goto done;

	/* if request is off the end or trying to write last block on out */
	if (bp->b_flags & B_MD1) {
		part_p  = &drive->label.d_partitions[MAXPARTITIONS];
	} else {
		part_p  = &drive->label.d_partitions[PARTITION(dev)];
	}
	if (part_p->p_size <= 0) {
		bp->b_error = ENXIO;
		goto bad;
	}
	size = part_p->p_size;
	start = part_p->p_offset;

	if (bp->b_blkno >= size) {
		bp->io_error = D_INVALID_RECNUM;
		bp->io_op |= IO_ERROR;
		bp->io_residual = bp->io_count;
		goto bad;
	}

	bp->b_cylin = (start + bp->b_blkno) / 
	              (drive->label.d_nsectors * drive->label.d_ntracks );
	opri = splbio();
	simple_lock(&drive->io_queue.io_req_lock);

	disksort(&drive->io_queue, bp);
	if (++drive->io_queue.io_total > ata_queue_length)
		ata_queue_length = drive->io_queue.io_total;
#if CHAINED_IOS
		if (!(bp->io_op & IO_SGLIST)) {
		  	bp->io_seg_count = 1;
			if (bp->io_prev && 
			    (bp->io_prev != drive->io_queue.b_actf ||
			     !ata_ctrl[ctrl]->state.controller_busy) &&
			    can_chain_io_reqs(bp->io_prev, bp)) {
				chain_io_reqs(bp->io_prev, bp,
					      &drive->io_queue);
				drive->io_queue.io_total--;
			}
			if (bp->io_next &&
			    can_chain_io_reqs(bp, bp->io_next)) {
				chain_io_reqs(bp, bp->io_next,
					      &drive->io_queue);
				drive->io_queue.io_total--;
			}
		}
#endif /* CHAINED_IOS */
	simple_unlock(&drive->io_queue.io_req_lock);
	if (!ata_ctrl[ctrl]->state.controller_busy)
		atastart(ctrl);
	splx(opri);
	return;
bad:
	bp->b_flags |= B_ERROR;
done:
#if CHAINED_IOS
	if (bp->io_op & IO_CHAINED) {
		chained_iodone(bp);
	}
	else 
#endif	/* CHAINED_IOS */
		iodone(bp);
	return;
}

/* atastart() is called at spl5 */
void
atastart(
	int		ctrl)
{
	register struct buf	*bp;
	int                     i;
	partition_t		part_p;
	ata_ctrl_t		ctrl_p = ata_ctrl[ctrl];
	ata_drive_t		drive;
	int 			start, size;

	if (!driver_lock(&ctrl_p->ata_lock, DRIVER_OP_START, FALSE)) {
		return;
	}

	if (ctrl_p->state.controller_busy) {
		goto done;
	}

	/* things should be quiet */
	if (i = ctrl_p->need_set_controller) {
		if (i&1) set_controller(ctrl<<1);
		if (i&2) set_controller((ctrl<<1)||1);
		ctrl_p->need_set_controller= 0;
	}

	if ((drive = ata_drive[ctrl_p->state.curdrive^1]) &&
	    (bp = drive->io_queue.b_actf))
		ctrl_p->state.curdrive ^= 1;
	else {
		drive = ata_drive[ctrl_p->state.curdrive];
		if (drive == NULL)
			goto done;

		if (!(bp = drive->io_queue.b_actf))
			goto done;
	}

	ctrl_p->state.controller_busy = 1;
	ctrl_p->state.blocktotal = (bp->b_bcount + 511) >> 9;
	/* see V_RDABS and V_WRABS in ataioctl() */
	if (bp->b_flags & B_MD1) {
		part_p = &drive->label.d_partitions[MAXPARTITIONS];
	} else {
		part_p = &drive->label.d_partitions[PARTITION(bp->b_dev)];
	}
	size = part_p->p_size;
	start = part_p->p_offset;

 	ctrl_p->state.physblock = start + bp->b_blkno;
	if ((bp->b_blkno + ctrl_p->state.blocktotal) > size)
	  	ctrl_p->state.blocktotal = size - bp->b_blkno;
	ctrl_p->state.blockcount = 0;
	ctrl_p->state.rw_addr = (int)bp->b_un.b_addr;
	ctrl_p->state.retry_count = 0;
	start_rw(bp->b_flags & B_READ, ctrl);
done:
	driver_unlock(&ctrl_p->ata_lock);
        return;
}

#if	MACH_ASSERT
int	ata_debug = 1;
#else	/* MACH_ASSERT */
int	ata_debug = 0;
#endif  /* MACH_ASSERT */

void
ata_dump_registers(
	int ctrl)
{
        int	base = ata_ctrl[ctrl]->address;

	if (!ata_debug)
		return;
	printf("Controller registers:\n");
	printf("Status Register: 0x%x\n", inb(PORT_STATUS(base)));
	waitcontroller(ctrl);
	printf("Error Register: 0x%x\n", inb(PORT_ERROR(base)));
	printf("Sector Count: 0x%x\n", inb(PORT_NSECTOR(base)));
	printf("Sector Number: 0x%x\n", inb(PORT_SECTOR(base)));
	printf("Cylinder High: 0x%x\n", inb(PORT_CYLINDERHIBYTE(base)));
	printf("Cylinder Low: 0x%x\n", inb(PORT_CYLINDERLOWBYTE(base)));
	printf("Drive/Head Register: 0x%x\n",
			inb(PORT_DRIVE_HEADREGISTER(base)));
}


// temporary versions of linw and loutw
void linw(UInt16 *addr, UInt16 *data, UInt32 count, boolean_t swap);
void loutw(UInt16 *addr, UInt16 *data, UInt32 count, boolean_t swap);

void
linw(UInt16 *addr, UInt16 *data, UInt32 count, boolean_t swap)
{
  while (count--)
    *data++ = inw((int)addr, swap);
}

void
loutw(UInt16 *addr, UInt16 *data, UInt32 count, boolean_t swap)
{
  while (count--)
    outw((int)addr, *data++, swap);
}


int ata_device_to_ctrl(int device);
int
ata_device_to_ctrl(int device)
{
  switch (device)
    {
    case OHARE_DEV_ATA0:
      return 0;
    case OHARE_DEV_ATA1:
      return 1;
    default:
      panic("ata: invalid interrupt\n");
      return 0;
    }
}

void ata_intr_dev0(int device, void *ssp)
{
	ata_intr(0, ata_ctrl[0]);
}

void ata_intr_dev1(int device, void *ssp)
{
	ata_intr(1, ata_ctrl[1]);
}

int ata_print_error = 1;
void
ata_intr(int ctrl, ata_ctrl_t ctrl_p)
{
	register struct buf	*bp = NULL;
	int			addr = ctrl_p->address,
				unit = ctrl_p->state.curdrive;
	u_char status, errstat;

	if (!driver_lock(&ctrl_p->ata_lock, DRIVER_OP_INTR, FALSE)) {
		return;
	}

	if (!ctrl_p->state.controller_busy) 
		goto done;

	bp = ata_drive[unit]->io_queue.b_actf;

	if (!bp) {
		/* there should be a read/write buffer queued at this point */
		//printf("ata%d: no bp buffer to read or write status - %x\n",unit, status);
		goto done;
	} else
		dbdma_stop(ctrl_p->dma_chan);

	waitcontroller(ctrl);
	status = inb(PORT_STATUS(addr));
	errstat = inb(PORT_ERROR(addr));

	if (ctrl_p->state.restore_request) { /* Restore command has completed */
		ctrl_p->state.restore_request = 0;
		if (status & STAT_ERROR)
			ataerror(bp,ctrl);
		else if (bp)
			start_rw(bp->b_flags & B_READ, ctrl);
		goto done;
	}


	if (status & STAT_WRITEFAULT) {
		printf("ataintr: write fault. status 0x%X\n", status);
		printf("ataintr: write fault. block %d, count %d, total %d\n",
		       ctrl_p->state.physblock,
		       ctrl_p->state.blockcount,
		       ctrl_p->state.blocktotal);
		printf("ataintr: write fault. cyl %d, head %d, sector %d\n",
		       ctrl_p->state.cylinder,
		       ctrl_p->state.head,
		       ctrl_p->state.sector);
		ata_dump_registers(ctrl);
		panic("ata: write fault\n");
	}
	if (status & STAT_ERROR) {
		if (ata_print_error) {
			 printf("ataintr: state error %x, error = %x\n",
			 	status, inb(PORT_ERROR(addr)));
			 printf("ataintr: state error. block %d, count %d, total %d\n",
			 	ctrl_p->state.physblock,
				ctrl_p->state.blockcount,
				ctrl_p->state.blocktotal);
			 printf("ataintr: state error. cyl %d, head %d, sector %d\n",
			 	ctrl_p->state.cylinder,
				ctrl_p->state.head,
				ctrl_p->state.sector);

		}
		ataerror(bp,ctrl);
		goto done;
	}

	if (status & STAT_ECC) 
		printf("ata: ECC soft error fixed, unit %d, partition %d, physical block %d \n",
			unit, PARTITION(bp->b_dev), ctrl_p->state.physblock);

	if (bp) {
		if (bp->io_op & IO_READ)
			ata_sync_request(bp);

		simple_lock(&ata_drive[unit]->io_queue.io_req_lock);
		ata_drive[unit]->io_queue.b_actf = ata_drive[unit]->io_queue.b_actf->av_forw;
		ata_drive[unit]->io_queue.io_total--;

		simple_unlock(&ata_drive[unit]->io_queue.io_req_lock);

		bp->b_resid = bp->b_bcount - (ctrl_p->state.blocktotal << 9);

		if (bp->b_resid < 0) 
			bp->b_resid = 0;
#if CHAINED_IOS
		if (bp->io_op & IO_CHAINED) {
			chained_iodone(bp);
		} else 
#endif	/* CHAINED_IOS */
			iodone(bp);
	}

	ctrl_p->state.controller_busy = 0;

	atastart(ctrl);

done:
	driver_unlock(&ctrl_p->ata_lock);
        return;
}

void
ataerror(
	struct buf	*bp,
	int 		ctrl)
{
	ata_ctrl_t	ctrl_p = ata_ctrl[ctrl];
	int		addr = ctrl_p->address,
			unit = ctrl_p->state.curdrive;
	hdisk_t		*parm;
	
	parm = &ata_drive[unit]->cmos_parm;
	if(++ctrl_p->state.retry_count > 3) {
		if(bp) {
			int i;
			/****************************************************
			* We have a problem with this block, set the error  *
			* flag, terminate the operation and move on to the  *
			* next request.  With every hard disk transaction   *
			* error we set the reset requested flag so that the *
			* controller is reset before the next operation is  *
			* started.					    *
			****************************************************/
#if CHAINED_IOS
			if (bp->io_op & IO_CHAINED) {
			    /*
			     * Since multiple IOs are chained, split them
			     * and restart prior to error handling
			     */
			    simple_lock(&ata_drive[unit]->io_queue.io_req_lock);
			    split_io_reqs(bp);
			    ata_drive[unit]->io_queue.io_total++;
			    bp->b_resid = 0;
			    simple_unlock(&ata_drive[unit]->io_queue.io_req_lock);
			} else 
#endif	/* CHAINED_IOS */
			{
			    simple_lock(&ata_drive[unit]->io_queue.io_req_lock);
			    ata_drive[unit]->io_queue.b_actf =
			         ata_drive[unit]->io_queue.b_actf->av_forw;
			    ata_drive[unit]->io_queue.io_total--;
			    simple_unlock(&ata_drive[unit]->io_queue.io_req_lock);
			    bp->b_flags |= B_ERROR;
			    bp->b_resid = 0;
			    iodone(bp);
			}
			panic("ata: don't know what to do");
		//	outb(FIXED_DISK_REG(ctrl), 4);
			for(i = 0; i < 10000; i++);
		//	outb(FIXED_DISK_REG(ctrl), 0);
			setcontroller(unit);
			ctrl_p->state.controller_busy = 0;
			atastart(ctrl);
		}
	}
	else {
		/* lets do a recalibration */
		waitcontroller(ctrl);
		ctrl_p->state.restore_request = 1;
		outb(PORT_PRECOMP(addr), parm->precomp>>2);
		outb(PORT_NSECTOR(addr), parm->nsec);
		outb(PORT_SECTOR(addr), ctrl_p->state.sector);
		outb(PORT_CYLINDERLOWBYTE(addr),
		     ctrl_p->state.cylinder & 0xff);
		outb(PORT_CYLINDERHIBYTE(addr),
		     (ctrl_p->state.cylinder>>8) & 0xff);
		outb(PORT_DRIVE_HEADREGISTER(addr), (unit&1)<<4);
		outb(PORT_COMMAND(addr), CMD_RESTORE);
	}
}

void
set_controller(
	int		unit)
{
	int	ctrl = unit >> 1;
	int	addr = ata_ctrl[ctrl]->address;
	hdisk_t	*parm;
	
	waitcontroller(ctrl);
	outb(PORT_DRIVE_HEADREGISTER(addr),
	     (ata_drive[unit]->label.d_ntracks - 1) |
	     ((unit&1) << 4) | FIXEDBITS);
	outb(PORT_NSECTOR(addr), ata_drive[unit]->label.d_nsectors);
	outb(PORT_COMMAND(addr), CMD_SETPARAMETERS);
	waitcontroller(ctrl);
}

void
waitcontroller(
	int		ctrl)
{
	u_int	n = PATIENCE;

	while (--n && inb(PORT_STATUS(ata_ctrl[ctrl]->address)) & STAT_BUSY);
	if (n)
		return;
	panic("ata: waitcontroller() timed out");
}

void
start_rw(
	int	read,
	int	ctrl)
{
	u_int			track, disk_block, xblk;
	ata_ctrl_t		ctrl_p = ata_ctrl[ctrl];
	int			addr = ctrl_p->address,
				unit = ctrl_p->state.curdrive;
	ata_drive_t		drive = ata_drive[unit];
	struct disklabel	*label = &drive->label;
	
	disk_block = ctrl_p->state.physblock;
	/*# blks to transfer*/
	xblk=ctrl_p->state.blocktotal - ctrl_p->state.blockcount;

#if 0
	if (!(drive->io_queue.b_actf->b_flags & B_MD1) &&
	    (ctrl_p->state.single_mode = xfermode(ctrl))) {
		xblk = 1;
		if (PARTITION(drive->io_queue.b_actf->b_dev) != PART_DISK)
			disk_block = badblock_mapping(ctrl);
	}
#endif

	/* disk is formatted starting sector 1, not sector 0 */
	ctrl_p->state.sector = (disk_block % label->d_nsectors) + 1;
	track = disk_block / label->d_nsectors;
	ctrl_p->state.head = track % label->d_ntracks | 
		(unit&1) << 4 | FIXEDBITS;
	ctrl_p->state.cylinder = track / label->d_ntracks ;
	ata_build_dma(ctrl_p, drive);

	outb(PORT_DRIVE_HEADREGISTER(addr), ctrl_p->state.head);
	waitcontroller(ctrl);

	if (!read)
		ata_sync_request(drive->io_queue.b_actf);

	outb(PORT_PRECOMP(addr), drive->cmos_parm.precomp >>2);
	outb(PORT_NSECTOR(addr), xblk);
	outb(PORT_SECTOR(addr), ctrl_p->state.sector);
	outb(PORT_CYLINDERLOWBYTE(addr), ctrl_p->state.cylinder & 0xff );
	outb(PORT_CYLINDERHIBYTE(addr), (ctrl_p->state.cylinder >> 8) & 0xff );
	outb(PORT_COMMAND(addr), read ? CMD_READ_DMA : CMD_WRITE_DMA);

	dbdma_start(ctrl_p->dma_chan, ctrl_p->dma_cmds);
}

int badblock_mapping(
	int	ctrl)
{
	u_short		n;
	u_int		track,
			unit = ata_ctrl[ctrl]->state.curdrive,
			block = ata_ctrl[ctrl]->state.physblock,
			nsec;
	ata_drive_t	drive = ata_drive[unit];
	struct alt_info *alt_info = drive->alt_info;;

	if (!alt_info)
		return(block);

	nsec = ata_drive[unit]->label.d_nsectors;

	track = block / nsec;

	panic("ata: bad block mapping");
//	for (n = 0; n < alt_info->alt_trk.alt_used; n++)
//		if (track == alt_info->alt_trk.alt_bad[n])
//			return alt_info->alt_trk.alt_base +
//			       nsec * n + (block % nsec);
//	/* BAD BLOCK MAPPING */
//	for (n = 0; n < alt_info->alt_sec.alt_used; n++)
//		if (block == alt_info->alt_sec.alt_bad[n])
//			return alt_info->alt_sec.alt_base + n;
	return block;
}

/*
 *  determine single block or multiple blocks transfer mode
 */
int
xfermode(
	int	ctrl)
{
	int		n, bblk, eblk, btrk, etrk;
	ata_ctrl_t	ctrl_p = ata_ctrl[ctrl];
	int		unit = ctrl_p->state.curdrive;
	ata_drive_t	drive = ata_drive[unit];
	struct alt_info *alt_info = drive->alt_info;;

	if (!alt_info)
		return 0;

	bblk = ctrl_p->state.physblock;
	eblk = bblk + ctrl_p->state.blocktotal - 1;
	btrk = bblk / drive->label.d_nsectors;
	etrk = eblk / drive->label.d_nsectors;
	
//	panic("ata: do I do this mapping");
//	for (n = 0; n < alt_info->alt_trk.alt_used; n++)
//		if ((btrk <= alt_info->alt_trk.alt_bad[n]) &&
//		    (etrk >= alt_info->alt_trk.alt_bad[n]))
//			return 1;
//	for (n = 0; n < alt_info->alt_sec.alt_used; n++)
//		if ((bblk <= alt_info->alt_sec.alt_bad[n]) &&
//		    (eblk >= alt_info->alt_sec.alt_bad[n]))
//			return 1;
	return 0;
}

/*
 *	Routine to return information to kernel.
 */
io_return_t
atadevinfo(
	dev_t		dev,
	dev_flavor_t	flavor,
	char		*info)
{
	register int	result;

	result = D_SUCCESS;

	switch (flavor) {
	case D_INFO_BLOCK_SIZE:
		*((int *) info) = SECSIZE;
		break;
	default:
		result = D_INVALID_OPERATION;
	}

	return(result);
}

boolean_t
getvtoc(
	int			dev)
{
	ata_drive_t		drive = ata_drive[UNIT(dev)];
	struct disklabel	*label = &drive->label;
	int			partition_index = 0;

	/* 
	 * Setup last partition that we can access entire physical disk
	 */

	label->d_secsize = SECSIZE;
	label->d_partitions[MAXPARTITIONS].p_size = -1;
	label->d_partitions[MAXPARTITIONS].p_offset = 0;

	if (read_ata_mac_label(dev,
			0,			/* sector 0 */
			0,			/* ext base */
			&partition_index,	/* first partition */
			label,
			ata_rw_abs) == D_SUCCESS) {
	  	setcontroller(UNIT(dev));
	} else {
		/* make partition 'c' the whole disk in case of failure */
		label->d_partitions[PART_DISK].p_offset = 0;
		label->d_partitions[PART_DISK].p_size =
		  	drive->cmos_parm.ncyl *
			drive->cmos_parm.nheads * 
			drive->cmos_parm.nsec;
	}
	return(TRUE);
}

io_return_t
ata_rw_abs(
	dev_t		dev,
	dev_status_t	data,
	int		rw,
	int		sec,
	int		count)
{
	io_req_t	ior;
	io_return_t	error;

	io_req_alloc(ior);
	ior->io_next = 0;
	ior->io_unit = dev;	
	ior->io_data = (io_buf_ptr_t)data;
	ior->io_count = count;
	ior->io_recnum = sec;
	ior->io_error = 0;
	if (rw == IO_READ)
		ior->io_op = IO_READ;
	else
		ior->io_op = IO_WRITE;
	ior->io_op |= B_MD1;
	atastrategy(ior);
	iowait(ior);
	error = ior->io_error;
	io_req_free(ior);
	return(error);
}

void 
ata_read_id (
	struct bus_device	*dev,
	int			unit)
{
	int		ctrl = unit >> 1;
	ata_ctrl_t	ctrl_p = ata_ctrl[ctrl];
	int		addr = ctrl_p->address;
	hd_id_t		id;
	register int 	i;
	long long	bytes;
	u_long		n;
	u_char		*tbl;
	hdisk_t		parm;
	spl_t		s;

	s = splbio();
	waitcontroller(ctrl);
	ctrl_p->state.restore_request = 1;
	outb(PORT_PRECOMP(addr), 0);
	outb(PORT_NSECTOR(addr), 0);
	outb(PORT_SECTOR(addr), 0);
	outb(PORT_CYLINDERLOWBYTE(addr), 0);
	outb(PORT_CYLINDERHIBYTE(addr), 0);
	outb(PORT_DRIVE_HEADREGISTER(addr), (unit&1)<<4);
	outb(PORT_COMMAND(addr), CMD_IDENTIFY);
	waitcontroller(ctrl);
	linw((UInt16*)PORT_DATA(addr), (UInt16 *)&id, sizeof(id)/2, TRUE);
#if 0
	// this is all DOS specific stuff, looking for drive information from the BIOS
	n = *(unsigned long *)phystokv((int)dev->address);
	tbl = (unsigned char *)phystokv((n&0xffff) + ((n>>12)&0xffff0));
	parm.ncyl   = *(unsigned short *)tbl;
	parm.nheads = *(unsigned char  *)(tbl+2);
	parm.nsec   = *(unsigned char  *)(tbl+14);

	parm.precomp= *(unsigned short *)(tbl+5);
	ata_drive[unit]->cmos_parm = parm;
#else
	parm.ncyl = id.cyls;
	parm.nheads = id.heads;
	parm.nsec = id.spt;
	parm.precomp = 0;

#endif
	if (id.val_cur_values & 1) {
		ata_drive[unit]->label.d_nsectors = id.cur_secs;
		ata_drive[unit]->label.d_ntracks = id.cur_heads;
		ata_drive[unit]->label.d_ncylinders = id.cur_cyls;
	} else {
	        panic("ata: don't have a BIOS\n");
#if 0
		ata_drive[unit]->label.d_nsectors = parm.nsec;
		ata_drive[unit]->label.d_ntracks = parm.nheads;
		ata_drive[unit]->label.d_ncylinders = parm.ncyl;
#endif
	}
	splx(s);

	bytes = parm.ncyl * parm.nheads * parm.nsec;
	bytes = bytes * (long long) 512;
	bytes = bytes / (long long) 1000000;

	if (unit < 2 || (id.val_cur_values & 1))
		printf(" ata%d: %d Meg, C:%d H:%d S:%d - ",
		       unit,
			(long) bytes,
		       parm.ncyl,
		       parm.nheads,
		       parm.nsec);
	else
		printf("ata%d:   Capacity not available through bios\n",unit);

	printf("%s", id.model);

}

static dbdma_command_t *
ata_construct_entry(vm_offset_t address, long count, dbdma_command_t *cmd,
		boolean_t isread)
{
	vm_offset_t	physaddr;
	long	real_count;
	int	i;

	while (count > 0) {
		physaddr = kvtophys(address);

		if (physaddr == 0)
			panic("ATA DMA - Zero physical address");

		real_count = 0x1000 - (physaddr & 0xfff);
		real_count = real_count < count ? real_count : count;


	        DBDMA_BUILD(cmd,
			isread ? DBDMA_CMD_IN_MORE : DBDMA_CMD_OUT_MORE,
                        0, real_count, physaddr,
			DBDMA_INT_NEVER, DBDMA_BRANCH_NEVER, DBDMA_WAIT_NEVER);
 
		count -= real_count;
		address += real_count;
		cmd++;
	}

	return cmd;

}

void
ata_build_dma(ata_ctrl_t cntrl_p, ata_drive_t drive)
{
	long		count;
	io_req_t	ior = drive->io_queue.b_actf;
	dbdma_command_t	*cmd = cntrl_p->dma_cmds;
	boolean_t 	isread = (ior->io_op & IO_READ) ? TRUE : FALSE;

	if ((ior->io_op & IO_CHAINED) == 0) {
		cmd = ata_construct_entry((vm_offset_t)ior->io_data,
					ior->io_count, cmd,isread);
	} else for (; ior; ior = ior->io_link) {
		count = ior->io_count;

		if (ior->io_link)
			count -= ior->io_link->io_count;

		cmd = ata_construct_entry((vm_offset_t) ior->io_data, count,
							cmd, isread);
	}

	DBDMA_BUILD(cmd, DBDMA_CMD_STOP, 0, 0, 0, DBDMA_INT_NEVER,
			DBDMA_BRANCH_NEVER, DBDMA_WAIT_NEVER);
}

static void
ata_sync_entry(boolean_t isread, vm_offset_t address, long count)
{
	if (isread) 
		invalidate_cache_for_io(address, count, FALSE);
	else
		flush_dcache(address, count, FALSE);
}

void
ata_sync_request(io_req_t ior)
{
	long	count;
	boolean_t	isread = (ior->io_op & IO_READ) ? TRUE : FALSE;

	if (ior->io_op & IO_CHAINED) {
		for (; ior; ior = ior->io_link) {
			count = ior->io_count;

			if (ior->io_link)
				count -= ior->io_link->io_count;

			ata_sync_entry(isread, (vm_offset_t)ior->io_data, count);
		}
	} else 
		ata_sync_entry(isread, (vm_offset_t)ior->io_data, ior->io_count);
	
}
