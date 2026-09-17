/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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

#ifdef KERNEL

/* MacMach driver for ramdisk device.
 *
 * Create by Zonnie L. Williamson at CMU, 1991
 *
 * Note that ramdisk #0 is the external image provided to the kernel by the
 * Mach INIT at boot time.  It is not allocated or deallocated.
 *
 * All "partitions" on a ramdisk are the same.  There is no disklabel.
 *
 * New ramdisks are created by the first device_write() done.  This should
 * write the LAST block in the ramdisk, because that is what it will be!
 *
 * Keep your ramdisk open!.  When it is closed it will be deallocated.
 */

#ifdef __GNUC__
#define GCCBUG /* external array references do not work right */
#endif

#include <mach/mach_types.h>

#include <device/device_types.h>
#include <device/io_req.h>

#include <vm/vm_kern.h>

#define ctlr(dev) (((dev) >> 5) & 0x7) /* like that in mhd.c */

#define RAMDISK_BLOCK_SIZE 512

#define MAX_RAMDISK 8

static struct rdisk {
  char *data;
  int size;
  int mode;
} rdisk[MAX_RAMDISK];

io_return_t ramdisk_open(int number, dev_mode_t mode, io_req_t ior)
{
  int n = ctlr(number);
  register struct rdisk *r;
  extern char *ramdisk_data;
  extern int ramdisk_size;

  if ((n < 0) || (n >= MAX_RAMDISK)) return D_NO_SUCH_DEVICE;

  r = &rdisk[n];

  /* ramdisk #0 can be preloaded */
  if (!n && (r->size = ramdisk_size)) {
#ifdef GCCBUG
    r->data = (char *)&ramdisk_data;
#else
    r->data = ramdisk_data;
#endif
  }

  /* if mode not specified, give warning and continue */
  if (!(r->mode = mode)) {
    printf("WARNING: ramdisk #%d opened with mode == 0\n", n);
    r->mode = D_READ | D_WRITE;
  }

  /* all done */
  return D_SUCCESS;

} /* ramdisk_open() */

io_return_t ramdisk_close(int number)
{
  register struct rdisk *r = &rdisk[ctlr(number)];
  extern int ramdisk_size;

  r->mode = 0;

  if (r->size && (ctlr(number) || !ramdisk_size))
    kmem_free(kernel_map, r->data, r->size);
  r->size = 0;
  r->data = 0;

  return D_SUCCESS;

} /* ramdisk_close() */

io_return_t ramdisk_read(int number, io_req_t ior)
{
  register struct rdisk *r = &rdisk[ctlr(number)];
  kern_return_t result;
  int offset;

  if (!(r->mode & D_READ)) return D_IO_ERROR;

  if (ior->io_count % RAMDISK_BLOCK_SIZE) return D_INVALID_SIZE;

  offset = ior->io_recnum * RAMDISK_BLOCK_SIZE;
  if ((offset + ior->io_count) > r->size) return D_INVALID_RECNUM;

  result = device_read_alloc(ior, (vm_size_t)ior->io_count);
  if (result != KERN_SUCCESS) return result;

  bcopy(&r->data[offset], ior->io_data, ior->io_count);

  return D_SUCCESS;

} /* ramdisk_read() */

io_return_t ramdisk_write(int number, io_req_t ior)
{
  register struct rdisk *r = &rdisk[ctlr(number)];
  kern_return_t result;
  int offset;
  boolean_t wait;

  if (!(r->mode & D_WRITE)) return D_READ_ONLY;

  if (ior->io_count % RAMDISK_BLOCK_SIZE) return D_INVALID_SIZE;

  if (!r->size) {
    r->size = (ior->io_recnum * RAMDISK_BLOCK_SIZE) + ior->io_count;
    r->data = 0;
    result = kmem_alloc(kernel_map, &r->data, r->size);
    if (result) return D_NO_MEMORY;
    bzero(r->data, r->size);
  }

  offset = ior->io_recnum * RAMDISK_BLOCK_SIZE;
  if ((offset + ior->io_count) > r->size) return D_INVALID_RECNUM;

  if ((ior->io_op & IO_INBAND) == 0) {
    result = device_write_get(ior, &wait);
    if (result != KERN_SUCCESS) return result;
  }

  bcopy(ior->io_data, &r->data[offset], ior->io_count);

  return D_SUCCESS;

} /* ramdisk_write() */

io_return_t ramdisk_info(int number, int code, int *data)
{
  switch (code) {
    case D_INFO_BLOCK_SIZE:
      *data = RAMDISK_BLOCK_SIZE;
      break;
    default:
      return D_IO_ERROR;
  }
  return D_SUCCESS;
}

#else /* KERNEL */

/* this program reads a ramdisk device and generates assembly code */

/* the generated code provides:
 *   extern char *ramdisk_data;
 *   extern int ramdisk_size;
 */

#include "/usr/include/stdio.h"

int generate_ramdisk(int f)
{
  char *buffer[512];
  int i, n, *d;

  printf(".globl _ramdisk_data\n");
  printf(".data\n");
  printf("	.even\n");
  printf("_ramdisk_data:\n");
  while (read(f, buffer, 512) == 512) {
    for (i = 0, d = (int *)buffer; i < 128; i++)
    printf("	.long 0x%X\n", d[i]);
    n += 512;
  }
  if (!n) printf("	.long 0\n");
  printf(".globl _ramdisk_size\n");
  printf("	.even\n");
  printf("_ramdisk_size:\n");
  printf("	.long %d\n", n);
  return n;
}

int generate_null_ramdisk(void)
{
  printf(".globl _ramdisk_data\n");
  printf(".data\n");
  printf("	.even\n");
  printf("_ramdisk_data:\n");
  printf("	.long 0\n");
  printf(".globl _ramdisk_size\n");
  printf("	.even\n");
  printf("_ramdisk_size:\n");
  printf("	.long 0\n");
  return 0;
}

main(int argc, char **argv)
{
  int f, n;

  if (argc != 2) {
    fprintf(stderr, "usage: xramdisk <ramdisk device>\n");
    exit(1);
  }
  if ((f = open(argv[1], 0)) == -1) n = generate_null_ramdisk();
  else n = generate_ramdisk(f);
  fprintf(stderr, "*** %d bytes inserted as ramdisk #0\n", n);
  exit(0);
}

#endif /* KERNEL */
