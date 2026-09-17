/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	pager_support.c,v $
 * Revision 2.2  91/09/12  16:41:59  bohman
 * 	Created.
 * 	[91/09/11  14:54:27  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2/pager_support.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mach/mach_types.h>
#include <mach/vm_param.h>
#include <device/device_types.h>
#include <device/io_req.h>
#include <mac2dev/macdisk.h>
#include <mach/mach_user_internal.h>
#include <mach/mach_port_internal.h>

/*
 * This module replaces the default_pager_setup() located in
 * kernel/boot_ufs/def_pager_setup.c.   It allows the default_pager to
 * use a spare apple partition for paging space.  It chooses the
 * first 'Apple_Scratch' partition it encounters starting with SCSI 0.
 * The default pager currently supports only a single paging space.
 */

static io_return_t read_page(device_t device,
                             vm_offset_t offset,
                             vm_size_t size,
                             vm_offset_t *addr,
                             vm_size_t *size_read)
{
  return device_read(device,
		     0,
		     (recnum_t) (offset >> MACDISK_REC_SIZE_LOG2),
		     size,
		     addr,
		     size_read);
}    

static io_return_t write_page(device_t device,
                              vm_offset_t offset,
                              vm_offset_t addr,
                              vm_size_t size,
                              vm_size_t *size_written)
{
  return device_write(device,
		      0,
		      (recnum_t) (offset >> MACDISK_REC_SIZE_LOG2),
		      addr,
		      size,
		      size_written);
}

void default_pager_setup(mach_port_t master_host_port,
                         mach_port_t master_device_port)
{
  register kern_return_t result;
  register int nparts;
  register unsigned int paging_size;
  device_t device;
  macdisk_part_info_t status_info;
  unsigned int status_count;
  char *type = PAGING_PARTITION_TYPE;
  unsigned char name[32];

  bcopy("macdisk0a", name, 10);
#define disk (name[7])
#define part (name[8])

  for (disk = '0'; disk < '7'; disk++) {
    part = 'a' + 1;
    device = MACH_PORT_NULL;
    if (device_open(master_device_port,
                    D_READ | D_WRITE,
                    name,
                    &device) != D_SUCCESS) continue;
    status_count = MACDISK_PART_INFO_COUNT;
    if (device_get_status(device,
                          MACDISK_PART_INFO,
                          &status_info, &status_count) != D_SUCCESS) {
      (void)device_close(device);
      (void)mach_port_deallocate(mach_task_self(), device);
      device = MACH_PORT_NULL;
      continue;
    }
    if (!strncmp(type, status_info.type, sizeof(status_info.type))) break;
    nparts = status_info.parts;
    (void)device_close(device);
    (void)mach_port_deallocate(mach_task_self(), device);
    device = MACH_PORT_NULL;
    for (part = 'a' + 2; part <= 'a' + nparts; part++) {
      if (device_open(master_device_port,
                      D_READ | D_WRITE,
                      name,
                      &device) != D_SUCCESS) continue;
      status_count = MACDISK_PART_INFO_COUNT;
      if (device_get_status(device,
                            MACDISK_PART_INFO,
                            &status_info, &status_count) != D_SUCCESS) {
        (void)device_close(device);
        (void)mach_port_deallocate(mach_task_self(), device);
        device = MACH_PORT_NULL;
        continue;
      }
      if (!strncmp(type, status_info.type, sizeof(status_info.type))) break;
      (void)device_close(device);
      (void)mach_port_deallocate(mach_task_self(), device);
      device = MACH_PORT_NULL;
    }
    if (device != MACH_PORT_NULL) break;
  }
  if (device != MACH_PORT_NULL) {
    printf("paging partition: %d blocks \"%s\" on \"%s\"\n",
	    status_info.length,
            type,
            name);
    paging_size = (status_info.length << MACDISK_REC_SIZE_LOG2);
  }
  else {
    printf("paging partition: \"%s\" not found on any disk\n", type);
    paging_size = 0;
  }
  create_default_partition(paging_size, read_page, write_page, device);
}
