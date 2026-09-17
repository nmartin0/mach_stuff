#ifndef _ALERTMSG_H_
#define _ALERTMSG_H_

#define MSG_EMPTY				0
#define MSG_OUT_OF_MEMORY		1
#define MSG_FILE_OPEN			2
#define MSG_BAD_FS				3
#define MSG_NULL_PATH			4
#define MSG_NOT_DIR				5
#define MSG_EMPTY_DIR			6
#define MSG_NAME_NOT_FOUND		7
#define MSG_NO_PART_INFO		8
#define MSG_BAD_DPME_MAGIC		9
#define MSG_BAD_LABEL_MAGIC		10
#define MSG_NO_MACH_PART		11
#define MSG_SCSIGET_ERR			12
#define MSG_SCSISEL_ERR			13
#define MSG_SCSICMD_ERR			14
#define MSG_SCSIREAD_ERR		15
#define MSG_SCSIMSG_ERR			16
#define MSG_HDR_RD_ERR			17
#define MSG_BAD_HDR_MAGIC		18
#define MSG_FILE_TRUNC			19
#define MSG_ZONE_PLACE			20
#define MSG_RESOURCE_ERR		21
#define MSG_BAD_DRIVE_TYPE		22
#define MSG_BAD_FS_TYPE			23
/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#define MSG_BAD_DEVICE_MAJ		24
#define MSG_BAD_HFS_OPEN		25
#define MSG_BAD_PORT_ROMOPEN	26
#define MSG_BAD_PORT_RAMOPEN	27
#define MSG_BAD_PORT_SERRESET	28
#define MSG_BAD_PORT_SERHSHAKE	29
#define MSG_BAD_PORT_SERDTROFF	30
#define MSG_LOCALTALK_ACTIVE	31
#define MSG_NO_KERNEL_SPACE		32
#define MSG_32_BIT_KERNEL		33
#define MSG_NOT_THIS_MAC		34

void AlertMsg();

#endif /* _ALERTMSG_H_ */
