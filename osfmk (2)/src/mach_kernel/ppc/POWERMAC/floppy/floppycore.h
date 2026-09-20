/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * Copyright 1991-1998 by Apple Computer, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */

/************************************************************************
*
*	File:		FloppyCore.h
*
*	Contains:	All of the defines for the Floppy driver genaric routines
*				that can be used under any OS and with any floppy controller
*				hardware.
*
************************************************************************/

#ifndef __FLOPPYCORE_H__
#define __FLOPPYCORE_H__

#include "portdef.h"
/************************************************************************
*
* Function prototypes for routines in FloppyCore.c
*
************************************************************************/

OSStatus
InitializeDrive(int unit, short DriveNumber,
		LogicalAddress floppyControllerBaseAddr,
		LogicalAddress DMAReadControllerBaseAddr,
		LogicalAddress DMAWriteControllerBaseAddr,
		LogicalAddress trackCacheLogicalBase,
		PhysicalAddress trackCachePhysicalBase,
		unsigned long trackCacheBufferLen,
		DriveStatusType ** DriveStatusPtr);

void
 ScanForDisketteChange(void);

OSStatus
VerifyDiskFormat(DriveStatusType * DriveStatus);

OSStatus
FormatDisk(DriveStatusType * DriveStatus,
	   short formatIndex);

OSStatus
EjectDisk(DriveStatusType * DriveStatus);

void
 InitFormatTable(void);

void
 AvailableFormats(DriveStatusType * DriveStatus,
		  short *firstFormat,
		  short *lastFormat,
		  short *defaultFormat);

OSStatus
LookupFormatTable(DriveStatusType * DriveStatus,
		  short *maxFormats,
		  short *firstTableEntry,
		  short *lastTableEntry,
		  short *defaultFormatEntry,
		  formatReturnType * formatReturnPtr);

OSStatus
CheckDriveOnLine(DriveStatusType * DriveStatus);

OSStatus
ReadDiskTrackToCache(DriveStatusType * DriveStatus);

OSStatus
WriteCacheToDiskTrack(DriveStatusType * DriveStatus);

OSStatus
ReadSectorFromCacheMemory(DriveStatusType * DriveStatus);

OSStatus
WriteSectorToCacheMemory(DriveStatusType * DriveStatus);

OSStatus
ReadBlocks(DriveStatusType * DriveStatus,
	   long *transferuint_t);

OSStatus
WriteBlocks(DriveStatusType * DriveStatus,
	    long *transferuint_t);

OSStatus
CheckDriveNumber(int unit, short DriveNumber,
		 DriveStatusType ** DriveStatusPtr);

OSStatus
PowerDriveUp(DriveStatusType * DriveStatus);

void
 PowerDriveDown(DriveStatusType * DriveStatus,
		long PowerDownTimeout);

short
 GetDisketteFormatType(DriveStatusType * DriveStatus);

OSStatus
SetDisketteFormat(DriveStatusType * DriveStatus,
		  short formatIndex);

OSStatus
GetDisketteFormat(DriveStatusType * DriveStatus);

void
 BuildTrackInterleaveTable(DriveStatusType * DriveStatus,
			   unsigned char sectorsPerTrack);

void
 SetCacheAddresses(DriveStatusType * DriveStatus);

void
 SetSectorsPerTrack(DriveStatusType * DriveStatus);

void
 SetSectorAddressBlocksize(DriveStatusType * DriveStatus);

void
 GetSectorAddress(DriveStatusType * DriveStatus,
		  short blockNumber);

void
 DumpTrackCache(DriveStatusType * DriveStatus);

OSStatus
FlushTrackCache(DriveStatusType * DriveStatus);

boolean_t
TestCacheDirtyState(DriveStatusType * DriveStatus);

boolean_t
TestTrackInCache(DriveStatusType * DriveStatus);

void
 AssignTrackInCache(DriveStatusType * DriveStatus);

uint_t
FPYComputeCacheDMAAddress(DriveStatusType * DriveStatus,
			  unsigned char targetSide,
			  unsigned char targetSector,
			  long postDMAAlignmentCount,
			  TransferAddressType * returnedCacheAddress);

unsigned char *
 DenibblizeGCRData(unsigned char *sourceAddress,
		   unsigned char *destAddress,
		   unsigned short transferLength,
		   unsigned long *initalChecksum);

void
 DenibblizeGCRChecksum(unsigned char *sourceAddress,
		       unsigned long *realChecksum);

OSStatus
FPYDenibblizeGCRSector(DriveStatusType * DriveStatus,
		       unsigned char *denibblizeBufferPtr,
		       unsigned char *sectorBufferPtr);

unsigned char *
 NibblizeGCRData(unsigned char *sourceAddress,
		 unsigned char *destAddress,
		 unsigned short transferLength,
		 unsigned long *initalChecksum);

unsigned char *
 NibblizeGCRChecksum(unsigned char *destAddress,
		     unsigned long realChecksum);

OSStatus
FPYNibblizeGCRSector(DriveStatusType * DriveStatus,
		     unsigned char *nibblizeBufferPtr,
		     unsigned char *sectorBufferPtr);

OSStatus
RecalDrive(DriveStatusType * DriveStatus);

OSStatus
SeekDrive(DriveStatusType * DriveStatus);

OSStatus
FlushCacheAndSeek(DriveStatusType * DriveStatus);

#endif				// if already included.
