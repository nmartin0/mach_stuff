/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <linux/autoconf.h>

#include <mach/mach_interface.h>
#include <mach/mach_ioctl.h>
#include <device/device_request.h>
#include <device/device.h>

#include <osfmach3/mach_init.h>
#include <osfmach3/device_reply_hdlr.h>
#include <osfmach3/assert.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/uniproc.h>
#include <osfmach3/block_dev.h>
#include <device/cdrom_status.h>

#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/cdrom.h>
#include <linux/mm.h>
#include <linux/malloc.h>

#define MAJOR_NR	SCSI_CDROM_MAJOR
#include <linux/blk.h>

#define MAX_MINORS	256
int sr_blocksizes[MAX_MINORS];
int sr_sizes[MAX_MINORS];
int sr_refs[MAX_MINORS];
int sr_mach_blocksizes[MAX_MINORS];

struct file_operations sr_fops;

extern void blkdev_get_reference(kdev_t );

extern void blkdev_release_reference(kdev_t );

#define SR_DEBUG(do_printk_debug, args)                                        \
MACRO_BEGIN                                                             \
        if (do_printk_debug) {                                                 \
                printk args;                                            \
        }                                                               \
MACRO_END

char sr_do_debug = 0;


int
cdrom_dev_to_name(
	kdev_t	dev,
	char	*name)
{
	unsigned int major, minor;

	major = MAJOR(dev);
	minor = MINOR(dev);

	if (major != SCSI_CDROM_MAJOR) {
		return -ENODEV;
	}
	if (osfmach3_blkdevs[major].name == NULL) {
		printk("cdrom_dev_to_name: no Mach info for dev %s\n",
		       kdevname(dev));
		panic("cdrom_dev_to_name");
	}

	if (minor >= MAX_MINORS) {
		return -ENODEV;
	}

	sprintf(name, "%s%d", osfmach3_blkdevs[major].name, minor);
	return 0;
}

int
sr_open(
	struct inode	*inode,
	struct file	*filp)
{

	return blkdev_fops.open(inode, filp);
}

void
sr_release(
	struct inode	*inode,
	struct file	*file)
{
	if (sr_refs[MINOR(inode->i_rdev)] == 1) {
		/*
		 * Releasing the last reference on the device.
		 * Clean buffers.
		 */
		sync_dev(inode->i_rdev);
		invalidate_inodes(inode->i_rdev);
		invalidate_buffers(inode->i_rdev);
	}

	blkdev_fops.release(inode, file);

	sr_sizes[MINOR(inode->i_rdev)] = 0;

	return;
}

int
sr_ioctl(
	struct inode	*inode,
	struct file	*file,
	unsigned int	cmd,
	unsigned long	arg)
{
	kdev_t		dev, real_dev;
	unsigned int	major, minor;
	mach_port_t	device_port;
	natural_t 	count, mach_count;
	register int	error = FALSE;
	unsigned int	mach_cmd  = 0;

	SR_DEBUG(sr_do_debug,
		    ("sr_ioctl: cmd= %x, arg= %lx \n", cmd, arg));

	real_dev = inode->i_rdev;
	dev = blkdev_get_alias(real_dev);

	major = MAJOR(dev);
	minor = MINOR(dev);

	if (major >= MAX_BLKDEV) {
		return -ENODEV;
	}

	if (minor >= MAX_MINORS) {
		return -ENXIO;
	}

	device_port = bdev_port_lookup(dev);
	if (device_port == MACH_PORT_NULL) {
	  SR_DEBUG(sr_do_debug,
		      ("sr_ioctl: device_port == MACH_PORT_NULL\n"));
		return ENXIO;	/* shouldn't happen */
	}

	ASSERT(MACH_PORT_VALID(device_port));

	if ((cmd & 0xffff) == DEV_GET_SIZE) {
		long status[DEV_GET_SIZE_COUNT];

		count = DEV_GET_SIZE_COUNT;

		error = verify_area(VERIFY_WRITE, (void *)arg, count*sizeof(long));

		if (error) {
		  SR_DEBUG(sr_do_debug,
			      ("verify_area error= %x\n", error));
		  return error;
		}

		error = device_get_status(device_port,
					  (cmd & 0xffff),
					  (int *) &status,
					  &count);
		if (error) {
		  MACH3_DEBUG(2, error,
			      (" device_get_status(DEV_GET_SIZE) error= %x\n",
			       error));
		  goto done;
		}

		memcpy_tofs((void *) arg, (const void *) status, 
			    count*sizeof(long)); 

		goto done;
	}

	switch (cmd & 0xffff) { /* Generic handling below the switch */
		/* Sun-compatible */
	    case CDROMPAUSE:
		mach_cmd = MACH_CDROMPAUSE;
		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMPAUSE\n"));
		break;
	    case CDROMRESUME:
		mach_cmd = MACH_CDROMRESUME;
		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMRESUME\n"));
		break;
	    case CDROMSTOP:
		mach_cmd = MACH_CDROMSTOP;
		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMSTOP\n"));
		break;
	    case CDROMSTART:
		mach_cmd = MACH_CDROMSTART;
		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMSTART\n"));
		break;
	    case CDROMEJECT:
		mach_cmd = MACH_CDROMEJECT;
		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMEJECT\n"));
		break;
	}

	if (mach_cmd) {
		long status;

		mach_count = ((sizeof(status) + 3) >> 2);

		status = 0;
		error = device_set_status(device_port, mach_cmd, 
				       (dev_status_t) &status, mach_count);
		if (error) {
		  MACH3_DEBUG(2, error,("device_set_status error= %x\n", error));
		  goto done;
		}
		goto done;
	}

	switch (cmd & 0xffff) {
	    case CDROMPLAYMSF:
	      {
		cd_msf_t status;
		struct cdrom_msf local_args;

		count = sizeof(local_args);		/* bytes */
		mach_count = (sizeof(status) + 3) >> 2;		/* ints */

		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMPLAYMSF\n"));
		
		error = verify_area(VERIFY_READ, (void *)arg, count);
		if (error) {
		  SR_DEBUG(sr_do_debug,("verify_area error= %x\n", error));
		  return error;
		}

		memcpy_fromfs((void *) &local_args, (const void *) arg, count);

		status.cdmsf_min0 = local_args.cdmsf_min0;
		status.cdmsf_sec0 = local_args.cdmsf_sec0;
		status.cdmsf_frame0 = local_args.cdmsf_frame0;
		status.cdmsf_min1 = local_args.cdmsf_min1;
		status.cdmsf_sec1 = local_args.cdmsf_sec1;
		status.cdmsf_frame1 = local_args.cdmsf_frame1;

		error = device_set_status(device_port, CDROMPLAYMSF, 
					  (dev_status_t) &status, mach_count);
		if (error) {
		  MACH3_DEBUG(2, error,(" device_set_status error= %x\n", error));
		  goto done;
		}
	      }
	    break;

	    case CDROMPLAYTRKIND:
	      {
		cd_ti_t status;
		struct cdrom_ti local_args;

		count = sizeof(local_args);		/* bytes */
		mach_count = (sizeof(status) + 3) >> 2;		/* ints */

		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMPLAYTRKIND\n"));
		
		error = verify_area(VERIFY_READ, (void *)arg, count);
		if (error) {
		  SR_DEBUG(sr_do_debug,("verify_area error= %x\n", error));
		  return error;
		}

		memcpy_fromfs((void *) &local_args, (const void *) arg, count);

		status.cdti_track0 = local_args.cdti_trk0;
		status.cdti_index0 = local_args.cdti_ind0;
		status.cdti_track1 = local_args.cdti_trk1;
		status.cdti_index1 = local_args.cdti_ind1;

		error = device_set_status(device_port, CDROMPLAYTRKIND, 
					  (dev_status_t) &status, mach_count);
		if (error) {
		  MACH3_DEBUG(2, error,(" device_set_status error= %x\n", error));
		  goto done;
		}
	      }
	    break;

	    case CDROMREADTOCHDR:
	      {
		cd_toc_header_t status;
		struct cdrom_tochdr local_args;

		count = sizeof(local_args);			/* bytes */
		mach_count = (sizeof(status) + 3) >> 2;		/* ints */

		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMREADTOCHDR\n"));

		error = verify_area(VERIFY_WRITE, (void *)arg, count);
		if (error) {
		  SR_DEBUG(sr_do_debug,("verify_area error= %x\n", error));
		  return error;
		}

		error = device_get_status(device_port, SCMD_READ_HEADER, 
					  (dev_status_t) &status,
					  &mach_count);
		if (error) {
		  MACH3_DEBUG(2, error,
			      (" device_get_status(SCMD_READ_HEADER) "
			       "error= %x\n", error));
		  goto done;
		}

		local_args.cdth_trk0 = status.first_track;
		local_args.cdth_trk1 = status.last_track;

		memcpy_tofs((void *) arg, (const void *) &local_args, count);
	      }
	    break;
	    case CDROMREADTOCENTRY:
	      {
		cd_toc_t *status;
		struct cdrom_tocentry local_args;
		unsigned int num_tracks = 1, req_track = 0;

		{   /* Get the number of tracks from the Toc Header */
			cd_toc_header_t lstatus;
			mach_msg_type_number_t lcount;

			lcount = ((sizeof(lstatus) + 3) >> 2);	/* ints */
			if (lcount == 0)
				lcount = 1;

			SR_DEBUG(sr_do_debug,("sr_ioctl:"
					   "CDROMREADTOCHDR FOR "
					   "CDROMREADTOCENTRY\n"));

			error = device_get_status(device_port, SCMD_READ_HEADER, 
						  (dev_status_t) &lstatus,
						  &lcount);
			if (error) {
			  MACH3_DEBUG(2, error,
				      (" device_get_status(SCMD_READ_HEADER) "
				       "error= %x\n", error));
			  goto done;
			}

			num_tracks = lstatus.last_track + 1;
			SR_DEBUG(sr_do_debug,
				    ("sr_ioctl: num_tracks= %d\n", num_tracks));
		}

		status = (cd_toc_t *) kmalloc(num_tracks * sizeof(cd_toc_t),
					      GFP_KERNEL);
		if (status == NULL) {
		  SR_DEBUG(sr_do_debug,("no memory for cd_toc_t status, ignoring\n"));
		  return -ENOMEM;
		}

		count = sizeof(local_args);		/* bytes */
		mach_count = (((num_tracks * sizeof(cd_toc_t)) + 3) >> 2); /*ints*/

		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMREADTOCENTRY\n"));
		error = verify_area(VERIFY_WRITE, (void *)arg, count);
		if (error) {
		  SR_DEBUG(sr_do_debug,("verify_area error= %x\n", error));
		  goto do_free;
		}

		memcpy_fromfs((void *) &local_args, (const void *) arg, count);

		if (local_args.cdte_format == CDROM_MSF)
			mach_cmd = MACH_SCMD_READ_TOC_MSF;
		else if (local_args.cdte_format == CDROM_LBA)
			mach_cmd = MACH_SCMD_READ_TOC_LBA;
		else { /* sanity */
			SR_DEBUG(sr_do_debug,
				    ("sr_ioctl: cmd= "
				     "CDROMREADTOCENTRY: Unknown format= %x\n",
				     local_args.cdte_format));
			goto do_free;
		}

		req_track = local_args.cdte_track;	/* requested track */
		SR_DEBUG(sr_do_debug,("req_track= %d\n", req_track));
		if (req_track > num_tracks)
			req_track = num_tracks;

		req_track--;	/* First track will be at offset 0 */

		error = device_get_status(device_port, mach_cmd, 
					  (dev_status_t) status,
					  &mach_count);
		if (error) {
		  MACH3_DEBUG(2, error,(" device_get_status(0x%x) error= %x\n",
					mach_cmd, error));
		  goto done;
		}

		local_args.cdte_ctrl = status[req_track].cdt_ctrl;
		local_args.cdte_adr = status[req_track].cdt_adr;
		local_args.cdte_track = status[req_track].cdt_track;
		if (status[req_track].cdt_type == MACH_CDROM_MSF) {
			local_args.cdte_format = CDROM_MSF;
			local_args.cdte_addr.msf.minute = 
				status[req_track].cdt_addr.msf.minute;
			local_args.cdte_addr.msf.second = 
				status[req_track].cdt_addr.msf.second;
			local_args.cdte_addr.msf.frame = 
				status[req_track].cdt_addr.msf.frame;
		}
		else if (status[req_track].cdt_type == MACH_CDROM_LBA) {
			local_args.cdte_format = CDROM_LBA;
			local_args.cdte_addr.lba = 
				status[req_track].cdt_addr.lba;
		}

		memcpy_tofs((void *) arg, (const void *) &local_args, count); 

do_free:
		kfree((void *) status);
	      }
		
	    break;

	    case CDROMVOLCTRL:
	      {
		cd_volctrl_t status;
		struct cdrom_volctrl local_args;

		count = sizeof(local_args);		/* bytes */
		mach_count = (sizeof(status) + 3) >> 2;		/* ints */

		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMVOLCTRL\n"));
		
		error = verify_area(VERIFY_READ, (void *)arg, count);
		if (error) {
		  SR_DEBUG(sr_do_debug,("verify_area error= %x\n", error));
		  return error;
		}

		memcpy_fromfs((void *) &local_args, (const void *) arg, count);

		status.channel0 = local_args.channel0;
		status.channel1 = local_args.channel1;
		status.channel2 = local_args.channel2;
		status.channel3 = local_args.channel3;

		error = device_set_status(device_port, CDROMVOLCTRL, 
					  (dev_status_t) &status, mach_count);
		if (error) {
		  if (error != D_INVALID_OPERATION)  /* Don't print this noise :-) */
   		     MACH3_DEBUG(2, error,(" device_set_status error= %x\n", error));
		  goto done;
		}
		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMVOLCTRL\n"));
	      }
	    break;

	    case CDROMSUBCHNL:
	      {
		cd_subchnl_t status;
		struct cdrom_subchnl local_args;

		count = sizeof(local_args);		/* bytes */
		mach_count = (sizeof(status) + 3) >> 2;		/* ints */

		SR_DEBUG(sr_do_debug,("sr_ioctl: CDROMSUBCHNL\n"));

		error = verify_area(VERIFY_WRITE, (void *)arg, count);
		if (error) {
		  SR_DEBUG(sr_do_debug,("verify_area error= %x\n", error));
		  return error;
		}

		memcpy_fromfs((void *) &local_args, (const void *) arg, count);

		if (local_args.cdsc_format == CDROM_MSF)
			mach_cmd = MACH_SCMD_READ_SUBCHNL_MSF;
		else if (local_args.cdsc_format == CDROM_LBA)
			mach_cmd = MACH_SCMD_READ_SUBCHNL_LBA;
		else { /* sanity */
			SR_DEBUG(sr_do_debug,
				    ("sr_ioctl: cmd= CDROMSUBCHNL: "
				     "Unknown format= %x\n",
				     local_args.cdsc_format));
			goto done;
		}
		
		error = device_get_status(device_port, mach_cmd, 
					  (dev_status_t) &status,
					  &mach_count);
		if (error) {
		  MACH3_DEBUG(2, error,
			      (" device_get_status(0x%x) error= %x\n",
			       mach_cmd, error));
		  goto done;
		}

		if (status.cdsc_type == MACH_CDROM_MSF) {
			local_args.cdsc_format = CDROM_MSF;
			local_args.cdsc_absaddr.msf.minute = 
			  status.cdsc_absolute_addr.msf.minute;
			local_args.cdsc_absaddr.msf.second =
			  status.cdsc_absolute_addr.msf.second;
			local_args.cdsc_absaddr.msf.frame =
			  status.cdsc_absolute_addr.msf.frame;

			local_args.cdsc_reladdr.msf.minute = 
			  status.cdsc_relative_addr.msf.minute;
			local_args.cdsc_reladdr.msf.second =
			  status.cdsc_relative_addr.msf.second;
			local_args.cdsc_reladdr.msf.frame =
			  status.cdsc_relative_addr.msf.frame;
		}
		else if (status.cdsc_type == MACH_CDROM_LBA) {
			local_args.cdsc_format = CDROM_LBA;
			local_args.cdsc_absaddr.lba = 
			  status.cdsc_absolute_addr.lba;

			local_args.cdsc_reladdr.lba = 
			  status.cdsc_relative_addr.lba;
		}

		local_args.cdsc_trk = status.cdsc_track;
		local_args.cdsc_ind = status.cdsc_index;
		local_args.cdsc_audiostatus = status.cdsc_audiostatus;
		local_args.cdsc_adr = status.cdsc_adr;
		local_args.cdsc_ctrl = status.cdsc_ctrl;

		memcpy_tofs((void *) arg, (const void *) &local_args, count); 
	      }
	    break;

	    case CDROMREADMODE2:
		return -EINVAL;

	    case CDROMREADMODE1:
		return -EINVAL;

	    case CDROMMULTISESSION:
		printk("sr_ioctl(dev=%s, cmd=CDROMMULTISESSION): "
		       "not implemented\n",
		       kdevname(dev));
		return -EOPNOTSUPP;

	    case BLKRASET:
		if (!suser()) return -EACCES;
		if (!inode->i_rdev) return -EINVAL;
		if (arg > 0xff) return -EINVAL;
		read_ahead[MAJOR(inode->i_rdev)] = arg;
		return 0;

	    RO_IOCTLS(dev,arg);
	    default:
		return blkdev_fops.ioctl(inode, file, cmd, arg);
	}

done:
	if (error) {
		SR_DEBUG(sr_do_debug,("Error 0x%x in sr_ioctl(dev=%s, cmd=0x%x) \n",
		       error, kdevname(inode->i_rdev), cmd));
		return -EINVAL;
	}
	else
		return 0;

}

int sr_init(void)
{
	static int	sr_registered = 0;
	int		minors_per_major;
	int		i;
	int		ret;

	if (sr_registered) {
		return 0;
	}

	sr_fops = blkdev_fops;
	sr_fops.ioctl = sr_ioctl;
	sr_fops.open = sr_open;
	sr_fops.release = sr_release;
	sr_fops.fsync = NULL;
	if (register_blkdev(MAJOR_NR, "sr", &sr_fops)) {
		printk("unable to get major %d for SCSI-CD\n",
		       MAJOR_NR);
		return -EBUSY;
	}

	for (minors_per_major = 0;
	     DEVICE_NR(minors_per_major) == 0 && minors_per_major < MAX_MINORS;
	     minors_per_major++);

	for (i = 0; i < MAX_MINORS; i++) {
		sr_blocksizes[i] = 2048;
		sr_sizes[i] = 0;
	}
	blksize_size[MAJOR_NR] = sr_blocksizes;
#if 0
	blk_size[MAJOR_NR] = sr_sizes;
#endif
	read_ahead[MAJOR_NR] = 32;

	ret = osfmach3_register_blkdev(MAJOR_NR,
				       "cdrom",
				       OSFMACH3_DEV_BSIZE,
				       sr_mach_blocksizes,
				       minors_per_major,
				       cdrom_dev_to_name,
				       NULL,
				       NULL);
	if (ret) {
		printk("sr_init: osfmach3_register_blkdev failed %d\n", ret);
		panic("sr_init: can't register Mach device");
	}

	for (i = 0; i < MAX_MINORS; i++) {
		sr_refs[i] = 0;
	}
	osfmach3_blkdev_refs[MAJOR_NR] = sr_refs;
	osfmach3_blkdev_alias[MAJOR_NR] = NULL;

	return 0;
}
