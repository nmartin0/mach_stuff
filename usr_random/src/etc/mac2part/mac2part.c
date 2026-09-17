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

/* mac2part.c -- create Mach partitions on Macintosh disks */

/* By Zonnie L. Williamson, CMU MacMach 1992 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/fs.h>
#include <sys/disklabel.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define PARTITION_TYPE_SIZE 32
#define PARTITION_NAME_SIZE 32

#define MACH_PARTITION_TYPE "Mach_UNIX_BSD4.3"
#define MACH_PARTITION_NAME "Mach Partition"

#define FREE_PARTITION_TYPE "Apple_Free"
#define FREE_PARTITION_NAME "Free Partition"

#define MAXDD 61
#define BLK_SIZE 512

int verbose = 0; /* if non-zero, output lots of noise */

struct ddm {
  unsigned short sbSig;
  unsigned short sbBlockSize;
  unsigned long sbBlkCount;
  unsigned short sbDevType;
  unsigned short sbDevID;
  unsigned long sbData;
  unsigned short sbDrvrCount;
  struct {
    unsigned long ddBlock;
    unsigned short ddSize;
    unsigned short ddType;
  } dd[MAXDD];
  char unused[6];
};

struct pme {
  unsigned short pmSig;
  unsigned short pmSigPad;
  unsigned long pmMapBlkCnt;
  unsigned long pmPyPartStart;
  unsigned long pmPartBlkCnt;
  unsigned char pmPartName[PARTITION_NAME_SIZE];
  unsigned char pmPartType[PARTITION_TYPE_SIZE];
  unsigned long pmLgDataStart;
  unsigned long pmDataCnt;
  unsigned long pmPartStatus;
  unsigned long pmLgBootStart;
  unsigned long pmBootSize;
  unsigned long pmBootLoad;
  unsigned long pmBootLoad2;
  unsigned long pmBootEntry;
  unsigned long pmBootEntry2;
  unsigned long pmBootCksum;
  unsigned char pmProcessor[16];
  unsigned char bootargs[128];
  char unused[248];
};

/* guess these parameters for Apple SCSI disks */
#define RPM      (3600)
#define NSECTORS (BBSIZE / BLK_SIZE)
#define NTRACKS  (NRPOS)
#define FSIZE    (1024)
#define FRAG     (8)

/* determine number of blocks in n mb */
#define MB(n) ((n * 1024 * 1024) / BLK_SIZE)

/* determine number of kb in n blocks (like /bin/du) */
#define KB(n) (howmany(dbtob(n), 1024))

/* sectors per cylinder */
#define SECPERCYL (NSECTORS * NTRACKS)

/* adjust size n to be a multiple of SECPERCYL */
#define SIZE(n) (SECPERCYL * (n / SECPERCYL))

/* UNIX DISK PARTITION STRATEGY
 * partition 'c' -- the whole disk
 * -OR-
 * partition 'a' -- 10MB root partition
 * partition 'g' -- the rest of the disk for /usr
 * -OR-
 * partition 'a' -- 10MB root partition
 * partition 'b' -- 90MB /usr partition
 * partition 'd' -- 50MB /usr/tmp partition
 * partition 'f' -- the rest of the disk for /usr/users
 * -OR-
 * partition 'a' -- 10MB root partition
 * partition 'b' -- 90MB /usr partition
 * partition 'd' -- 50MB /usr/tmp partition
 * partition 'e' -- 165MB /usr/src partition
 * partition 'f' -- the rest of the disk for /usr/users
 */

#define A_SIZE SIZE(MB(10))
#define B_SIZE SIZE(MB(90))
#define D_SIZE SIZE(MB(50))
#define E_SIZE SIZE(MB(165))

/* write block, return non-zero if error */
int write_block(int f, int n, char *block)
{
  long position;
  int result;

  if (verbose) fprintf(stderr, "write_block(%d, %d, 0x%08X)\n", f, n, block);
  position = n * 512;
  if (lseek(f, position, 0) != position) {
    if (verbose) fprintf(stderr, "lseek %d failed\n", position);
    return -1;
  }
  if ((result = write(f, block, 512)) != 512) {
    if (verbose) fprintf(stderr, "write returned %d\n", result);
    return -1;
  }
  return 0;
}

/* read block, return non-zero if error */
int read_block(int f, int n, char *block)
{
  long position;
  int result;

  if (verbose) fprintf(stderr, "read_block(%d, %d, 0x%08X)\n", f, n, block);
  position = n * 512;
  if (lseek(f, position, 0) != position) {
    if (verbose) fprintf(stderr, "lseek %d failed\n", position);
    return -1;
  }
  if ((result = read(f, block, 512)) != 512) {
    if (verbose) fprintf(stderr, "read returned %d\n", result);
    return -1;
  }
  return 0;
}

/* read ddm, return non-zero if error */
int read_ddm(int f, struct ddm *ddm)
{
  if (read_block(f, 0, (char *)ddm)) return -1;
  if (ddm->sbSig != 0x4552) {
    fprintf(stderr, "ddm.sbSig (0x%04X) != 0x4552\n", ddm->sbSig);
    return -1;
  }
  return 0;
}

/* write ddm, return non-zero if error */
int write_ddm(int f, struct ddm *ddm)
{
  return write_block(f, 0, (char *)ddm);
}

/* read a pme, return non-zero if error */
int read_pme(int f, int n, struct pme *pme)
{
  if (read_block(f, n + 1, (char *)pme)) return -1;
  if (pme->pmSig != 0x504D) {
    if (verbose) fprintf(stderr,
                         "pme[%d].pmSig (0x%04X) != 0x504D\n",
                         n,
                         pme->pmSig);
    return -1;
  }
  return 0;
}

/* write a pme, return non-zero if error */
int write_pme(int f, int n, struct pme *pme)
{
  return write_block(f, n + 1, (char *)pme);
}

/* open rdisk, given any disk; return non-zero if error */
int open_rdisk(char *disk, int read_only)
{
  struct stat sbuf;
  char rdisk[128];
  int fd, scsiid;

  if (isdigit(*disk)) {
    if ((scsiid = atoi(disk)) > 6) {
      fprintf(stderr, "Disk SCSI ID's are 0 through 6.\n");
      return -1;
    };
    sprintf(rdisk, "/dev/rdisk%da", scsiid);
  }
  else if (*disk != '/') sprintf(rdisk, "/dev/%s", disk);
  else strcpy(rdisk, disk);

  if (stat(rdisk, &sbuf)) {
    fprintf(stderr, "Can not stat \"%s\".\n", rdisk);
    return -1;
  }

  if (major(sbuf.st_rdev) == 7) {

    if ((sbuf.st_mode & S_IFMT) != S_IFCHR) {
      fprintf(stderr, "\"%s\" is not a character device.\n", rdisk);
      return -1;
    }

    sprintf(rdisk, "/dev/rdisk%da", minor(sbuf.st_rdev) / 32);

  }
  else {

    if (major(sbuf.st_rdev) != 6) {
      fprintf(stderr, "\"%s\" is not a disk device.\n", rdisk);
      return -1;
    }

    switch(sbuf.st_mode & S_IFMT) {
      case S_IFCHR:
        break;
      case S_IFBLK:
        sprintf(rdisk, "/dev/rdisk%da", minor(sbuf.st_rdev) / 8);
        break;
      default:
        fprintf(stderr,
                "\"%s\" is not a block or character device.\n",
                rdisk);
        return -1;
    }

  }

  if (verbose) fprintf(stderr, "rdisk = \"%s\"\n", rdisk);

  if (stat(rdisk, &sbuf)) {
    fprintf(stderr, "Can not stat \"%s\".\n", rdisk);
    return -1;
  }

  if (major(sbuf.st_rdev) != 6) {
    fprintf(stderr, "\"%s\" is not a disk device.\n", rdisk);
    return -1;
  }

  if ((sbuf.st_mode & S_IFMT) != S_IFCHR) {
    fprintf(stderr, "\"%s\" is not a character device.\n", rdisk);
    return -1;
  }

  if ((fd = open(rdisk, read_only ? O_RDONLY : O_RDWR)) < 0) {
    fprintf(stderr,
            "Can not open disk \"%s\" %s.\n",
            rdisk,
            read_only ? "read-only" : "read/write");
    return -1;
  }

  if (verbose) fprintf(stderr,
                       "Disk \"%s\" (\"%s\") is open.\n",
                       disk,
                       rdisk);

  /* all done, no error */
  return fd;

} /* open_rdisk() */

/* open macdisk, given any disk; return fd, -1 if error */
int open_macdisk(char *disk, int read_only)
{
  struct ddm ddm;
  struct stat sbuf;
  char macdisk[128];
  int fd, scsiid;

  if (isdigit(*disk)) {
    if ((scsiid = atoi(disk)) > 6) {
      fprintf(stderr, "Disk SCSI ID's are 0 through 6.\n");
      return -1;
    };
    sprintf(macdisk, "/dev/macdisk%d", scsiid);
  }
  else if (*disk != '/') sprintf(macdisk, "/dev/%s", disk);
  else strcpy(macdisk, disk);

  if (stat(macdisk, &sbuf)) {
    fprintf(stderr, "Can not stat \"%s\".\n", macdisk);
    return -1;
  }

  if (major(sbuf.st_rdev) != 7) {

    if (major(sbuf.st_rdev) != 6) {
      fprintf(stderr, "\"%s\" is not a disk device.\n", macdisk);
      return -1;
    }

    switch (sbuf.st_mode & S_IFMT) {
      case S_IFBLK:
      case S_IFCHR:
        sprintf(macdisk, "/dev/macdisk%d", minor(sbuf.st_rdev) / 8);
        break;
      default:
        fprintf(stderr,
                "\"%s\" is not a block or character device.\n",
                macdisk);
        return -1;
    }

  }

  if (verbose) fprintf(stderr, "macdisk = \"%s\"\n", macdisk);

  if (stat(macdisk, &sbuf)) {
    fprintf(stderr, "Can not stat \"%s\".\n", macdisk);
    return -1;
  }

  if (major(sbuf.st_rdev) != 7) {
    fprintf(stderr, "\"%s\" is not a macdisk device.\n", macdisk);
    return -1;
  }

  if ((sbuf.st_mode & S_IFMT) != S_IFCHR) {
    fprintf(stderr, "\"%s\" is not a character device.\n", macdisk);
    return -1;
  }

  if ((fd = open(macdisk, read_only ? O_RDONLY : O_RDWR)) < 0) {
    fprintf(stderr,
            "Can not open disk \"%s\" %s.\n",
            macdisk,
            read_only ? "read-only" : "read/write");
    return -1;
  }

  if (verbose) fprintf(stderr,
                       "Disk \"%s\" (\"%s\") is open.\n",
                       disk,
                       macdisk);

  /* read device descriptor map to verify the magic number */
  if (read_ddm(fd, &ddm)) {
    fprintf(stderr, "Can not read ddm on disk \"%s\".\n", disk);
    return -1;
  }
  if (verbose) fprintf(stderr, "The ddm is valid.\n");

  /* all done, no error */
  return fd;

} /* open_macdisk() */

/* display partition map for specified disk, return non-zero if error */
int display_partition_map(char *disk, int pme_index)
{
  struct ddm ddm;
  struct pme pme;
  unsigned short magic;
  int f, i, j, npartitions;
  char fsid[5], partname[33], parttype[33], processor[17];

  if ((f = open_macdisk(disk, 1)) < 0) return -1;

  printf("Apple partition on disk \"%s\".\n", disk);
  printf("See Inside Mac, V-577, V-579, IV-292 for partition documentation.\n");

  if (read_ddm(f, &ddm)) {
    fprintf(stderr, "Can not read ddm on disk \"%s\".\n", disk);
    return -1;
  }

  if (pme_index < 0) {
    printf("\ndriver descriptor map\n\n");
    printf("sbSig = 0x%X, sbBlockSize = %d (0x%X), sbBlkCount = %d (0x%X) (%dMB)\n",
      ddm.sbSig, ddm.sbBlockSize, ddm.sbBlockSize, ddm.sbBlkCount, ddm.sbBlkCount,
      (ddm.sbBlkCount * ddm.sbBlockSize) / 1000000);
    printf("sbDevType = %d (0x%X), sbDevID = %d (0x%X), sbData = %d (0x%X)\n",
      ddm.sbDevType, ddm.sbDevType, ddm.sbDevID, ddm.sbDevID, ddm.sbData, ddm.sbData);
    printf("sbDrvrCount = %d\n", ddm.sbDrvrCount);
    for (i = 0; i < ddm.sbDrvrCount; i++) {
      printf("\n%2d ddBlock = %d (0x%X), ddSize = %d, (0x%X), ddType = %d (0x%X)\n",
        i, ddm.dd[i].ddBlock, ddm.dd[i].ddBlock, ddm.dd[i].ddSize, ddm.dd[i].ddSize,
        ddm.dd[i].ddType, ddm.dd[i].ddType);
    }
  }

  if (read_block(f, 1, (char *)&pme)) {
    fprintf(stderr, "Can not read pme[0] on disk \"%s\".\n", disk);
    return -1;
  }
  if (pme.pmSig == 0x5453) {
    printf("Old partition map format found on disk \"%s\".\n", disk);
    return -1;
  }
  else if (pme.pmSig != 0x504D) {
    fprintf(stderr, "pme[0].pmSig (0x%04X) != 0x504D\n", pme.pmSig);
    return -1;
  }

  npartitions = pme.pmPartBlkCnt;
  if (verbose) fprintf(stderr, "There are %d partitions.\n", npartitions);

  for (i = 0; i < npartitions; i++) {

    if ((pme_index >= 0) && (i != pme_index)) continue;

    if (read_pme(f, i, &pme)) {
      if (verbose) fprintf(stderr,
                           "Can not read pme[%d] on disk \"%s\".\n",
                           i,
                           disk);
      continue;
    }

    printf("\npartition map entry [%d]\n\n", i);

    printf("pmSig = 0x%X, pmSigPad = 0x%X, pmMapBlkCnt = %d (0x%X)\n",
      pme.pmSig, pme.pmSigPad, pme.pmMapBlkCnt, pme.pmMapBlkCnt);

    printf("pmPyPartStart = %d (0x%X), pmPartBlkCnt = %d (0x%X) (%dMB)\n",
      pme.pmPyPartStart, pme.pmPyPartStart, pme.pmPartBlkCnt,
      pme.pmPartBlkCnt, (pme.pmPartBlkCnt * 512) / 1000000);

    strncpy(partname, pme.pmPartName, 32); partname[32] = 0;
    strncpy(parttype, pme.pmPartType, 32); parttype[32] = 0;

    printf("pmPartName = \"%s\", pmPartType =\"%s\"\n", partname, parttype);
    if (!strcmp(parttype, "Apple_MFS"))
      printf("  Flat file system (64K ROM)\n");
    else if (!strcmp(parttype, "Apple_HFS"))
      printf("  Hierarchical file system (128K ROM and later)\n");
    else if (!strcmp(parttype, "Apple_Unix_SVR2"))
      printf("  Partition for UNIX\n");
    else if (!strcmp(parttype, "Apple_partition_map"))
      printf("  Partition containing partition map\n");
    else if (!strcmp(parttype, "Apple_Driver"))
      printf("  Partition contains a name driver\n");
    else if (!strcmp(parttype, "Apple_PRODOS"))
      printf("  Partition designated for Apple IIGS\n");
    else if (!strcmp(parttype, FREE_PARTITION_TYPE))
      printf("  Partition unused and available for assignment\n");
    else if (!strcmp(parttype, "Apple_Scratch"))
      printf("  Partition empty and free for use\n");
    else if (!strcmp(parttype, MACH_PARTITION_TYPE))
      printf("  Partition for MACH BSD UNIX\n");
    else printf("  Unknown pmPartType\n");

    printf("pmLgDataStart = %d (0x%X), pmDataCnt = %d (0x%X)\n",
      pme.pmLgDataStart, pme.pmLgDataStart, pme.pmDataCnt, pme.pmDataCnt);

    printf("pmPartStatus = 0x%X\n", pme.pmPartStatus);
    printf("  %s\n", pme.pmPartStatus & 0x1 ?
      "valid partition map entry" : "not a valid partition map entry");
    printf("  partition %s\n", pme.pmPartStatus & 0x2 ?
      "allocated" : "available");
    printf("  partition %s in use\n", pme.pmPartStatus & 0x4 ?
      "is" : "is not");
    printf("  partition %s boot information\n", pme.pmPartStatus & 0x8 ?
      "contains valid" : "does not contain");
    printf("  partition %s reading\n", pme.pmPartStatus & 0x10 ?
      "allows" : "does not allow");
    printf("  partition %s writing\n", pme.pmPartStatus & 0x20 ?
      "allows" : "does not allow");
    printf("  boot code %s position independent\n", pme.pmPartStatus & 0x40 ?
      "is" : "is not");
    printf("  %s\n", pme.pmPartStatus & 0x80 ?
      "user bit set" : "user bit clear");

    printf("pmLgBootStart = %d (0x%X), pmBootSize = %d (0x%X)\n",
      pme.pmLgBootStart, pme.pmLgBootStart, pme.pmBootSize, pme.pmBootSize);

    printf("pmBootLoad = %d (0x%X), pmBootLoad2 = %d (0x%X)\n",
      pme.pmBootLoad, pme.pmBootLoad, pme.pmBootLoad2, pme.pmBootLoad2);

    printf("pmBootEntry = %d (0x%X), pmBootEntry2 = %d (0x%X)\n",
      pme.pmBootEntry, pme.pmBootEntry, pme.pmBootEntry2, pme.pmBootEntry2);

    strncpy(processor, pme.pmProcessor, 16); processor[16] = 0;
    printf("pmBootCksum = 0x%X, pmProcessor = \"%s\"\n",
      pme.pmBootCksum, processor);

  }

  /* all done, no error */
  return 0;

} /* display_partition_map() */

/* flush the disk driver, return non-zero if error */
int flush_driver(char *disk)
{
  struct ddm ddm;
  int f, i, j, start, end;
  char zero[512];

  if ((f = open_macdisk(disk, 0)) < 0) return -1;

  /* read device descriptor map, verify magic number */
  if (read_ddm(f, &ddm)) {
    fprintf(stderr, "Can not read ddm on disk \"%s\".\n", disk);
    return -1;
  }
  if (verbose) fprintf(stderr, "The ddm is valid.\n");

  bzero(zero, sizeof(zero));

  for (i = 0; i < ddm.sbDrvrCount; i++) {
    start = ddm.dd[i].ddBlock;
    end = start + ddm.dd[i].ddSize;
    for (j = start; j < end; j++) {
      if (write_block(f, j, zero)) {
        fprintf(stderr, "Can not write block %d of disk \"%s\".\n", j, disk);
        return -1;
      }
    }
    if (verbose) fprintf(stderr,
                         "Driver[%d], type %d zeroed.",
                         i,
                         ddm.dd[i].ddType);
    ddm.dd[i].ddBlock = 0;
    ddm.dd[i].ddSize = 0;
    ddm.dd[i].ddType = 0;
  }

  ddm.sbDrvrCount = 0;

  if (write_ddm(f, &ddm)) {
    fprintf(stderr, "Can not write ddm on disk \"%s\".\n", disk);
    return -1;
  }

  close(f);

  /* all done, no error */
  return 0;

} /* flush_driver() */

/* display the disklabel on a disk, return non-zero if error */
int display_disklabel(char *disk)
{
  int f, i;
  struct disklabel dl;

  if ((f = open_rdisk(disk, 1)) < 0) return -1;
  if(ioctl(f, DIOCGDINFO, &dl) < 0) {
    perror("DIOCGDINFO");
    fprintf(stderr, "Can not read disklabel on disk \"%s\".\n", disk);
    return -1;
  }

  for (i = 0; i < dl.d_npartitions; i++)
    if (dl.d_partitions[i].p_size)
      fprintf(stderr, "%7d kbytes on partition %c (sectors %d through %d)\n",
        KB(dl.d_partitions[i].p_size),
        i + 'a',
        dl.d_partitions[i].p_offset,
        dl.d_partitions[i].p_offset + dl.d_partitions[i].p_size - 1);

  close(f);

  /* all done, no error */
  return 0;

} /* display_disklabel() */

/* write unix disklabel on newly created Mach partition */
int create_disklabel(char *disk, int size)
{
  int f, used;
  struct disklabel dl;

  if ((f = open_rdisk(disk, 0)) < 0) return -1;

  /* adjust size to be multiple of secpercyl */
  if (verbose) fprintf(stderr, "disk size = %d\n", size);
  size = SIZE(size);
  if (verbose) fprintf(stderr, "adjusted disk size = %d\n", size);

  bzero(&dl, sizeof(dl));

  dl.d_checksum = 0;
  dl.d_magic = DISKMAGIC;
  dl.d_magic2 = DISKMAGIC;
  strcpy(dl.d_typename,  "mac2");
  dl.d_type = DTYPE_SCSI;
  dl.d_rpm = RPM;
  dl.d_secsize = BLK_SIZE;
  dl.d_bbsize = BBSIZE;
  dl.d_sbsize = SBSIZE;
  dl.d_nsectors = NSECTORS;
  dl.d_ntracks = NTRACKS;
  dl.d_npartitions = 7; /* a, b, c, d, e, f, g */
  dl.d_interleave = 1;
  dl.d_secpercyl = SECPERCYL;
  dl.d_ncylinders = size / SECPERCYL;
  dl.d_secperunit = size;

  /* 'a' section, standard root */
  used = dl.d_partitions[0].p_size = A_SIZE;
  dl.d_partitions[0].p_offset = 0;
  dl.d_partitions[0].p_fsize = FSIZE;
  dl.d_partitions[0].p_frag = FRAG;
  dl.d_partitions[0].p_fstype = FS_BSDFFS;

  /* 'b' section, /usr on big disk */
  if ((size - used) > B_SIZE) {
    dl.d_partitions[1].p_size = B_SIZE;
    dl.d_partitions[1].p_offset = used;
    used += dl.d_partitions[1].p_size;
    dl.d_partitions[1].p_fsize = FSIZE;
    dl.d_partitions[1].p_frag = FRAG;
    dl.d_partitions[1].p_fstype = FS_BSDFFS;
  }

  /* 'c' section, entire disk (Mach partition) */
  dl.d_partitions[2].p_size = size;
  dl.d_partitions[2].p_offset = 0;
  dl.d_partitions[2].p_fsize = FSIZE;
  dl.d_partitions[2].p_frag = FRAG;
  dl.d_partitions[2].p_fstype = FS_BSDFFS;

  /*'d' section, /usr/tmp on big disk */
  if ((size - used) > D_SIZE) {
    dl.d_partitions[3].p_size = D_SIZE;
    dl.d_partitions[3].p_offset = used;
    used += dl.d_partitions[3].p_size;
    dl.d_partitions[3].p_fsize = FSIZE;
    dl.d_partitions[3].p_frag = FRAG;
    dl.d_partitions[3].p_fstype = FS_BSDFFS;
  }

  /* 'e' section, /usr/src on big disk */
  if ((size - used) > E_SIZE) {
    dl.d_partitions[4].p_size = E_SIZE;
    dl.d_partitions[4].p_offset = used;
    used += dl.d_partitions[4].p_size;
    dl.d_partitions[4].p_fsize = FSIZE;
    dl.d_partitions[4].p_frag = FRAG;
    dl.d_partitions[4].p_fstype = FS_BSDFFS;
  }

  /* 'f' section, /usr/users on big disk */
  if ((size - used) > 0) {
    dl.d_partitions[5].p_size = size - used;
    dl.d_partitions[5].p_offset = used;
    used += dl.d_partitions[5].p_size;
    dl.d_partitions[5].p_fsize = FSIZE;
    dl.d_partitions[5].p_frag = FRAG;
    dl.d_partitions[5].p_fstype = FS_BSDFFS;
  }

  /* 'g' section, /usr on small disk */
  dl.d_partitions[6].p_size = size - dl.d_partitions[0].p_size;
  dl.d_partitions[6].p_offset = dl.d_partitions[0].p_size;
  dl.d_partitions[6].p_fsize = FSIZE;
  dl.d_partitions[6].p_frag = FRAG;
  dl.d_partitions[6].p_fstype = FS_BSDFFS;

  /* set checksum */
  dl.d_checksum = dkcksum(&dl);

  /* write disklabel to disk */
  if (ioctl(f, DIOCWDINFO, &dl) < 0) {
    perror("DIOCWDINFO");
    fprintf(stderr, "Can not write disklabel on \"%s\".\n", disk);
    return 1;
  }

  close(f);

  if (verbose) fprintf(stderr, "Disklabel created on \"%s\"\n", disk);

  /* all done */
  return 0;

} /* create_disklabel() */

/* find first partition of specified type, return pme index, -1 if error */
int find_partition(int f, char *type, struct pme *pme)
{
  int npartitions, i;
  char partition_type[PARTITION_TYPE_SIZE + 1];

  /* read pme[0] to get size of partition map */
  if (read_pme(f, 0, pme)) {
    fprintf(stderr, "Can not read pme[0].\n");
    return -1;
  }
  npartitions = pme->pmPartBlkCnt;
  if (verbose) fprintf(stderr, "There are %d partitions.\n", npartitions);

  for (i = 0; i < npartitions; i++) {

    if (read_pme(f, i, pme)) {
      if (verbose) fprintf(stderr, "Can not read pme[%d].\n", i);
      continue;
    }

    strncpy(partition_type, pme->pmPartType, PARTITION_TYPE_SIZE);
    partition_type[PARTITION_TYPE_SIZE] = 0;

    if (verbose) fprintf(stderr,
                         "pme[%d] type is \"%s\", %d blocks from %d to %d.\n",
                         i,
                         partition_type,
                         pme->pmPartBlkCnt,
                         pme->pmPyPartStart,
                         pme->pmPyPartStart + pme->pmPartBlkCnt - 1);

    if (!strcmp(partition_type, type)) return i;

  }

  return -1;

} /* find_partition() */

/* create a mach partition on a disk, return non-zero if error */
int create_mach_partition(char *disk, char *name)
{
  int f, i;
  char partition_name[PARTITION_NAME_SIZE + 1];
  struct pme pme;

  if ((f = open_macdisk(disk, 0)) < 0) return -1;

  if ((i = find_partition(f, MACH_PARTITION_TYPE, &pme)) < 0) {

    if ((i = find_partition(f, FREE_PARTITION_TYPE, &pme)) < 0) {
      fprintf(stderr,
              "There is no \"%s\" partition on disk \"%s\".\n",
              FREE_PARTITION_TYPE,
              disk);
      return -1;
    }

    fprintf(stderr,
            "Creating \"%s\" partition, pme[%d], on disk \"%s\".\n",
            MACH_PARTITION_TYPE,
            i,
            disk);

    bzero(&pme.pmPartName, PARTITION_NAME_SIZE);
    strncpy(pme.pmPartName,
            name ? name : MACH_PARTITION_NAME,
            PARTITION_NAME_SIZE);

    bzero(&pme.pmPartType, PARTITION_TYPE_SIZE);
    strcpy(pme.pmPartType, MACH_PARTITION_TYPE);

    pme.pmPartStatus = 0x33; /* valid, allocated, read, write */

    if (write_pme(f, i, &pme)) {
       fprintf(stderr, 
               "Can not write pme[%d] on disk \"%s\".\n",
               i,
               disk);
       return -1;
    }

    if (create_disklabel(disk, pme.pmPartBlkCnt)) return -1;

  }

  if ((i = find_partition(f, MACH_PARTITION_TYPE, &pme)) < 0) {
    fprintf(stderr, "There is no \"%s\" partition on disk \"%s\".\n",
            MACH_PARTITION_TYPE,
            disk);
    return -1;
  }

  if (name && strncmp(pme.pmPartName, name)) {
    bzero(&pme.pmPartName, PARTITION_NAME_SIZE);
    strncpy(pme.pmPartName,
            name ? name : MACH_PARTITION_NAME,
            PARTITION_NAME_SIZE);
    if (verbose) fprintf("Changing partition name.\n");
    if (write_pme(f, i, &pme)) {
       fprintf(stderr, 
               "Can not write pme[%d] on disk \"%s\".\n",
               i,
               disk);
       return -1;
    }
  }

  strncpy(partition_name, pme.pmPartName, PARTITION_NAME_SIZE);
  partition_name[PARTITION_NAME_SIZE] = 0;

  fprintf(stderr,
          "\"%s\" pme[%d], \"%s\", is %d blocks from %d to %d.\n",
          MACH_PARTITION_TYPE,
          i,
          partition_name,
          pme.pmPartBlkCnt,
          pme.pmPyPartStart,
          pme.pmPyPartStart + pme.pmPartBlkCnt - 1);

  fprintf(stderr,
          "Partition is %s.\n",
          (pme.pmPartStatus & 0x20) ? "unlocked" : "locked");

  if (display_disklabel(disk)) return -1;

  /* all done, no error */
  return 0;

} /* create_mach_partition() */

/* create a free partition from a mach partition on a disk */
/* return non-zero if error */
int create_free_partition(char *disk)
{
  int f, i, n;
  struct pme pme;
  char *zero, question[128];

  printf("WARNING: this will erase any exising \"%s\" partition!\n",
    MACH_PARTITION_TYPE); 

  sprintf(question, "Ok to create free partition on \"%s\"?", disk); 
  if (!solicit_yesno(question)) return -1;

  if ((f = open_macdisk(disk, 0)) < 0) return -1;

  if ((i = find_partition(f, MACH_PARTITION_TYPE, &pme)) < 0) {
    fprintf(stderr, "There was no \"%s\" partition on disk \"%s\".\n",
            MACH_PARTITION_TYPE,
            disk);
  }
  else {

    fprintf(stderr,
            "Creating \"%s\" partition, pme[%d] on disk \"%s\".\n",
            FREE_PARTITION_TYPE,
            i,
            disk);

    bzero(&pme.pmPartName, PARTITION_NAME_SIZE);
    strcpy(pme.pmPartName, FREE_PARTITION_NAME);

    bzero(&pme.pmPartType, PARTITION_TYPE_SIZE);
    strcpy(pme.pmPartType, FREE_PARTITION_TYPE);

    pme.pmPartStatus = 0x33; /* valid, allocated, read, write */

    if (write_pme(f, i, &pme)) {
       fprintf(stderr, 
               "Can not write pme[%d] on disk \"%s\".\n",
               i,
               disk);
       return -1;
    }

  }

  if ((i = find_partition(f, FREE_PARTITION_TYPE, &pme)) < 0) {
    fprintf(stderr, "There is no \"%s\" partition on disk \"%s\".\n",
            FREE_PARTITION_TYPE,
            disk);
    return -1;
  }

  fprintf(stderr,
          "\"%s\" pme[%d] is %d blocks from %d to %d.\n",
          FREE_PARTITION_TYPE,
          i,
          pme.pmPartBlkCnt,
          pme.pmPyPartStart,
          pme.pmPyPartStart + pme.pmPartBlkCnt - 1);

  fprintf(stderr, "Clearing free partition");
  n = 1024 * 1024;
  if (vm_allocate(mach_task_self(), &zero, n, 1))
    fprintf(stderr, " [vm_allocate %d failed]", n);
  else {
    bzero(zero, 1024 * 1024);
    n = pme.pmPyPartStart * 512;
    if (lseek(f, n, 0) != n) fprintf(stderr, " [lseek %d failed]", n);
    else {
      i = 0;
      while (i < pme.pmPartBlkCnt) {
        fprintf(stderr, ".");
        n = 1024 * 1024;
        if ((i + n / 512) < pme.pmPartBlkCnt) i += n / 512;
        else {
          n = (pme.pmPartBlkCnt - i) * 512;
          i == pme.pmPartBlkCnt;
        }
        if (write(f, zero, n) != n) {
          fprintf(stderr, " [write %d bytes failed]", n);
          break;
        }
      }
    }
  }
  fprintf(stderr, "\n");

  /* all done, no error */
  return 0;

} /* create_free_partition() */

/* set the read/write lock for the mach partition on a disk */
/* return non-zero if error */
int set_lock(char *disk, int lock)
{
  int f, i;
  struct pme pme;

  if ((f = open_macdisk(disk, 0)) < 0) return -1;

  if ((i = find_partition(f, MACH_PARTITION_TYPE, &pme)) < 0) {
    fprintf(stderr,
           "There is no \"%s\" partition on disk \"%s\".\n",
           MACH_PARTITION_TYPE,
           disk);
    return -1;
  }

  if (lock)
    pme.pmPartStatus = 0x13; /* valid, allocated, read only */
  else
    pme.pmPartStatus = 0x33; /* valid, allocated, read, write */

  if (write_pme(f, i, &pme)) {
    fprintf(stderr,
            "Can not write pme[%d] on disk \"%s\".\n",
            i,
            disk);
    return -1;
  }

  fprintf(stderr, "\"%s\" partition on disk \"%s\" is %slocked.\n",
          MACH_PARTITION_TYPE,
          disk,
          lock ? "" : "un");

  /* all done, no error */
  return 0;

} /* set_lock() */

main(int argc, char **argv)
{
  int lock = 0;

  argc--; argv++;

  if (argc && !strcmp(*argv, "-help")) goto usage;

  if (argc && !strcmp(*argv, "-v")) {
    verbose++;
    argc--; argv++;
  }

  if (!argc) goto usage;

  else if (!strcmp(*argv, "-display")) {
    argc--; argv++;
    if ((argc == 1) || (argc == 2))
      exit(display_partition_map(*argv, (argc > 1) ? atoi(argv[1]) : -1));
  }

  else if (!strcmp(*argv, "-flush_driver")) {
    argc--; argv++;
    if (argc == 1) exit(flush_driver(*argv) ? 1 : 0);
  }

  else if (!strcmp(*argv, "-free")) {
    argc--; argv++;
    if (argc == 1) exit(create_free_partition(*argv) ? 1 : 0);
  }

  else if (!strcmp(*argv, "-mach")) {
    argc--; argv++;
    if ((argc == 1) || (argc == 2))
      exit(create_mach_partition(*argv, (argc > 1) ? argv[1] : 0) ? 1 : 0);
  }

  else if (!strcmp(*argv, "-lock")) {
    argc--; argv++;
    if (argc == 1) exit(set_lock(*argv, 1) ? 1 : 0);
  }

  else if (!strcmp(*argv, "-unlock")) {
    argc--; argv++;
    if (argc == 1) exit(set_lock(*argv, 0) ? 1 : 0);
  }

usage:

  fprintf(stderr, "usage: mac2part [-v] -mach <disk> [ <name> ]\n");
  fprintf(stderr, "usage: mac2part [-v] -free <disk>\n");
  fprintf(stderr, "usage: mac2part [-v] -lock <disk>\n");
  fprintf(stderr, "usage: mac2part [-v] -unlock <disk>\n");
  fprintf(stderr, "usage: mac2part [-v] -flush_driver <disk>\n");
  fprintf(stderr, "usage: mac2part [-v] -display <disk> [ <pme index> ]\n");

  exit(1);

} /* main() */
