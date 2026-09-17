/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */



#include <linux/autoconf.h>

#include <linux/major.h>
#include <linux/tty.h>
#include <hp_pa/grfioctl.h>

#include <mach/mach_interface.h>

#include <osfmach3/mach_init.h>
#include <osfmach3/device_utils.h>
#include <osfmach3/console.h>
#include <osfmach3/parent_server.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/char_dev.h>
#include <linux/mm.h>

#include <linux/kernel.h>

extern void chrdev_get_reference(kdev_t);
extern void chrdev_release_reference(kdev_t);

#define cdev_port_register(dev, port) \
	dev_to_object_register((dev), S_IFCHR, (char *) (port))
#define cdev_port_lookup(dev) \
	(mach_port_t) dev_to_object_lookup((dev), S_IFCHR)
#define cdev_port_deregister(dev) \
	dev_to_object_deregister((dev), S_IFCHR)

int grf_ioctl(
	struct inode	*inode,
	struct file	*file,
	unsigned int    cmd,
	unsigned long   arg)
{
	vm_size_t	size;
	struct grf_fbinfo grfinfo;
	kern_return_t		kr;
	int addrp;
	mach_port_t	device_port;

	device_port = cdev_port_lookup(inode->i_rdev);

	switch(cmd) {
	case GCMAP:
	case GCMAP_HPUX:
		size = (sizeof(grfinfo) + sizeof(int) - 1) / sizeof(int);
		kr = device_get_status(device_port,
				       GCDESCRIBE,
				       (dev_status_t) &grfinfo,
				       &size);
		if (kr != D_SUCCESS) 
			return -EIO;

		osfmach3_video_map_base = (vm_offset_t)grfinfo.regbase & 0xFC000000;
		osfmach3_video_map_size = 0x2000000;	/* XXX fixme */

		if(verify_area(VERIFY_READ, (long *) arg, sizeof (char *)))
			return -EIO;

		memcpy_fromfs(&addrp, (void *)arg, sizeof(char *));

		/* passed in address is ignored */
		kr = device_set_status(device_port, GCMAP, &addrp, 1);
		if (kr != KERN_SUCCESS)
			return -EIO;

		kr = device_map(device_port,
				VM_PROT_READ | VM_PROT_WRITE,
				0,
				osfmach3_video_map_size,
				&osfmach3_video_memory_object,
				0);
		if (kr != D_SUCCESS) 
			return -EIO;   

		kr = vm_map(current->osfmach3.task->mach_task_port,
			    &osfmach3_video_map_base,		/* address */
			    osfmach3_video_map_size,		/* size */
			    0,					/* mask */
			    FALSE,				/* anywhere */
			    osfmach3_video_memory_object,	/* memory object */
			    0,					/* offset */
			    FALSE,				/* copy */
			    VM_PROT_READ | VM_PROT_WRITE,	/* cur_protection */
			    VM_PROT_READ | VM_PROT_WRITE,	/* max_protection */
			    VM_INHERIT_SHARE);			/* inheritance */
		if (kr != KERN_SUCCESS) 
			return -EINVAL;

		memcpy_tofs((void*)arg, &osfmach3_video_map_base, sizeof(osfmach3_video_map_base));

		break;

	case GCUNMAP:
	case GCUNMAP_HPUX:
		(void) vm_deallocate(current->osfmach3.task->mach_task_port,
				     osfmach3_video_map_base,
				     osfmach3_video_map_size);

		osfmach3_video_map_base = 0;

		if(verify_area(VERIFY_READ, (long *) arg, sizeof (char *)))
			return -EIO;

		memcpy_fromfs(&addrp, (void *)arg, sizeof(char *));

		kr = device_set_status(device_port, GCUNMAP, &addrp, 1);
		if (kr != KERN_SUCCESS)
			return -EIO;

		break;
	case GCLOCK:
	case GCFASTLOCK:
	case GCLOCK_MINIMUM:
	case GCUNLOCK:
	case GCUNLOCK_MINIMUM:
	case GCSTATIC_CMAP:
	case GCVARIABLE_CMAP:
		/* These are currently no-ops */
		break;

	case GCDESCRIBE:
		size = (sizeof(grfinfo) + sizeof(int) - 1) / sizeof(int);

		if(verify_area(VERIFY_WRITE, (long *) arg, sizeof(grfinfo))) {
			return -EIO;
		}

		kr = device_get_status(device_port,
				       GCDESCRIBE,
				       (dev_status_t) &grfinfo,
				       &size);
		if (kr != D_SUCCESS) 
			return -EIO;
		
		memcpy_tofs((void*)arg, &grfinfo, sizeof(grfinfo));

		break;

	default:
		printk("grf_ioctl 0x%x not implemented\n", cmd);
		return -EINVAL;
	}

	return 0;
}

void grf_close(
	struct inode	*inode,
	struct file	*file)
{
	if (osfmach3_video_map_base) {
		mach_port_t	device_port;

		(void) vm_deallocate(current->osfmach3.task->mach_task_port,
				     osfmach3_video_map_base,
				     osfmach3_video_map_size);

		osfmach3_video_map_base = 0;
		
		device_port = cdev_port_lookup(inode->i_rdev);
		(void) device_set_status(device_port, GCUNMAP, &osfmach3_video_map_base, 1);
	}

	chrdev_release_reference(inode->i_rdev);
}

int grf_open(
	struct inode	*inode,
	struct file	*file)
{
	kern_return_t kr;
	int dev;
	mach_port_t device_port;

	dev = inode->i_rdev;
	device_port = cdev_port_lookup(dev);
	if (device_port != MACH_PORT_NULL) {
		chrdev_get_reference(dev);
		return 0;
	}

	kr = device_open(device_server_port,
			 MACH_PORT_NULL,
			 D_READ|D_WRITE,
			 server_security_token, "grf0", &device_port);
	if (kr != KERN_SUCCESS) {
		if (kr != D_NO_SUCH_DEVICE) {
			MACH3_DEBUG(1, kr,
				    ("grf_open(0x%x): "
				     "device_open(\"grf0\")",
				     inode->i_rdev));
		}
		return -ENODEV;
	}
	
	chrdev_get_reference(dev);
	cdev_port_register(dev, device_port);

	return 0;
}

int grf_write(struct inode * inode, struct file * file, const char * buf, int count)
{
	printk("grf_write not implemented !\n");
	return -ENODEV;
}

static struct file_operations grf_fops =
{
	NULL,		/* lseek */
	NULL,	        /* read */
	grf_write,      /* write */
	NULL,		/* readdir */
	NULL,		/* select */
	grf_ioctl,      /* ioctl */
	NULL,           /* mmap */
	grf_open,       /* open */
	grf_close,	/* close */
};

int grf_init(void)
{
	if (register_chrdev(GRF_MAJOR,"grf", &grf_fops)) {
		printk("grf: unable to get major %d\n", GRF_MAJOR);
		return -EIO;
	}
	return 0;
}

#if 0
unsigned long
con_type_init(unsigned long kmem_start, const char **display_desc)
{
        can_do_color = 1;

        video_mem_base = kmem_start;
        kmem_start += video_screen_size;
        video_mem_term = kmem_start;

        video_type = VIDEO_TYPE_HP;

        *display_desc = "HP GRF";

	return kmem_start;
}
#endif
