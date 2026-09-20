/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <linux/miscdevice.h>
#include <linux/fs.h>

extern struct file_operations mouse_fops;	/* generic mouse operations */

static struct miscdevice atixl_mouse = {
	ATIXL_BUSMOUSE_MINOR, "atixl", &mouse_fops
};

int atixl_busmouse_init(void)
{
	misc_register(&atixl_mouse);
	return 0;
}
