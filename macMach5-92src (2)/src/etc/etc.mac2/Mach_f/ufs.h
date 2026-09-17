#ifndef _UFS_H_
#define _UFS_H_

#include "fs.h"
#include "inode.h"

#define E_BAD_FS -2
#define E_BAD_LABEL_MAGIC -3
#define E_BAD_DRIVE_TYPE -4
#define E_BAD_FS_TYPE -5
#define E_NOT_DIR -6
#define E_EMPTY_DIR -7
#define E_NAME_NOT_FOUND -8
#define E_NULL_PATH -9
#define E_SCSIGET_ERR -10
#define E_SCSISEL_ERR -11
#define E_SCSICMD_ERR -12
#define E_SCSIREAD_ERR -13
#define E_SCSIMSG_ERR -14
#define E_BAD_DEVICE_MAJ -15
#define E_NO_PART_INFO -16
#define E_BAD_DPME_MAGIC -17
#define E_NO_MACH_PART -18

typedef struct iob UFSparam;

int UFSopen(UFSparam *, char *, dev_t);
long UFSread(UFSparam *, char *, long);

#endif /* _UFS_H_ */