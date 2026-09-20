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
*	File:		SwimIIICommonHAL.c
*
*	Contains:	Floppy driver HAL routines for SWIMIII on all machines.
*
************************************************************************/
//#include <DriverSupport.h>
#include <types.h>
#include <kern/spl.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include <sys/ioctl.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#include <fd.h>

#if NFD > 0

#include "portdef.h"
#include "floppypriv.h"
#include "floppysal.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "swimiii.h"
#include "floppy_amic.h"
#include <ppc/proc_reg.h>
#include <ppc/POWERMAC/dbdma.h>

#include "swimiiicommonhal.h"
//#define printf donone
/************************************************************************
*
* Private data for HAL.
*
************************************************************************/

SwimIIIRegs FloppySWIMIIIRegs;	/* Floppy controller register sets */

unsigned char lastErrorsPending;

unsigned char lastSectorsPerTrack;

UInt32 *driveOSEventIDptr;	// This is also referenced in grcswimiiihal.c as extern


/************************************************************************
*
*	FUNCTION : SleepUntilReady
*
*	Sleep for the time period specifed in milliseconds then scan in
*	one millisecond intervals waiting for the currently selected drive's
*	/READY signal to become asserted. If we timeout after waiting one
*	second, return a -1.
*
************************************************************************/

OSStatus
SleepUntilReady(long PreScanTimeout)
{

    short ScanCount;
    OSStatus errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Wait a little before we start scanning.
     */

    FloppyTimedSleep(PreScanTimeout);

    /*
     * For a max of one second, loop looking for the drive's
     * /READY signal to become asserted.
     */

    ScanCount = 1000;

    while (ScanCount--) {

	/*
	 * If the drive is NOT ready, sleep for a millisecond.
	 * Otherwise, break out of the loop.
	 */

	if (SwimIIISenseSignal(S_NOTREADY)) {

	    FloppyTimedSleep(1);

	} else
	    break;

    }

    /*
     * If we reached the end of the loop without seeing the drive
     * go ready, return an error.
     */

    if (ScanCount == 0)
	errorCode = RecordError(-1);

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : WaitForEvent
*
*	Wait for the wanted interrupt/event to occur and return true, or
*	return false if we timed out waiting for the wanted interrupt/event.
*
************************************************************************/

OSStatus
WaitForEvent(unsigned long milliseconds,
	     unsigned char actionBitMask,
	     unsigned char interruptMask)
{

    OSStatus errorCode;
    UInt32 pendingInterrupts;
    int retval;

    /*
     * Prepare for the worst in case we time out.
     */

    errorCode = noNybErr;

    /*
     * Clear any pending interrupts and error status.
     */

    lastErrorsPending = 0;

    pendingInterrupts = *FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO();

    /*
     * Enable the interrupt we want and the master interrupt enable.
     * Finally, turn on the "action" the caller requested.
     */

    *FloppySWIMIIIRegs.rwInterruptMask = interruptMask;
    SynchronizeIO();

    *FloppySWIMIIIRegs.rHandshakewModeOnes = MS_ENABLE_INTS | actionBitMask;
    SynchronizeIO();

rewait:

    /*
     * Wait for either an interrupt or a timeout to occur.
     */
#ifdef OLD_WAIT
    if (WaitForOSEvent(driveOSEventIDptr,
		       interruptMask,
		       milliseconds,
		       (UInt32 *) & pendingInterrupts)) {
#else
    retval = WaitForOSEvent(driveOSEventIDptr,
		       interruptMask,
		       milliseconds,
		       (UInt32 *) & pendingInterrupts);
    {
#endif

	/*
	 * Check to see if any interrupts or errors occured.
	 */

	//printf("WaitForEvent: wait over pend intrs=%d,mask=%d ",pendingInterrupts,interruptMask); 
	if (pendingInterrupts != 0) {

	    /*
	     * Did no errors occur?
	     */

	    if (lastErrorsPending == 0) {

		/*
		 * Make sure the interrupt we wanted was the one that occured.
		 */

		if (pendingInterrupts & interruptMask) {

		    /*
		     * Now that we got our interrupt, turn off the "action" the caller requested.
		     */

		    *FloppySWIMIIIRegs.rModewModeZeroes = actionBitMask;
		    SynchronizeIO();

		    /*
		     * Return to the caller saying that the wanted interrupt
		     * occurred.
		     */

		    errorCode = noErr;
		} else {
		    unsigned char error; unsigned long errornum;

		    /* A Hardware ERROR Occurred.  Say SOMETHING! */

		    printf("Wrong Interrupt.  Assuming cross-wired bits.\n");

		    *FloppySWIMIIIRegs.rModewModeZeroes = actionBitMask;
		    SynchronizeIO();

		    *FloppySWIMIIIRegs.rModewModeZeroes = IT_SENSE_INPUT_CHANGE;
		    SynchronizeIO();

		    errorCode = noErr;
		}
	    } else {

		/*
		 * Since we got an error, turn off the "action" the caller requested.
		 */

		*FloppySWIMIIIRegs.rModewModeZeroes = actionBitMask;
		SynchronizeIO();

		/*
		 * Decode what kind of error we got and return it.
		 */

		if (lastErrorsPending & ER_CRC_ON_ADDRESS_FIELD) {

		    /*
		     * Return a "Bad Address Mark Checksum" error.
		     */

		    errorCode = badCksmErr;

		} else {

		    if (lastErrorsPending & ER_CRC_ON_DATA_FIELD) {

			/*
			 * Return a "Bad Data Mark Checksum" error.
			 */

			errorCode = badDCksum;

		    } else {

			if (lastErrorsPending & (ER_UNDERRUN | ER_OVERRUN)) {

			    /*
			     * Return a "Write Underrun/Overrun Occurred" error.
			     */

			    errorCode = wrUnderrun;

			} else {

			    /*
			     * Return a "I have no idea" error.
			     */

			    errorCode = noAdrMkErr;

			}

		    }

		}

	    }

	} else {
	    /* This could potentially be interrupted by something else.... */
	    goto rewait;
	}
    }
#ifdef OLD_WAIT
    else			// timeout case
#else
    if (!retval)
#endif
     {
	*FloppySWIMIIIRegs.rModewModeZeroes = actionBitMask;
	SynchronizeIO();
    }

    /*
     * Return to the caller saying that we timed out waiting for
     * the wanted interrupt.
     */
#if MACH_DEBUG
    printf("Wait For Event: ret=%d intr=%d", errorCode, pendingInterrupts);
#endif
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : ByteMove
*
*	Block move data bytes. Buffers can overlap forward.
*
************************************************************************/

void ByteMove(void *srcPtr,
	      void *destPtr,
	      long byteCount)
{

    BytePtr sourcePtr;
    BytePtr destinationPtr;


    sourcePtr = (BytePtr) srcPtr;
    destinationPtr = (BytePtr) destPtr;

    while (byteCount > 0) {

	*destinationPtr = *sourcePtr;

	destinationPtr += 1;

	sourcePtr += 1;

	byteCount -= 1;

    }

}


/************************************************************************
*
*	FUNCTION : FormatGCRCacheSWIMIIIData
*
*	Set up the track cache buffer with all of the GCR intra-sector SWIMIII
*	escaped data. 
*
************************************************************************/

void FormatGCRCacheSWIMIIIData(DriveStatusType * DriveStatus)
{

    BytePtr nextCacheByteAddr;
    uint_t sectorDataAlignment;
    uint_t sideIndex;
    uint_t sectorIndex;
    Byte AddressMarkCheckSum;
    rawSwimIIIGCRSectorFormat *nextSwimIIIGCRSector;

/* DAG ??? */
    // if (powermac_info.class == POWERMAC_CLASS_PCI)
/* naga */
	return;

    /*
     * Point to the begining of the track cache buffer.
     */
    nextCacheByteAddr = (BytePtr) DriveStatus->tcBuffer.logicalAddress;

    /*
     * Write the data for the number of sides.
     */

    sideIndex = 0;

    while (sideIndex < 2) {

	/*
	 * Are we only updating the Address Marks?
	 */

	if (lastSectorsPerTrack == DriveStatus->diskFormat.sectorsPerTrack) {

	    lastSectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;

	    /*
	     * Offset to the first sector.
	     */

	    nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat *) nextCacheByteAddr;

	    /*
	     * Write out the updated data for all of the sectors.
	     */

	    for (sectorIndex = 0; sectorIndex < DriveStatus->diskFormat.sectorsPerTrack; sectorIndex++) {

		/*
		 * Write out the Address Mark Field followed by the checksum byte and
		 * slip bytes terminater.
		 */

		AddressMarkCheckSum = 0;

		nextSwimIIIGCRSector->track = DriveStatus->wantedAddress.address.items.track & 0x3F;
		AddressMarkCheckSum ^= nextSwimIIIGCRSector->track;

		nextSwimIIIGCRSector->sector = sectorIndex;
		AddressMarkCheckSum ^= sectorIndex;

		nextSwimIIIGCRSector->side = (sideIndex << 5) | (DriveStatus->wantedAddress.address.items.track & ~0x3F);
		AddressMarkCheckSum ^= nextSwimIIIGCRSector->side;

		nextSwimIIIGCRSector->format = DriveStatus->wantedAddress.address.items.blockSize;
		AddressMarkCheckSum ^= nextSwimIIIGCRSector->format;

		nextSwimIIIGCRSector->checkSum = AddressMarkCheckSum;

		/*
		 * Offset to the next sector.
		 */

		nextSwimIIIGCRSector++;

	    }

	    /*
	     * Offset to the next side of sectors.
	     */

	    nextCacheByteAddr = (BytePtr) nextSwimIIIGCRSector + (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);

	} else {

	    /*
	     * Offset to the first sector.
	     */

	    nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat *) nextCacheByteAddr;

	    /*
	     * Write out the escaped data for all of the sectors.
	     */

	    for (sectorIndex = 0; sectorIndex < DriveStatus->diskFormat.sectorsPerTrack; sectorIndex++) {

		/*
		 * Make sure that the sector data address is rounded up to a DMA byte boundry
		 * for those DMA engines that has that restriction. Lengthen the write splice gap
		 * by the amount of DMA byte forward alignment to begin the sector data on a
		 * DMA byte boundry. Shorten inter-sector gap by the same amount so that Address
		 * Marks always stay at calculated addresses. Use the same alignment method as the
		 * upper floppy layers will use.
		 */

		sectorDataAlignment = 0;

		if (DriveStatus->DMAByteBoundryAlignment > 1)
		    sectorDataAlignment = (((unsigned long) &nextSwimIIIGCRSector->sectorNumber & ~(DriveStatus->DMAByteBoundryAlignment - 1)) + DriveStatus->DMAByteBoundryAlignment) - (unsigned long) &nextSwimIIIGCRSector->sectorNumber;

		/*
		 * Write out a sync byte field.
		 */

		ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextSwimIIIGCRSector->addressMarkSync,
			 sizeof(nextSwimIIIGCRSector->addressMarkSync));

		/*
		 * Now copy in the escaped Address Mark.
		 */

		ByteMove(SWIM3_GCR_ADDRESS_MARK,
			 nextSwimIIIGCRSector->addressMark,
			 sizeof(nextSwimIIIGCRSector->addressMark));

		/*
		 * Write out the Address Mark Field followed by the checksum byte and
		 * slip bytes terminater.
		 */

		AddressMarkCheckSum = 0;

		nextSwimIIIGCRSector->track = DriveStatus->wantedAddress.address.items.track & 0x3F;
		AddressMarkCheckSum ^= nextSwimIIIGCRSector->track;

		nextSwimIIIGCRSector->sector = sectorIndex;
		AddressMarkCheckSum ^= sectorIndex;

		nextSwimIIIGCRSector->side = (sideIndex << 5) | (DriveStatus->wantedAddress.address.items.track & ~0x3F);
		AddressMarkCheckSum ^= nextSwimIIIGCRSector->side;

		nextSwimIIIGCRSector->format = DriveStatus->wantedAddress.address.items.blockSize;
		AddressMarkCheckSum ^= nextSwimIIIGCRSector->format;

		nextSwimIIIGCRSector->checkSum = AddressMarkCheckSum;

		ByteMove(SWIM3_GCR_SLIP_BYTE,
			 nextSwimIIIGCRSector->addressMarkTerminator,
		    sizeof(nextSwimIIIGCRSector->addressMarkTerminator));

		/*
		 * Now output the write splice gap, made with self sync bytes that are
		 * lengthened to the DMA byte alignment adjustment.
		 */

		ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextSwimIIIGCRSector->writeSpliceGap,
			 sizeof(nextSwimIIIGCRSector->writeSpliceGap));

		ByteMove(nextSwimIIIGCRSector->writeSpliceGap,
			 nextSwimIIIGCRSector->writeSpliceGap + sizeof(nextSwimIIIGCRSector->writeSpliceGap),
			 sectorDataAlignment);

		nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat *) ((BytePtr) nextSwimIIIGCRSector + sectorDataAlignment);

		/*
		 * Write out a sync byte field.
		 */

		ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextSwimIIIGCRSector->dataMarkSync,
			 sizeof(nextSwimIIIGCRSector->dataMarkSync));

		/*
		 * Now copy in the escaped Data Mark.
		 */

		ByteMove(SWIM3_GCR_DATA_MARK,
			 nextSwimIIIGCRSector->dataMark,
			 sizeof(nextSwimIIIGCRSector->dataMark));

		/*
		 * Write out the tag data sector byte.
		 */

		nextSwimIIIGCRSector->sectorNumber = sectorIndex;

		/*
		 * Write out a blank set of tag data.
		 */

		*nextSwimIIIGCRSector->tagData = 0;

		ByteMove(nextSwimIIIGCRSector->tagData,
			 nextSwimIIIGCRSector->tagData + 1,
			 sizeof(nextSwimIIIGCRSector->tagData) - 1);

		/*
		 * Write out the data field followed by room for nibblization expansion
		 * and data field checksum, then write out the slip bytes terminater.
		 */

		*nextSwimIIIGCRSector->dataField = DriveStatus->formatByte;

		ByteMove(nextSwimIIIGCRSector->dataField,
			 nextSwimIIIGCRSector->dataField + 1,
			 sizeof(nextSwimIIIGCRSector->dataField) - 1);

		ByteMove(SWIM3_GCR_SLIP_BYTE,
			 nextSwimIIIGCRSector->dataFieldTerminator,
		      sizeof(nextSwimIIIGCRSector->dataFieldTerminator));

		/*
		 * Finish up the sector field by writing out the inter-sector gap with DMA byte
		 * alignment adjustments.
		 */

		ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextSwimIIIGCRSector->interSectorGap,
			 sizeof(SWIM3_GCR_SELF_SYNC) - 1);

		ByteMove(nextSwimIIIGCRSector->interSectorGap,
			 nextSwimIIIGCRSector->interSectorGap + (sizeof(SWIM3_GCR_SELF_SYNC) - 1),
			 sizeof(nextSwimIIIGCRSector->interSectorGap) - (sizeof(SWIM3_GCR_SELF_SYNC) - 1) - sectorDataAlignment);

		/*
		 * Offset to the next sector and back up for the DMA byte alignment adjustment.
		 */

		nextSwimIIIGCRSector++;

		nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat *) ((BytePtr) nextSwimIIIGCRSector - sectorDataAlignment);

	    }

	    /*
	     * Offset to the next side of sectors.
	     */

	    nextCacheByteAddr = (BytePtr) nextSwimIIIGCRSector;

	    /*
	     * Lastly, write out the "End Of Transfer" command at the end.
	     */

	    ByteMove(SWIM3_TERMINATE_DMA_TRANSFER,
		     nextCacheByteAddr,
		     (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1));

	    nextCacheByteAddr += (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);

	}

	/*
	 * Go do the other side if double sided. Otherwise, terminate the sides loop.
	 */

	if (DriveStatus->doubleSided == false)
	    sideIndex = 3;
	else
	    sideIndex += 1;

    }

}


/************************************************************************
*
*	FUNCTION : FormatMFMCacheSWIMIIIData
*
*	Set up the track cache buffer with all of the MFM intra-sector SWIMIII
*	escaped data. 
*
************************************************************************/

void FormatMFMCacheSWIMIIIData(DriveStatusType * DriveStatus)
{

    BytePtr nextCacheByteAddr;
    uint_t tempSectorDataCacheAddr;
    uint_t sectorDataAlignment;
    uint_t sideIndex;
    uint_t sectorIndex;

    /*
     * Point to the begining of the track cache buffer.
     */
    nextCacheByteAddr = (BytePtr) DriveStatus->tcBuffer.logicalAddress;

    /*
     * Write the data for the number of sides.
     */

    sideIndex = 0;

    while (sideIndex < 2) {

	/*
	 * Are we only updating the Address Marks?
	 */

	if (lastSectorsPerTrack == DriveStatus->diskFormat.sectorsPerTrack) {

	    lastSectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;

	    /*
	     * Offset to the first sector.
	     */

	    nextCacheByteAddr += DriveStatus->diskFormat.gap4ALength +
		SWIM3_MFM_MARK_SYNC_GAP_LEN +
		(sizeof(SWIM3_MFM_INDEX_MARK) - 1) +
		DriveStatus->diskFormat.gap1Length;

	    /*
	     * Write out the updated data for all of the sectors.
	     */

	    for (sectorIndex = 1; sectorIndex <= DriveStatus->diskFormat.sectorsPerTrack; sectorIndex++) {

		/*
		 * Offset to the sector's Address Mark fields.
		 */

		nextCacheByteAddr += SWIM3_MFM_MARK_SYNC_GAP_LEN +
		    (sizeof(SWIM3_MFM_ADDRESS_MARK) - 1);

		/*
		 * Write out the Address Mark Field followed by the "Write CRC bytes" command.
		 */

		*nextCacheByteAddr++ = DriveStatus->wantedAddress.address.items.track;
		*nextCacheByteAddr++ = sideIndex;
		*nextCacheByteAddr++ = sectorIndex;
		*nextCacheByteAddr++ = DriveStatus->wantedAddress.address.items.blockSize;

		/*
		 * Offset to the next sector.
		 */

		nextCacheByteAddr += (sizeof(SWIM3_WRITE_MFM_CRC) - 1) +
		    DriveStatus->diskFormat.gap2Length +
		    SWIM3_MFM_MARK_SYNC_GAP_LEN +
		    (sizeof(SWIM3_MFM_DATA_MARK) - 1) +
		    (sizeof(SWIM3_DISABLE_DATA_ESCAPING) - 1) +
		    DriveStatus->diskFormat.logicalSectorSize +
		    (sizeof(SWIM3_WRITE_MFM_CRC) - 1) +
		    DriveStatus->diskFormat.gap3Length;

	    }

	    /*
	     * Offset to the next side of sectors.
	     */

	    nextCacheByteAddr += DriveStatus->diskFormat.gap4BLength +
		(sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);

	} else {

	    /*
	     * Start building the Index Mark with the gap4A of 4Es.
	     */

	    *nextCacheByteAddr = 0x4E;

	    ByteMove(nextCacheByteAddr,
		     nextCacheByteAddr + 1,
		     DriveStatus->diskFormat.gap4ALength - 1);

	    nextCacheByteAddr += DriveStatus->diskFormat.gap4ALength;

	    /*
	     * Write out a sync byte field.
	     */

	    *nextCacheByteAddr = 0x00;

	    ByteMove(nextCacheByteAddr,
		     nextCacheByteAddr + 1,
		     SWIM3_MFM_MARK_SYNC_GAP_LEN - 1);

	    nextCacheByteAddr += SWIM3_MFM_MARK_SYNC_GAP_LEN;

	    /*
	     * Now copy in the escaped Index Mark.
	     */

	    ByteMove(SWIM3_MFM_INDEX_MARK,
		     nextCacheByteAddr,
		     (sizeof(SWIM3_MFM_INDEX_MARK) - 1));

	    nextCacheByteAddr += (sizeof(SWIM3_MFM_INDEX_MARK) - 1);

	    /*
	     * Finish up the Index Mark the gap1 of 4Es.
	     */

	    *nextCacheByteAddr = 0x4E;

	    ByteMove(nextCacheByteAddr,
		     nextCacheByteAddr + 1,
		     DriveStatus->diskFormat.gap1Length - 1);

	    nextCacheByteAddr += DriveStatus->diskFormat.gap1Length;

	    /*
	     * Write out the escaped data for all of the sectors.
	     */

	    for (sectorIndex = 1; sectorIndex <= DriveStatus->diskFormat.sectorsPerTrack; sectorIndex++) {

		/*
		 * Make sure that the sector data address is rounded up to a DMA byte boundry
		 * for those DMA engines that has that restriction. Lengthen gap2 by the amount
		 * of DMA byte forward alignment to begin the sector data on a DMA byte
		 * boundry. Shorten gap3 by the same amount so that Address Marks always stay at
		 * calculated addresses. Use the same alignment method as the upper floppy layers
		 * will use.
		 */

		tempSectorDataCacheAddr = (uint_t) nextCacheByteAddr + DriveStatus->preSectorCachePadding;

		sectorDataAlignment = 0;

		if (DriveStatus->DMAByteBoundryAlignment > 1)
		    sectorDataAlignment = ((tempSectorDataCacheAddr & ~(DriveStatus->DMAByteBoundryAlignment - 1)) + DriveStatus->DMAByteBoundryAlignment) - tempSectorDataCacheAddr;

		/*
		 * Write out a sync byte field.
		 */

		*nextCacheByteAddr = 0x00;

		ByteMove(nextCacheByteAddr,
			 nextCacheByteAddr + 1,
			 SWIM3_MFM_MARK_SYNC_GAP_LEN - 1);

		nextCacheByteAddr += SWIM3_MFM_MARK_SYNC_GAP_LEN;

		/*
		 * Now copy in the escaped Address Mark.
		 */

		ByteMove(SWIM3_MFM_ADDRESS_MARK,
			 nextCacheByteAddr,
			 (sizeof(SWIM3_MFM_ADDRESS_MARK) - 1));

		nextCacheByteAddr += (sizeof(SWIM3_MFM_ADDRESS_MARK) - 1);

		/*
		 * Write out the Address Mark Field followed by the "Write CRC bytes" command.
		 */

		*nextCacheByteAddr++ = DriveStatus->wantedAddress.address.items.track;
		*nextCacheByteAddr++ = sideIndex;
		*nextCacheByteAddr++ = sectorIndex;
		*nextCacheByteAddr++ = DriveStatus->wantedAddress.address.items.blockSize;

		ByteMove(SWIM3_WRITE_MFM_CRC,
			 nextCacheByteAddr,
			 (sizeof(SWIM3_WRITE_MFM_CRC) - 1));

		nextCacheByteAddr += (sizeof(SWIM3_WRITE_MFM_CRC) - 1);

		/*
		 * Finish up the Address Mark with gap2 of 4Es with DMA byte
		 * alignment adjustments.
		 */

		*nextCacheByteAddr = 0x4E;

		ByteMove(nextCacheByteAddr,
			 nextCacheByteAddr + 1,
			 DriveStatus->diskFormat.gap2Length - 1 + sectorDataAlignment);

		nextCacheByteAddr += DriveStatus->diskFormat.gap2Length + sectorDataAlignment;

		/*
		 * Write out a sync byte field.
		 */

		*nextCacheByteAddr = 0x00;

		ByteMove(nextCacheByteAddr,
			 nextCacheByteAddr + 1,
			 SWIM3_MFM_MARK_SYNC_GAP_LEN - 1);

		nextCacheByteAddr += SWIM3_MFM_MARK_SYNC_GAP_LEN;

		/*
		 * Now copy in the escaped Data Mark.
		 */

		ByteMove(SWIM3_MFM_DATA_MARK,
			 nextCacheByteAddr,
			 (sizeof(SWIM3_MFM_DATA_MARK) - 1));

		nextCacheByteAddr += (sizeof(SWIM3_MFM_DATA_MARK) - 1);

		/*
		 * Copy out the "Disable Escaping for 512 Bytes" command.
		 */

		ByteMove(SWIM3_DISABLE_DATA_ESCAPING,
			 nextCacheByteAddr,
			 (sizeof(SWIM3_DISABLE_DATA_ESCAPING) - 1));

		nextCacheByteAddr += (sizeof(SWIM3_DISABLE_DATA_ESCAPING) - 1);

		/*
		 * Write out the data field followed by the "Write CRC bytes" command.
		 */

		*nextCacheByteAddr = DriveStatus->formatByte;

		ByteMove(nextCacheByteAddr,
			 nextCacheByteAddr + 1,
			 DriveStatus->diskFormat.logicalSectorSize - 1);

		nextCacheByteAddr += DriveStatus->diskFormat.logicalSectorSize;

		ByteMove(SWIM3_WRITE_MFM_CRC,
			 nextCacheByteAddr,
			 (sizeof(SWIM3_WRITE_MFM_CRC) - 1));

		nextCacheByteAddr += (sizeof(SWIM3_WRITE_MFM_CRC) - 1);

		/*
		 * Finish up the sector field by writing out the gap3 4Es with DMA byte
		 * alignment adjustments.
		 */

		*nextCacheByteAddr = 0x4E;

		ByteMove(nextCacheByteAddr,
			 nextCacheByteAddr + 1,
			 DriveStatus->diskFormat.gap3Length - 1 - sectorDataAlignment);

		nextCacheByteAddr += DriveStatus->diskFormat.gap3Length - sectorDataAlignment;

	    }

	    /*
	     * Lastly, write out the trailing gap4B with an "End Of Transfer" command
	     * at the end.
	     */

	    *nextCacheByteAddr = 0x4E;

	    ByteMove(nextCacheByteAddr,
		     nextCacheByteAddr + 1,
		     DriveStatus->diskFormat.gap4BLength - 1);

	    nextCacheByteAddr += DriveStatus->diskFormat.gap4BLength;

	    ByteMove(SWIM3_TERMINATE_DMA_TRANSFER,
		     nextCacheByteAddr,
		     (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1));

	    nextCacheByteAddr += (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);

	}

	/*
	 * Go do the other side if double sided. Otherwise, terminate the sides loop.
	 */

	if (DriveStatus->doubleSided == false)
	    sideIndex = 3;
	else
	    sideIndex += 1;

    }

}


/************************************************************************
*
*	FUNCTION : SwimIIISmallWait
*
*	Provide a consistant timer for small time delays, i.e. 1 - 254
*	microseconds.
*
************************************************************************/

void SwimIIISmallWait(unsigned short microseconds)
{

    /*
     * Set the SWIMBASE's microsecond timer to the specified delay
     * plus one to make sure that the time out is at least one
     * microsecond long.
     */

    *FloppySWIMIIIRegs.rwTimer = microseconds + 1;
    SynchronizeIO();

    /*
     * Wait for the time delay to expire.
     */
    while (*FloppySWIMIIIRegs.rwTimer != 0)
	SynchronizeIO();

}


/************************************************************************
*
*	FUNCTION : SwimIIITimeOut
*
*	Provide a scanned timeout timer for timing polled operations.
*
************************************************************************/

#define MilliSecondsBetweenScans	( kDurationMillisecond * 5 )

boolean_t
SwimIIITimeOut(unsigned long *milliseconds)
{

    /*
     * Have we used up all of the requested timeout time?
     */

    if (*milliseconds == 0) {

	/*
	 * Tell the caller that the entire time has timed out.
	 */

	return true;

    } else {

	/*
	 * Is the time remaining less than the minimum scan time?
	 */

	if (*milliseconds <= MilliSecondsBetweenScans) {

	    /*
	     * Set the remaining time in the hardware timer and zero the
	     * remaining timeout time.
	     */

	    FloppyTimedSleep(MilliSecondsBetweenScans);

	    *milliseconds = 0;

	} else {

	    /*
	     * Set the hardware timer for the next time interval and subtract
	     * that time interval from the remaining timeout time.
	     */

	    FloppyTimedSleep(MilliSecondsBetweenScans);

	    *milliseconds -= MilliSecondsBetweenScans;

	}

    }

    /*
     * Tell the caller that the timed out time is still ticking.
     */

    return false;

}


/************************************************************************
*
*	FUNCTION : SwimIIIAddrSignal
*
*	Set the drive's address lines for the status/command bits through
*	the SWIMBASE's Phase lines and HDSEL signal. Signal correlation is:
*
*		Drive signal	SWIMBASE signal
*			CA0				PH0
*			CA1				PH1
*			CA2				PH2
*			LSTRB			PH3
*			SEL				HDSEL
*
************************************************************************/

void SwimIIIAddrSignal(unsigned char stat_cmd)
{

    /*
       * They say that CA0 and CA1 must be high before SEL goes to 0.
     */

    *FloppySWIMIIIRegs.rwPhase = (M_CA1 | M_CA0 | M_PHxOUTPUT);
    SynchronizeIO();

    /*
     * Test to see how the drive's SEL line should be set, Then
     * do it through the HDSEL on the SWIMBASE.
     */

    if (stat_cmd & M_SEL) {

	*FloppySWIMIIIRegs.rHandshakewModeOnes = MS_SIDE1_NOT_SIDE0;

    } else {

	*FloppySWIMIIIRegs.rModewModeZeroes = MS_SIDE1_NOT_SIDE0;

    }

    SynchronizeIO();

    /*
     * Now set the SWIMBASE's Phase lines to address the drive's
     * status/command bits (get rid of the SEL signal state,
     * as here it would toogle the SWIMBASE's PH3 line and the
     * drive's LSTRB line.
     */
    *FloppySWIMIIIRegs.rwPhase = stat_cmd & ~M_SEL;
    SynchronizeIO();

}


/************************************************************************
*
*	FUNCTION : SwimIIISenseSignal
*
*	Test one of the drives status bits.
*
************************************************************************/

boolean_t
SwimIIISenseSignal(unsigned char status)
{

    register boolean_t returnResult;


    /*
       * Address to the correct status bit.
     */
    SwimIIIAddrSignal(status);

    /*
     * Test to see how the drive responded.
     */

    returnResult = (*FloppySWIMIIIRegs.rHandshakewModeOnes & HR_SENSE_IN) != 0;
    SynchronizeIO();

    return returnResult;

}


/************************************************************************
*
*	FUNCTION : SwimIIISetSignal
*
*	Test one of the drives status bits.
*
************************************************************************/

void SwimIIISetSignal(unsigned char command)
{

    /*
       * Address to the correct command bit.
     */

    SwimIIIAddrSignal(command);

    /*
     * Toogle the LSTRB signal. Wait in between bit changes so
     * that the pulse is exactly 1 microsecond wide
     */

    *FloppySWIMIIIRegs.rwPhase = *FloppySWIMIIIRegs.rwPhase | M_LSTRB;
    SynchronizeIO();
    SwimIIISmallWait(1);

    *FloppySWIMIIIRegs.rwPhase = *FloppySWIMIIIRegs.rwPhase & ~M_LSTRB;
    SynchronizeIO();

}


/************************************************************************
*
*	FUNCTION : SwimIIIHeadSelect
*
*	Make one of the disk drives the currently selected one.
*
************************************************************************/

void SwimIIIHeadSelect(short HeadNumber)
{

    /*
     * Select the correct head.
     */

    if (HeadNumber == 0) {

	(void) SwimIIISenseSignal(S_RDDATA0);

    } else
	(void) SwimIIISenseSignal(S_RDDATA1);

}


/************************************************************************
*
*	FUNCTION : SwimIIIDiskSelect
*
*	Make one of the disk drives the currently selected one.
*
************************************************************************/

void SwimIIIDiskSelect(DriveStatusType * DriveStatus)
{

    /*
     * Is it the internal drive?
     */

    if (DriveStatus->driveNumber == 0) {

	/*
	 * De-select the external drive and select and enable
	 * the internal drive.
	 */

	*FloppySWIMIIIRegs.rModewModeZeroes = MS_ENABLE_DRIVE_2;
	SynchronizeIO();

	*FloppySWIMIIIRegs.rHandshakewModeOnes = MS_ENABLE_DRIVE_1;
	SynchronizeIO();

    } else {

	/*
	 * De-select the internal drive and select and enable
	 * the external drive.
	 */

	*FloppySWIMIIIRegs.rModewModeZeroes = MS_ENABLE_DRIVE_1;
	SynchronizeIO();

	*FloppySWIMIIIRegs.rHandshakewModeOnes = MS_ENABLE_DRIVE_2;
	SynchronizeIO();

    }

}


/************************************************************************
*
*	FUNCTION : SwimIIIStepDrive
*
*	Step, the currently selected drive, the a relative number of positive
*	or negative steps. Negative steps go towards track 0, positive towards
*	track 79. An error is returned if the drive fails to accept a step
*	request.
*
************************************************************************/

OSStatus
SwimIIIStepDrive(short StepCount)
{

    OSStatus errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * If its zero, I guess we're done.
     */

    if (StepCount != 0) {

	/*
	 * Decide which way we're stepping.
	 */

	if (StepCount < 0) {

	    /*
	     * Step towards track zero (outside of the disk),
	     * and two's complement step count so that it can
	     * be used as a loop count.
	     */

	    SwimIIISetSignal(C_STEPTOZERO);

	    StepCount = 0 - StepCount;

	} else {

	    /*
	     * Step away from track zero (inside of the disk).
	     */

	    SwimIIISetSignal(C_STEPFROMZERO);

	}

	/*
	 * Make sure there is a 1 microsecond delay before we start stepping.
	 */

	SwimIIISmallWait(1);

	/*
	 * Make sure that we don't get any false triggers by
	 * preclearing the events we're waiting for.
	 */

	CancelOSEvent(driveOSEventIDptr,
		      IT_STEPPING_DONE);

	/*
	 * Address the stepping signal in the disk drive and set the number
	 * of steps to step.
	 */

	SwimIIIAddrSignal(C_STEP);

	*FloppySWIMIIIRegs.rwStepCount = StepCount;
	SynchronizeIO();

	/*
	 * Wait for stepping to complete. Wait for 10 seconds before timing out.
	 */
	if ((errorCode = WaitForEvent(10000, MS_DO_STEPPING, IT_STEPPING_DONE)) != noErr)
	    errorCode = cantStepErr;

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : SwimIIIDisableRWMode
*
*	Reset the SWIM chip from being in read or write mode mode.
*
************************************************************************/

void SwimIIIDisableRWMode(void)
{

    /*
     * Clear the action bit and put the SWIM into read mode. This
     * will also reset the CRC register and guarantee that we can
     * read the next group of mark bytes.
     */

    *FloppySWIMIIIRegs.rModewModeZeroes = (MS_NOT_READ_WRITE | MS_DO_ACTION);
    SynchronizeIO();

}


/************************************************************************
*
*	FUNCTION : SwimIIISetReadMode
*
*	Setup the SWIM chip to got in to, and operate in read mode.
*
************************************************************************/

void SwimIIISetReadMode(void)
{
    register unsigned char dummyValue;

    /*
     * Clear the action bit and put the SWIM into read mode before
     * we touch any other register in case doing so goofs up the SWIM.
     */

    SwimIIIDisableRWMode();

    /*
     * Make that the error register is cleared.
     */

    dummyValue = *FloppySWIMIIIRegs.rError;
    SynchronizeIO();

    /*
     * Clear the action bit and put the SWIMBASE into read mode. This
     * will also reset the CRC register and guarantee that we can
     * read the next group of mark bytes.
     */

    SwimIIIDisableRWMode();

    /*
     * Clear the action bit and put the SWIM into read mode. This
     * will also reset the CRC register and guarantee that we can
     * read the next group of mark bytes.
     */

    *FloppySWIMIIIRegs.rModewModeZeroes = MS_NOT_READ_WRITE;
    SynchronizeIO();

    /*
     * Clear any pending interrupts.
     */

    dummyValue = *FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO();
}


/************************************************************************
*
*	FUNCTION : SwimIIISetWriteMode
*
*	Setup the SWIM chip to got in to, and operate in write mode.
*
************************************************************************/

void SwimIIISetWriteMode(void)
{

    register unsigned char dummyValue;


    /*
     * Clear the action bit and put the SWIM into read mode before
     * we touch any other register in case doing so goofs up the SWIM.
     */

    SwimIIIDisableRWMode();

    /*
     * Make that the error register is cleared.
     */

    dummyValue = *FloppySWIMIIIRegs.rError;
    SynchronizeIO();

    /*
     * Clear the action bit and put the SWIM into write mode. This
     * will also reset the CRC register and guarantee that we can
     * read the next group of mark bytes.
     */

    SwimIIIDisableRWMode();

    *FloppySWIMIIIRegs.rHandshakewModeOnes = MS_NOT_READ_WRITE;
    SynchronizeIO();

    /*
     * Clear any pending interrupts.
     */

    dummyValue = *FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO();

}


/************************************************************************
*
*	FUNCTION : HALISRHandler
*
*	Handle interrupts form the SWIMIII and DMA hardware.
*
************************************************************************/
extern void PrintDMA(void);
void HALISR_DMA(int device, void *ssp)
{
    HALISRHandler(device, ssp);
}
void HALISRHandler(int device,
		   void *ssp)
{


    UInt32 pendingInterrupts = 0;

    /*
     * Save the pending interrupts that may have occured.
     */


    pendingInterrupts = *FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO();

    // See if the Floppy was the one that requested an interrupt.
    printf("HALISR:0x%x ",pendingInterrupts);              
    //if(pendingInterrupts &0x08)PrintDMA();
    if (pendingInterrupts != 0) {

	/*
	 * Signal the rest of the HAL what interrupts occured.
	 */
/* naga moving the following line 4 lines after */
//              SetOSEvent( driveOSEventIDptr,
	//                                      pendingInterrupts );

	/*
	 * Disable any further interrupts.
	 */

	*FloppySWIMIIIRegs.rwInterruptMask = 0;
	SynchronizeIO();

	*FloppySWIMIIIRegs.rModewModeZeroes = MS_ENABLE_INTS;
	SynchronizeIO();

	SetOSEvent(driveOSEventIDptr,
		   pendingInterrupts);
	/*
	 * Did an error occur.
	 */

	if (pendingInterrupts & IT_ERROR_OCCURRED) {

	    /*
	     * Save the errors away for the rest of the HAL.
	     */

	    lastErrorsPending = *FloppySWIMIIIRegs.rError;
	    SynchronizeIO();

	}
	// Tell the system that this interrupt has been serviced.
	if (powermac_info.class == POWERMAC_CLASS_PDM) {
	    floppy_amic_ack();
	}
    } else {
#if MACH_DEBUG
	printf("HALISR: Pending interrupt is ZERO ");
    // Tell the system that this interrupt was for somebody else.
#endif
    }
}

//#define printf donone
/************************************************************************
*
*	FUNCTION : HALSetFormatMode
*
*	Set the SWIM and drive to either MFM or GCR mode. 
*
************************************************************************/

OSStatus
HALSetFormatMode(DriveStatusType * DriveStatus)
{

    unsigned char modeCommand;


    /*
     * Say what kind of controller hardware we're using.
     */

    DriveStatus->FDCType = isSWIM3;
//printf("swimiiicommonhal.c:HALSetFormatMode:\n");     
    if (DriveStatus->formatType == GCR_DISKETTE) {

	/*
	 * GCR sectors just start at where ever the head first lands
	 */

	DriveStatus->preFirstSectorCachePadding = 0;

	/*
	 * SMIWIII needs and pre padding of the sector data in the track cache
	 * for all of the gap, sync, mark and escaping bytes.
	 */

	DriveStatus->preSectorCachePadding = (sizeof(SWIM3_GCR_SELF_SYNC) - 1) +	// Address Mark
	     (sizeof(SWIM3_GCR_ADDRESS_MARK) - 1) +
	    (GCRAddrMarkDataSize) +
	    (sizeof(SWIM3_GCR_SLIP_BYTE) - 1) +		// Terminate Address Mark
	     (sizeof(SWIM3_GCR_SELF_SYNC) - 1) +	// Write splice gap
	     (sizeof(SWIM3_GCR_SELF_SYNC) - 1) +	// Data Mark
	     (sizeof(SWIM3_GCR_DATA_MARK) - 1);

	/*
	 * Make room for the sector and tag data to expand into nibblized data, then add
	 * in room for the sector terminator and the inter-sector gap.
	 */

	DriveStatus->postSectorCachePadding = (DriveStatus->diskFormat.DMASectorSize - DriveStatus->diskFormat.logicalSectorSize) +
	    (sizeof(SWIM3_GCR_SLIP_BYTE) - 1) +		// Terminate CRC and sector data
	     ((5 * (sizeof(SWIM3_GCR_SELF_SYNC) - 1)) - 1);	// Inter-sector gap (one less to make the sector size even).

	/*
	 * GCR sector are evenly spaced around the track, so just add in the
	 * the terminator of the track DMA transfer.
	 */

	DriveStatus->postLastSectorCachePadding = (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);	// The escaped command to terminate DMA transfers.

	/*
	 * Now setup the track cache buffer with an image all of the intra-sector
	 * SWIMIII escaped data bytes.
	 */

	lastSectorsPerTrack = -1;
	FormatGCRCacheSWIMIIIData(DriveStatus);

	/*
	 * Now setup the ISM registers.
	 *
	 *    1. Motor timer off, disable Trans-Space machine, read/write data as
	 *       pulses, disable Error Correction machine, GCR mode, reset 3.5SEL
	 *       pin, Q3/HDSEL pin is an input.
	 */

	*FloppySWIMIIIRegs.rwSetup = (RS_GCR_NOT_MFM | RS_GCR_WRITE_NOT_MFM_WRITE | RS_FCLK_DIV_2_NOT_FCLK);
	SynchronizeIO();

	/*
	 * Now setup the parameter ram data for the Trans-Space machine
	 * to do MFM The write the the "rModewModeZeroes" register will reset
	 * the parameter register index to zero without changing the
	 * contents of the mode register.
	 *
	 * These parameters were taken from the Horror Mac ROM code. The timing
	 * values Time1, Time0, xSx, xLx and Min have a resolution to the nearest
	 * half clock, so their actual register values are doubled so that bits
	 * 1-7 become the integer part and bit 0 becomes the ".5" part. Values
	 * xSx and xLx are reduced by 2 clocks, and Min is reduced by 3 clocks
	 * to take into account gate delays in the SWIM. These parameters are
	 * for a 15.6672MHz input clock (Mac SE, Mac II, etc).
	 */

	*FloppySWIMIIIRegs.rModewModeZeroes = ~*FloppySWIMIIIRegs.rModewModeZeroes;
	SynchronizeIO();

	*FloppySWIMIIIRegs.rwParams = 0x88;	// Late/Early

	SynchronizeIO();

	/*
	 * Put the drive into GCR mode.
	 */

	modeCommand = C_SETGCR;

    } else {

	/*
	 * Set the padding offset to the first sector in the track cache.
	 *
	 * NOTE: This padding points to the preSectorCachePadding point of a
	 * sector, NOT to the data field.
	 */

	DriveStatus->preFirstSectorCachePadding = DriveStatus->diskFormat.gap4ALength +		// The 4Es before the Index Mark.
	     SWIM3_MFM_MARK_SYNC_GAP_LEN +	// The 12 0x00 sync bytes before Index mark.
	     (sizeof(SWIM3_MFM_INDEX_MARK) - 1) +	// The SWIM3 escaped Index mark.
	     DriveStatus->diskFormat.gap1Length;	// The 4Es after the Index Mark.

	/*
	 * SMIWIII needs and pre or post padding of the data in the track cache
	 * for all of the gap, sync, mark and escaping bytes.
	 */

	DriveStatus->preSectorCachePadding = SWIM3_MFM_MARK_SYNC_GAP_LEN +	// The 12 0x00 sync bytes before Address mark.
	     (sizeof(SWIM3_MFM_ADDRESS_MARK) - 1) +	// The SWIM3 escaped Address mark.
	     4 +		// Address Mark field bytes.
	     (sizeof(SWIM3_WRITE_MFM_CRC) - 1) +	// The SWIM3 escaped Address Mark field CRC bytes.
	     DriveStatus->diskFormat.gap2Length +	// The 4Es in gap2.
	     SWIM3_MFM_MARK_SYNC_GAP_LEN +	// The 12 0x00 sync bytes before data mark.
	     (sizeof(SWIM3_MFM_DATA_MARK) - 1) +	// The SWIM3 escaped data mark.
	     (sizeof(SWIM3_DISABLE_DATA_ESCAPING) - 1);		// SWIM3 turn off escaping for sector data.

	DriveStatus->postSectorCachePadding = (sizeof(SWIM3_WRITE_MFM_CRC) - 1) +	// The SWIM3 escaped data field CRC bytes.
	     DriveStatus->diskFormat.gap3Length;	// The 4Es in gap3.

	/*
	 * Finally, this is the remaining bytes in the track cache image
	 * after the last sector and before the head 2 image in the track
	 * cache buffer.
	 */

	DriveStatus->postLastSectorCachePadding = DriveStatus->diskFormat.gap4BLength +		// The 4Es after the last sector.
	     (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);	// The escaped command to terminate DMA transfers.

	/*
	 * Now setup the track cache buffer with an image all of the intra-sector
	 * SWIMIII escaped data bytes.
	 */

	lastSectorsPerTrack = -1;

	FormatMFMCacheSWIMIIIData(DriveStatus);

	/*
	 * Now setup the ISM registers.
	 *
	 *    1. Motor timer off, use Trans-Space machine, read/write data as
	 *       pulses, disable Error Correction machine, MFM mode, reset 3.5SEL
	 *       pin, Q3/HDSEL pin is an input.
	 */

	if (DriveStatus->disketteType == DISKETTE_4MEG) {

	    *FloppySWIMIIIRegs.rwSetup = RS_IBM_NOT_APPLE_DRIVE;

	} else
	    *FloppySWIMIIIRegs.rwSetup = (RS_IBM_NOT_APPLE_DRIVE | RS_FCLK_DIV_2_NOT_FCLK);

	SynchronizeIO();

	/*
	 * Now setup the parameter ram data for the Trans-Space machine
	 * to do MFM The write the the "rModewModeZeroes" register will reset
	 * the parameter register index to zero without changing the
	 * contents of the mode register.
	 *
	 * These parameters were taken from the 68030 Mac ROM code. The timing
	 * values Time1, Time0, xSx, xLx and Min have a resolution to the nearest
	 * half clock, so their actual register values are doubled so that bits
	 * 1-7 become the integer part and bit 0 becomes the ".5" part. Values
	 * xSx and xLx are reduced by 2 clocks, and Min is reduced by 3 clocks
	 * to take into account gate delays in the SWIM. These parameters are
	 * for a 15.6672MHz input clock (Mac SE, Mac II, etc).
	 */

	*FloppySWIMIIIRegs.rModewModeZeroes = ~*FloppySWIMIIIRegs.rModewModeZeroes;
	SynchronizeIO();

	*FloppySWIMIIIRegs.rwParams = 0x95;	// Late/Early

	SynchronizeIO();

	/*
	 * Put the drive into MFM mode.
	 */

	modeCommand = C_SETMFM;

    }

    /*
     * Set the drive state to the desired state.
     */

    SwimIIISetSignal(modeCommand);

    /*
     * Start looking for the drive's ready signal after 30 milliseconds,
     * and return when the drive is ready.
     */
    //printf (" HALSetFormatMode - SleepUntilReady "); 
    {
	OSStatus rc;
	rc = SleepUntilReady(30);
	//printf (" HALSetFormatMode - SleepUntilReady returned! ");
	return rc;
    }

}


/************************************************************************
*
* FUNCTION : HALDiskettePresence
*
************************************************************************/

boolean_t
HALDiskettePresence(DriveStatusType * DriveStatus)
{

    SwimIIIDiskSelect(DriveStatus);

    /*
     * Setup the diskette's write protect flag.
     */

    DriveStatus->writeProt = false;

    if (!SwimIIISenseSignal(S_WRTENAB))
	DriveStatus->writeProt = true;

    return (SwimIIISenseSignal(S_NODISK) == 0);

}


/************************************************************************
*
* FUNCTION : HALGetMediaType
*
************************************************************************/

void HALGetMediaType(DriveStatusType * DriveStatus)
{

    /*
     * If we're not on a Super Drive, then default the format field
     * to off and non-MFM.
     */

    DriveStatus->formatType = GCR_DISKETTE;
    DriveStatus->disketteType = DISKETTE_1MEG;

    if (SwimIIISenseSignal(S_2MBDRIVE)) {

	/*
	 * If we're on a Super Drive and we've got 1 meg media,
	 * we know the we have a DD diskette, and maybe MFM.
	 */

	if (SwimIIISenseSignal(S_1MBMEDIA)) {

	    DriveStatus->formatType = MFM_DISKETTE;
	    DriveStatus->disketteType = DISKETTE_1MEG;

	} else {

	    /*
	     * If we're on a Super Drive and we've got 2 meg media,
	     * we know the we have a HD or ED MFM diskette.
	     */

	    DriveStatus->formatType = MFM_DISKETTE;

	    /*
	     * Select media type checking and see if we have a 2 MB (i.e. HD)
	     * or a 4 MB (i.e. ED) diskette.
	     *
	     * If this is on a SuperDrive (FDHD), this will end up checking
	     * the One/Two sided bit. However, a SuperDrive is two sided
	     * which ends up looking like a 2 MB diskette which is correct
	     * for a SuperDrive (this was by design, not by accident). Only
	     * a Typhoon drive will ever return this as FALSE unless, God
	     * forbid, we decide to try to support the old 400K drives with
	     * this driver.
	     */

	    SwimIIISetSignal(C_2MBMEDIACHECK);

	    if (SwimIIISenseSignal(S_2MBDRVDSK)) {

		/*
		 * We must have a 2 MB (i.e. HD) diskette.
		 */

		DriveStatus->disketteType = DISKETTE_2MEG;

	    } else {

		/*
		 * We must have a 4 MB (i.e. ED) diskette.
		 */

		DriveStatus->disketteType = DISKETTE_4MEG;

	    }

	}

    }
}


/************************************************************************
*
* FUNCTION : HALGetDriveType
*
************************************************************************/

boolean_t
HALGetDriveType(DriveStatusType * DriveStatus)
{

    boolean_t installedStatus;


    /*
     * Now physically test to see if the drive is physically there.
     */

    SwimIIIDiskSelect(DriveStatus);

    if ((installedStatus = !SwimIIISenseSignal(S_NODRIVE)) == true) {

	/*
	 * Default to a 800k drive.
	 */

	DriveStatus->driveType = DRIVE_1MEG_800k;

	/*
	 * See if it is a Super Drive or a Typhoon drive.
	 */

	if (SwimIIISenseSignal(S_2MBDRIVE)) {

	    SwimIIISetSignal(C_2MBMEDIACHECK);

	    if (SwimIIISenseSignal(S_2MBDRVDSK)) {

		/*
		 * We must have a 2 MB (i.e. FDHD) drive.
		 */

		DriveStatus->driveType = DRIVE_2MEG_FDHD;

	    } else {

		/*
		 * We must have a 4 MB (i.e. Typhoon) drive.
		 */

		DriveStatus->driveType = DRIVE_4MEG_TYPHOON;

	    }

	}
    }
    return installedStatus;

}


/************************************************************************
*
*	FUNCTION : HALPowerUpDrive
*
*	Make a drive the currently selected drive and power it on. Start
*	looking for the drive's /READY signal after "ReadyLookDelay" and
*	wait for "PostReadyDelay" before returning. An error is returned
*	we timeout waiting for the drive's /READY signal.
*
************************************************************************/

OSStatus
HALPowerUpDrive(DriveStatusType * DriveStatus)
{


    register short errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Power up the drive in GCR mode and wait the drive's /READY signal.
     */
    SwimIIISetSignal(C_SETGCR);

    SwimIIISetSignal(C_MOTORON);

    /*
     * Wait one millisecond before looking for the ready signal.
     */

    errorCode = SleepUntilReady(1);

//printf("swimiiicommonhal.c:HALPowerUpDrive:ret=%d ",errorCode );
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : HALPowerDownDrive
*
*	Power down the currently selected drive.
*
************************************************************************/

void HALPowerDownDrive(DriveStatusType * DriveStatus)
{

    /*
     * Make sure that the drive we're trying to power down
     * is selected so that we can talk to it.
     */

    SwimIIIDiskSelect(DriveStatus);

    /*
     * Turn off the drive's motor and kill it's power.
     */

    SwimIIISetSignal(C_MOTOROFF);

}


/************************************************************************
*
*	FUNCTION : HALSeekDrive
*
*	Seek the currently selected drive to an absolute track number (0 - 79).
*	An error is returned if the drive fails to accept a step request.
*
************************************************************************/

OSStatus
HALSeekDrive(DriveStatusType * DriveStatus)
{

    register short errorCode;
    register short trackDelta;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Figure the relative track difference from were we are to were
     * we want to be and seek that amount.
     */

    trackDelta = DriveStatus->wantedAddress.address.items.track - DriveStatus->currentAddress.address.items.track;

    if ((errorCode = SwimIIIStepDrive(trackDelta)) == noErr) {

	/*
	 * Wait 30 millisecond of head settle time, then wait for the
	 * drive's ready signal.
	 */

	errorCode = SleepUntilReady(30);

    }
    /*
     * If we didn't get a error, update the track cache buffer with an
     * image all of the intra-sector SWIMIII escaped data bytes for the
     * new track.
     */

    if (errorCode == noErr) {

	if (DriveStatus->diskFormat.GCR)
	    FormatGCRCacheSWIMIIIData(DriveStatus);
	else
	    FormatMFMCacheSWIMIIIData(DriveStatus);

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : HALRecalDrive
*
*	Seek the head for the currently selected drive to track zero and
*	update the drive's current track status.
*
************************************************************************/

OSStatus
HALRecalDrive(DriveStatusType * DriveStatus)
{

    register short errorCode;
    register short StepCount;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * While we're not on track zero, seek towards track zero
     * a maximum of 80 steps.
     */
    StepCount = DriveStatus->diskFormat.drvTrackMax;

//      printf("swim3commonhal.c:HALRecalDrive: stepcount=%d ",StepCount);
    while (StepCount-- &&
	   SwimIIISenseSignal(S_NOTTRK0)) {

	if ((errorCode = SwimIIIStepDrive(-1)) != noErr)
	    break;

    }

    /*
     * Update the current track location.
     */
    //printf("RecalDrive: out of big stepcount=%d loop err=%d ",StepCount,errorCode); 
    if (errorCode == noErr) {

	/*
	 * Wait 30 millisecond of head settle time, then look for the
	 * drive's track zero signal.
	 */

	if ((errorCode = SleepUntilReady(30)) == noErr) {

	    /*
	     * If we ran out of steps and we're not on track zero,
	     * return an error.
	     */

	    if ((StepCount == 0) && SwimIIISenseSignal(S_NOTTRK0))
		errorCode = RecordError(tk0BadErr);

	}
    }
    /*
     * If we didn't get a error, update the track cache buffer with an
     * image all of the intra-sector SWIMIII escaped data bytes for the
     * new track.
     */

    if (errorCode == noErr) {

	if (DriveStatus->diskFormat.GCR)
	    FormatGCRCacheSWIMIIIData(DriveStatus);
	else
	    FormatMFMCacheSWIMIIIData(DriveStatus);

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : HALEjectDiskette
*
*	Eject the diskette in the currently selected drive.
*
************************************************************************/

OSStatus
HALEjectDiskette(DriveStatusType * DriveStatus)
{


    register short EjectingCount;


    /*
     * Start the disk ejecting process.
     */

    SwimIIISetSignal(C_EJECT);

    /*
     * Wait for the disk to disappear from out of the drive. The
     * loop will wait a maximum 30 * 0.1 seconds, e.i. 3 seconds.
     */

    EjectingCount = 30;

    while (EjectingCount--) {

	/*
	 * Scan for the disk to disappear in 0.1 second
	 * intervals.
	 */

	(void) FloppyTimedSleep(100);

	/*
	 * Test to see if the disk is still there. If gone,
	 * cancel the loop.
	 */

	if (SwimIIISenseSignal(S_NODISK))
	    break;

    }

    /*
     * The drive spec says the that the /ENB signal must stay
     * asserted for 150usec after the /CSTIN signal de-asserts.
     * Therefore, keep the caller of this function from changing
     * the /ENB signal to quickly by waiting a millisecond
     * before returning.
     */

    (void) FloppyTimedSleep(1);

#if 0
    if (SwimIIISenseSignal(S_NODISK))
	return noErr;
    else
	return -1;
#else

    return noErr;
#endif

}


/************************************************************************
*
*	FUNCTION : HALGetNextAddressID
*
*	Read the next MFM/GCR sector address field from the byte stream from the
*	currently selected drive. The drive's head needs be on the correct
*	track before this function is called.
*
************************************************************************/

OSStatus
HALGetNextAddressID(DriveStatusType * DriveStatus)
{

    short errorCode;

// printf("Leading:\n");
// PrintRegs();
// PrintDMA();
// printf("\n");

    /*
     * Reset the DMA channel we're trying to do error recovery.
     */
    ResetDMAChannel();

    /*
     * Make sure that we don't get any false triggers by
     * preclearing the events we're waiting for.
     */

    CancelOSEvent(driveOSEventIDptr,
		  IT_ID_HEADER_READ);

    /*
     * Get the byte stream from the correct head and put us into read mode.
     */

//printf("HALGetNextAddressID: calling SwimIIIHeadSelect ");
    SwimIIIHeadSelect(DriveStatus->wantedAddress.address.items.side);

//printf("HALGetNextAddressID: calling SwimIIISetReadMode ");

    SwimIIISetReadMode();

    /*
     * Setup the timeout to trying to capture the first sector
     * address to fly bye. Wait a maximum of two diskette rotations
     * (0.4 seconds).
     *
     * If we get a noAdrMkErr, we'll go through the entire retry loop
     * without receiving a sector header, conclude that there are no
     * sector headers on the diskette, i.e. unformated, and indicate
     * we timed out.
     */

    /* DAG changed 400 to 4000... */
    if ((errorCode = WaitForEvent(4000, MS_DO_ACTION, IT_ID_HEADER_READ)) == noErr) {

	SwimIIIDisableRWMode();

	DriveStatus->currentAddress.address.items.track = *FloppySWIMIIIRegs.rCurrentTrack & 0x7F;
	SynchronizeIO();

	DriveStatus->currentAddress.address.items.side = (*FloppySWIMIIIRegs.rCurrentTrack & 0x80) >> 7;
	SynchronizeIO();

	DriveStatus->currentAddress.address.items.sector = *FloppySWIMIIIRegs.rCurrentSector & 0x7F;
	SynchronizeIO();

	DriveStatus->currentAddress.address.items.blockSize = *FloppySWIMIIIRegs.rwGapSize;
	SynchronizeIO();

	/*
	 * Exit the retry loop as successful.
	 */

    } {
	long xxx; dbdma_regmap_t *regmap;
      xxx = *FloppySWIMIIIRegs.rInterrupt; eieio();
	      // SynchronizeIO();
	     printf("HALGetNextAddr: 0x%x ",xxx);
      regmap = POWERMAC_IO(powermac_info.io_base_phys + 0x8100);
      xxx = regmap->d_status; eieio();
             printf("DMASTATUS= 0x%x ",xxx);
    }
    /*
     * Cancel read mode and return any errors.
     */

    SwimIIIDisableRWMode();

// printf("Trailing:\n");
// PrintRegs();
// PrintDMA();
// printf("\n");

    return RecordError(errorCode);

}


/************************************************************************
*
*	FUNCTION : HALReadSector
*
************************************************************************/
extern char TrackBuffer[][];
OSStatus
HALReadSector(DriveStatusType * DriveStatus)
{

    register short errorCode;

//printf("swimiiicommonhal.c:HALReadSector: ");
    /*
     * Start the SWIM looking for the wanted sector.
     */

    *FloppySWIMIIIRegs.rwSectorCount = 1;
    SynchronizeIO();

    *FloppySWIMIIIRegs.rwGapSize = 0;
    SynchronizeIO();
//printf("sector=%d side=%d ", DriveStatus->wantedAddress.address.items.sector,DriveStatus->wantedAddress.address.items.side );
    //printf("rd:%d:%d:%d ",DriveStatus->wantedAddress.address.items.side,DriveStatus->wantedAddress.address.items.track,DriveStatus->wantedAddress.address.items.sector);
    *FloppySWIMIIIRegs.rwFirstSector = DriveStatus->wantedAddress.address.items.sector;
    SynchronizeIO();

    /*
     * Get the byte stream from the correct head.
     */

    SwimIIIHeadSelect(DriveStatus->wantedAddress.address.items.side);

    /*
     * Start up the DMA engine in read mode and wait for the DMA to complete.
     * Always make the DMA count the real number of bytes to be transfered
     * plus the gap size in the gap register so that only the SWIM's interrupt
     * is activated when the transfer is completed.
     */
//printf("HALReadSector:databufferlogical=0x%04x,phys=0x%04x ",DriveStatus->transferAddress.logicalAddress,DriveStatus->transferAddress.physicalAddress);
    errorCode = StartDMAChannel(DriveStatus->transferAddress.physicalAddress,
				DriveStatus->diskFormat.DMASectorSize,
				DMA_READ);

    /*
     * Cancel read mode, turn off sector searching and return our error code.
     */

    if (errorCode != 0)
	printf("HALRead failed (%d) for %d:%d:%d ", errorCode, DriveStatus->wantedAddress.address.items.side, DriveStatus->wantedAddress.address.items.track, DriveStatus->wantedAddress.address.items.sector);
//     printf("R:%d ",DriveStatus->wantedAddress.address.items.sector); 
    SwimIIIDisableRWMode();

    *FloppySWIMIIIRegs.rwFirstSector = 0xFF;
    SynchronizeIO();
    return errorCode;

}

//#define printf donone
/************************************************************************
*
*	FUNCTION : HALWriteSector
*
************************************************************************/

OSStatus
HALWriteSector(DriveStatusType * DriveStatus)
{

    register short errorCode;
    register BytePtr DMACorrectedAddress;
    register uint_t DMACorrectionLength;
    register uint_t DMACorrectionAdjustment;
//printf("HALWrtSec: sec=%d ",DriveStatus->wantedAddress.address.items.sector); 

    /*
     * Start the SWIMBASE looking for the wanted sector.
     */

    *FloppySWIMIIIRegs.rwSectorCount = 1;
    SynchronizeIO();

    *FloppySWIMIIIRegs.rwGapSize = 0;
    SynchronizeIO();

    *FloppySWIMIIIRegs.rwFirstSector = DriveStatus->wantedAddress.address.items.sector;
    SynchronizeIO();

    /*
     * Put a "Terminate DMA Transfer" command at the end of the sector data. Add
     * a clearance set of 4Es before the terminate transfer command so that we
     * make sure that the CRC bytes have cleared the SWIMIII's FIFO before we
     * shut off the DMA.
     */

    DMACorrectedAddress = (BytePtr) DriveStatus->transferAddress.logicalAddress +
	DriveStatus->diskFormat.DMASectorSize +
	SWIM3_FIFO_CLEARANCE_LEN;

    ByteMove(SWIM3_TERMINATE_DMA_TRANSFER,
	     DMACorrectedAddress,
	     (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1));

    /*
     * Back the cache buffer up to the begining of gap2 since the SWIMIII has to
     * write the Data Mark along with the sector data.
     */

    if (DriveStatus->formatType == GCR_DISKETTE) {

	DMACorrectionLength = (sizeof(SWIM3_GCR_SELF_SYNC) - 1) +	// Write splice gap
	     (sizeof(SWIM3_GCR_SELF_SYNC) - 1) +	// Data Mark sync
	     (sizeof(SWIM3_GCR_DATA_MARK) - 1);		// Data Mark

    } else {

	DMACorrectionLength = DriveStatus->diskFormat.gap2Length +	// The 4Es in gap2.
	     SWIM3_MFM_MARK_SYNC_GAP_LEN +	// The 12 0x00 sync bytes before data mark.
	     (sizeof(SWIM3_MFM_DATA_MARK) - 1) +	// The SWIM3 escaped data mark.
	     (sizeof(SWIM3_DISABLE_DATA_ESCAPING) - 1);		// SWIM3 turn off escaping for sector data.

    }

    DMACorrectedAddress = (BytePtr) DriveStatus->transferAddress.physicalAddress - DMACorrectionLength;

    /*
     * Make sure that the cache addresses is rounded up to a DMA byte boundry
     * for those AMIC DMA. This will move the DMA point a short ways into gap2
     * where the write splice is suppose to happen. The gap2 may be written
     * sligthly smaller than what the spec says, but can only be smaller by
     * a total of seven bytes which is within tolerable limits.
     */

    DMACorrectionAdjustment = 0;

    if (DriveStatus->DMAByteBoundryAlignment > 1)
	DMACorrectionAdjustment = (((uint_t) DMACorrectedAddress & ~(DriveStatus->DMAByteBoundryAlignment - 1)) + DriveStatus->DMAByteBoundryAlignment) - (uint_t) DMACorrectedAddress;

    /*
     * Finally, adjust the cache DMA Address and DMA length to the actual amount
     * of data to be transfered.
     */

    DMACorrectedAddress += DMACorrectionAdjustment;

    DMACorrectionLength -= DMACorrectionAdjustment;

    DMACorrectionLength += DriveStatus->diskFormat.DMASectorSize +
	SWIM3_FIFO_CLEARANCE_LEN +
	(sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);

    /*
     * Get the byte stream from the correct head.
     */

    SwimIIIHeadSelect(DriveStatus->wantedAddress.address.items.side);

    /*
     * Start up the DMA engine in read mode and wait for the DMA to complete.
     * Always make the DMA count the real number of bytes to be transfered
     * plus the gap size in the gap register so that only the SWIM's interrupt
     * is activated when the transfer is completed.
     */

    errorCode = StartDMAChannel(DMACorrectedAddress,
				DMACorrectionLength,
				DMA_WRITE);
//printf("wr:%d:%d:%d:%c ",DriveStatus->wantedAddress.address.items.side,DriveStatus->wantedAddress.address.items.track,DriveStatus->wantedAddress.address.items.sector,*(char *)(DMACorrectedAddress+100));
    //printf("w:%d ",DriveStatus->wantedAddress.address.items.sector);
    /*
     * Cancel read mode, turn off sector searching and return our error code.
     */
    if (errorCode != 0)
	printf("HALWrite failed for %d:%d:%d ", DriveStatus->wantedAddress.address.items.side, DriveStatus->wantedAddress.address.items.track, DriveStatus->wantedAddress.address.items.sector);
    SwimIIIDisableRWMode();

    *FloppySWIMIIIRegs.rwFirstSector = 0xFF;
    SynchronizeIO();

    return errorCode;

}


/************************************************************************
*
* FUNCTION : HALFormatTrack
*
************************************************************************/

OSStatus
HALFormatTrack(DriveStatusType * DriveStatus)
{

    register short errorCode;
    register unsigned char diskSide;
    register unsigned char maxSides;
    register unsigned char sectorsPerTrack;
    unsigned char gap1Length;
    unsigned char gap2Length;
    unsigned char gap3Length;
    unsigned char gap4ALength;


    errorCode = noNybErr;

    /*
     * Assume success. Set the default items in the sector
     * ID data. Preset a bunch of variables for speed.
     */

    errorCode = noErr;

    gap2Length = DriveStatus->diskFormat.gap2Length;
    gap3Length = DriveStatus->diskFormat.gap3Length;
    gap4ALength = DriveStatus->diskFormat.gap4ALength;
    sectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;

    /*
     * Do both sides of the diskette.
     */

    diskSide = 0;

    maxSides = (DriveStatus->diskFormat.flagsNSides & FmtSidesMask);

    while ((diskSide < maxSides) &&
	   (errorCode == noErr)) {

	/*
	 * Preset gap1 for the first time through a side loop, plus
	 * set the head and sector number in the sector ID data.
	 */

	gap1Length = DriveStatus->diskFormat.gap1Length;

	DriveStatus->wantedAddress.address.items.side = diskSide;
	DriveStatus->wantedAddress.address.items.sector = 1;

	/*
	 * Wait for the rising edge of the index pulse.
	 */

	while (SwimIIISenseSignal(S_INDEX));

	/*
	 * Wait for the falling edge of the index pulse.
	 */

	while (!SwimIIISenseSignal(S_INDEX));

	/*
	 * Send the byte stream to the correct head and setup the
	 * head number for the next side.
	 */

	SwimIIIHeadSelect(diskSide);

	diskSide++;

	/*
	 * Turn on write mode.
	 */

	SwimIIISetWriteMode();

	/*
	 * Now write out the bytes in gap 4A, the 12 sync bytes and the
	 * address mark that precedes the sector header data, but if we
	 * get an error, return it.
	 */

// €€€€         if( ( errorCode = SwimIIIMFMWriteMark( MFM_INDEX_MARK, gap4ALength ) ) == noErr )
	{

	    /*
	     * Write out all of the sectors plus their gaps, starting
	     * with sector number one (MFM numbers sectors starting at
	     * one).
	     */

	    while ((errorCode == noErr) &&
		   (DriveStatus->wantedAddress.address.items.sector <= sectorsPerTrack)) {

		/*
		 * Write out the sector ID head field.
		 */

// €€€€                         if( ( errorCode = SwimIIIMFMWriteSectorID( &DriveStatus->wantedAddress,
		// €€€€                                                                                                            gap1Length ) ) == noErr )
		{

		    /*
		     * Write the next sector data from the dummy sector space.
		     * Exit the loop with any errors. Otherwise, s
		     */

// €€€€                                 if( ( errorCode = SwimIIIMFMWriteDataField( DriveStatus->logicalAddress,
		    // €€€€                                                                                                                     gap2Length ) ) == noErr )
		    {

			/*
			 * Set the sector ID data data pointer for the next sector.
			 */

			DriveStatus->wantedAddress.address.items.sector++;

//                                              DriveStatus->wantedAddress.address.items.sector += sectorsPerTrack / 2;
			//                                              
			//                                              if( DriveStatus->wantedAddress.address.items.sector > sectorsPerTrack )
			//                                                      DriveStatus->wantedAddress.address.items.sector -= sectorsPerTrack - 1;

			DriveStatus->transferAddress.logicalAddress += DriveStatus->diskFormat.logicalSectorSize;

			/*
			 * Turn off any gap1s and let gap3s take their
			 * place preceeding the sector header.
			 */

			gap1Length = gap3Length;

		    }

		}

	    }


	    /*
	     * Write out the gap bytes so that we're sure that the last sector's
	     * CRC has fully cleared the FIFO before we disable writing.
	     */

	    if (errorCode == noErr) {

// €€€€                         errorCode = SwimIIIMFMWriteGapField( gap3Length );

	    }
	}

	/*
	 * Cancel write mode.
	 */

	SwimIIIDisableRWMode();

    }

    return errorCode;

}
#endif // NFD > 0
