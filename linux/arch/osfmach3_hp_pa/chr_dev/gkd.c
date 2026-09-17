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

#include <mach/mach_interface.h>

#include <osfmach3/server_thread.h>
#include <osfmach3/uniproc.h>
#include <osfmach3/mach_init.h>
#include <osfmach3/device_utils.h>
#include <osfmach3/console.h>
#include <osfmach3/parent_server.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/char_dev.h>
#include <linux/mm.h>
#include <linux/kernel.h>

/* Values for PS2_PORTSTAT first return byte */

#define PS2_KEYBD       0
#define PS2_MOUSE       1

/* Values for PS2_PORTSTAT second return byte */

#define INTERFACE_HAS_ITE       0x01
#define PORT_HAS_FIRST_KEYBD	0x02
#define PORT_HAS_FIRST_MOUSE	0x04

#define PS2_INDICATORS          0x80045001
#define PS2_IDENT               0x40045002
#define PS2_SCANCODE            0xc0045003
#define PS2_ENABLE              0x20005004
#define PS2_DISABLE             0x20005005
#define PS2_STREAMMODE          0x20005006
#define PS2_SAMPLERATE          0x80045007
#define PS2_RESET               0x40045008
#define PS2_RESOLUTION          0x80045009
#define PS2_PORTSTAT            0x40045012
#define PS2_ALL_TMAT_MKBRK      0x2000500c
#define PS2_RATEDELAY           0x80045011

#define GKD_BUF_SIZE	2048
struct gkd_queue_struct {
	unsigned long	head;
	unsigned long	tail;
	struct wait_queue	*proc_list;
	unsigned char		buf[GKD_BUF_SIZE];
};
struct gkd_queue_struct gkd_queue[16];

#define gkd_queue_empty(hqueue) (hqueue->head == hqueue->tail)

unsigned int get_from_gkd_queue(struct gkd_queue_struct *hqueue)
{
	unsigned int result;

	result = hqueue->buf[hqueue->tail];
	hqueue->tail = (hqueue->tail + 1) & (GKD_BUF_SIZE-1);
	return result;
}

extern void chrdev_get_reference(kdev_t);
extern void chrdev_release_reference(kdev_t);

#define cdev_port_register(dev, port) \
	dev_to_object_register((dev), S_IFCHR, (char *) (port))
#define cdev_port_lookup(dev) \
	(mach_port_t) dev_to_object_lookup((dev), S_IFCHR)
#define cdev_port_deregister(dev) \
	dev_to_object_deregister((dev), S_IFCHR)

void *
gkd_input_thread(void *gkd_handle)
{
	struct server_thread_priv_data	priv_data;
	kern_return_t			kr;
	io_buf_ptr_inband_t		inbuf;	/* 128 chars */
	mach_msg_type_number_t 		count;
	int				head, maxhead;
	int 				i;
	int dev = (int)gkd_handle;
	int minor, major;
	mach_port_t	device_port;
	struct gkd_queue_struct *hqueue;

	cthread_set_name(cthread_self(), "gkd thread");
	server_thread_set_priv_data(cthread_self(), &priv_data);

	device_port = cdev_port_lookup(dev);
	major = MAJOR(dev);
	minor = MINOR(dev);
	hqueue = &gkd_queue[minor];

	uniproc_enter();

	for (;;) {
		count = sizeof inbuf;
		server_thread_blocking(FALSE);
		kr = device_read_inband(device_port, 0, 0,
					sizeof inbuf, inbuf, &count);
		server_thread_unblocking(FALSE);
		if (kr != D_SUCCESS) {
			if (kr == D_DEVICE_DOWN ||
			    kr == MACH_SEND_INVALID_DEST ||
			    kr == MACH_SEND_INTERRUPTED ||
			    kr == MACH_RCV_INTERRUPTED) {
				/* device has been closed */
				uniproc_exit();
				cthread_detach(cthread_self());
				cthread_exit((void *) 0);
			}
			MACH3_DEBUG(0, kr,
				    ("gkd_input_thread: device_read_inband"));
			panic("gkd_input_thread: input error");
		}

		head = hqueue->head;
		maxhead = (hqueue->tail-1) & (GKD_BUF_SIZE-1);
		for (i = 0; i < count; i++) {
			hqueue->buf[head] = inbuf[i];
			if (head != maxhead) {
				head++;
				head &= GKD_BUF_SIZE-1;
			}
		}
		hqueue->head = head;
		wake_up_interruptible(&hqueue->proc_list);
	}
	/*NOTREACHED*/

}

int gkd_open(
	struct inode	*inode,
	struct file	*file)
{
	int dev;
	unsigned int	major, minor;
	char		name[16];
	mach_port_t	device_port;
	kern_return_t kr;
	struct gkd_queue_struct *hqueue;

	dev = inode->i_rdev;
	major = MAJOR(dev);
	minor = MINOR(dev);

	sprintf(name, "gkd%d", minor);

	device_port = cdev_port_lookup(dev);
	if (device_port != MACH_PORT_NULL) {
		chrdev_get_reference(dev);
		return 0;
	}

	kr = device_open(device_server_port,
			 MACH_PORT_NULL,
			 D_READ|D_WRITE,
			 server_security_token, name, &device_port);
	if (kr != KERN_SUCCESS) {
		if (kr != D_NO_SUCH_DEVICE) {
			MACH3_DEBUG(1, kr,
				    ("gkd_open(0x%x): "
				     "device_open(\"%s\")",
				     inode->i_rdev, name));
		}

		return -ENODEV;
	}
	
	chrdev_get_reference(dev);
	cdev_port_register(dev, device_port);

	hqueue = &gkd_queue[minor];
	hqueue->head = hqueue->tail = 0;
	hqueue->proc_list = NULL;
	server_thread_start(gkd_input_thread, (void *) dev);

	return 0;
}

int gkd_ioctl(
	struct inode	*inode,
	struct file	*file,
	unsigned int    cmd,
	unsigned long   arg)
{
	int dev;
	int minor;
	int size;
	kern_return_t	kr = 0;
	mach_port_t	device_port;
	unsigned char data[32];

	dev = inode->i_rdev;
	minor = MINOR(dev);
	device_port = cdev_port_lookup(dev);


	switch (cmd) {
	case PS2_PORTSTAT:
		if(verify_area(VERIFY_WRITE, (long *) arg, 2)) {
			return -EIO;
		}

		data[1] = INTERFACE_HAS_ITE;
		if(minor) {
			data[0] = PS2_MOUSE;
			data[1] |= PORT_HAS_FIRST_MOUSE;
		} else {
			data[0] = PS2_KEYBD;
			data[1] |= PORT_HAS_FIRST_KEYBD;
		}

		memcpy_tofs((void*)arg, data, 2);
		break;

	case PS2_IDENT:
		if(verify_area(VERIFY_WRITE, (long *) arg, 2)) {
			return -EIO;
		}

		if(minor) {
			data[0] = 0;
		}
		else {
			data[0] = 0xab;
			data[1] = 0x83;
		}
		
		memcpy_tofs((void*)arg, data, 2);
		break;

	case PS2_ENABLE:
	case PS2_ALL_TMAT_MKBRK:
	case PS2_DISABLE:
	case PS2_STREAMMODE:
	case PS2_RESET:
		size = 0;

		kr = device_set_status(device_port,
				       cmd,
				       (dev_status_t) data,
				       size);
		if (kr != D_SUCCESS) {
			return -EIO;
		}

		break;

	case PS2_INDICATORS:
	case PS2_SCANCODE:
	case PS2_RATEDELAY:
	case PS2_SAMPLERATE:
	case PS2_RESOLUTION:
		size = 1;

		if(verify_area(VERIFY_READ, (long *) arg, 1)) {
			return -EIO;
		}

		memcpy_fromfs(data, (void *)arg, 1);

		kr = device_set_status(device_port,
				       cmd,
				       (dev_status_t) data,
				       size);
		if (kr != D_SUCCESS) {
			return -EIO;
		}

		break;
	default:
		printk("!! gkd ioctl failed %x !! \n", cmd);

	}

	return 0;
}


int gkd_select(struct inode *inode,
	       struct file *file,
	       int sel_type,
	       select_table *wait)
{
	struct gkd_queue_struct *hqueue;
	mach_port_t	device_port;
	int dev;
	int major, minor;

	dev = inode->i_rdev;
	device_port = cdev_port_lookup(dev);
	major = MAJOR(dev);
	minor = MINOR(dev);
	hqueue = &gkd_queue[minor];

	if (sel_type != SEL_IN)
		return 0;
	if(! gkd_queue_empty(hqueue)) {
		return 1;
	}

	select_wait(&hqueue->proc_list, wait);
	return 0;
}

void gkd_close(
	struct inode	*inode,
	struct file	*file)
{
	if (cdev_port_lookup(inode->i_rdev) != MACH_PORT_NULL) 
		chrdev_release_reference(inode->i_rdev);
}

int gkd_read(
	      struct inode *inode,
	      struct file *file,
	      char *buffer,
	      int count)
{
	struct gkd_queue_struct *hqueue;
	mach_port_t	device_port;
	int dev;
	struct wait_queue wait = { current, NULL };
	int i = count;
	int minor, major;
	unsigned char c;

	dev = inode->i_rdev;
	device_port = cdev_port_lookup(dev);
	major = MAJOR(dev);
	minor = MINOR(dev);
	hqueue = &gkd_queue[minor];

	if (gkd_queue_empty(hqueue)) {
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		add_wait_queue(&hqueue->proc_list, &wait);
	    repeat:
		current->state = TASK_INTERRUPTIBLE;
		if (gkd_queue_empty(hqueue) &&
		    !(current->signal & ~current->blocked)) {
			schedule();
			goto repeat;
		}
		current->state = TASK_RUNNING;
		remove_wait_queue(&hqueue->proc_list, &wait);
	}
	while (i > 0 && !gkd_queue_empty(hqueue)) {
		c = get_from_gkd_queue(hqueue);
		put_fs_byte(c, buffer++);
		i--;
	}
	if (count - i) {
		inode->i_atime = CURRENT_TIME;
		return count - i;
	}
	if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
	return 0;
}


static struct file_operations gkd_fops =
{
	NULL,		/* lseek */
	gkd_read,	/* read */
	NULL,	        /* write */
	NULL,		/* readdir */
	gkd_select,	/* select */
	gkd_ioctl,      /* ioctl */
	NULL,           /* mmap */
	gkd_open,       /* open */
	gkd_close,	/* close */
};

int gkd_init(void)
{
	if (register_chrdev(PS2_MAJOR, "ps2", &gkd_fops)) {
		printk("kgd: unable to get major %d\n", PS2_MAJOR);
		return -EIO;
	}
	return 0;
}

