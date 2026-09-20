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
*	File:		FloppyCore.c
*
*	Contains:	Floppy driver genaric routines that can be used under any
*				OS and with any floppy controller hardware.
*
************************************************************************/

//#include <IOMemoryLists.h>

#include <types.h>
#include <sys/ioctl.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include <ppc/pmap.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppycore.h"
#include "floppysal.h"
#include "floppyhal.h"
#include <fd.h>

#if NFD > 0

#if (!MACH_DEBUG)
#define printf donone
#endif
/************************************************************************
*
* Global data for driver.
*
************************************************************************/

extern
SonyVarsType SonyVariables[];


/************************************************************************
*
*	FUNCTION : TestBitArray
*
************************************************************************/

void ResetBitArray(BitArrayByte * bitArray,
		   uint_t bitArraySize)
{

    short index;


    for (index = 0; index < bitArraySize; index += sizeof(uint_t))
	*(uint_t *) (bitArray + index) = 0;

}


/************************************************************************
*
*	FUNCTION : TestBitArray
*
************************************************************************/

boolean_t
TestBitArray(BitArrayByte * bitArray,
	     uint_t bitArraySize)
{

    short index;
    uint_t combinedBitsInArray;


    combinedBitsInArray = 0;

    for (index = 0; index < bitArraySize; index += sizeof(uint_t))
	combinedBitsInArray |= *(uint_t *) (bitArray + index);

    return combinedBitsInArray != 0;

}



/************************************************************************
*
* FUNCTION : InitializeDrive
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
		DriveStatusType ** DriveStatusPtr)
{

    short errorCode;
    DriveStatusType *DriveStatus;

#if MACH_DEBUG
    printf("InitializeDrive called. ");
#endif


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Get a pointer to the next drive status struct.
     */

    if ((errorCode = CheckDriveNumber(unit, DriveNumber,
				      DriveStatusPtr)) == noErr) {

	DriveStatus = *DriveStatusPtr;

	/*
	 * Create a hardware access lock resources for this drive.
	 */

	if ((errorCode = CreateOSHardwareLockResources(&DriveStatus->OSHardwareLockID)) == noErr) {

	    /*
	     * Create event resources for this drive for the HAL reset.
	     */

	    if ((errorCode = CreateOSEventResources(&DriveStatus->OSEventID)) == noErr) {

		/*
		 * Disable any other hardware accesses we're acting on a request.
		 */

		DriveStatus->OSHardwareLockID = EnterHardwareLockSection();

		/*
		 * Set the track cache address info that the caller may have defined.
		 */

		DriveStatus->tcBuffer.logicalAddress = trackCacheLogicalBase;
		DriveStatus->tcBuffer.physicalAddress = trackCachePhysicalBase;
		DriveStatus->tcBufferLen = trackCacheBufferLen;

		/*
		 * Reset the Floppy controller chip and preallocate any fixed track
		 * cache buffers if the caller didn't define one.
		 */
		if ((errorCode = HALReset(DriveStatus,
					  floppyControllerBaseAddr,
					  DMAReadControllerBaseAddr,
				 DMAWriteControllerBaseAddr)) == noErr) {

		    /*
		     * Say which kind of DRVR call we're executing.
		     */

		    DriveStatus->DRVROperation = DRVROpenCC;

		    /*
		     * Start out assuming that the drive isn't installed.
		     */

		    DriveStatus->installed = DRV_NOT_INSTALLED;

		    /*
		     * Now physically test to see if its there.
		     */

		    if (HALGetDriveType(DriveStatus)) {

			/*
			 * Forget any old track cache data we might have had for this drive.
			 */

			DumpTrackCache(DriveStatus);

			/*
			 * Remember that its installed but since we don't know
			 * where the drive head is, invalidate the current
			 * track number so that the first seek will recalibrate
			 * the drive head. Also, default the diskette presense
			 * to "none". If a diskette is really in the drive, the
			 * first scan of the VBL will re-post an insert event.
			 */

			DriveStatus->installed = DRV_IS_INSTALLED;
			DriveStatus->diskInPlace = NO_DISKETTE;
			DriveStatus->currentAddress.address.items.track = -1;
			DriveStatus->tagDataAddress = 0;

			/*
			 * Make sure that the motor state variable, and the
			 * drive, is in the proper state.
			 */

			PowerDriveDown(DriveStatus,
				       0);

		    }
		}
		/*
		 * Re-enable any other hardware accesses.
		 */

		ExitHardwareLockSection(DriveStatus->OSHardwareLockID);

	    }
	}
    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : ScanForDisketteChange
*
************************************************************************/

void ScanForDisketteChange(void)
{

    short driveIndex;
    DriveStatusType *DriveStatus;
    int unit;


    //printf( " SDC " );
    /*
     * Scan all of the installed drives and see if a diskette
     * has been inserted or ejected.
     */
    for (unit = 0; unit < NFD; unit++)
	for (driveIndex = 1; driveIndex <= maxDrvNum; driveIndex++) {

	    /*
	     * Make sure we have a valid drive number.
	     */

	    if (CheckDriveNumber(unit, driveIndex,
				 &DriveStatus) == noErr) {

		/*
		 * Skip any drives that aren't installed.
		 */

		if (DriveStatus->installed == DRV_IS_INSTALLED) {

		    /*
		     * If the driver is acting on a request, wait.
		     */

		    DriveStatus->OSHardwareLockID = EnterHardwareLockSection();

		    /*
		     * If the drive is waiting for power down, check to see if the
		     * timeout time has been reached. Otherwise, scan for diskette
		     * presences changes.
		     */

		    if (SonyVariables[unit].timeOut > 0) {

			/*
			 * Decrement the timer and see if the drive has timed out. If
			 * so, turn off the drive.
			 */

			SonyVariables[unit].timeOut--;

			if (SonyVariables[unit].timeOut <= 0) {

			    /*
			     * Power down the drive immediately.
			     */

			    PowerDriveDown(SonyVariables[unit].timeOutDrive,
					   0);

			}
		    } else {

			/*
			 * See if the next drive's "Diskette In Place" state variable
			 * agrees with the actual state of the "Diskette Presence" signal
			 * from the drive.
			 */

			if (DriveStatus->diskInPlace == NO_DISKETTE) {

			    /*
			     * Check to see if a diskette is really in the drive, if so update
			     * the state variable and post a disk inserted event. Also, get a
			     * first guess as to the diskette's physical format.
			     */

			    if (HALDiskettePresence(DriveStatus) == true) {

				/*
				 * Say that a diskette is present and get the media type.
				 */

				DriveStatus->diskInPlace = DISK_PRESENT;

				HALGetMediaType(DriveStatus);

				/*
				 * Post an event to the system that a diskette has been inserted.
				 * If we get an error posting the event, pretend the diskette
				 * isn't there.
				 */

				if (PostDisketteEvent(unit, DISK_PRESENT, driveIndex) != noErr)
				    DriveStatus->diskInPlace = NO_DISKETTE;

			    }
			} else {

			    /*
			     * See if the drive ejected a diskette, if so update the
			     * state variables and post a disk ejected event.
			     */

			    if (HALDiskettePresence(DriveStatus) == false) {

				// Unexpected disk removal.     

				if (DriveStatus->diskInPlace == DISK_AUTO_EJECTING) {

				    /*
				     * An auto ejected event.
				     */

				    PostDisketteEvent(unit, DISK_AUTO_EJECTING,
						      driveIndex);

				} else {

				    /*
				     * A manually ejected event.
				     */

				    PostDisketteEvent(unit, NO_DISKETTE,
						      driveIndex);

				}

				DriveStatus->diskInPlace = NO_DISKETTE;
				DriveStatus->formatType = GCR_DISKETTE;
				DriveStatus->disketteType = DISKETTE_1MEG;
				DriveStatus->writeProt = false;

			    }
			}

		    }

		    ExitHardwareLockSection(DriveStatus->OSHardwareLockID);

		}
	    }
	}

}

#if 0
/************************************************************************
*
* FUNCTION : VerifyDiskFormat
*
************************************************************************/

OSStatus
VerifyDiskFormat(DriveStatusType * DriveStatus)
{

    short errorCode;
    unsigned char trackCount;
    long index;
    TransferAddressType tempTransferAddress;
    ParamBlockRec dummyPBlock;
    DCtlEntry dummyDCE;

    /*
     * Powered up and select the drive immediately, wait the power up time
     * after the drive is powered up to make sure the it is up to speed.
     */

    if ((errorCode = PowerDriveUp(DriveStatus)) == noErr) {

	/*
	 * Dump any track cache data we might have had for this drive.
	 */

	DumpTrackCache(DriveStatus);

	/*
	 * Create a dummy track in the track cache with blank format data.
	 */

	*&tempTransferAddress = *&DriveStatus->tcBuffer;

	for (index = 0; index < trackCacheSize; index++) {

	    *tempTransferAddress.logicalAddress = 0xF6;

	    tempTransferAddress.logicalAddress++;

	}

	/*
	 * Setup the dummy DCE to start at the begining of the diskette. The
	 * verify-read operation will update this position.
	 */

	dummyDCE.dCtlPosition = 0;

	dummyPBlock.ioParam.ioPosMode = prmRdVerifyCode;
	dummyPBlock.ioParam.ioBuffer = (Ptr) DriveStatus->tcBuffer.logicalAddress;

	/*
	 * Start with track zero and step through and verify one track
	 * worth of the diskette at a time.
	 */

	(void) RecalDrive(DriveStatus);

	trackCount = 0;

	while ((errorCode == noErr) &&
	       (trackCount < DriveStatus->diskFormat.drvTrackMax)) {

	    /*
	     * Setup the dummy paramter block to verify one track (both sides)
	     * of the diskette at a time; be in verify mode; and use the track
	     * cache buffer as the data to verify against (this is done inside
	     * the loop mostly for GCR since it has variable sectors/track).
	     */

	    DriveStatus->wantedAddress.address.items.track = trackCount;

	    dummyPBlock.ioParam.ioReqCount = GetSectorsPerTrack(DriveStatus) *
		DriveStatus->diskFormat.logicalSectorSize *
		(DriveStatus->diskFormat.flagsNSides & FmtSidesMask);

	    /*
	     * Verify the next track. The verify-read will put the head
	     * over the correct track based on the byte position in the
	     * dummy DCE. Exit the track loop and return if an error
	     * occurs.
	     */

	    errorCode = ReadBlocks(DriveStatus,
				   &dummyPBlock,
				   &dummyDCE);

	    trackCount++;

	}

	/*
	 * We're finished with the request, so schedule the drive
	 * to be powered down in 6 VBL scan times (3 seconds).
	 */

	PowerDriveDown(DriveStatus,
		       6);

    }
    return errorCode;

}

#endif
/************************************************************************
*
* FUNCTION : FormatDisk
*
************************************************************************/

OSStatus
FormatDisk(DriveStatusType * DriveStatus,
	   short formatIndex)
{

    short errorCode;
    short firstFormat;
    short lastFormat;
    short currentFormat;


    /*
     * Can't format a write protected diskette.
     */

    if (DriveStatus->writeProt) {

	errorCode = RecordError(wPrErr);

    } else {

	/*
	 * Powered up and select the drive immediately, wait the power up time
	 * after the drive is powered up to make sure the it is up to speed.
	 */

	if ((errorCode = PowerDriveUp(DriveStatus)) == noErr) {

	    /*
	     * Dump any track cache data we might have had for this drive.
	     */

	    DumpTrackCache(DriveStatus);

	    /*
	     * Get the range of available formats, and remember the default
	     * format for this diskette.
	     */

	    AvailableFormats(DriveStatus,
			     &firstFormat,
			     &lastFormat,
			     &currentFormat);

	    SetDisketteFormat(DriveStatus,
			      currentFormat);

	    /*
	     * If a format index was specified, index into the available formats.
	     * Otherwise, use the current format.
	     */

	    if (formatIndex != 0) {

		SetDisketteFormat(DriveStatus,
				  firstFormat + formatIndex - 1);

	    }
	    /*
	     * Set the blank format data byte.
	     */

	    DriveStatus->formatByte = 0xF6;

	    /*
	     * Start with track zero and step through every track.
	     */

	    (void) RecalDrive(DriveStatus);

	    GetSectorAddress(DriveStatus,
			     0);

	    while ((errorCode == noErr) &&
		   (DriveStatus->wantedAddress.address.items.track < DriveStatus->diskFormat.drvTrackMax)) {

		/*
		 * Put the head over the correct track.
		 */

		if ((errorCode = SeekDrive(DriveStatus)) == noErr) {

		    /*
		     * Format the next track. Exit the track loop and return
		     * if an error occurs.
		     */

		    errorCode = HALFormatTrack(DriveStatus);

		    DriveStatus->wantedAddress.address.items.track++;

		}
	    }

	    /*
	     * We're finished with the request, so schedule the drive
	     * to be powered down in 6 VBL scan times (3 seconds).
	     */

	    PowerDriveDown(DriveStatus,
			   6);

	}
    }

    return errorCode;

}


/************************************************************************
*
* FUNCTION : EjectDisk
*
************************************************************************/

OSStatus
EjectDisk(DriveStatusType * DriveStatus)
{

    short errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * If we've seen a disk in this drive, park the head in a safe
     * place before we eject it. Try it twice; if we get errors
     * (SeekDrive will perform recalibrates if needed). Don't worry
     * about returning any seek errors at this point since it's
     * more important to get the diskette out of the drive.
     */

    if (DriveStatus->diskInPlace != NO_DISKETTE) {

	/*
	 * Make sure the drive is powered up so that we can do seeks.
	 */

	if ((errorCode = PowerDriveUp(DriveStatus)) == noErr) {

	    /*
	     * Flush any track cache data we might have had for this drive.
	     */

	    (void) FlushTrackCache(DriveStatus);

	    /*
	     * Make sure that Seek moves don't get optimized.
	     */

	    DumpTrackCache(DriveStatus);

	    /*
	     * Try to park.
	     */

	    DriveStatus->wantedAddress.address.items.track = 40;

	    if (SeekDrive(DriveStatus) != noErr) {

		DriveStatus->wantedAddress.address.items.track = 40;

		(void) SeekDrive(DriveStatus);

	    }
	    /*
	     * Do the eject of the diskette which will tell the VBL task
	     * that this is a auto (i.e. expected) eject.
	     */

	    DriveStatus->diskInPlace = DISK_AUTO_EJECTING;

	    errorCode = HALEjectDiskette(DriveStatus);

	}
	/*
	 * Power down the drive immediately
	 */

	PowerDriveDown(DriveStatus,
		       0);

    }
    return errorCode;

}


/************************************************************************
*
* FUNCTION : InitFormatTable
*
************************************************************************/

void InitFormatTable(void)
{
    int unit;
    formatTableEntry *nextFormatTableEntry;


    for (unit = 0; unit < NFD; unit++) {
	// 400K GCR format.

	nextFormatTableEntry = &SonyVariables[unit].formatTable[GCR400K];

	nextFormatTableEntry->drvBlockMax = 800;
	nextFormatTableEntry->flagsNSides = FmtTSSValid | 1;
	nextFormatTableEntry->sectorsPerTrack = 10;
	nextFormatTableEntry->drvTrackMax = 80;
	nextFormatTableEntry->logicalSectorSize = GCRSectorsSize;
	nextFormatTableEntry->DMASectorSize = GCRNibblizedSectorSize;
	nextFormatTableEntry->firstSectorInTrack = 0;
	nextFormatTableEntry->sectorInterleave = 2;
	nextFormatTableEntry->GCR = true;


	// 800K GCR format.

	nextFormatTableEntry = &SonyVariables[unit].formatTable[GCR800K];

	nextFormatTableEntry->drvBlockMax = 1600;
	nextFormatTableEntry->flagsNSides = FmtTSSValid | 2;
	nextFormatTableEntry->sectorsPerTrack = 10;
	nextFormatTableEntry->drvTrackMax = 80;
	nextFormatTableEntry->logicalSectorSize = GCRSectorsSize;
	nextFormatTableEntry->DMASectorSize = GCRNibblizedSectorSize;
	nextFormatTableEntry->firstSectorInTrack = 0;
	nextFormatTableEntry->sectorInterleave = 2;
	nextFormatTableEntry->GCR = true;


	// 720K MFM format.

	nextFormatTableEntry = &SonyVariables[unit].formatTable[MFM720K];

	nextFormatTableEntry->drvBlockMax = 1440;
	nextFormatTableEntry->flagsNSides = FmtTSSValid | 2;
	nextFormatTableEntry->sectorsPerTrack = 9;
	nextFormatTableEntry->drvTrackMax = 80;
	nextFormatTableEntry->logicalSectorSize = MFMSectorsSize;
	nextFormatTableEntry->DMASectorSize = MFMSectorsSize;
	nextFormatTableEntry->firstSectorInTrack = 1;
	nextFormatTableEntry->sectorInterleave = 1;
	nextFormatTableEntry->GCR = false;
	nextFormatTableEntry->gap1Length = 50;
	nextFormatTableEntry->gap2Length = 22;
	nextFormatTableEntry->gap3Length = 84;
	nextFormatTableEntry->gap4ALength = 80;
	nextFormatTableEntry->gap4BLength = 182;


	// 1440K MFM format.

	nextFormatTableEntry = &SonyVariables[unit].formatTable[MFM1440K];

	nextFormatTableEntry->drvBlockMax = 2880;
	nextFormatTableEntry->flagsNSides = FmtTSSValid | FmtDblDensity | 2;
	nextFormatTableEntry->sectorsPerTrack = 18;
	nextFormatTableEntry->drvTrackMax = 80;
	nextFormatTableEntry->logicalSectorSize = MFMSectorsSize;
	nextFormatTableEntry->DMASectorSize = MFMSectorsSize;
	nextFormatTableEntry->firstSectorInTrack = 1;
	nextFormatTableEntry->sectorInterleave = 1;
	nextFormatTableEntry->GCR = false;
	nextFormatTableEntry->gap1Length = 50;
	nextFormatTableEntry->gap2Length = 22;
	nextFormatTableEntry->gap3Length = 101;
	nextFormatTableEntry->gap4ALength = 80;
	nextFormatTableEntry->gap4BLength = 204;


	// 1680K MFM format.

	nextFormatTableEntry = &SonyVariables[unit].formatTable[MFM1680K];

	nextFormatTableEntry->drvBlockMax = 3360;
	nextFormatTableEntry->flagsNSides = FmtTSSValid | FmtDblDensity | 2;
	nextFormatTableEntry->sectorsPerTrack = 21;
	nextFormatTableEntry->drvTrackMax = 80;
	nextFormatTableEntry->logicalSectorSize = MFMSectorsSize;
	nextFormatTableEntry->DMASectorSize = MFMSectorsSize;
	nextFormatTableEntry->firstSectorInTrack = 1;
	nextFormatTableEntry->sectorInterleave = 2;
	nextFormatTableEntry->GCR = false;
	nextFormatTableEntry->gap1Length = 50;
	nextFormatTableEntry->gap2Length = 22;
	nextFormatTableEntry->gap3Length = 8;
	nextFormatTableEntry->gap4ALength = 80;
	nextFormatTableEntry->gap4BLength = 132;


	// 2880K MFM format.

	nextFormatTableEntry = &SonyVariables[unit].formatTable[MFM2880K];

	nextFormatTableEntry->drvBlockMax = 5760;
	nextFormatTableEntry->flagsNSides = FmtTSSValid | FmtDblDensity | 2;
	nextFormatTableEntry->sectorsPerTrack = 36;
	nextFormatTableEntry->drvTrackMax = 80;
	nextFormatTableEntry->logicalSectorSize = MFMSectorsSize;
	nextFormatTableEntry->DMASectorSize = MFMSectorsSize;
	nextFormatTableEntry->firstSectorInTrack = 1;
	nextFormatTableEntry->sectorInterleave = 1;
	nextFormatTableEntry->GCR = false;
	nextFormatTableEntry->gap1Length = 50;
	nextFormatTableEntry->gap2Length = 41;
	nextFormatTableEntry->gap3Length = 83;
	nextFormatTableEntry->gap4ALength = 80;
	//nextFormatTableEntry->gap4BLength        = 518;

    }
}


/************************************************************************
*
* FUNCTION : AvailableFormats
*
************************************************************************/

void AvailableFormats(DriveStatusType * DriveStatus,
		      short *firstFormat,
		      short *lastFormat,
		      short *defaultFormat)
{

#if (NEWAGE_STD_HARDWARE | INTEL_STD_HARDWARE)

    switch ((*defaultFormat = GetDisketteFormatType(DriveStatus))) {
    case MFM720K:
	*firstFormat = MFM720K;
	*lastFormat = MFM720K;
	break;

    case MFM1440K:
	*firstFormat = MFM1440K;
	*lastFormat = MFM1440K;
	break;
    }

    return;
#endif

    /*
       * Find out which and how many format entries to return. Default
       * the first table entry to be a 400K single sided GCR diskette.
       *
       * Assume that we've got a single sided diskette, but old 800K
       * drive formats.
     */

    *firstFormat = GCR400K;
    *defaultFormat = GCR400K;
    *lastFormat = GCR800K;


    /*
     * Test to see if the diskette is double sided and adjust the
     * current format.
     */

    if (DriveStatus->doubleSided == true)
	*defaultFormat = GCR800K;

    /*
     * Is this a FDHD drive?
     */

    if (DriveStatus->driveType == DRIVE_2MEG_FDHD) {

	/*
	 * A FDHD drive has three single density formats.
	 */

	*lastFormat = MFM720K;

	/*
	 * Is this a MFM diskette?
	 */

	if (DriveStatus->formatType == MFM_DISKETTE) {

	    /*
	     * Decode what kind of MFM diskette we have.
	     */

	    switch ((*defaultFormat = GetDisketteFormatType(DriveStatus))) {

		/*
		 * This must be a 720K diskette.
		 */

	    case MFM720K:
		*firstFormat = MFM720K;
		*lastFormat = MFM720K;
		break;

		/*
		 * This must be a 1440K HD diskette.
		 */

	    case MFM1440K:
		*firstFormat = MFM1440K;
		*lastFormat = MFM1680K;
		break;

		/*
		 * This must be a 2880K HD diskette.
		 */

	    case MFM2880K:
		*firstFormat = MFM2880K;
		*lastFormat = MFM2880K;
		break;

	    }

	}
    }
}


/************************************************************************
*
* FUNCTION : LookupFormatTable
*
************************************************************************/

OSStatus
LookupFormatTable(DriveStatusType * DriveStatus,
		  short *maxFormats,
		  short *firstTableEntry,
		  short *lastTableEntry,
		  short *defaultFormatEntry,
		  formatReturnType * formatReturnPtr)
{

    short errorCode;
    short tableEntryIndex;
    short maxFormatEntries;
    int unit = DriveStatus->unit;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * We can only return format info if the diskette has been read/written
     * so that we know what it's real format is.
     */

    if (DriveStatus->diskInPlace < DISK_WAS_ACCESSED) {

	errorCode = RecordError(offLinErr);

    } else {

	/*
	 * We must have a valid format request count.
	 */

	if (*maxFormats < 1) {

	    errorCode = RecordError(paramErr);

	} else {

	    /*
	     * Get the range of available formats.
	     */

	    AvailableFormats(DriveStatus,
			     firstTableEntry,
			     lastTableEntry,
			     defaultFormatEntry);

	    /*
	     * If the user wanted less formats than we have, return only the wanted
	     * count. Otherwise, tell the user how many we're returning.
	     */

	    maxFormatEntries = *lastTableEntry - *firstTableEntry + 1;

	    if (*maxFormats < maxFormatEntries) {

		maxFormatEntries = *maxFormats;

	    } else
		*maxFormats = maxFormatEntries;

	    /*
	     * Now transfer the format entries into the user's data space.
	     */

	    tableEntryIndex = *firstTableEntry;

	    while (tableEntryIndex <= *lastTableEntry) {

		formatReturnPtr->drvBlockMax = SonyVariables[unit].formatTable[tableEntryIndex].drvBlockMax;
		formatReturnPtr->sectorsPerTrack = SonyVariables[unit].formatTable[tableEntryIndex].sectorsPerTrack;
		formatReturnPtr->drvTrackMax = SonyVariables[unit].formatTable[tableEntryIndex].drvTrackMax;

		/*
		 * If we're transfering the current format, set the flag bit.
		 */

		if (tableEntryIndex == *defaultFormatEntry) {

		    formatReturnPtr->flagsNSides = SonyVariables[unit].formatTable[tableEntryIndex].flagsNSides | FmtCurrentFormat;

		} else
		    formatReturnPtr->flagsNSides = SonyVariables[unit].formatTable[tableEntryIndex].flagsNSides;

		formatReturnPtr++;

		tableEntryIndex++;

	    }

	}

    }

    return errorCode;

}


/************************************************************************
*
* FUNCTION : CheckDriveOnLine
*
************************************************************************/

OSStatus
CheckDriveOnLine(DriveStatusType * DriveStatus)
{

    register short errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Return an error for requests on drives that aren't there.
     */

    if (DriveStatus->installed != DRV_IS_INSTALLED) {

	errorCode = noDriveErr;

    } else {

	/*
	 * Make sure that the diskette hasn't been ejected.
	 */

	if (DriveStatus->diskInPlace == NO_DISKETTE) {

	    // For boot time.

	    if (HALDiskettePresence(DriveStatus) == true) {

		/*
		 * Say that a diskette is present and get the media type.
		 */

		DriveStatus->diskInPlace = DISK_PRESENT;

		HALGetMediaType(DriveStatus);

	    } else
		errorCode = offLinErr;

	}
    }

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : ReadDiskTrackToCache
*
************************************************************************/

OSStatus
ReadDiskTrackToCache(DriveStatusType * DriveStatus)
{

    register short errorCode;
    register short sectorCount;
    register short readRetries;
    register unsigned char sectorsPerTrack;
    register unsigned char firstSectorInTrack;
    diskAddressType saveDiskAddress;

    /*
     * Setup some register variables for speed.
     */

    sectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;
    firstSectorInTrack = DriveStatus->diskFormat.firstSectorInTrack;

    /*
     * Save the caller's wanted disk address while we use it to read
     * in the sectors in the track.
     */

    saveDiskAddress.address.blockAddress = DriveStatus->wantedAddress.address.blockAddress;
    /*
     * Capture the first sector address to fly bye and start reading
     * the track from where the head is currently are on the diskette.
     * This avoids taking a whole spin to always start reading from
     * the first sector of the track.
     */
    printf("core.c:ReadDiskTrackToCache:calling HALGetNextAddressID ");
    if ((errorCode = HALGetNextAddressID(DriveStatus)) == noErr) {

	/*
	 * If we're on a track that is different then the
	 * one we want, set the error.
	 */

	if (DriveStatus->wantedAddress.address.items.track != DriveStatus->currentAddress.address.items.track) {
	    printf("ReadDiskTrackToCache:seek error--wanttrack=%d,actual=%d ", DriveStatus->wantedAddress.address.items.track, DriveStatus->currentAddress.address.items.track);
	    errorCode = RecordError(seekErr);

	} else {

	    /*
	     * Prepare the CPU's cache for the read.
	     */

	    printf("core.c:ReadDiskTrackToCache:calling PrepareCPUCacheForDMARead ");
	    PrepareCPUCacheForDMARead();

	    /*
	     * Setup to start reading all of the sectors in the track from the
	     * current head location, or until we get an error. However, since
	     * the sector from the GetNextID has already passed by, the loop
	     * will start reading from the sector after the GetNextID one.
	     */

	    DriveStatus->wantedAddress.address.blockAddress = DriveStatus->currentAddress.address.blockAddress;

	    /*
	     * Setup the loop counter to read one track worth of sectors.
	     */

	    sectorCount = sectorsPerTrack;

	    readRetries = 0;

	    while ((sectorCount) &&
		   (errorCode == noErr)) {

		/*
		 * Don't increment to the next sector if we're retrying a
		 * sector read.
		 */

		if (readRetries == 0) {

		    /*
		     * Increment to the next sector to read.
		     */

		    DriveStatus->wantedAddress.address.items.sector =
			DriveStatus->sectorInterleaveLookup[DriveStatus->wantedAddress.address.items.sector];

		    /*
		     * Only read in the sector if the copy in the track cache
		     * hasn't been modified.
		     */
		}
#ifdef printf
#undef printf
#endif

//                                      printf("read %d sec ",DriveStatus->wantedAddress.address.items.sector); 
		if (!TestBit(DriveStatus->sectorDirty[DriveStatus->wantedAddress.address.items.side],
		      DriveStatus->wantedAddress.address.items.sector)) {

		    /*
		     * Set the cache transfer address for the next sector.
		     */

		    DriveStatus->transferAddress = DriveStatus->intraCacheSectorAddress[DriveStatus->wantedAddress.address.items.side][DriveStatus->wantedAddress.address.items.sector];
		    //                              printf("core.c:ReadDiskTrackToCache:calling HALReadSector ");   
		    errorCode = HALReadSector(DriveStatus);

		}
		//  naga }
		//else printf("skipping read %d sec ",DriveStatus->wantedAddress.address.items.sector);
#if (!MACH_DEBUG)
#define printf donone
#endif

		/*
		 * Read the sector data into it's cache location. Exit the
		 * loop if we get an error.
		 */

		if (errorCode == noErr) {

		    /*
		     * Reset the retry counter when no errors and go to
		     * the next sector.
		     */

		    readRetries = 0;

		    sectorCount -= 1;

		} else {

		    /*
		     * Did we get a Data or Address Mark checksum error?
		     */

		    //nagaif( ( errorCode == badCksmErr ) || ( errorCode == badDCksum ) )
		    {

			/*
			 * If we got three errors in a row, exit the loop and
			 * return the error code to the caller which will
			 * cause a recal.
			 */

			if (readRetries > 3) {

			    break;

			} else {

			    /*
			     * Keep track of how many errors we get in a row
			     * and allow the sector read to be tried again.
			     */

			    readRetries += 1;

			    errorCode = noErr;

			}

		    }

		}

	    }

	    /*
	     * Make sure that any data that was DMA'ed is flushed to
	     * main proccessor memory.
	     */

	    FlushDMAedDataFromCPUCache();

	}

    }
    /*
     * Restore the caller's wanted disk address.
     */

    DriveStatus->wantedAddress.address.blockAddress = saveDiskAddress.address.blockAddress;

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : WriteCacheToDiskTrack
*
************************************************************************/

OSStatus
WriteCacheToDiskTrack(DriveStatusType * DriveStatus)
{

    register short errorCode;
    register short sectorCount;
    register short writeRetries;
    register unsigned char sectorsPerTrack;
    register unsigned char firstSectorInTrack;
    diskAddressType saveDiskAddress;


    /*
     * Setup some register variables for speed.
     */

    sectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;
    firstSectorInTrack = DriveStatus->diskFormat.firstSectorInTrack;

    /*
     * Save the caller's wanted disk address while we use it to read
     * in the sectors in the track.
     */

    saveDiskAddress.address.blockAddress = DriveStatus->wantedAddress.address.blockAddress;

    /*
     * Capture the first sector address to fly bye and start writing
     * the track from where the head is currently are on the diskette.
     * This avoids taking a whole spin to always start writing from
     * the first sector of the track.
     */
    printf("wrt:cachetodisktrack:call getaddr ");
    if ((errorCode = HALGetNextAddressID(DriveStatus)) == noErr) {

	/*
	 * If we're on a track that is different then the
	 * one we want, set the error.
	 */

	if (DriveStatus->wantedAddress.address.items.track != DriveStatus->currentAddress.address.items.track) {

	    errorCode = RecordError(seekErr);

	} else {

	    /*
	     * Prepare the CPU's cache for the write.
	     */

	    PrepareCPUCacheForDMAWrite();

	    /*
	     * Setup to start writing all of the sectors in the track from the
	     * current head location, or until we get an error. However, since
	     * the sector from the GetNextID has already passed by, the loop
	     * will start writing from the sector after the GetNextID one.
	     */

	    DriveStatus->wantedAddress.address.blockAddress = DriveStatus->currentAddress.address.blockAddress;

	    /*
	     * Setup the loop counter to write on track worth of sectors.
	     */

	    sectorCount = sectorsPerTrack;

	    writeRetries = 0;

	    while ((sectorCount) &&
		   (errorCode == noErr)) {

		/*
		 * Don't increment to the next sector if we're retrying a
		 * sector write.
		 */

		if (writeRetries == 0) {

		    /*
		     * Increment to the next sector to write.
		     */

		    DriveStatus->wantedAddress.address.items.sector =
			DriveStatus->sectorInterleaveLookup[DriveStatus->wantedAddress.address.items.sector];

		    /*
		     * Only write those sectors that have been modified.
		     */
		    //naga added the following line }
		}
		if (TestBit(DriveStatus->sectorDirty[DriveStatus->wantedAddress.address.items.side],
		      DriveStatus->wantedAddress.address.items.sector)) {

		    /*
		     * Set the cache transfer address for the next sector.
		     */

		    DriveStatus->transferAddress = DriveStatus->intraCacheSectorAddress[DriveStatus->wantedAddress.address.items.side][DriveStatus->wantedAddress.address.items.sector];

		    /*
		     * Nibblize the GCR sector data starting at where the sector byte and tag data
		     * begins, into where the sector data begins.
		     */

		    if (DriveStatus->diskFormat.GCR) {

			errorCode = FPYNibblizeGCRSector(DriveStatus,
							 (unsigned char *) DriveStatus->transferAddress.logicalAddress,
							 (unsigned char *) DriveStatus->transferAddress.logicalAddress + (GCRSectorbyteSize + sizeof(TagDataType)));

		    }
		    if (errorCode == noErr)
			errorCode = HALWriteSector(DriveStatus);

		}
		//naga}
		else		//naga added the else clause for test
		 {
#ifdef printf
#undef printf
#endif
//   printf("sec %d !dirty ",DriveStatus->wantedAddress.address.items.sector);
#if (!MACH_DEBUG)
#define printf donone
#endif
		}
		/*
		 * Write the sector data from it's cache location. Exit the
		 * loop if we get an error.
		 */

		if (errorCode == noErr) {

		    /*
		     * Reset the dirty bit for this sector.
		     */

		    ClearBit(DriveStatus->sectorDirty[DriveStatus->wantedAddress.address.items.side],
			DriveStatus->wantedAddress.address.items.sector);

		    /*
		     * Reset the retry counter when no errors and go to
		     * the next sector.
		     */

		    writeRetries = 0;

		    sectorCount -= 1;

		} else {

		    /*
		     * Did we get a Data or Address Mark checksum error?
		     */

//naga commented out the following line to allow retry on timeout
		    //                                      if( ( errorCode == badCksmErr ) || ( errorCode == badDCksum ) )
		    {

			/*
			 * If we got three errors in a row, exit the loop and
			 * return the error code to the caller which will
			 * cause a recal.
			 */

			if (writeRetries > 3) {

			    break;

			} else {

			    /*
			     * Keep track of how many errors we get in a row
			     * and allow the sector write to be tried again.
			     */

			    writeRetries += 1;

			    errorCode = noErr;

			}

		    }

		}

	    }

	    /*
	     * Make sure that any data that was DMA'ed is flushed to
	     * main proccessor memory.
	     */

	    FlushDMAedDataFromCPUCache();

	}

    }
    /*
     * Restore the caller's wanted disk address.
     */

    DriveStatus->wantedAddress.address.blockAddress = saveDiskAddress.address.blockAddress;

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : ReadSectorFromCacheMemory
*
************************************************************************/

OSStatus
ReadSectorFromCacheMemory(DriveStatusType * DriveStatus)
{

    short errorCode;
    TransferAddressType cacheAddress;
    uint_t firstDifferentByte;
    boolean_t verifyCompareIsDifferent;

    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * If we want data from a different track and head then what we
     * currently have data for, read in the entire track and remember
     * which track and head is cached.
     */
    if (!TestTrackInCache(DriveStatus)) {

	/*
	 * Read in the track that contains the wanted sector, and if
	 * successful mark the track as the currently cached track for
	 * the drive.
	 */
	printf("floppycore.c:ReadSectorFromTheCache:calling ReadDiskTrackToCache ");
	if ((errorCode = ReadDiskTrackToCache(DriveStatus)) == noErr)
	    AssignTrackInCache(DriveStatus);

    }
    /*
     * If we already had the sector data cached, or we just successfully
     * read the track in, transfer the sector data to the user's data space.
     */

    if (errorCode == noErr) {

	/*
	 * Get the correct cache address depending on whether we have a GCR or MFM
	 * sector.
	 */

	cacheAddress = DriveStatus->intraCacheSectorAddress[DriveStatus->wantedAddress.address.items.side][DriveStatus->wantedAddress.address.items.sector];

	if (DriveStatus->diskFormat.GCR) {

	    /*
	     * Get the caching buffer address of the wanted GCR sector, backing up
	     * the GCR cache buffer address to start at the begining of the sector
	     * data.
	     */

	    cacheAddress.logicalAddress += (GCRSectorbyteSize + sizeof(TagDataType));
	    cacheAddress.physicalAddress += (GCRSectorbyteSize + sizeof(TagDataType));

	    /*
	     * Denibblize GCR sector data starting at where the sector byte and tag data
	     * begins, into where the sector data begins.
	     */

	    errorCode = FPYDenibblizeGCRSector(DriveStatus,
					       (unsigned char *) cacheAddress.logicalAddress - (GCRSectorbyteSize + sizeof(TagDataType)),
			  (unsigned char *) cacheAddress.logicalAddress);

	}
	/*
	 * If no errors thus far.
	 */

	if (errorCode == noErr) {

	    /*
	     * Test to see if we're doing a sector read, or a read-verify.
	     */

	    if (DriveStatus->commandModifier & prmRdVerifyCode) {

		/*
		 * Compare the sector data to the user's data space.
		 */

		if ((errorCode = MemListDescriptorDataCompareWithMemory(DriveStatus->transferSGList,
					     cacheAddress.logicalAddress,
			       DriveStatus->diskFormat.logicalSectorSize,
					       &verifyCompareIsDifferent,
					&firstDifferentByte)) != noErr) {

		    if (verifyCompareIsDifferent)
			errorCode = RecordError(dataVerErr);

		}
	    } else {

		/*
		 * Transfer the sector data to the user's data space.
		 */
		printf("floppycore.c:ReadSectorFromTheCache:copying the data srx=0x%x,tgt=0x%x ", (long) cacheAddress.logicalAddress, (long) DriveStatus->transferSGList);
//#define printf donone

		errorCode = MemListDescriptorDataCopyFromMemory(cacheAddress.logicalAddress,
					     DriveStatus->transferSGList,
			      DriveStatus->diskFormat.logicalSectorSize);

//naga added the following code
		DriveStatus->transferSGList = (char *) DriveStatus->transferSGList + DriveStatus->diskFormat.logicalSectorSize;		//update the user buffer

	    }

	}
    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : WriteSectorToCacheMemory
*
************************************************************************/

OSStatus
WriteSectorToCacheMemory(DriveStatusType * DriveStatus)
{

    short errorCode;
    TransferAddressType cacheAddress;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * If we want data from a different track and head then what we
     * currently have data for, read in the entire track first and
     * remember which track and head is cached.
     */

    if (!TestTrackInCache(DriveStatus)) {

	/*
	 * Mark the track as the currently cached track for
	 * the drive.
	 */

	AssignTrackInCache(DriveStatus);

    }
    /*
     * If we already had the wanted sector cached, or we just successfully
     * read the track in, transfer the sector data from the user's data space.
     */

    if (errorCode == noErr) {

	/*
	 * Get the correct cache address depending on whether we have a GCR or MFM
	 * sector.
	 */

	cacheAddress = DriveStatus->intraCacheSectorAddress[DriveStatus->wantedAddress.address.items.side][DriveStatus->wantedAddress.address.items.sector];

	if (DriveStatus->diskFormat.GCR) {

	    /*
	     * Get the caching buffer address of the wanted GCR sector, backing up
	     * the GCR cache buffer address to start at the begining of the sector
	     * data.
	     */

	    cacheAddress.logicalAddress += (GCRSectorbyteSize + sizeof(TagDataType));
	    cacheAddress.physicalAddress += (GCRSectorbyteSize + sizeof(TagDataType));

	    /*
	     * Since we're just going to blindly write the data to the track cache (of
	     * which may or may not have denibblized already in it), flag this sector
	     * as now being denibblized data.
	     */

	    SetBit(DriveStatus->sectorDenibblized[DriveStatus->wantedAddress.address.items.side],
		   DriveStatus->wantedAddress.address.items.sector);

	}
	/*
	 * If no errors thus far.
	 */

	if (errorCode == noErr) {

	    /*
	     * Transfer the sector data from the user's data space.
	     */

	    if ((errorCode = MemListDescriptorDataCopyToMemory(DriveStatus->transferSGList,
					     cacheAddress.logicalAddress,
		  DriveStatus->diskFormat.logicalSectorSize)) == noErr) {

		/*
		 * Mark the track cache sector as "write" dirty (i.e. needs flushing).
		 */
//naga added the following code
		DriveStatus->transferSGList = (char *) DriveStatus->transferSGList + DriveStatus->diskFormat.logicalSectorSize;		//update the user buffer

		SetBit(DriveStatus->sectorDirty[DriveStatus->wantedAddress.address.items.side],
		       DriveStatus->wantedAddress.address.items.sector);

	    }
	}
    }
    return errorCode;

}


/************************************************************************
*
* FUNCTION : ReadBlocks
*
************************************************************************/
//naga added this line
OSStatus
ReadBlocks(DriveStatusType * DriveStatus,
	   long *transferByteCount)
{

    register short errorCode;
    register long lastBlockNumber;
    short recalCount;


    /*
     * Bail if we don't have a diskette in the drive, or a drive installed.
     */
    printf("floppycore.c:ReadBlock:calling CheckDriveOnLine ");
    if ((errorCode = CheckDriveOnLine(DriveStatus)) == noErr) {

	/*
	 * Powered up and select the drive immediately. However, wait the
	 * power up time after the drive is powered up to make sure the it
	 * is up to speed.
	 */

	printf("floppycore.c:ReadBlock:calling powerdriveup ");
	if ((errorCode = PowerDriveUp(DriveStatus)) == noErr) {
	    printf("core.c:ReadBlock:PowerDriveUp returned SUCCESS ");
	    /*
	     * Compute the last block number in the request.
	     */
	    lastBlockNumber = DriveStatus->firstBlockNumber + DriveStatus->blockCount - 1;
	    printf("lastblk=%d,firstblk=%d,blkcount=%d\n", lastBlockNumber, DriveStatus->firstBlockNumber, DriveStatus->blockCount);
	    /*
	     * Make sure that the requested block number is not bigger than
	     * the diskette.
	     */

	    if (lastBlockNumber >= DriveStatus->diskFormat.drvBlockMax) {
		printf("last block number more than MAX ");
		errorCode = RecordError(seekErr);

	    } else {

		/*
		 * Transfer the required number of blocks, or return an error.
		 */

		errorCode = noErr;
		*transferByteCount = 0;
		printf("core.c:readblocks:firstblk=%d,lastblk=%d ", DriveStatus->firstBlockNumber, lastBlockNumber);
		while ((errorCode == noErr) &&
		    (DriveStatus->firstBlockNumber <= lastBlockNumber)) {

		    /*
		     * If we get an error, recalibrate the drive and retry the
		     * read a max of four times before we return the error.
		     */

		    recalCount = 4;
		    while (recalCount) {

			/*
			 * Calculate the next sector address from the block number.
			 */
			printf("floppycore.c:ReadBlock:calling Get Sectoraddr ");
//naga added this line
			GetSectorAddress(DriveStatus,
					 DriveStatus->firstBlockNumber);

			/*
			 * Check to see if we're going to move to different track
			 * and if so, flush any cached write data to diskette and
			 * seek to the requested track.
			 */
			printf("core.c:ReadBlocks:call FlushCacheAndSeek ");
			if ((errorCode = FlushCacheAndSeek(DriveStatus)) == noErr) {

			    /*
			     * Read the next sector data into the user's buffer space.
			     * If no errors occur, keep track of the number of bytes
			     * transfered, where to put them, and abort the recal
			     * loop.
			     */

			    printf("floppycore.c:ReadBlock:calling Readsector from cache ");
			    if ((errorCode = ReadSectorFromCacheMemory(DriveStatus)) == noErr) {

				*transferByteCount += DriveStatus->diskFormat.logicalSectorSize;

				DriveStatus->firstBlockNumber++;

				break;

			    }
			}
			/*
			 * If there was an error, recalibrate the drive and keep
			 * track of the number of times we got an error.
			 */

			if (errorCode != noErr) {
			    printf("Found error,recalibrating ");
			    (void) RecalDrive(DriveStatus);

			    DriveStatus->driveErrs++;

			    recalCount--;

			}
		    }

		}

	    }

	    /*
	     * We're finished with the request, so schedule the drive
	     * to be powered down in 6 VBL scan times (3 seconds).
	     */
	    printf("core.c:Powerdowning the drive ");
	    PowerDriveDown(DriveStatus,
			   6);

	}
    }
    printf("core.c:returning from ReadBlocks ");
    return errorCode;

}


/************************************************************************
*
* FUNCTION : WriteBlocks
*
************************************************************************/

OSStatus
WriteBlocks(DriveStatusType * DriveStatus,
	    long *transferByteCount)
{

    register short errorCode;
    register short lastBlockNumber;
    short recalCount;


    /*
     * Bail if we don't have a diskette in the drive, or a drive installed.
     */

    printf("wrt:call checkdriveonline ");
    if ((errorCode = CheckDriveOnLine(DriveStatus)) == noErr) {

	/*
	 * Powered up and select the drive immediately. However, wait the
	 * power up time after the drive is powered up to make sure the it
	 * is up to speed.
	 */

	printf("wrt:call powerup drive ");
	if ((errorCode = PowerDriveUp(DriveStatus)) == noErr) {

	    /*
	     * Compute the last block number in the request.
	     */

	    lastBlockNumber = DriveStatus->firstBlockNumber + DriveStatus->blockCount - 1;

	    /*
	     * Make sure that the requested block number is not bigger than
	     * the diskette.
	     */

	    if (lastBlockNumber >= DriveStatus->diskFormat.drvBlockMax) {

		errorCode = RecordError(seekErr);

	    } else {

		/*
		 * Transfer the required number of blocks, or return an error.
		 */

		errorCode = noErr;
		*transferByteCount = 0;

		while ((errorCode == noErr) &&
		    (DriveStatus->firstBlockNumber <= lastBlockNumber)) {

		    /*
		     * If we get an error, recalibrate the drive and retry the
		     * write a max of four times before we return the error.
		     */

		    recalCount = 4;

		    while (recalCount) {

			/*
			 * Calculate the next sector address from the block number.
			 */
			printf("wrtblks: call getsectoraddr ");
			GetSectorAddress(DriveStatus,
					 DriveStatus->firstBlockNumber);

			/*
			 * Check to see if we're going to move to different track
			 * and if so, flush any cached write data to diskette and
			 * seek to the requested track.
			 */

			if ((errorCode = FlushCacheAndSeek(DriveStatus)) == noErr) {

			    /*
			     * Write the next sector data from the user's buffer space.
			     * If we got an error, recalibrate the drive and retry the
			     * write. Otherwise, keep track of the number of bytes
			     * transfered, where they came from and abort the recal
			     * loop.
			     */
			    printf("wrt:calling wrttocache ");
			    if ((errorCode = WriteSectorToCacheMemory(DriveStatus)) == noErr) {

				*transferByteCount += DriveStatus->diskFormat.logicalSectorSize;

				DriveStatus->firstBlockNumber++;

				break;

			    }
			}
			/*
			 * If there was an error, recalibrate the drive and keep
			 * track of the number of times we got an error.
			 */

			if (errorCode != noErr) {

			    printf("wrtblks:call RecalDrive ");
			    (void) RecalDrive(DriveStatus);

			    DriveStatus->driveErrs++;

			    recalCount--;

			}
		    }

		}

	    }

	    /*
	     * We're finished with the request, so schedule the drive
	     * to be powered down in 6 VBL scan times (3 seconds).
	     */

	    PowerDriveDown(DriveStatus,
			   6);

	}
    }
//naga added the following line
    //WriteCacheToDiskTrack( DriveStatus );
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : CheckDriveNumber
*
*	Test the drive number to make sure that the device exists. Returns
*	a pointer to the drive status table for the wanted drive.
*
************************************************************************/

OSStatus
CheckDriveNumber(int unit, short DriveNumber,
		 DriveStatusType ** DriveStatusPtr)
{

    /*
     * If the drive number is out of range return an error.
     */

#if MACH_DEBUG
    printf("CheckDriveNumber called. ");
#endif

    if ((DriveNumber < 1) ||
	(DriveNumber > maxDrvNum)) {

	return nsDrvErr;

    } else {

	/*
	 * Return the pointer to the drive status struct.
	 */

	*DriveStatusPtr = &SonyVariables[unit].driveStatus[DriveNumber - 1];

	/*
	 * Remember the internal drive number for those times when we
	 * need a drive number.
	 */

	(*DriveStatusPtr)->driveNumber = DriveNumber - 1;

#if MACH_DEBUG
	printf("CheckDriveNumber returning. ");
#endif
	return noErr;

    }

}


/************************************************************************
*
*	FUNCTION : PowerDriveUp
*
*	Power up the currently selected drive, and if we actually power up
*	the drive, re-test the diskette's format. If this is the first time
*	the diskette has been accessed, dump any stall track data.
*
************************************************************************/

OSStatus
PowerDriveUp(DriveStatusType * DriveStatus)
{

    short errorCode;
    short newMotorState;
    int unit = DriveStatus->unit;


    /*
     * Assume success and select the drive we're powering up.
     */

    errorCode = noErr;

    /*
     * If a different drive is waiting for power down than the
     * one we're about to power up, flush out any written data
     * to the other drive and power it down immediately.
     */

    if ((SonyVariables[unit].timeOut != 0) &&
	(SonyVariables[unit].timeOutDrive != DriveStatus)) {

	PowerDriveDown(SonyVariables[unit].timeOutDrive,
		       0);

    }
    /*
     * Cancel any previous power down timeouts for this drive.
     */

    SonyVariables[unit].timeOut = 0;

    /*
     * If the drive was already powered up, just exit.
     */

    newMotorState = MOTOR_WAS_ON;

    if (DriveStatus->motorOnState == MOTOR_IS_OFF) {
	printf("core.c:PowerDriveUp:calling HALPowerUpDrive ");
	if ((errorCode = HALPowerUpDrive(DriveStatus)) == noErr) {

	    /*
	     * Check to see what kind of DRVR function we're about to do.
	     * If this is anything but a Prime Read, wait. Otherwise, just
	     * return with zero wait time for Prime Reads.
	     */

	    if (!((DriveStatus->DRVROperation == DRVRPrimeCC) &&
		  (DriveStatus->currentCommand == prmReadCode))) {

		/*
		 * Now wait 600 milliseconds to make sure that the motor is
		 * up to speed in all cases that could write.
		 */

		errorCode = FloppyTimedSleep(600);

	    }
	    /*
	     * Remember that the motor is now on.
	     */

	    newMotorState = MOTOR_WAS_OFF;

	    /*
	     * Since the drive has been power down, re-test the diskette
	     * (if a diskette is in the drive) to see how it is formated
	     * and setup it's state variables.
	     */

	    if (DriveStatus->diskInPlace != NO_DISKETTE) {

		/*
		 * Has the diskette not been accessed yet?
		 */

		if (DriveStatus->diskInPlace == DISK_PRESENT) {

		    /*
		     * Decipher what kind of diskette was inserted.
		     */
		    printf("core.c:PowerDriveUp:calling GetDisketteFormat ");

		    if ((errorCode = GetDisketteFormat(DriveStatus)) == noErr) {

			/*
			 * Tell the rest of the driver that we've touched this
			 * diskette and we know it's format.
			 */

			DriveStatus->diskInPlace = DISK_WAS_ACCESSED;

		    }
		    printf("core.c:PowerDriveUp:return code from GetdisketteFormat=%d ", errorCode);

		} else {

		    /*
		     * If the diskette has already been accessed, make sure
		     * the drive and FDC are setup correctly for this MFM/GCR
		     * diskette.
		     */

		    if (DriveStatus->diskInPlace == DISK_WAS_ACCESSED) {

			errorCode = SetDisketteFormat(DriveStatus,
				     GetDisketteFormatType(DriveStatus));

		    }
		}

	    }
	}
    }
    /*
     * Set the new motor state.
     */

    DriveStatus->motorOnState = newMotorState;
    printf("core.c:PowerDriveUp:return=%d ", errorCode);
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : PowerDriveDown
*
*	Power down the currently selected drive after the time period specifed.
*
************************************************************************/

void PowerDriveDown(DriveStatusType * DriveStatus,
		    long PowerDownTimeout)
{
    int unit = DriveStatus->unit;

    /*
     * Has a timeout period been specified?
     */

    if (PowerDownTimeout == 0) {

	/*
	 * Flush the cache before the drive LED gets shut off. The user is 
	 * allowed to remove manually ejected disks only when the drive LED
	 * is off.
	 */

	FlushTrackCache(DriveStatus);

	/*
	 * When the drive is powered back up again, its required to reset
	 * format mode (MFM vs GCR. This is because the SWIM hardware powers
	 * up in GCR mode as the default). Setting the diskette format may
	 * re-write a blank track cache (once again, a SWIM requirement).
	 * Therefore, dump what ever we have in the track cache right now to
	 * make sure that on power up we read fresh sector data into the track
	 * cache after the format mode is set.
	 */

	DumpTrackCache(DriveStatus);

	/*
	 * Turn off the drive's motor and kill it's power.
	 */

	HALPowerDownDrive(DriveStatus);

	/*
	 * Remember that the motor is off.
	 */

	DriveStatus->motorOnState = MOTOR_IS_OFF;

	/*
	 * Cancel any previous power down timeouts.
	 */

	SonyVariables[unit].timeOut = 0;

    } else {

	/*
	 * Remember the timeout period, and for which drive.
	 */

	SonyVariables[unit].timeOut = (short) PowerDownTimeout;

	SonyVariables[unit].timeOutDrive = DriveStatus;

    }

}


/************************************************************************
*
*	FUNCTION : GetDisketteFormatType
*
************************************************************************/

short GetDisketteFormatType(DriveStatusType * DriveStatus)
{

    /*
     * Return the format type for MFM diskettees.
     */
    printf("core.c:GetDisketteFormatType: ");
    if (DriveStatus->formatType == MFM_DISKETTE) {

	switch (DriveStatus->disketteType) {

	    /*
	     * This must be a 720K diskette.
	     */

	case DISKETTE_1MEG:
	    return MFM720K;

	    /*
	     * This must be a 1440K HD diskette.
	     */

	case DISKETTE_2MEG:
	    return MFM1440K;

	    /*
	     * This must be a 2880K HD diskette.
	     */

	case DISKETTE_4MEG:
	    return MFM2880K;

	}

    }
    /*
     * Return the format type for GCR diskettees.
     */

    if (DriveStatus->formatType == GCR_DISKETTE) {

	/*
	 * Only 1 Meg diskettes can be formated as GCR.
	 */

	if (DriveStatus->disketteType == DISKETTE_1MEG) {

	    /*
	     * Is this a single sided diskette.
	     */

	    if (DriveStatus->doubleSided == false) {

		/*
		 * This must be a 400K, single sided diskette.
		 */

		return GCR400K;

	    } else {

		/*
		 * This must be a 800K, double sided diskette.
		 */

		return GCR800K;

	    }

	}
    }
    return 0;

}


/************************************************************************
*
*	FUNCTION : SetDisketteFormat
*
*	Set a diskette's format. 
*
************************************************************************/

OSStatus
SetDisketteFormat(DriveStatusType * DriveStatus,
		  short formatIndex)
{
    int unit = DriveStatus->unit;
    printf("core.c:SetDisketteFormat: ");
    /*
     * Set the Drive Status record to the requested format.
     */

    DriveStatus->diskFormat = SonyVariables[unit].formatTable[formatIndex];

    /*
     * Make sure that the BlockSize field in the wantedAddress is set
     * so that it will appear properly in Address Mark headers.
     */
    printf("core.c:SetDisketteFormat:Calling SetSectorAddressBlocksize ");
    SetSectorAddressBlocksize(DriveStatus);

    /*
     * Make sure that the sectors/track field is set for the current track
     * (this mostly for GCR since it has variable sectors/track).
     */

    DriveStatus->wantedAddress.address.items.track = DriveStatus->currentAddress.address.items.track;
    printf("Core.c:SetDisketteFormat:Calling SetSectorPerTrack ");
    SetSectorsPerTrack(DriveStatus);

    /*
     * Now tell the HAL what format to initialize to.
     */
    printf("core.c:SetDisketteFormat:Calling HALSetFormatMode ");
    return HALSetFormatMode(DriveStatus);

}


/************************************************************************
*
*	FUNCTION : GetDisketteFormat
*
*	Test a diskette to see how it is formated and setup it's state
*	variables. 
*
************************************************************************/

OSStatus
GetDisketteFormat(DriveStatusType * DriveStatus)
{

    short errorCode;


    /*
     * Is this a FDHD drive? Assume that we'll timeout.
     */

    errorCode = noNybErr;

    if (DriveStatus->driveType != DRIVE_1MEG_800k) {

	/*
	 * Test for MFM mode by setting to the diskette's default format.
	 *
	 * Also, all MFM diskettes are double sided.
	 */

	DriveStatus->formatType = MFM_DISKETTE;
	DriveStatus->doubleSided = true;

	if ((errorCode = SetDisketteFormat(DriveStatus,
			 GetDisketteFormatType(DriveStatus))) == noErr) {

	    /*
	     * Put the drive's head in a known position.
	     */
	    printf("core:GetDisketteFormat:calling RecalDrive ");
	    if ((errorCode = RecalDrive(DriveStatus)) == noErr) {

		/*
		 * Can we find a valid MFM address mark on side zero of
		 * the diskette? If so, find it's format in the format
		 * tables. Otherwise, use the default format.
		 *
		 * (Add the searching through the format tables later)
		 */

		DriveStatus->wantedAddress.address.items.side = 0;

		printf("core:GetDisketteFormat:calling HALGetNextAddressID ");
		if ((errorCode = HALGetNextAddressID(DriveStatus)) != noErr) {

		    /*
		     * If we got a timeout error on a two or four meg diskette
		     * (i.e. most likely not formatted), return OK which will
		     * use the default format.
		     */

		    if (DriveStatus->disketteType != DISKETTE_1MEG)
			errorCode = noErr;

		}
	    }
	}
    }
    /*
     * If we didn't have a FDHD drive, or we didn't find MFM formating
     * on a one meg diskette, test for GCR.
     */

    if ((DriveStatus->driveType == DRIVE_1MEG_800k) ||
	((DriveStatus->disketteType == DISKETTE_1MEG) &&
	 (errorCode != noErr))) {

	/*
	 * Test for GCR mode. Default to a single sided diskette
	 * in case this is a 400k diskette.
	 */

	DriveStatus->formatType = GCR_DISKETTE;
	DriveStatus->doubleSided = false;
	printf("core:GetDisketteFormat:calling SetDisketteFormat ");
	if ((errorCode = SetDisketteFormat(DriveStatus,
			 GetDisketteFormatType(DriveStatus))) == noErr) {

	    /*
	     * Put the drive's head in a known position.
	     */

	    printf("core:GetDisketteFormat:calling RecalDrive again ");
	    if ((errorCode = RecalDrive(DriveStatus)) == noErr) {

		/*
		 * Can we find a valid GCR address mark on side zero of
		 * the diskette?
		 */

		DriveStatus->wantedAddress.address.items.side = 0;

		printf("core:GetDisketteFormat:calling HALGetNextAddressID again ");
		if ((errorCode = HALGetNextAddressID(DriveStatus)) == noErr) {

		    /*
		     * Now see if we find a valid GCR address mark on side one of
		     * the diskette. If so, this a 800k GCR diskette, otherwise
		     * this must be a 400k GCR diskette.
		     */

		    DriveStatus->wantedAddress.address.items.side = 1;

		    printf("core:GetDisketteFormat:calling HALGetNextAddressID 3rd again ");
		    if ((errorCode = HALGetNextAddressID(DriveStatus)) == noErr) {

			/*
			 * 800k diskettes are double sided, and the diskette's
			 * default format is 800k GCR.
			 */

			DriveStatus->doubleSided = true;
			printf("core:calling SetDisketteFormat ");
			SetDisketteFormat(DriveStatus,
				     GetDisketteFormatType(DriveStatus));

		    }
		}
	    }
	}
    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : BuildTrackInterleaveTable
*
*	Build a sector interleave lookup table to lookup which sector will
*	physically follow the current one.
*
*	NOTE: This function only completely works for the cases that are
*	needed right now. Namely, all GCR 2 to 1 and MFM 1 to 1. Should be
*	fixed as soon as GCR 4 to 1 and DMF 2 to 1 are needed.
*
************************************************************************/

void BuildTrackInterleaveTable(DriveStatusType * DriveStatus,
			       unsigned char sectorsPerTrack)
{

    unsigned char tableIndex;
    unsigned char sectorNumber;
    unsigned char nextSectorNumber;
    unsigned char sectorWrapLimit;
    unsigned char sectorWrapAmount;


    /*
     * Build a sector interleave lookup table to lookup which sector will physically
     * follow the current one.
     *
     * First, check to see if the track is interleaved.
     */

    if (DriveStatus->diskFormat.sectorInterleave > 1) {

	/*
	 * First, calculate the number of sectors to wrap around to the begining of the
	 * sector lookup table. If the sectors/track is odd, round up one.
	 *
	 * Start out with the first table entry with the first interleave value.
	 */

	sectorWrapAmount = (sectorsPerTrack / DriveStatus->diskFormat.sectorInterleave) + (sectorsPerTrack & 1);

	nextSectorNumber = sectorWrapAmount;

    } else {

	/*
	 * When there is no interleave, just build a linear lookup table that wraps
	 * arounds.
	 */

	sectorWrapAmount = 1;	//naga changed 1 to 2  back to 1

	nextSectorNumber = 2;	//naga changed 2 to 3  back to 2 

    }

    /*
     * For speed, setup a variable for the sector number wrap limit.
     */

    sectorWrapLimit = sectorsPerTrack + DriveStatus->diskFormat.firstSectorInTrack;

    /*
     * Fill in all of the table entries but the last one, starting out with the
     * first sector entry.
     */

    tableIndex = sectorsPerTrack - 1;

    sectorNumber = DriveStatus->diskFormat.firstSectorInTrack;

    while (tableIndex--) {

	DriveStatus->sectorInterleaveLookup[sectorNumber] = nextSectorNumber;

	sectorNumber = nextSectorNumber;

	nextSectorNumber += sectorWrapAmount;

	if (nextSectorNumber >= sectorWrapLimit)
	    nextSectorNumber -= sectorsPerTrack - ((sectorsPerTrack & 1) ^ 1);

    }

    /*
     * Fill the last table entry with a wrap around to tbe begining sector.
     */

    DriveStatus->sectorInterleaveLookup[sectorNumber] = DriveStatus->diskFormat.firstSectorInTrack;

}


/************************************************************************
*
*	FUNCTION : SetCacheAddresses
*
*	Set the addresses of sectors inside the track cache buffer.
*
************************************************************************/

void SetCacheAddresses(DriveStatusType * DriveStatus)
{

    short headIndex;
    short sectorIndex;

    for (headIndex = 0; headIndex < maxGCRHeads; headIndex++) {

	for (sectorIndex = 0; sectorIndex < DriveStatus->diskFormat.sectorsPerTrack; sectorIndex++) {

	    FPYComputeCacheDMAAddress(DriveStatus,
				      headIndex,
		sectorIndex + DriveStatus->diskFormat.firstSectorInTrack,
				      0,
				      &DriveStatus->intraCacheSectorAddress[headIndex][sectorIndex + DriveStatus->diskFormat.firstSectorInTrack]);

	}

    }

}


/************************************************************************
*
*	FUNCTION : SetSectorsPerTrack
*
*	Set the number of sectors in the requested track. This number is
*	constant for MFM and variable for GCR.
*
************************************************************************/

void SetSectorsPerTrack(DriveStatusType * DriveStatus)
{

    /*
     * Is this a GCR diskette. Otherwise, just return the MFM value directly.
     */

    if (DriveStatus->diskFormat.GCR) {

	/*
	 * Divide the track number to get the track zone minus one, and use
	 * that value to calculate the sectors/track for that zone.
	 */

	DriveStatus->diskFormat.sectorsPerTrack = 12 - (DriveStatus->wantedAddress.address.items.track >> 4);

    } else
	DriveStatus->diskFormat.sectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;

    /*
     * Fill in the sector interleave table for this track.
     */

    BuildTrackInterleaveTable(DriveStatus,
			      DriveStatus->diskFormat.sectorsPerTrack);

}


/************************************************************************
*
*	FUNCTION : SetSectorAddressBlocksize
*
*	Turn the specified diskette block number into the it's Track, Sector,
*	Head equivalent.
*
************************************************************************/

void SetSectorAddressBlocksize(DriveStatusType * DriveStatus)
{

    register short headsPerCylinder;
    register short sectorSize;


    /*
     * This is the only value we can get without knowing the format type.
     */

    headsPerCylinder = DriveStatus->diskFormat.flagsNSides & FmtSidesMask;

    /*
     * Is this a GCR diskette.
     */

    if (DriveStatus->diskFormat.GCR) {

	/*
	 * Higher nibble is number of formated sides.
	 * Lower nibble is sector interleave. 2 to 1 for Macs, 4 to 1 for IIe and IIgs.
	 */

	DriveStatus->wantedAddress.address.items.blockSize = (headsPerCylinder << 4) | DriveStatus->diskFormat.sectorInterleave;

    } else {

	/*
	 * Now calculate the block size byte based on sector size.
	 *
	 * blockSize     sectorSize
	 *     0            N/A
	 *     1            256
	 *     2            512
	 *     3           1024
	 *     4           2048
	 *     5           4096
	 *     6           8192
	 */

	sectorSize = DriveStatus->diskFormat.logicalSectorSize / 256;

	DriveStatus->wantedAddress.address.items.blockSize = 0;

	while (sectorSize != 0) {

	    DriveStatus->wantedAddress.address.items.blockSize++;

	    sectorSize = sectorSize >> 1;

	}

    }

}


/************************************************************************
*
*	FUNCTION : GetSectorAddress
*
*	Turn the specified diskette block number into the it's Track, Sector,
*	Head equivalent.
*
************************************************************************/

void GetSectorAddress(DriveStatusType * DriveStatus,
		      short blockNumber)
{

    register short sectorsPerTrack;
    register short headsPerCylinder;
    short trackOffset;
    short zone2StartBlk;
    short zone3StartBlk;
    short zone4StartBlk;
    short zone5StartBlk;


    /*
     * This is the only value we can get without knowing the format type.
     */

    headsPerCylinder = DriveStatus->diskFormat.flagsNSides & FmtSidesMask;

    /*
     * Is this a GCR diskette.
     */

    if (DriveStatus->diskFormat.GCR) {

	/*
	 * Determine if this is a 800k or 400k diskette to set where the
	 * begining of each track zone starts by block number.
	 */

	if (headsPerCylinder == 0) {

	    zone2StartBlk = 192;
	    zone3StartBlk = 368;
	    zone4StartBlk = 528;
	    zone5StartBlk = 672;

	} else {

	    zone2StartBlk = 384;
	    zone3StartBlk = 736;
	    zone4StartBlk = 1056;
	    zone5StartBlk = 1344;

	}

	/*
	 * Now based on which track zone the desired block falls in, get the
	 * number of sectors/track for that track zone, and adjust the block
	 * address calculation as if the track zone was the begining of the
	 * diskette.
	 *
	 * Is the block number in track zone 1.
	 */
	printf("GetSectoraddress:blk=%d ", blockNumber);
	if (blockNumber < zone2StartBlk) {

	    sectorsPerTrack = 12;

	    trackOffset = 0;

	} else {

	    /*
	     * Is the block number in track zone 2.
	     */

	    if (blockNumber < zone3StartBlk) {

		sectorsPerTrack = 11;

		trackOffset = 16;

		blockNumber -= zone2StartBlk;

	    } else {

		/*
		 * Is the block number in track zone 3.
		 */

		if (blockNumber < zone4StartBlk) {

		    sectorsPerTrack = 10;

		    trackOffset = 32;

		    blockNumber -= zone3StartBlk;

		} else {

		    /*
		     * Is the block number in track zone 4, else its in track zone 5.
		     */

		    if (blockNumber < zone5StartBlk) {

			sectorsPerTrack = 9;

			trackOffset = 48;

			blockNumber -= zone4StartBlk;

		    } else {

			sectorsPerTrack = 8;

			trackOffset = 64;

			blockNumber -= zone5StartBlk;

		    }

		}

	    }

	}

	/*
	 * Calculate the GCR sector address from the adjusted block number.
	 */

	DriveStatus->wantedAddress.address.items.track = (blockNumber / (short) (sectorsPerTrack * headsPerCylinder)) + trackOffset;
	DriveStatus->wantedAddress.address.items.sector = blockNumber % sectorsPerTrack;
	DriveStatus->wantedAddress.address.items.side = (blockNumber / sectorsPerTrack) & 1;

    } else {

	/*
	 * Use the MFM values in the format table.
	 */

	sectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;

	/*
	 * Calculate the next MFM sector address from the block number.
	 */

	DriveStatus->wantedAddress.address.items.track = blockNumber / (short) (sectorsPerTrack * headsPerCylinder);
	DriveStatus->wantedAddress.address.items.sector = (blockNumber % sectorsPerTrack) + 1;
	DriveStatus->wantedAddress.address.items.side = (blockNumber / sectorsPerTrack) & 1;

    }
#ifdef printf
#undef printf
#endif

}

#if (!MACH_DEBUG)
#define printf donone
#endif

/************************************************************************
*
*	FUNCTION : DumpTrackCache
*
*	Invalidates the cached track data we might have for the specified drive. 
*
************************************************************************/

void DumpTrackCache(DriveStatusType * DriveStatus)
{
    int unit = DriveStatus->unit;

    SonyVariables[unit].tcDrive = -1;

    DriveStatus->cachedTrack[0] = -1;
    DriveStatus->cachedTrack[1] = -1;

    /*
     * Mark the track cache as not needing to be flushed.
     */

    ResetBitArray((BitArrayByte *) DriveStatus->sectorDirty,
		  sizeof(DriveStatus->sectorDirty));

    /*
     * Reset the bit array for remembering which sectors in the
     * track cache, for GCR, have and haven't been nibblized.
     * Also, reset the bit array for which sector in the track
     * cache are "write" dirty.
     */

    ResetBitArray((BitArrayByte *) DriveStatus->sectorDenibblized,
		  sizeof(DriveStatus->sectorDenibblized));

}


/************************************************************************
*
*	FUNCTION : FlushTrackCache
*
*	Write out any dirty the cached track data we might have for the
*	specified drive, but leave the track data active. 
*
************************************************************************/

OSStatus
FlushTrackCache(DriveStatusType * DriveStatus)
{

    register short errorCode;
    diskAddressType saveDiskAddress;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * If the track cache has any dirty blocks, write them out.
     */

    if (TestCacheDirtyState(DriveStatus)) {

	/*
	 * Save the caller's wanted disk address while we write the track cache.
	 */

	saveDiskAddress.address.blockAddress = DriveStatus->wantedAddress.address.blockAddress;

	DriveStatus->wantedAddress.address.items.track = DriveStatus->currentAddress.address.items.track;

	/*
	 * Is side 0 of the track cache dirty?
	 */

	DriveStatus->wantedAddress.address.items.side = 0;

	if (TestCacheDirtyState(DriveStatus)) {

	    /*
	     * Write out side 0 of the track cache, leaving side 0
	     * of the track cache marked as "read" clean.
	     */

	    errorCode = WriteCacheToDiskTrack(DriveStatus);

	}
	/*
	 * Is side 1 of the track cache dirty?
	 */

	DriveStatus->wantedAddress.address.items.side = 1;

	if ((errorCode == noErr) &&
	    TestCacheDirtyState(DriveStatus)) {

	    /*
	     * Write out side 1 of the track cache, leaving side 1
	     * of the track cache marked as "read" clean..
	     */

	    errorCode = WriteCacheToDiskTrack(DriveStatus);

	}
	/*
	 * Restore the caller's wanted diskaddress and mark the whole
	 * track cache as "read" clean.
	 */

	DriveStatus->wantedAddress.address.blockAddress = saveDiskAddress.address.blockAddress;

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : TestCacheDirtyState
*
*	Test to see if the specified drive's track cache is dirty or clean. 
*
************************************************************************/

boolean_t
TestCacheDirtyState(DriveStatusType * DriveStatus)
{

    boolean_t dirtyResult;
    int unit = DriveStatus->unit;


    if (SonyVariables[unit].tcDrive != DriveStatus->driveNumber)
	return false;

    dirtyResult = TestBitArray((BitArrayByte *) DriveStatus->sectorDirty,
			       sizeof(DriveStatus->sectorDirty));

    return dirtyResult;

}


/************************************************************************
*
*	FUNCTION : TestTrackInCache
*
*	Test if the wanted sector data is cached for the specified drive. 
*
************************************************************************/

boolean_t
TestTrackInCache(DriveStatusType * DriveStatus)
{
    int unit = DriveStatus->unit;

    if (SonyVariables[unit].tcDrive != DriveStatus->driveNumber)
	return false;

    return DriveStatus->cachedTrack[DriveStatus->wantedAddress.address.items.side] == DriveStatus->wantedAddress.address.items.track;

}


/************************************************************************
*
*	FUNCTION : AssignTrackInCache
*
*	Remember the which track data we have cached for the specified drive. 
*
************************************************************************/

void AssignTrackInCache(DriveStatusType * DriveStatus)
{
    int unit = DriveStatus->unit;

    SonyVariables[unit].tcDrive = DriveStatus->driveNumber;

    DriveStatus->cachedTrack[DriveStatus->wantedAddress.address.items.side] = DriveStatus->wantedAddress.address.items.track;

}


/************************************************************************
*
*	FUNCTION : FPYComputeCacheDMAAddress
*
************************************************************************/

uint_t
FPYComputeCacheDMAAddress(DriveStatusType * DriveStatus,
			  unsigned char targetSide,
			  unsigned char targetSector,
			  long postDMAAlignmentCount,
			  TransferAddressType * returnedCacheAddress)
{

    register unsigned long offsetInCache;
    register unsigned short sectorSizeInCache;
    register uint_t DMASectorDataAlignment;


    /*
     * Calculate the actual number of bytes used in the track
     * buffer by a single sector.
     */

    sectorSizeInCache = DriveStatus->preSectorCachePadding +
	DriveStatus->diskFormat.logicalSectorSize +
	DriveStatus->postSectorCachePadding;

    /*
     * Compute the byte offset of the sector data in the track cache buffer.
     *
     * First make sure the sector number is zero based. Second, turn the sector
     * number into a byte offset. Last, if the diskette is double sided, adjust
     * the byte offset for the second side.
     */

    offsetInCache = targetSector - DriveStatus->diskFormat.firstSectorInTrack;

    offsetInCache = DriveStatus->preFirstSectorCachePadding +
	(offsetInCache * sectorSizeInCache) +
	DriveStatus->preSectorCachePadding;

    if (targetSide) {

	offsetInCache = offsetInCache +
	    DriveStatus->preFirstSectorCachePadding +
	    (sectorSizeInCache * DriveStatus->diskFormat.sectorsPerTrack) +
	    DriveStatus->postLastSectorCachePadding;

    }
    /*
     * Return the offset track cache buffer address pointing passed
     * the pre-sector padding bytes, to the sector data bytes.
     */

    returnedCacheAddress->logicalAddress = DriveStatus->tcBuffer.logicalAddress + offsetInCache;
    returnedCacheAddress->physicalAddress = DriveStatus->tcBuffer.physicalAddress + offsetInCache;

    /*
     * Make sure that the cache addresses is rounded UP to a DMA byte boundry
     * for those DMA engines that has that restriction. Figure out how much
     * that alignment will be. Do this from the physical address since thats
     * the one that matters.
     */

    DMASectorDataAlignment = 0;

    if (DriveStatus->DMAByteBoundryAlignment > 1)
	DMASectorDataAlignment = (((unsigned long) returnedCacheAddress->physicalAddress & ~(DriveStatus->DMAByteBoundryAlignment - 1)) + DriveStatus->DMAByteBoundryAlignment) - (unsigned long) returnedCacheAddress->physicalAddress;

    /*
     * Now add in the DMA byte alignment adjustments.
     */

    returnedCacheAddress->logicalAddress += (DMASectorDataAlignment + postDMAAlignmentCount);
    returnedCacheAddress->physicalAddress += (DMASectorDataAlignment + postDMAAlignmentCount);

    /*
     * Return the amount that the cache buffer pointer was DMA byte boundry
     * adjusted in case the caller wants to know.
     */

    return DMASectorDataAlignment;

}


/************************************************************************
*
*	FUNCTION : DenibblizeGCRData
*
*	Given a pointer to a buffer of nibblized data, decode it into its
*	real values, plus calculate the running checksum. 
*
************************************************************************/

unsigned char *
 DenibblizeGCRData(unsigned char *sourceAddress,
		   unsigned char *destAddress,
		   unsigned short transferLength,
		   unsigned long *initalChecksum)
{

    register short dataCount;
    register short errorCode;
    register unsigned char topBitsByte;
    register unsigned char topBitsMask;
    register unsigned short ChecksumA;
    register unsigned short ChecksumB;
    register unsigned short ChecksumC;
    register unsigned short Checksum_Carry;
    register unsigned short Byte_mask;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Start off using the checksum values that have been accumulated
     * so far.
     */

    Byte_mask = 0xFF;

    ChecksumA = (*initalChecksum >> 16) & Byte_mask;
    ChecksumB = (*initalChecksum >> 8) & Byte_mask;
    ChecksumC = *initalChecksum & Byte_mask;

    /*
     * Transfer the sector data that is whole units of four nibble-bytes groups
     * to three real bytes.
     */

    dataCount = transferLength;

    topBitsMask = 0xC0;

    while ((dataCount > 0) &&
	   (errorCode == noErr)) {

	/*
	 * Rotate the top bit of checksum C around to bit zero
	 * and remember it as checksum C's carry.
	 */

	Checksum_Carry = ChecksumC >> 7;
	ChecksumC = (ChecksumC << 1) | Checksum_Carry;

	/*
	 * Get the top-bits, denibblize it and get ready for the first
	 * combine operation.
	 */

	topBitsByte = *sourceAddress++ << 2;

	/*
	 * Get the first data byte value, recombine it's top two bits,
	 * prepare top-bits byte for the next combine operation and add
	 * this byte to the total count.
	 */

	*destAddress = (topBitsByte & topBitsMask) | *sourceAddress++;

	topBitsByte = topBitsByte << 2;

	/*
	 * Get the true value of byte A, update checksum A with
	 * the value of byte A and the carry from checksum C,
	 * seperate checksum A's carry from update calculation.
	 */

	*destAddress = *destAddress ^ ChecksumC;
	ChecksumA = ChecksumA + *destAddress + Checksum_Carry;

	Checksum_Carry = ChecksumA >> 8;
	ChecksumA = ChecksumA & Byte_mask;

	destAddress++;

	dataCount--;

	/*
	 * Get the second data byte value, recombine it's top two bits,
	 * prepare top-bits byte for the next combine operation and add
	 * this byte to the total count.
	 */

	*destAddress = (topBitsByte & topBitsMask) | *sourceAddress++;

	topBitsByte = topBitsByte << 2;

	/*
	 * Get the true value of byte B, update checksum B with
	 * the value of byte B and the carry from checksum A,
	 * seperate checksum B's carry from update calculation.
	 */

	*destAddress = *destAddress ^ ChecksumA;
	ChecksumB = ChecksumB + *destAddress + Checksum_Carry;

	Checksum_Carry = ChecksumB >> 8;
	ChecksumB = ChecksumB & Byte_mask;

	destAddress++;

	dataCount--;

	/*
	 * This is a little bit of a hack. Since we know that with 512
	 * byte sectors that we will be one byte short of a even multible
	 * of three, so test for the length counter going to zero before
	 * we try to get byte C. This allows us to bail out at the proper
	 * time. Since it is highly unlikly that GCR diskettes will ever
	 * have a sector size other than 512, this should work forever.
	 */

	if (dataCount > 0) {

	    /*
	     * Get the third data byte value, recombine it's top two bits and
	     * add this byte to the total count.
	     */

	    *destAddress = (topBitsByte & topBitsMask) | *sourceAddress++;

	    /*
	     * Get the true value of byte C, update checksum C with
	     * the value of byte C and the carry from checksum B,
	     * truncate checksum C's carry from update calculation.
	     */

	    *destAddress = *destAddress ^ ChecksumB;
	    ChecksumC = (ChecksumC + *destAddress + Checksum_Carry) & Byte_mask;

	    destAddress++;

	    dataCount--;

	}
    }

    /*
     * Return the accumulated checksum value and the updated data address.
     */

    *initalChecksum = ChecksumA & Byte_mask;
    *initalChecksum = (*initalChecksum << 8) | (ChecksumB & Byte_mask);
    *initalChecksum = (*initalChecksum << 8) | (ChecksumC & Byte_mask);

    return sourceAddress;

}


/************************************************************************
*
*	FUNCTION : DenibblizeGCRChecksum
*
*	Decode nibblized GCR checksum data to the checksum's real value.
*
************************************************************************/

void DenibblizeGCRChecksum(unsigned char *sourceAddress,
			   unsigned long *realChecksum)
{

    register unsigned char topBitsByte;
    register unsigned char topBitsMask;
    register unsigned long ChecksumValue;


    /*
     * Setup the high bits bit mask.
     */

    topBitsMask = 0xC0;

    /*
     * Get the top-bits, denibblize it and get ready for the first
     * combine operation.
     */

    topBitsByte = *(unsigned char *) sourceAddress++ << 2;

    /*
     * Get the first checksum byte value, recombine it's top two bits,
     * prepare top-bits byte for the next combine operation.
     */

    ChecksumValue = ((topBitsByte & topBitsMask) | *(unsigned char *) sourceAddress++) << 16;

    topBitsByte = topBitsByte << 2;

    /*
     * Get the second checksum byte value, recombine it's top two bits,
     * prepare top-bits byte for the next combine operation.
     */

    ChecksumValue |= ((topBitsByte & topBitsMask) | *(unsigned char *) sourceAddress++) << 8;

    topBitsByte = topBitsByte << 2;

    /*
     * Get the last checksum byte value and recombine it's top two bits.
     */

    ChecksumValue |= (topBitsByte & topBitsMask) | *(unsigned char *) sourceAddress++;

    /*
     * Return the checksum value from the diskette and any errors.
     */

    *realChecksum = ChecksumValue;

}


/************************************************************************
*
*	FUNCTION : FPYDenibblizeGCRSector
*
*	Given a pointer to a buffer of nibblized sector data, decode it into
*	its sector and tag data components. 
*
************************************************************************/

OSStatus
FPYDenibblizeGCRSector(DriveStatusType * DriveStatus,
		       unsigned char *denibblizeBufferPtr,
		       unsigned char *sectorBufferPtr)
{

    short errorCode;
    lowMemTagDataType *TagDenibbleBufferPtr;
    TagDataType *localTagDenibbleBufferPtr;
    short localTagSectorNum;
    unsigned long computedChecksum;
    unsigned long realChecksum;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Test to see if the GCR sector needs to be denibblized.
     */

    if (!TestBit(DriveStatus->sectorDenibblized[DriveStatus->wantedAddress.address.items.side],
		 DriveStatus->wantedAddress.address.items.sector)) {

	/*
	 * Remember the sector number we got from the tag data.
	 */

	localTagSectorNum = *denibblizeBufferPtr++;

	/*
	 * Remember where the denibblized tag data begins.
	 */

	localTagDenibbleBufferPtr = (TagDataType *) denibblizeBufferPtr;

	computedChecksum = 0;

	denibblizeBufferPtr = DenibblizeGCRData(denibblizeBufferPtr,
			     (unsigned char *) localTagDenibbleBufferPtr,
						sizeof(TagDataType),
						&computedChecksum);

	denibblizeBufferPtr = DenibblizeGCRData(denibblizeBufferPtr,
						sectorBufferPtr,
			       DriveStatus->diskFormat.logicalSectorSize,
						&computedChecksum);

	DenibblizeGCRChecksum(denibblizeBufferPtr,
			      &realChecksum);

	/*
	 * If we didn't get any errors transfering the sector data,
	 * check the data CRC, and if the calculated CRC didn't
	 * match the one from the diskette, return an error.
	 */

	if (realChecksum != computedChecksum)
	    errorCode = RecordError(badDCksum);

	/*
	 * Copy the denibblize the Tag data if we've been given  a place to put it. Otherwise,
	 * just bit-bucket the Tag data. MacOS 7.5 would normaly put this data in a lowMem
	 * variable if we didn't have a client address. In this driver's case, it is the task
	 * of the SAL interface to support lowMem variables.
	 */

	if (DriveStatus->tagDataAddress != 0) {

	    TagDenibbleBufferPtr = (lowMemTagDataType *) DriveStatus->tagDataAddress;

	    TagDenibbleBufferPtr->sectorNum = localTagSectorNum;
	    TagDenibbleBufferPtr->defaultTagData.TagItems.BufTgFNum = localTagDenibbleBufferPtr->TagItems.BufTgFNum;
	    TagDenibbleBufferPtr->defaultTagData.TagItems.BufTgFFlg = localTagDenibbleBufferPtr->TagItems.BufTgFFlg;
	    TagDenibbleBufferPtr->defaultTagData.TagItems.BufTgFBkNum = localTagDenibbleBufferPtr->TagItems.BufTgFBkNum;
	    TagDenibbleBufferPtr->defaultTagData.TagItems.BufTgDate = localTagDenibbleBufferPtr->TagItems.BufTgDate;

	}
	/*
	 * Flag this sector as being denibblized before returning back to the caller.
	 */

	SetBit(DriveStatus->sectorDenibblized[DriveStatus->wantedAddress.address.items.side],
	       DriveStatus->wantedAddress.address.items.sector);

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : NibblizeGCRData
*
*	Given a pointer to a buffer of denibblized data, encode it into its
*	nibblized values, plus calculate the running checksum. 
*
************************************************************************/

unsigned char *
 NibblizeGCRData(unsigned char *sourceAddress,
		 unsigned char *destAddress,
		 unsigned short transferLength,
		 unsigned long *initalChecksum)
{

    unsigned char *topBitsAddress;
    register unsigned char topBitsByte;
    register unsigned char topBitsMask;
    register unsigned short ChecksumA;
    register unsigned short ChecksumB;
    register unsigned short ChecksumC;
    register unsigned short Checksum_Carry;
    register unsigned short Byte_mask;
    unsigned char byteValueA;
    unsigned char byteValueB;
    unsigned char byteValueC;


    /*
     * Start off using the checksum values that have been accumulated
     * so far.
     */

    Byte_mask = 0xFF;

    ChecksumA = (*initalChecksum >> 16) & Byte_mask;
    ChecksumB = (*initalChecksum >> 8) & Byte_mask;
    ChecksumC = *initalChecksum & Byte_mask;

    /*
     * Transfer the sector data that is whole units of four nibble-bytes groups
     * to three real bytes.
     */

    topBitsMask = 0xC0;

    Checksum_Carry = 0;

    while (transferLength > 0) {

	/*
	 * Remember the address of where we need to put the top bits
	 * byte for this encode group, and bump the pointer to point
	 * to the byte A location.
	 */

	topBitsAddress = destAddress++;

	/*
	 * Update checksum A with the value of byte A and the
	 * carry from checksum C, then encode the byte A value
	 * with checksum C. Seperate checksum A's carry from
	 * update calculation.
	 */

	ChecksumA = ChecksumA + *sourceAddress + Checksum_Carry;
	byteValueA = *sourceAddress++ ^ ChecksumC;

	Checksum_Carry = ChecksumA >> 8;
	ChecksumA = ChecksumA & Byte_mask;

	/*
	 * Remember byte A's top two bits and nibblize the bottom six
	 * bits. Include byte A in the total count.
	 */

	topBitsByte = (byteValueA & topBitsMask) >> 2;

	*destAddress++ = byteValueA & ~topBitsMask;

	transferLength--;

	/*
	 * Update checksum B with the value of byte B and the
	 * carry from checksum A, then encode the byte B value
	 * with checksum A. Seperate checksum A's carry from
	 * update calculation.
	 */

	ChecksumB = ChecksumB + *sourceAddress + Checksum_Carry;
	byteValueB = *sourceAddress++ ^ ChecksumA;

	Checksum_Carry = ChecksumB >> 8;
	ChecksumB = ChecksumB & Byte_mask;

	/*
	 * Remember byte B's top two bits and nibblize the bottom six
	 * bits. Include byte B in the total count.
	 */

	topBitsByte |= (byteValueB & topBitsMask) >> 4;

	*destAddress++ = byteValueB & ~topBitsMask;

	transferLength--;

	/*
	 * This is a little bit of a hack. Since we know that with 512
	 * byte sectors that we will be one byte short of a even multible
	 * of three, so test for the length counter going to zero before
	 * we try to do byte C. This allows us to bail out at the proper
	 * time. Since it is highly unlikly that GCR diskettes will ever
	 * have a sector size other than 512, this should work forever.
	 */

	if (transferLength > 0) {

	    /*
	     * Update checksum C with the value of byte C and the
	     * carry from checksum B, then encode the byte C value
	     * with checksum B. Seperate checksum C's carry from
	     * update calculation.
	     */

	    ChecksumC = ChecksumC + *sourceAddress + Checksum_Carry;
	    byteValueC = *sourceAddress++ ^ ChecksumB;

	    Checksum_Carry = ChecksumC >> 8;
	    ChecksumC = ChecksumC & Byte_mask;

	    /*
	     * Remember byte C's top two bits and nibblize the bottom six
	     * bits. Include byte C in the total count.
	     */

	    topBitsByte |= (byteValueC & topBitsMask) >> 6;

	    *destAddress++ = byteValueC & ~topBitsMask;

	    transferLength--;

	    /*
	     * Rotate the top bit of checksum C around to bit zero
	     * and remember it as checksum C's carry.
	     */

	    Checksum_Carry = ChecksumC >> 7;
	    ChecksumC = (ChecksumC << 1) | Checksum_Carry;

	}
	/*
	 * Nibblize the top bits byte and put it in its place ahead
	 * of byte A, byte B and byte C.
	 */

	*topBitsAddress = topBitsByte;

    }

    /*
     * Return the accumulated checksum value and the updated data address.
     */

    *initalChecksum = ChecksumA & Byte_mask;
    *initalChecksum = (*initalChecksum << 8) | (ChecksumB & Byte_mask);
    *initalChecksum = (*initalChecksum << 8) | (ChecksumC & Byte_mask);

    return destAddress;

}


/************************************************************************
*
*	FUNCTION : NibblizeGCRChecksum
*
*	Encode denibblized GCR checksum data from the checksum's real value.
*
************************************************************************/

unsigned char *
 NibblizeGCRChecksum(unsigned char *destAddress,
		     unsigned long realChecksum)
{

    register unsigned char topBitsByte;
    register unsigned char topBitsMask;
    register unsigned char ChecksumA;
    register unsigned char ChecksumB;
    register unsigned char ChecksumC;
    register unsigned char Byte_mask;
    register unsigned char *topBitsAddress;


    /*
     * Setup the high bits bit mask.
     */

    topBitsMask = 0xC0;

    /*
     * Start off by seperating the checksum values into their individual
     * pieces.
     */

    Byte_mask = 0xFF;

    ChecksumA = (realChecksum >> 16) & Byte_mask;
    ChecksumB = (realChecksum >> 8) & Byte_mask;
    ChecksumC = realChecksum & Byte_mask;

    /*
     * Remember the address of where we need to put the top bits
     * byte for this encode group, and bump the pointer to point
     * to the byte A location.
     */

    topBitsAddress = destAddress++;

    /*
     * Remember byte A's top two bits and nibblize the bottom six
     * bits. Include byte A to the total count.
     */

    topBitsByte = (ChecksumA & topBitsMask) >> 2;

    *destAddress++ = ChecksumA & ~topBitsMask;

    /*
     * Remember byte B's top two bits and nibblize the bottom six
     * bits. Include byte B to the total count.
     */

    topBitsByte |= (ChecksumB & topBitsMask) >> 4;

    *destAddress++ = ChecksumB & ~topBitsMask;

    /*
     * Remember byte C's top two bits and nibblize the bottom six
     * bits. Include byte C to the total count.
     */

    topBitsByte |= (ChecksumC & topBitsMask) >> 6;

    *destAddress++ = ChecksumC & ~topBitsMask;

    /*
     * Nibblize the top bits byte and put it in its place ahead
     * of byte A, byte B and byte C.
     */

    *topBitsAddress = topBitsByte;

    /*
     * Return the updated destination pointer.
     */

    return destAddress;

}


/************************************************************************
*
*	FUNCTION : FPYNibblizeGCRSector
*
*	Given a pointer to a buffer of denibblized sector data, encode it into
*	its nibblized sector and tag data values. 
*
************************************************************************/

OSStatus
FPYNibblizeGCRSector(DriveStatusType * DriveStatus,
		     unsigned char *nibblizeBufferPtr,
		     unsigned char *sectorBufferPtr)
{

    short errorCode;
    short index;
    TagDataType *TagNibblizeBufferPtr;
    unsigned long computedChecksum;
    unsigned char *shiftedSectorDestinationPtr;
    unsigned char *shiftedSectorSourcePtr;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Test to see if the GCR sector needs to be nibblized.
     */

    if (TestBit(DriveStatus->sectorDenibblized[DriveStatus->wantedAddress.address.items.side],
		DriveStatus->wantedAddress.address.items.sector)) {

	/*
	 * The first byte in the data field is the sector number of this sector.
	 */

	*nibblizeBufferPtr = DriveStatus->wantedAddress.address.items.sector;

	nibblizeBufferPtr++;

	/*
	 * Copy and shift the denibblized data down against the bottom of the nibblized data
	 * field in the destination buffer so that we can nibblize the data in-place in the
	 * destination buffer. Copy the data first, before nibblizing, and from the bottom of
	 * the denibblized data up in case the nibblize and denibblize data buffers overlap.
	 *
	 * Leave the destination pointer at the first byte of copied tag data.
	 */

	shiftedSectorSourcePtr = sectorBufferPtr + DriveStatus->diskFormat.logicalSectorSize - 1;

	shiftedSectorDestinationPtr = nibblizeBufferPtr + (GCRNibblizedTagDataSize + GCRNibblizedSectorsSize) - 1;

	for (index = (DriveStatus->diskFormat.logicalSectorSize + GCRTagDataSize); index > 0; index--)
	    *shiftedSectorDestinationPtr-- = *shiftedSectorSourcePtr--;

	shiftedSectorDestinationPtr++;

	/*
	 * Nibblize the Tag data if we have a place to put it. Otherwise,
	 * just use what ever Tag data was already there.
	 */

	if (DriveStatus->tagDataAddress != 0)
	    TagNibblizeBufferPtr = (TagDataType *) DriveStatus->tagDataAddress;
	else
	    TagNibblizeBufferPtr = (TagDataType *) shiftedSectorDestinationPtr;

	/*
	 * Adjust the denibblized sector data pointer past the tag to the begining of
	 * the sector data.
	 */

	shiftedSectorDestinationPtr += GCRTagDataSize;

	/*
	 * Output the tag data followed by the sector data, while computing the
	 * the sector data checksum, then output the computed checksum.
	 */

	computedChecksum = 0;

	nibblizeBufferPtr = NibblizeGCRData((unsigned char *) TagNibblizeBufferPtr->TagBytes,
					    nibblizeBufferPtr,
					    sizeof(TagDataType),
					    &computedChecksum);

	nibblizeBufferPtr = NibblizeGCRData(shiftedSectorDestinationPtr,
					    nibblizeBufferPtr,
			       DriveStatus->diskFormat.logicalSectorSize,
					    &computedChecksum);

	nibblizeBufferPtr = NibblizeGCRChecksum(nibblizeBufferPtr,
						computedChecksum);

	/*
	 * Flag this sector as being nibblized before returning back to the caller.
	 */

	ClearBit(DriveStatus->sectorDenibblized[DriveStatus->wantedAddress.address.items.side],
		 DriveStatus->wantedAddress.address.items.sector);

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : RecalDrive
*
*	Seek the currently selected drive to an absolute track number 0.
*	An error is returned if the drive fails to accept a step request. 
*
************************************************************************/

OSStatus
RecalDrive(DriveStatusType * DriveStatus)
{

    short errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Dump any track cache data we might have had for this drive.
     */
    DumpTrackCache(DriveStatus);

    /*
     * Make sure that the sectors/track field is set for this track
     * (this mostly for GCR since it has variable sectors/track).
     */

    DriveStatus->wantedAddress.address.items.track = 0;

    printf("core.c:RecalDrive:calling SetSectorsPerTrack ");
    SetSectorsPerTrack(DriveStatus);

    /*
     * Recalibrate the drive head and update the drive's current
     * track status. Otherwise, invalidate the current track location.
     */

    printf("core.c:RecalDrive:calling HALRecalDrive ");
    if ((errorCode = HALRecalDrive(DriveStatus)) == noErr) {

	DriveStatus->currentAddress.address.items.track = DriveStatus->wantedAddress.address.items.track;

	SetCacheAddresses(DriveStatus);

    } else
	DriveStatus->currentAddress.address.items.track = -1;

    printf("core.c:RecalDrive:ret=%d ", errorCode);
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : SeekDrive
*
*	Seek the currently selected drive to an absolute track number (0 - 79).
*	An error is returned if the drive fails to accept a step request.
*	Also, if something went wrong with the seek, the current track for
*	the drive is set to -1 to indicate that the current track is unknown. 
*
************************************************************************/

OSStatus
SeekDrive(DriveStatusType * DriveStatus)
{

    short errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * If the current track is unknown, recalibrate drive first.
     */
    printf("core.c:SeekDrive:track=%d ", DriveStatus->currentAddress.address.items.track);
    if (DriveStatus->currentAddress.address.items.track == -1)
	errorCode = RecalDrive(DriveStatus);

    /*
     * If we're already at the target track or we had a recalibrate
     * error, just return.
     */

    if ((errorCode == noErr) &&
	(DriveStatus->currentAddress.address.items.track != DriveStatus->wantedAddress.address.items.track)) {

	/*
	 * Make sure that the sectors/track field is set for this track
	 * (this mostly for GCR since it has variable sectors/track).
	 */

	SetSectorsPerTrack(DriveStatus);

	/*
	 * Seek the drive from where we are to where we want to be and if
	 * successful, update the drive's current track status. Otherwise,
	 * invalidate the current track location.
	 */
	printf("SeekDrive:calling HALSeekDrive ");
	if ((errorCode = HALSeekDrive(DriveStatus)) == noErr) {

	    DriveStatus->currentAddress.address.items.track = DriveStatus->wantedAddress.address.items.track;

	    SetCacheAddresses(DriveStatus);

	} else
	    DriveStatus->currentAddress.address.items.track = -1;

    }
    return errorCode;

}


/************************************************************************
*
* FUNCTION : FlushCacheAndSeek
*
************************************************************************/

OSStatus
FlushCacheAndSeek(DriveStatusType * DriveStatus)
{

    register short errorCode;

    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Test to see if we're going to move to different track.
     */

    if (DriveStatus->currentAddress.address.items.track != DriveStatus->wantedAddress.address.items.track) {

	/*
	 * First we need to flush out any "write" data.
	 */
	if (TestCacheDirtyState(DriveStatus))
	    FlushTrackCache(DriveStatus);

	/*
	 * Dump any track cache data we might have had for this drive.
	 */

	DumpTrackCache(DriveStatus);

	/*
	 * Seek the head to the correct track.
	 */

	errorCode = SeekDrive(DriveStatus);

    }
    return errorCode;

}

#endif // NFD > 0
