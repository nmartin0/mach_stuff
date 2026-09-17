/*
 * Copyright (c) 1991-1998 Open Software Foundation, Inc. 
 *  
 * 
 */
/*
 * MkLinux
 */

#include <osfmach3/block_dev.h>

#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/fs.h>

#define MAJOR_NR	SCSI_GENERIC_MAJOR
#include <linux/blk.h>
#include <scsi/sg.h>
#include "scsi_ioctl.h"
#undef SG_BIG_BUFF

#define MAX_MINORS	256
#define MAX_COMMAND_SIZE    12

int sg_blocksizes[MAX_MINORS];
int sg_hardsizes[MAX_MINORS];
int sg_refs[MAX_MINORS];	/* number of refs on Mach devices */
int exclude[MAX_MINORS];
int pending[MAX_MINORS];
int complete[MAX_MINORS];
kdev_t sg_aliases[MAX_MINORS];	/* aliases for special minors */
int sg_mach_blocksizes[MAX_MINORS];

extern const unsigned char scsi_command_size[8];
#define COMMAND_SIZE(opcode) scsi_command_size[((opcode) >> 5) & 7]

static int sg_read(struct inode *inode,struct file *filp,char *buf,int count);
static int sg_write(struct inode *inode,struct file *filp,const char *buf,int count);
// static int sg_select(struct inode *inode, struct file *file, int sel_type, select_table * wait);
static int sg_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg);
static int sg_open(struct inode * inode, struct file * filp);
static void sg_close(struct inode * inode, struct file * filp);



#if 0
static int sg_attach(Scsi_Device * SDp);
static void sg_detach(Scsi_Device * SDp);
static int sg_detect(Scsi_Device * SDp);
static int sg_init(void);
static void sg_command_done(Scsi_Cmnd * SCpnt);
static void sg_free(char *buff,int size);
#endif

#if 0
struct Scsi_Device_Template sg_template = {NULL, NULL, "sg", NULL, 0xff, 
					       SCSI_GENERIC_MAJOR, 0, 0, 0, 0,
					       sg_detect, sg_init,
					       NULL, sg_attach, sg_detach};

#endif

// struct file_operations sg_fops;
// static struct scsi_generic *scsi_generics=NULL;

int scsi_generic_dev_to_name(
	kdev_t	dev,
	char	*name);


static struct file_operations sg_fops = {
    NULL,            /* lseek */
    sg_read,         /* read */
    sg_write,        /* write */
    NULL,            /* readdir */
    NULL,	 //sg_select,       /* select */
    sg_ioctl,        /* ioctl */
    NULL,            /* mmap */
    sg_open,         /* open */
    sg_close,        /* release */
    NULL             /* fsync */
};

int sg_init(void)
{
	int minors_per_major;
	int i;
	int ret;

	sg_fops = blkdev_fops;
	sg_fops.ioctl = sg_ioctl;
	if (register_blkdev(MAJOR_NR, "sg", &sg_fops)) {
		printk("Unable to get major %d for SCSI generic\n", MAJOR_NR);
		return -EBUSY;
	}

#if 0
	for (minors_per_major = 0;
	     DEVICE_NR(minors_per_major) == 0 && minors_per_major < MAX_MINORS;
	     minors_per_major++);
#else
	minors_per_major = 1;
#endif

	for (i = 0; i < MAX_MINORS; i++) {
		sg_blocksizes[i] = 1024;
		sg_hardsizes[i] = 512;
	}
	blksize_size[MAJOR_NR] = sg_blocksizes;
	hardsect_size[MAJOR_NR] = sg_hardsizes;
	read_ahead[MAJOR_NR] = 256;	/* 256 sector read-ahead */

	ret = osfmach3_register_blkdev(MAJOR_NR,
				       "sg",
				       OSFMACH3_DEV_BSIZE,
				       sg_mach_blocksizes,
				       minors_per_major,
				       scsi_generic_dev_to_name,
				       NULL,
				       NULL);
	if (ret) {
		printk("sg_init: osfmach3_register_blkdev failed %d\n", ret);
		panic("sg_init: can't register Mach device");
	}
	
	for (i = 0; i < MAX_MINORS; i++) {
		sg_refs[i] = 0;
		exclude[i] = 0;
#if 0
		if (i % minors_per_major == 0) {
			/*
			 * Minor 0 of a unit is the whole unit.
			 * We don't have that concept in Mach, so just
			 * make an alias to the first real minor in the
			 * partition and hope it works...
			 */
			sg_aliases[i] = MKDEV(MAJOR_NR, i + 1);
		} else {
			sg_aliases[i] = 0;
		}
#endif
	}
	osfmach3_blkdev_refs[MAJOR_NR] = sg_refs;
	osfmach3_blkdev_alias[MAJOR_NR] = sg_aliases;


#ifdef DEBUG
	printk("sg: Init generic device.\n");
#endif
	return 0;
}

int scsi_generic_dev_to_name(
	kdev_t	dev,
	char	*name)
{
	unsigned int major, minor;

	dev = blkdev_get_alias(dev);

	major = MAJOR(dev);
	minor = MINOR(dev);
	
	if (major >= MAX_BLKDEV) {
		return -ENODEV;
	}
	if (osfmach3_blkdevs[major].name == NULL) {
		panic("scsi_generic_dev_to_name: no Mach info for dev %s\n",
		      kdevname(dev));
	}

	sprintf(name, "%s%d",
		osfmach3_blkdevs[major].name,
		minor / osfmach3_blkdevs[major].minors_per_major);

	return 0;
}

static int
sg_ioctl(
	struct inode * inode,
	struct file * file,
	unsigned int cmd,
	unsigned long arg)
{
	kdev_t			dev;
	unsigned int		major, minor;
	mach_port_t		device_port;
#if 0
	int			result;
#endif

	dev = inode->i_rdev;
	dev = blkdev_get_alias(dev);

	major = MAJOR(dev);
	minor = MINOR(dev);
	if (major >= MAX_BLKDEV) {
		return -ENODEV;
	}

	if (osfmach3_blkdevs[major].name == NULL) {
		panic("sg_ioctl: no mach info for device %s\n",
		      kdevname(dev));
	}

	device_port = bdev_port_lookup(dev);
	if (!MACH_PORT_VALID(device_port)) {
		panic("sg_ioctl: invalid device port for dev %s\n",
		      kdevname(dev));
	}

#ifdef	GENDISK_DEBUG
	if (gendisk_debug) {
		printk("sg_ioctl: dev %s cmd 0x%x\n", kdevname(dev), cmd);
	}
#endif	/* GENDISK_DEBUG */

	switch (cmd) {

	    case SG_SET_TIMEOUT:
		return -EOPNOTSUPP;
#if 0
		result = verify_area(VERIFY_READ, (const void *)arg, sizeof(int));
		if (result) return result;

		scsi_generics[dev].timeout=get_user((int *) arg);
		return 0;
#endif
	    case SG_GET_TIMEOUT:
		return 0;
#if 0
		return scsi_generics[dev].timeout;
#endif
	    case SCSI_IOCTL_SEND_COMMAND:
		/*
		  Allow SCSI_IOCTL_SEND_COMMAND without checking suser() since the
		  user already has read/write access to the generic device and so
		  can execute arbitrary SCSI commands.
		*/

		return (device_set_status(device_port, cmd, (dev_status_t) &arg, (sizeof(arg) +3) >>2));

		// return scsi_ioctl_send_command(inode, file, cmd, (void *) arg);
	    default:
		return gen_disk_ioctl(inode, file, cmd, arg);
	}
}

static int sg_open(struct inode * inode, struct file * filp)
{
    int dev=MINOR(inode->i_rdev);
    int flags=filp->f_flags;

    if (dev>15)
	return -ENXIO;

  /*
   * If we want exclusive access, then wait until the device is not
   * busy, and then set the flag to prevent anyone else from using it.
   */
    if (flags & O_EXCL)
    {
	while(sg_refs)
	{
	    if (flags & O_NONBLOCK)
		return -EBUSY;
	    /* @@@ FIX ME!!! @@@ */
#if 0
	    interruptible_sleep_on(&scsi_generics[dev].generic_wait);
	    if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
#endif
	}
	exclude[dev]=1;
    }
    else
        /*
         * Wait until nobody has an exclusive open on
         * this device.
         */
	while(exclude[dev])
	{
	    if (flags & O_NONBLOCK)
		return -EBUSY;
	    /* @@@ FIX ME!!! @@@ */
#if 0
	    interruptible_sleep_on(&scsi_generics[dev].generic_wait);
	    if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
#endif
	}

    /*
     * OK, we should have grabbed the device.  Mark the thing so
     * that other processes know that we have it, and initialize the
     * state variables to known values.
     */
    sg_refs[dev]++;

    return block_open(inode, filp);
}

static void sg_close(struct inode * inode, struct file * filp)
{
    int dev=MINOR(inode->i_rdev);
    sg_refs[dev]--;

    exclude[dev] = 0;
    block_release(inode, filp);

#if 0
    wake_up(&scsi_generics[dev].generic_wait);
#endif

    return;
}

#if 0
static char *sg_malloc(int size)
{
    if (size<=4096)
	return (char *) kmalloc(size, GFP_KERNEL);  /* @@@ FIX ME to 512 byte blocks (was scsi_malloc) @@@ */
#ifdef SG_BIG_BUFF
    if (size<=SG_BIG_BUFF)
    {
	while(big_inuse)
	{
	    interruptible_sleep_on(&big_wait);
	    if (current->signal & ~current->blocked)
		return NULL;
	}
	big_inuse=1;
	return big_buff;
    }
#endif   
    return NULL;
}

static void sg_free(char *buff,int size) 
{
#ifdef SG_BIG_BUFF
    if (buff==big_buff)
    {
	big_inuse=0;
	wake_up(&big_wait);
	return;
    }
#endif
    // scsi_free(buff,size);
    kfree(buff);
}

#endif

/*
 * Read back the results of a previous command.  We use the pending and
 * complete semaphores to tell us whether the buffer is available for us
 * and whether the command is actually done.
 */
static int sg_read(struct inode *inode,struct file *filp,char *buf,int count)
{
#if 0
    int dev=MINOR(inode->i_rdev);
    int i;
    unsigned long flags;
#endif

return block_read(inode, filp, buf, count);

#if 0
    struct scsi_generic *device=&scsi_generics[dev];
    if ((i=verify_area(VERIFY_WRITE,buf,count)))
	return i;


    /*
     * Wait until the command is actually done.
     */
    save_flags(flags);
    cli();

    while(!pending[dev] || !complete[dev])
    {
	if (filp->f_flags & O_NONBLOCK)
	{
	    restore_flags(flags);
	    return -EAGAIN;
	}
	interruptible_sleep_on(&device->read_wait);
	if (current->signal & ~current->blocked)
	{
	    restore_flags(flags);
	    return -ERESTARTSYS;
	}
    }
    restore_flags(flags);

    /*
     * Now copy the result back to the user buffer.
     */
    device->header.pack_len=device->header.reply_len;

    if (count>=sizeof(struct sg_header))
    {
	memcpy_tofs(buf,&device->header,sizeof(struct sg_header));
	buf+=sizeof(struct sg_header);
	if (count>device->header.pack_len)
	    count=device->header.pack_len;
	if (count > sizeof(struct sg_header)) {
	    memcpy_tofs(buf,device->buff,count-sizeof(struct sg_header));
	}
    }
    else
	count= device->header.result==0 ? 0 : -EIO;
    
    /*
     * Clean up, and release the device so that we can send another
     * command.
     */
    sg_free(device->buff,device->buff_len);
    device->buff = NULL;
    device->pending=0;
    wake_up(&device->write_wait);
    return count;
#endif
}

#if 0 /* @@@ WRITE ME!!! @@@ */
/*
 * This function is called by the interrupt handler when we
 * actually have a command that is complete.  Change the
 * flags to indicate that we have a result.
 */
static void sg_command_done(Scsi_Cmnd * SCpnt)
{
    int dev = MINOR(SCpnt->request.rq_dev);


    struct scsi_generic *device = &scsi_generics[dev];
    if (!device->pending)
    {
	printk("unexpected done for sg %d\n",dev);
	SCpnt->request.rq_status = RQ_INACTIVE;
	return;
    }

    /*
     * See if the command completed normally, or whether something went
     * wrong.
     */
    memcpy(device->header.sense_buffer, SCpnt->sense_buffer,
	   sizeof(SCpnt->sense_buffer));
    switch (host_byte(SCpnt->result)) {
    case DID_OK:
      device->header.result = 0;
      break;
    case DID_NO_CONNECT:
    case DID_BUS_BUSY:
    case DID_TIME_OUT: 
      device->header.result = EBUSY;
      break;
    case DID_BAD_TARGET: 
    case DID_ABORT: 
    case DID_PARITY: 
    case DID_RESET:
    case DID_BAD_INTR: 
      device->header.result = EIO;
      break;
    case DID_ERROR:
      /*
       * There really should be DID_UNDERRUN and DID_OVERRUN error values,
       * and a means for callers of scsi_do_cmd to indicate whether an
       * underrun or overrun should signal an error.  Until that can be
       * implemented, this kludge allows for returning useful error values
       * except in cases that return DID_ERROR that might be due to an
       * underrun.
       */
      if (SCpnt->sense_buffer[0] == 0 &&
	  status_byte(SCpnt->result) == GOOD)
	device->header.result = 0;
      else device->header.result = EIO;
      break;
    }

    /*
     * Now wake up the process that is waiting for the
     * result.
     */
    device->complete=1;
    SCpnt->request.rq_status = RQ_INACTIVE;
    wake_up(&scsi_generics[dev].read_wait);
}
#endif

static int sg_write(struct inode *inode,struct file *filp,const char *buf,int count)
{
#if 0
    int			  bsize,size,amt,i;
    unsigned char	  cmnd[MAX_COMMAND_SIZE];
    kdev_t		  devt = inode->i_rdev;
    int			  dev = MINOR(devt);
    struct scsi_generic   * device=&scsi_generics[dev];
    int			  input_size;
    unsigned char	  opcode;
    Scsi_Cmnd		* SCpnt;
#endif

return block_write(inode, filp, buf, count);

#if 0
    if ((i=verify_area(VERIFY_READ,buf,count)))
	return i;
    /*
     * The minimum scsi command length is 6 bytes.  If we get anything
     * less than this, it is clearly bogus.  
     */
    if (count<(sizeof(struct sg_header) + 6))
	return -EIO;

    /*
     * If we still have a result pending from a previous command,
     * wait until the result has been read by the user before sending
     * another command.
     */
    while(device->pending)
    {
	if (filp->f_flags & O_NONBLOCK)
	    return -EAGAIN;
#ifdef DEBUG
	printk("sg_write: sleeping on pending request\n");
#endif     
	interruptible_sleep_on(&device->write_wait);
	if (current->signal & ~current->blocked)
	    return -ERESTARTSYS;
    }

    /*
     * Mark the device flags for the new state.
     */
    device->pending=1;
    device->complete=0;
    memcpy_fromfs(&device->header,buf,sizeof(struct sg_header));

    device->header.pack_len=count;
    buf+=sizeof(struct sg_header);

    /*
     * Now we need to grab the command itself from the user's buffer.
     */
    opcode = get_user(buf);
    size=COMMAND_SIZE(opcode);
    if (opcode >= 0xc0 && device->header.twelve_byte) size = 12;

    /*
     * Determine buffer size.
     */
    input_size = device->header.pack_len - size;
    if( input_size > device->header.reply_len)
    {
        bsize = input_size;
    } else {
        bsize = device->header.reply_len;
    }
    
    /*
     * Don't include the command header itself in the size.
     */
    bsize-=sizeof(struct sg_header);
    input_size-=sizeof(struct sg_header);

    /*
     * Verify that the user has actually passed enough bytes for this command.
     */
    if( input_size < 0 )
    {
	device->pending=0;
        wake_up( &device->write_wait );
	return -EIO;
    }
    
    /*
     * Allocate a buffer that is large enough to hold the data
     * that has been requested.  Round up to an even number of sectors,
     * since scsi_malloc allocates in chunks of 512 bytes.
     */
    amt=bsize;
    if (!bsize)
	bsize++;
    bsize=(bsize+511) & ~511;

    /*
     * If we cannot allocate the buffer, report an error.
     */
    if ((bsize<0) || !(device->buff=sg_malloc(device->buff_len=bsize)))
    {
	device->pending=0;
	wake_up(&device->write_wait);
	return -ENOMEM;
    }

#ifdef DEBUG
    printk("allocating device\n");
#endif

    /*
     * Grab a device pointer for the device we want to talk to.  If we
     * don't want to block, just return with the appropriate message.
     */
    if (!(SCpnt=allocate_device(NULL,device->device, !(filp->f_flags & O_NONBLOCK))))
    {
	device->pending=0;
	wake_up(&device->write_wait);
	sg_free(device->buff,device->buff_len);
	device->buff = NULL;
	return -EAGAIN;
    } 
#ifdef DEBUG
    printk("device allocated\n");
#endif    

    SCpnt->request.rq_dev = devt;
    SCpnt->request.rq_status = RQ_ACTIVE;
    SCpnt->sense_buffer[0]=0;
    SCpnt->cmd_len = size;

    /*
     * Now copy the SCSI command from the user's address space.
     */
    memcpy_fromfs(cmnd,buf,size);
    buf+=size;

    /*
     * If we are writing data, copy the data we are writing.  The pack_len
     * field also includes the length of the header and the command,
     * so we need to subtract these off.
     */
    if (input_size > 0) memcpy_fromfs(device->buff, buf, input_size);
    
    /*
     * Set the LUN field in the command structure.
     */
    cmnd[1]= (cmnd[1] & 0x1f) | (device->device->lun<<5);

#ifdef DEBUG
    printk("do cmd\n");
#endif

    /*
     * Now pass the actual command down to the low-level driver.  We
     * do not do any more here - when the interrupt arrives, we will
     * then do the post-processing.
     */
    scsi_do_cmd (SCpnt,(void *) cmnd,
		 (void *) device->buff,amt,
		 sg_command_done,device->timeout,SG_DEFAULT_RETRIES);

#ifdef DEBUG
    printk("done cmd\n");
#endif               

    return count;
#endif
}

#if 0

/* What the heck does this do!?! */

static int sg_select(struct inode *inode, struct file *file, int sel_type, select_table * wait)
{
    int dev=MINOR(inode->i_rdev);
    int r = 0;
    struct scsi_generic *device=&scsi_generics[dev];

    if (sel_type == SEL_IN) {
        if(device->pending && device->complete)
        {
            r = 1;
    	} else {
	    select_wait(&scsi_generics[dev].read_wait, wait);
    	}
    }
    if (sel_type == SEL_OUT) {
        if(!device->pending){
            r = 1;
        }
        else
        {
	    select_wait(&scsi_generics[dev].write_wait, wait);
        }
    }

    return(r);
}

static int sg_attach(Scsi_Device * SDp)
{
    struct scsi_generic * gpnt;
    int i;
    
    if(sg_template.nr_dev >= sg_template.dev_max) 
    {
	SDp->attached--;
	return 1;
    }
    
    for(gpnt = scsi_generics, i=0; i<sg_template.dev_max; i++, gpnt++) 
	if(!gpnt->device) break;
    
    if(i >= sg_template.dev_max) panic ("scsi_devices corrupt (sg)");
    
    scsi_generics[i].device=SDp;
    scsi_generics[i].users=0;
    scsi_generics[i].generic_wait=NULL;
    scsi_generics[i].read_wait=NULL;
    scsi_generics[i].write_wait=NULL;
    scsi_generics[i].buff=NULL;
    scsi_generics[i].exclude=0;
    scsi_generics[i].pending=0;
    scsi_generics[i].timeout=SG_DEFAULT_TIMEOUT;
    sg_template.nr_dev++;
    return 0;
};

#endif


#if 0
static int sg_detect(Scsi_Device * SDp){

    return 1;

    switch (SDp->type) {
	case TYPE_DISK:
	case TYPE_MOD:
	case TYPE_ROM:
	case TYPE_WORM:
	case TYPE_TAPE: break;
	default: 
	printk("Detected scsi generic sg%c at scsi%d, channel %d, id %d, lun %d\n",
           'a'+sg_template.dev_noticed,
           SDp->host->host_no, SDp->channel, SDp->id, SDp->lun);
    }
    sg_template.dev_noticed++;
    return 1;
}

static void sg_detach(Scsi_Device * SDp)
{
    struct scsi_generic * gpnt;
    int i;
    
    for(gpnt = scsi_generics, i=0; i<sg_template.dev_max; i++, gpnt++) 
	if(gpnt->device == SDp) {
	    gpnt->device = NULL;
	    SDp->attached--;
	    sg_template.nr_dev--;
            /* 
             * avoid associated device /dev/sg? bying incremented 
             * each time module is inserted/removed , <dan@lectra.fr>
             */
            sg_template.dev_noticed--;
	    return;
	}
    return;
}

#endif

#if 0
#ifdef MODULE

int init_module(void) {
    sg_template.usage_count = &mod_use_count_;
    return scsi_register_module(MODULE_SCSI_DEV, &sg_template);
}

void cleanup_module( void) 
{
    scsi_unregister_module(MODULE_SCSI_DEV, &sg_template);
    unregister_chrdev(SCSI_GENERIC_MAJOR, "sg");
    
    if(scsi_generics != NULL) {
	scsi_init_free((char *) scsi_generics,
		       (sg_template.dev_noticed + SG_EXTRA_DEVS) 
		       * sizeof(struct scsi_generic));
    }
    sg_template.dev_max = 0;
#ifdef SG_BIG_BUFF
    if(big_buff != NULL)
	scsi_init_free(big_buff, SG_BIG_BUFF);
#endif
}
#endif /* MODULE */
#endif /* 0 */

/*
 * Overrides for Emacs so that we almost follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-brace-imaginary-offset: 0
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * c-continued-statement-offset: 4
 * c-continued-brace-offset: 0
 * indent-tabs-mode: nil
 * tab-width: 8
 * End:
 */

