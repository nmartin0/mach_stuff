/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <osfmach3/block_dev.h>

#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/fs.h>

#define MAJOR_NR	HD_MAJOR
#include <linux/blk.h>

#define MAX_MINORS	256
int hd_blocksizes[MAX_MINORS];
int hd_refs[MAX_MINORS];	/* number of refs on Mach devices */
kdev_t hd_aliases[MAX_MINORS];	/* aliases for special minors */
int hd_mach_blocksizes[MAX_MINORS];

struct file_operations hd_fops;

int hd_init(void)
{
	int minors_per_major;
	int i;
	int ret;

	hd_fops = blkdev_fops;
	hd_fops.ioctl = gen_disk_ioctl;
	if (register_blkdev(MAJOR_NR, "hd", &hd_fops)) {
		printk("Unable to get major %d for harddisk\n", MAJOR_NR);
		return -EBUSY;
	}

	for (minors_per_major = 0;
	     DEVICE_NR(minors_per_major) == 0 && minors_per_major < MAX_MINORS;
	     minors_per_major++);

	for (i = 0; i < MAX_MINORS; i++) {
		hd_blocksizes[i] = 1024;
	}
	blksize_size[MAJOR_NR] = hd_blocksizes;
	/* no hardsect_size ... */
	read_ahead[MAJOR_NR] = 256;	/* 256 sector (128kB) read-ahead */

	ret = osfmach3_register_blkdev(MAJOR_NR,
				       "hd",
				       OSFMACH3_DEV_BSIZE,
				       hd_mach_blocksizes,
				       minors_per_major,
				       gen_disk_dev_to_name,
				       gen_disk_name_to_dev,
				       gen_disk_process_req_for_alias);
	if (ret) {
		printk("hd_init: osfmach3_register_blkdev failed %d\n", ret);
		panic("hd_init: can't register Mach device");
	}

	for (i = 0; i < MAX_MINORS; i++) {
		hd_refs[i] = 0;
		if (i % minors_per_major == 0) {
			/*
			 * Minor 0 of a unit is the whole unit.
			 * We don't have that concept in Mach, so just
			 * make an alias to the first real minor in the
			 * partition and hope it works...
			 */
			hd_aliases[i] = MKDEV(MAJOR_NR, i + 1);
		} else {
			hd_aliases[i] = 0;
		}
	}
	osfmach3_blkdev_refs[MAJOR_NR] = hd_refs;
	osfmach3_blkdev_alias[MAJOR_NR] = hd_aliases;

	return 0;
}
