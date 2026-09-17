#ifndef _SCSI_H_
#define _SCSI_H_

int scsi_disk_read(register struct iob *);
int scsi_disk_open(register struct iob *);

#endif /* _SCSI_H_ */