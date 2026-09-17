/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#ifndef _SCSI_H_
#define _SCSI_H_

int scsi_disk_read(register struct iob *);
int scsi_disk_open(register struct iob *);

#endif /* _SCSI_H_ */