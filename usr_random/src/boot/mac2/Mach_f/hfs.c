/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/* read-only access to the Macintosh file system */

#include <Files.h>

#include "hfs.h"
#include "AlertMsg.h"

/* open HFS file using specified parameter block, return non-zero if error */
int HFSopen(param, Name)
ParamBlockRec *param;
char *Name;
{
	char PascalName[256];
	char *s1, *s2;
	int n;

	n = 0;
	s1 = PascalName + 1;
	s2 = Name;
	while (*s1++ = *s2++) n++;
	*PascalName = (char)n;

	param->fileParam.ioCompletion = (ProcPtr)0;
	param->fileParam.ioNamePtr = PascalName;
	param->fileParam.ioVRefNum = (short)0;
	param->fileParam.ioFVersNum = (char)0;
	param->ioParam.ioMisc = (char *)0;

	if (PBGetFInfo(param, 0)) return -1;

	param->ioParam.ioPermssn = fsRdPerm;

	if (PBOpen(param, 0)) return -1;

	param->ioParam.ioPosOffset = 0;

	return 0;

} /* HFSopen() */

/* read HFS file according to parameter block, return count */
int HFSread(param, Buffer, Count)
ParamBlockRec *param;
char *Buffer;
long Count;
{
	long Position;

	param->ioParam.ioBuffer = Buffer;
	param->ioParam.ioReqCount = Count;
	param->ioParam.ioPosMode = fsFromStart;
	Position = param->ioParam.ioPosOffset;

	if (PBRead(param, 0)) return -1;

	param->ioParam.ioPosOffset = Position + Count;

	return Count;

} /* HFSread() */
