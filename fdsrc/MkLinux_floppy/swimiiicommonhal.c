/*
 * MkLinux Public Source License
 * v1.1
 *
 * Portions Copyright (c) 1999-2002 The MkLinux Project.
 *              All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice and disclaimer
 * appear in all copies of the software, derivative works or modified
 * versions, and any portions thereof, and that these notices also
 * appear in supporting documentation, and provided that all advertising
 * materials mentioning features or use of this software displays the
 * following acknowledgement:
 *
 *      This product includes software developed by The MkLinux Project
 *      and its contributors.
 *
 * Neither the name of The MkLinux Project nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND THE MKLINUX PROJECT HEREBY DISCLAIMS ALL
 * SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT."
 * 
 * IN NO EVENT SHALL THE MKLINUX PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN ACTION OF CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF OR IN CONNECTION WITH THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Software whose source code is freely available, including those licensed
 * under the GNU General Public License (GPL) are exempt from all advertising
 * requirements.  Software collections are not exempt unless they include
 * non MPSL versions of this code as allowed below.
 *
 * License is hereby granted to freely redistribute this code as part of
 * another software product under the terms of the GPL or any other OSI-
 * certified open source license agreement provided that this copyright notice
 * remain intact, and provided that all copies of the source code contain, in
 * some reasonable, human-readable form, information telling where any party
 * may obtain a copy of this source code that is not burdened by the GPL or
 * other license's potentially more restrictive licensing terms.
 */
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
#include <fd.h>
#include <sys/types.h>
#include <machine/spl.h>
#if 0
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#endif
#include <sys/ioctl.h>
#if 0
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#endif

#if NFD > 0

#include <kern/lock.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppysal.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "swimiii.h"
#if 0
#include "floppy_amic.h"
#endif
#include <ppc/proc_reg.h>
#if 0
#include <ppc/POWERMAC/dbdma.h>
#else
#include "floppy_darwin_dbdma.h"
#endif

#include "swimiiicommonhal.h"
#include <sys/systm.h> // for printf
#include "dprintf.h"

#if (SERIAL_DEBUG)
#undef dprintf
#define dprintf(DEBUG_VERBOSE, a, b...) kprintf(a, ## b)
#define printf(a, b...) kprintf(a, ## b)
#endif

/* DAG NOTE: This file contains CRITICAL timing information.
   Much of this information was derived from other drivers.
   This also folds in all relevant changes from the Mac OS
   X Server 1.0 port of this driver.
 */

/* Offset of the second side in the track cache buffer, used for
   formatting,
 */
uint_t track_offset[NFD];

/************************************************************************
*
* Private data for HAL.
*
************************************************************************/

SwimIIIRegs FloppySWIMIIIRegs;	/* Floppy controller register sets */
extern SwimIIIRegs FloppySWIMIIIRegsList[];

unsigned char lastErrorsPending;

unsigned char lastSectorsPerTrack;

UInt32 *driveOSEventIDptr;	// This is also referenced in grcswimiiihal.c as extern

extern caddr_t org_mklinux_swim3_dmvaddr[2];


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
     * Some Floppy Drives in PowerMacs require 10 msec. to be ready
     * AFTER they assert ready.
     */
    FloppyTimedSleep(10);

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
#define OLD_WAIT
#ifndef OLD_WAIT
    int retval;
#endif
    int loops;

    /*
     * Prepare for the worst in case we time out.
     */

    errorCode = noNybErr;

    /*
     * Let the SWIM hardware settle into a steady state.
     */
    // FloppyTimedSleep(10);

    /*
     * Clear any pending interrupts and error status.
     */

    lastErrorsPending = 0;

    pendingInterrupts = *FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO(); delay(10);

    /*
     * Enable the interrupt we want and the master interrupt enable.
     * Finally, turn on the "action" the caller requested.
     */

    *FloppySWIMIIIRegs.rwInterruptMask = interruptMask;
    SynchronizeIO(); delay(10);

    *FloppySWIMIIIRegs.rHandshakewModeOnes = MS_ENABLE_INTS | actionBitMask;
    SynchronizeIO(); delay(10);

    loops = 0;
// rewait:

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

	dprintf(DEBUG_VERBOSE, "WaitForEvent: wait over pend intrs=%d,mask=%d, err=%d ",pendingInterrupts,interruptMask,
		lastErrorsPending);
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
		    SynchronizeIO(); delay(10);

		    /*
		     * Return to the caller saying that the wanted interrupt
		     * occurred.
		     */

		    errorCode = noErr;
		} else {
#if 0
		    unsigned char error; unsigned long errornum;
#endif

		    /* A Hardware ERROR Occurred.  Say SOMETHING! */

#if 0
		    printf("Wrong Interrupt.  Assuming cross-wired bits.\n");

		    *FloppySWIMIIIRegs.rModewModeZeroes = actionBitMask;
		    SynchronizeIO(); delay(10);

		    *FloppySWIMIIIRegs.rModewModeZeroes = IT_SENSE_INPUT_CHANGE;
		    SynchronizeIO(); delay(10);

		    errorCode = noErr;
#else
		    dprintf(DEBUG_VERBOSE, "Wrong Interrupt.  Returning error.\n");
#endif
		}
	    } else {

		if (!(pendingInterrupts & IT_ERROR_OCCURRED)) {
			printf("WARNING: STALE ERRORS!\n");
		}
		/*
		 * Since we got an error, turn off the "action" the caller requested.
		 */

		*FloppySWIMIIIRegs.rModewModeZeroes = actionBitMask;
		SynchronizeIO(); delay(10);

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
			     * Return a "Write Underrun/Read Overrun Occurred"
			     * error.
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

// Removed because this is pointless.
	} else {
	    dprintf(DEBUG_VERBOSE, "Spurious interrupt.\n");
	    // /* This could potentially be interrupted by something else.... */
	    // if (loops++ < 6) {
		// printf("lastErrorsPending is 0x%x\n", lastErrorsPending);
		// dprintf(DEBUG_VERBOSE, "REWAIT!\n");
		// goto rewait;
	    // }
	}
#ifdef OLD_WAIT
    } else			// timeout case
#else
    }
    if (!retval)
#endif
     {
	dprintf(DEBUG_VERBOSE, "lastErrorsPending is 0x%x\n", lastErrorsPending);

	*FloppySWIMIIIRegs.rModewModeZeroes = actionBitMask;
	SynchronizeIO(); delay(10);
    }
    PrintRegs(0);
    PrintRegs(1);

    /*
     * Return to the caller saying that we timed out waiting for
     * the wanted interrupt.
     */
    dprintf(DEBUG_VERBOSE, "Wait For Event: ret=%d intr=%d", errorCode, pendingInterrupts);
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
    int i;

/* DAG ??? */
    // if (powermac_info.class == POWERMAC_CLASS_PCI)
/* naga */
	// return;

    /*
     * Point to the begining of the track cache buffer.
     */
    nextCacheByteAddr = (BytePtr) DriveStatus->tcBuffer.logicalAddress;

    kprintf("Point 1: NCBA 0x%x\n", nextCacheByteAddr);

    /*
     * Write the data for the number of sides.
     */

    sideIndex = 0;

    while (sideIndex < 2) {
        kprintf("Top of while: NCBA 0x%x\n", nextCacheByteAddr);

	/*
	 * Are we only updating the Address Marks?
	 */

// #define GCR_ALWAYS_FROM_SCRATCH
// #ifndef GCR_ALWAYS_FROM_SCRATCH
	if (lastSectorsPerTrack == DriveStatus->diskFormat.sectorsPerTrack)
	{

	    lastSectorsPerTrack = DriveStatus->diskFormat.sectorsPerTrack;

	    /*
	     * Offset to the first sector.
	     */

	    nextCacheByteAddr =
		(nextCacheByteAddr +
		 (200 * (sizeof(SWIM3_GCR_SELF_SYNC)-1)));

	    /*
	     * Write out the updated data for all of the sectors.
	     */

	    dprintf(DEBUG_VERBOSE, "Initializing %d sectors subsequent time\n", DriveStatus->diskFormat.sectorsPerTrack);

	    for (sectorIndex = 0; sectorIndex < DriveStatus->diskFormat.sectorsPerTrack; sectorIndex++) {
        	kprintf("Top of for (update): NCBA 0x%x\n", nextCacheByteAddr);

		nextCacheByteAddr += (sizeof(SWIM3_GCR_SELF_SYNC) *
		    (DriveStatus->diskFormat.gap2Length - 1));

	        nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat *)
		    nextCacheByteAddr;



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

	    nextCacheByteAddr = (BytePtr) nextSwimIIIGCRSector + (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1); // + 1204;

	}
	else
// #endif // GCR_ALWAYS_FROM_SCRATCH
	{

	    /*
	     * Offset to the first sector.
	     */

	    nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat *) nextCacheByteAddr;

            kprintf("Top of else: NCBA 0x%x\n", nextCacheByteAddr);
	    for (i=0; i<200; i++) {
		ByteMove(SWIM3_GCR_SELF_SYNC, nextCacheByteAddr,
		    sizeof(SWIM3_GCR_SELF_SYNC));
		// X Server says sizeof(nextSwimIIIGCRSector->addressMarkSync)
		nextCacheByteAddr += 6;
	    }
            kprintf("Done Writing Sync Marks: NCBA 0x%x\n", nextCacheByteAddr);

	    /*
	     * Write out the escaped data for all of the sectors.
	     */

	    // DriveStatus->diskFormat.sectorsPerTrack = 12;
	    nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat*)nextCacheByteAddr;
	    for (sectorIndex = 0; sectorIndex < DriveStatus->diskFormat.sectorsPerTrack; sectorIndex++) {

            	kprintf("Top of for (scratch): NCBA 0x%x\n", nextCacheByteAddr);
		dprintf(DEBUG_VERBOSE, "Initializing track with %d gap.\n",
		    DriveStatus->diskFormat.gap2Length);
		for (i=1; i<DriveStatus->diskFormat.gap2Length; i++) {
		    ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextCacheByteAddr,
			 sizeof(SWIM3_GCR_SELF_SYNC));
		    nextCacheByteAddr += 6;
		}
            	kprintf("Done with inter-sector gap: NCBA 0x%x\n", nextCacheByteAddr);
		nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat*)nextCacheByteAddr;

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

            	kprintf("Writing sync byte field: NCBA 0x%x\n", nextCacheByteAddr);
		// for (i=0; i<8; i++)
		    ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextSwimIIIGCRSector->addressMarkSync,
			 sizeof(nextSwimIIIGCRSector->addressMarkSync));

		/*
		 * Now copy in the escaped Address Mark.
		 */
            	kprintf("Writing Address Mark: NCBA 0x%x\n", nextCacheByteAddr);

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
            	kprintf("Writing splice gap: NCBA 0x%x\n", nextCacheByteAddr);

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

            	kprintf("Writing data mark sync: NCBA 0x%x\n", nextCacheByteAddr);
		ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextSwimIIIGCRSector->dataMarkSync,
			 sizeof(nextSwimIIIGCRSector->dataMarkSync));

		/*
		 * Now copy in the escaped Data Mark.
		 */

            	kprintf("Writing data mark: NCBA 0x%x\n", nextCacheByteAddr);
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

            	kprintf("Writing blank set of tag data...: NCBA 0x%x\n", nextCacheByteAddr);
            	kprintf("tagdata address is 0x%x\n", nextSwimIIIGCRSector->tagData);
		*nextSwimIIIGCRSector->tagData = 0;

		ByteMove(nextSwimIIIGCRSector->tagData,
			 nextSwimIIIGCRSector->tagData + 1,
			 sizeof(nextSwimIIIGCRSector->tagData) - 1);

		/*
		 * Write out the data field followed by room for nibblization expansion
		 * and data field checksum, then write out the slip bytes terminater.
		 */

            	kprintf("Writing data field: NCBA 0x%x\n", nextCacheByteAddr);
		*nextSwimIIIGCRSector->dataField = DriveStatus->formatByte;

		ByteMove(nextSwimIIIGCRSector->dataField,
			 nextSwimIIIGCRSector->dataField + 1,
			 sizeof(nextSwimIIIGCRSector->dataField) - 1);

            	kprintf("Writing slip byte: NCBA 0x%x\n", nextCacheByteAddr);
		ByteMove(SWIM3_GCR_SLIP_BYTE,
			 nextSwimIIIGCRSector->dataFieldTerminator,
		      sizeof(nextSwimIIIGCRSector->dataFieldTerminator));

		/*
		 * Finish up the sector field by writing out the inter-sector gap with DMA byte
		 * alignment adjustments.
		 */

            	kprintf("Writing interSectorGap: NCBA 0x%x\n", nextCacheByteAddr);
#if 0
		ByteMove(SWIM3_GCR_SELF_SYNC,
			 nextSwimIIIGCRSector->interSectorGap,
			 sizeof(SWIM3_GCR_SELF_SYNC) - 1);
#endif

#if 0
		ByteMove(nextSwimIIIGCRSector->interSectorGap,
			 nextSwimIIIGCRSector->interSectorGap + (sizeof(SWIM3_GCR_SELF_SYNC) - 1),
			 sizeof(nextSwimIIIGCRSector->interSectorGap) - (sizeof(SWIM3_GCR_SELF_SYNC) - 1) - sectorDataAlignment);
#endif

		/*
		 * Offset to the next sector and back up for the DMA byte alignment adjustment.
		 */

		ByteMove(SWIM3_GCR_SELF_SYNC,
		    &nextSwimIIIGCRSector->interSectorGap,1);
		nextSwimIIIGCRSector++;

		/* Don't substract sectorDataAlignment here. */
		//nextSwimIIIGCRSector = (rawSwimIIIGCRSectorFormat *) ((BytePtr) nextSwimIIIGCRSector - sectorDataAlignment);
		nextCacheByteAddr = (BytePtr) nextSwimIIIGCRSector;

	    }

	    /*
	     * Offset to the next side of sectors.
	     */

	    nextCacheByteAddr = (BytePtr) nextSwimIIIGCRSector;

	    /*
	     * Lastly, write out the "End Of Transfer" command at the end.
	     */

	    for (i=0; i<4; i++) *nextCacheByteAddr++=0x3F;
	    ByteMove(SWIM3_TERMINATE_DMA_TRANSFER,
		     nextCacheByteAddr,
		     (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1));

	    nextCacheByteAddr += (sizeof(SWIM3_TERMINATE_DMA_TRANSFER) - 1);

	    if (sideIndex == 0)
	        track_offset[DriveStatus->unit]=(uint_t)(nextCacheByteAddr - (BytePtr)DriveStatus->tcBuffer.logicalAddress);

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

	    if (sideIndex == 0)
		track_offset[DriveStatus->unit] = (uint_t)(nextCacheByteAddr - (BytePtr)DriveStatus->tcBuffer.logicalAddress);

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
    SynchronizeIO(); delay(10);

    /*
     * Wait for the time delay to expire.
     */
    while (*FloppySWIMIIIRegs.rwTimer != 0)
	SynchronizeIO(); delay(10);

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
    SynchronizeIO(); delay(10);

    /*
     * Test to see how the drive's SEL line should be set, Then
     * do it through the HDSEL on the SWIMBASE.
     */

    if (stat_cmd & M_SEL) {

	*FloppySWIMIIIRegs.rHandshakewModeOnes = MS_SIDE1_NOT_SIDE0;

    } else {

	*FloppySWIMIIIRegs.rModewModeZeroes = MS_SIDE1_NOT_SIDE0;

    }

    SynchronizeIO(); delay(10);

    /*
     * Now set the SWIMBASE's Phase lines to address the drive's
     * status/command bits (get rid of the SEL signal state,
     * as here it would toogle the SWIMBASE's PH3 line and the
     * drive's LSTRB line.
     */
    *FloppySWIMIIIRegs.rwPhase = stat_cmd & ~M_SEL;
    SynchronizeIO(); delay(10);

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

    delay(10); /* Added DAG */

    /*
     * Test to see how the drive responded.
     */

    returnResult = (*FloppySWIMIIIRegs.rHandshakewModeOnes & HR_SENSE_IN) != 0;
    SynchronizeIO(); delay(10);

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
     * that the pulse is exactly 1 microsecond wide.  Do a
     * SWIMIIISmallWait(1) first to reset the SWIM chip's timer
     * to ensure roughly a full microsecond clock.
     */

    delay(10); /* Added DAG */
    *FloppySWIMIIIRegs.rwPhase = *FloppySWIMIIIRegs.rwPhase | M_LSTRB;
    SynchronizeIO();
    SwimIIISmallWait(2); /* Changed to 2 DAG */

    *FloppySWIMIIIRegs.rwPhase = *FloppySWIMIIIRegs.rwPhase & ~M_LSTRB;
    SynchronizeIO();

    delay(10); /* Added DAG */

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

    /* The old Mac OS X Server 1.0 stuff showed 500 here, but that
     * isn't sufficient.  The delay must be 650 usec. between any
     * write operation and a head change.  With unaccelerated machines,
     * it will work with a 500 usec. delay, but that behavior is
     * not guaranteed.  Feel free to change it if you really just
     * have to get that last 150 microseconds of performance out
     * of this driver....  DAG :-p
     */

    delay(650);
    if (HeadNumber == 0) {

	(void) SwimIIISenseSignal(S_RDDATA0);

    } else
	(void) SwimIIISenseSignal(S_RDDATA1);

    /* DAG This should really be 10 usec, not 100 msec, but.... */
    FloppyTimedSleep(100);
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
	SynchronizeIO(); delay(10);

	*FloppySWIMIIIRegs.rHandshakewModeOnes = MS_ENABLE_DRIVE_1;
	SynchronizeIO(); delay(10);

    } else {

	/*
	 * De-select the internal drive and select and enable
	 * the external drive.
	 */

	*FloppySWIMIIIRegs.rModewModeZeroes = MS_ENABLE_DRIVE_1;
	SynchronizeIO(); delay(10);

	*FloppySWIMIIIRegs.rHandshakewModeOnes = MS_ENABLE_DRIVE_2;
	SynchronizeIO(); delay(10);

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
     * The floppy drive requires a minimum of 650 usec. after
     * a write before stepping.  Assume we're stepping after a
     * write, and give it a whole msec.
     *
     * Note: for older drives, need to wait 12 ms.  This is not
     * necessary for any recent drives, AFAIK, but might be of interest
     * if this code base is ever used on 68k Macs.
     */

    FloppyTimedSleep(1);

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
	SynchronizeIO(); delay(10);

	/* Note that the SWIM III chip handles multiple steps at once,
	 * and we have some built-in delays in the main seek routine,
	 * so we don't have to worry about the timing between pulses.
	 * However, on old machines (fast 68k Macs and non-SWIM III
	 * PowerMacs), this could be an issue.  In that case, this
	 * step code would be in a loop, sending one step pulse per track
	 * to step.  The code would need to wait at least 18 usec.
	 * after the /Step line (IT_STEPPING_DONE) goes high before
	 * sending the next step pulse, or else you may lose steps.
	 */

	/*
	 * Wait for stepping to complete. Wait for 10 seconds before timing out.
	 */
	/* DAG: Changing 10000 for wait time so that it's relative to the
	 * step count.  The step is supposed to take 80 ms.  Fudge it a little
	 * and give it 100 ms.  This will potentially improve the worst
	 * case for a recalibrate on a hosed floppy drive from 400 seconds to
	 * just over 4 seconds.
	 */
	if ((errorCode = WaitForEvent((StepCount * 100), MS_DO_STEPPING, IT_STEPPING_DONE)) != noErr)
	    errorCode = cantStepErr;

	/* We have to wait a bit after stepping is finished.
	 * READY may not go low until up to 150 usec after
	 * stepping done (in theory) so wait a millisecond
	 * before checking, then wait for it to go high.
	 */

	SleepUntilReady(1);

	/* Now that it has gone high (this may take almost a second),
	   We have to wait for it to settle.
	 */
	/*
	 * DAG Adding additional delay here causes the hardware to work
	 * when it otherwise would not.  In other words, drive is not
	 * really ready when it says it is.  This delay has been moved
	 * out to the calling functions.  The delay is 300 msec
	 * (yes, that's milliseconds) for a read, 600 for a write.
	 */
	// FloppyTimedSleep(300);

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
    SynchronizeIO(); delay(10);

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
     * In the worst case, we have to wait up to 620 usec before it's
     * safe to read.  Wait a whole millisecond.
     */
    FloppyTimedSleep(1);

    /*
     * Make that the error register is cleared.
     */

    lastErrorsPending = 0;
    dummyValue = *FloppySWIMIIIRegs.rError;
    SynchronizeIO(); delay(10);

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
    SynchronizeIO(); delay(10);

    /*
     * Clear any pending interrupts.
     */

    dummyValue = *FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO(); delay(10);
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

    lastErrorsPending = 0;
    dummyValue = *FloppySWIMIIIRegs.rError;
    SynchronizeIO(); delay(10);

    /*
     * Clear the action bit and put the SWIM into write mode. This
     * will also reset the CRC register and guarantee that we can
     * read the next group of mark bytes.
     */

    SwimIIIDisableRWMode();

    *FloppySWIMIIIRegs.rHandshakewModeOnes = MS_NOT_READ_WRITE;
    SynchronizeIO(); delay(10);

    /*
     * Clear any pending interrupts.
     */

    dummyValue = *FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO(); delay(10);

}

void SwimIIISetFormatMode(void)
{
    register unsigned char dummyValue;

    dprintf(DEBUG_VERBOSE, "SetFormatMode called.\n");

    /*
     * Make sure that the error register is cleared.
     */
    lastErrorsPending = 0;
    dummyValue = *FloppySWIMIIIRegs.rError;
    SynchronizeIO(); delay(10);

    /*
     * Clear the action bit and put the SWIM into read mode before
     * we touch any other register in case doing so goofs up the SWIM.
     */

    *FloppySWIMIIIRegs.rModewModeZeroes = MS_DO_ACTION;
    SynchronizeIO(); delay(10);

    /*
     * This flag sets the SWIM III into a state where it
     * writes immediately after it sees an index mark.  This
     * allows for track-at-once writing, which is required
     * for formatting in any sane fashion (i.e. without doing
     * painfully accurate timing in the driver itself...).
     */
    *FloppySWIMIIIRegs.rHandshakewModeOnes=MS_FORMAT_MODE;
    SynchronizeIO(); delay(10);

    /*
     * Possible small timing constraint. Give it 10 msec.
     * just to hedge our bets.
     */
    FloppyTimedSleep(10);

    /*
     * Clear any pending interrupts.
     */
    dummyValue=*FloppySWIMIIIRegs.rInterrupt;
    SynchronizeIO(); delay(10);
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
    SwimIIIRegs Regs;

    if ((device < 0) || device > NFD) {
	printf("WARNING: Invalid device in HALISR_DMA!\n");
	return;
    }

    Regs =  FloppySWIMIIIRegsList[device];
    if (!Regs.rwInterruptMask) {
	printf("WARNING: Uninitialized device in HALISR_DMA!\n");
	return;
    }

    dprintf(DEBUG_VERBOSE, "HALISRDMA: ");
    // HALISRHandler(device, ssp);
    dprintf(DEBUG_VERBOSE, "Interrupt mask of SWIM is 0x%x\n", *Regs.rwInterruptMask);
    PrintRegs(device);

}
void HALISRHandler(int device,
		   void *ssp)
{
    SwimIIIRegs Regs;
    UInt32 pendingInterrupts = 0;

    if ((device < 0) || device > NFD) {
	printf("WARNING: Invalid device in HALISR!\n");
	return;
    }

    Regs =  FloppySWIMIIIRegsList[device];
    if (!Regs.rwInterruptMask) {
	printf("WARNING: Uninitialized device in HALISR!\n");
	return;
    }

    /*
     * Save the pending interrupts that may have occured.
     */

    pendingInterrupts = *Regs.rInterrupt;
    SynchronizeIO(); delay(10);

    // See if the Floppy was the one that requested an interrupt.
    dprintf(DEBUG_VERBOSE, "HALISR:0x%x ",pendingInterrupts);              
    if(pendingInterrupts &0x08)PrintDMA();
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

	*Regs.rwInterruptMask = 0;
	SynchronizeIO(); delay(10);

	*Regs.rModewModeZeroes = MS_ENABLE_INTS;
	SynchronizeIO(); delay(10);

	/*
	 * DAG This was a POTENTIAL RACE CONDITION in Mac OS X,
	 * since interrupts are handled in a thread instead of
	 * on a primary interrupt.  In some cases, this could cause
	 * the code to check the error condition before it has been
	 * stored by the interrupt handler thread.  The call to
	 * SetOSEvent -=MUST=- be after ALL hardware information
	 * has been stored.
	 */
	// SetOSEvent(driveOSEventIDptr,
		   // pendingInterrupts);

	/*
	 * Did an error occur.
	 */

	if (pendingInterrupts & IT_ERROR_OCCURRED) {

	    /*
	     * Save the errors away for the rest of the HAL.
	     */

	    lastErrorsPending = *Regs.rError;
	    SynchronizeIO(); delay(10);

	}
	// Tell the system that this interrupt has been serviced.

#ifdef PDMSUPPORT
	if (powermac_info.class == POWERMAC_CLASS_PDM) {
	    floppy_amic_ack();
	}
#endif
	SetOSEvent(driveOSEventIDptr,
		   pendingInterrupts);
    } else {
	dprintf(DEBUG_VERBOSE, "HALISR: Pending interrupt is ZERO ");
    // Tell the system that this interrupt was for somebody else.
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
	dprintf(DEBUG_VERBOSE, "GCR Disk\n");

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
	SynchronizeIO(); delay(10);

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
	SynchronizeIO(); delay(10);

	*FloppySWIMIIIRegs.rwParams = 0x88;	// Late/Early

	SynchronizeIO(); delay(10);

	/*
	 * Put the drive into GCR mode.
	 */

	modeCommand = C_SETGCR;

    } else {

	dprintf(DEBUG_VERBOSE, "MFM Disk\n");
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

	SynchronizeIO(); delay(10);

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
	SynchronizeIO(); delay(10);

	*FloppySWIMIIIRegs.rwParams = 0x95;	// Late/Early

	SynchronizeIO(); delay(10);

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
    //dprintf (" HALSetFormatMode - SleepUntilReady "); 
    {
	OSStatus rc;
	rc = SleepUntilReady(30);
	//dprintf (" HALSetFormatMode - SleepUntilReady returned! ");

	/* SWIM and friends require 600 msec. after format change before
	 * you do anything else with them.
	 */
	FloppyTimedSleep(600);
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

	    dprintf(DEBUG_VERBOSE, "1 meg media\n");
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
		dprintf(DEBUG_VERBOSE, "2 meg media\n");

	    } else {

		/*
		 * We must have a 4 MB (i.e. ED) diskette.
		 */

		DriveStatus->disketteType = DISKETTE_4MEG;
		dprintf(DEBUG_VERBOSE, "4 meg media!?!?!\n");

	    }

	}

    } else {
	dprintf((DEBUG_VERBOSE | DEBUG_GENERAL), "Bogus disk drive!\n");
	// DriveStatus->installed = DRV_NOT_INSTALLED;
	DriveStatus->diskInPlace = NO_DISKETTE;
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
     * Wait one millisecond before looking for the ready signal.Q
     * Bzzt.  Spec requires 50 msec.
     */

    errorCode = SleepUntilReady(50);

dprintf(DEBUG_VERBOSE, "swimiiicommonhal.c:HALPowerUpDrive:ret=%d ",errorCode );
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

    /* Wait 200 msec. to comply with spec. */
    FloppyTimedSleep(200);
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

	/* Then wait 300 msec. for the drive speed to become constant. */
	FloppyTimedSleep(300);

	if (errorCode != noErr) {
		dprintf(DEBUG_VERBOSE, "HALSeekDrive: drive did not become ready!\n");
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

//      dprintf(DEBUG_VERBOSE, "swim3commonhal.c:HALRecalDrive: stepcount=%d ",StepCount);
    FloppyTimedSleep(6);
    while (StepCount-- &&
	   SwimIIISenseSignal(S_NOTTRK0)) {

	if ((errorCode = SwimIIIStepDrive(-1)) != noErr)
	    break;

	dprintf(DEBUG_VERBOSE, "Stepping...\n");
	/*
	 * We have to wait at least 6 msec. before the track 0 sensor
	 * stops lying to us.  On older (pre-superdrive) hardware,
	 * we would have to wait at least 12 msec.
	 */
	FloppyTimedSleep(6);
    }

    /* Now we need to wait 300 msec. for the drive speed to become constant. */
    FloppyTimedSleep(300);
    /*
     * Update the current track location.
     */
    //dprintf(DEBUG_VERBOSE, "RecalDrive: out of big stepcount=%d loop err=%d ",StepCount,errorCode); 
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
*	FUNCTION : HALSectorZeroFound
*
*	Try to find the first sector on the track.  Return success or
*	failure.
*
************************************************************************/

OSStatus
HALSectorZeroFound(DriveStatusType * DriveStatus)
{

    short errorCode;
    int i, found=0, sector, retrycount=0;
    register unsigned char dummyValue;

/* @@@ BUG BUG BUG @@@ DAG For now, disable this. */
/* This causes GCR 800k formatting to fail in ways that make no
   sense -- it wedges the SWIM chip so that subsequent writes
   never generate an interrupt! */
// return noErr;

// dprintf(DEBUG_VERBOSE, "Leading:\n");
// PrintRegs();
PrintDMA();
// dprintf(DEBUG_VERBOSE, "\n");

    dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: Calling ResetDMAChannel\n");
    /*
     * Reset the DMA channel we're trying to do error recovery.
     */
    ResetDMAChannel(DriveStatus->unit);

    /*
     * Make sure that we don't get any false triggers by
     * preclearing the events we're waiting for.
     */

    dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: Calling CancelOSEvent\n");
    CancelOSEvent(driveOSEventIDptr,
		  IT_ID_HEADER_READ);

    /*
     * Get the byte stream from the correct head and put us into read mode.
     */

    dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: Calling SwimIIIHeadSelect\n");

//dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: calling SwimIIIHeadSelect ");
    SwimIIIHeadSelect(DriveStatus->wantedAddress.address.items.side);

//dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: calling SwimIIISetReadMode ");
    // moved delay into head select code.
    // delay(100000);

    dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: Calling SwimIIISetReadMode\n");
    SwimIIISetReadMode();


    /*
     * Setup the timeout to trying to capture the first sector
     * address to fly bye. Wait a maximum of two diskette rotations
     * (0.4 seconds).
     *
     * If we get a noAdrMkErr, assume that this track didn't format
     * correctly.  If we get a sector header, then keep polling
     * the chip approx. 80 times per revolution until we find the
     * first sector in the track or until we time out (2 full
     * rotations of the medium).
     */

    /* DAG changed 400 to 4000... */
    dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: Calling WaitForEvent\n");
    while ((++retrycount <= 3) && (!found)) {
#if 0
	if ((errorCode = WaitForEvent(400, MS_DO_ACTION, IT_ID_HEADER_READ)) == noErr)
#endif
	{

	    /*
	     * DAG - Don't waste a debug slot on this, since it is only
	     * interesting/useful when debugging the cache format code....
	     */
#define DEBUG_SZF
#ifdef DEBUG_SZF
	    /* Give logging time to catch up. */
	    // FloppyTimedSleep(10000);
#endif

	    SwimIIISetReadMode(); delay(10);

	    *FloppySWIMIIIRegs.rHandshakewModeOnes = MS_DO_ACTION;
	    delay(10);
	    if ((*FloppySWIMIIIRegs.rCurrentSector & 0x7F) ==
		DriveStatus->diskFormat.firstSectorInTrack) {
		    printf("FOUND IMMEDIATELY.  GOOD.\n");
	    }
	    delay(10);
	    /* Try this for a while -- 1 sec. plus processing time */
	    for (i=0; i<100000; i++) {

		sector = *FloppySWIMIIIRegs.rCurrentSector & 0x7F;
		// dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: Calling SynchronizeIO()\n");
#ifdef DEBUG_SZF
		printf("%d\n", sector);
#endif
		SynchronizeIO(); delay(10);

		if (sector == DriveStatus->diskFormat.firstSectorInTrack) {
		    found = 1;
		    break;
		}

		/*
		 * Exit the retry loop as successful.
		 */

		// dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: Exiting retry loop\n");
#if 0
		switch(DriveStatus->diskFormat.drvBlockMax) {
		    case 2880:
			/* Wait 5 milliseconds between polls */
			FloppyTimedSleep(5);
			break;
		    case 1440:
		    case 1600:
		    case 800:
			/* Wait 10 milliseconds between polls */
			FloppyTimedSleep(10);
			break;
		}
#endif

	    }
	    *FloppySWIMIIIRegs.rModewModeZeroes = MS_DO_ACTION;
	    break;
#if 0
	} else {
	    dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: No address mark found.  %s\n",
		(retrycount < 3) ? "Retrying." : "Bailing.");
#endif
	}
    }

    SwimIIIDisableRWMode();

    lastErrorsPending = 0;
    dummyValue = *FloppySWIMIIIRegs.rError;
    SynchronizeIO(); delay(10);

    dprintf(DEBUG_VERBOSE, "HALSectorZeroFound: returning.\n");
    if (found) {
	printf("Sector %d found.\n", DriveStatus->diskFormat.firstSectorInTrack);
	return noErr;
    } else {
	printf("WARNING: NO SECTOR %d FOUND!\n", DriveStatus->diskFormat.firstSectorInTrack);
	return noNybErr;
    }
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

// dprintf(DEBUG_VERBOSE, "Leading:\n");
// PrintRegs();
PrintDMA();
// dprintf(DEBUG_VERBOSE, "\n");

    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling ResetDMAChannel\n");
    /*
     * Reset the DMA channel we're trying to do error recovery.
     */
    ResetDMAChannel(DriveStatus->unit);

    /*
     * Make sure that we don't get any false triggers by
     * preclearing the events we're waiting for.
     */

    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling CancelOSEvent\n");
    CancelOSEvent(driveOSEventIDptr,
		  IT_ID_HEADER_READ);

    /*
     * Get the byte stream from the correct head and put us into read mode.
     */

    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SwimIIIHeadSelect\n");

//dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: calling SwimIIIHeadSelect ");
    SwimIIIHeadSelect(DriveStatus->wantedAddress.address.items.side);

//dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: calling SwimIIISetReadMode ");
    // moved delay into head select code.
    // delay(100000);

    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SwimIIISetReadMode\n");
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
    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling WaitForEvent\n");
    if ((errorCode = WaitForEvent(400, MS_DO_ACTION, IT_ID_HEADER_READ)) == noErr) {

	dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SwimIIIDisableRWMode\n");
	SwimIIIDisableRWMode();

	DriveStatus->currentAddress.address.items.track = *FloppySWIMIIIRegs.rCurrentTrack & 0x7F;
	dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SynchronizeIO()\n");
	SynchronizeIO(); delay(10);

	DriveStatus->currentAddress.address.items.side = (*FloppySWIMIIIRegs.rCurrentTrack & 0x80) >> 7;
	dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SynchronizeIO()\n");
	SynchronizeIO(); delay(10);

	DriveStatus->currentAddress.address.items.sector = *FloppySWIMIIIRegs.rCurrentSector & 0x7F;
	dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SynchronizeIO()\n");
	SynchronizeIO(); delay(10);

	DriveStatus->currentAddress.address.items.blockSize = *FloppySWIMIIIRegs.rwGapSize;
	dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SynchronizeIO()\n");
	SynchronizeIO(); delay(10);

	/*
	 * Exit the retry loop as successful.
	 */

	dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Exiting retry loop\n");
    } {
// #ifdef NOTYETDAG
	long xxx; dbdma_regmap_t *regmap;
      xxx = *FloppySWIMIIIRegs.rInterrupt; eieio(); delay(10);
	      // SynchronizeIO();
	     dprintf(DEBUG_VERBOSE, "HALGetNextAddr: 0x%lx ",xxx);
#ifdef MKLINUX_SOURCE
      regmap = (dbdma_regmap_t *)POWERMAC_IO(org_mklinux_iokit_swim3_get_io_base_addr() + 0x8100);
#else
      regmap = (dbdma_regmap_t *)org_mklinux_swim3_dmvaddr[DriveStatus->unit];
#endif
      xxx = regmap->d_status; eieio();
             dprintf(DEBUG_VERBOSE, "DMASTATUS= 0x%lx ",xxx);
// #endif
    }
    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Done with mess.\n");
    /*
     * Cancel read mode and return any errors.
     */

    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: Calling SwimIIIDisableRWMode.\n");
    SwimIIIDisableRWMode();

// dprintf(DEBUG_VERBOSE, "Trailing:\n");
// PrintRegs();
PrintDMA();
// dprintf(DEBUG_VERBOSE, "\n");

    dprintf(DEBUG_VERBOSE, "HALGetNextAddressID: returning.\n");
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

//dprintf(DEBUG_VERBOSE, "swimiiicommonhal.c:HALReadSector: ");
    /*
     * Start the SWIM looking for the wanted sector.
     */

    *FloppySWIMIIIRegs.rwSectorCount = 1;
    SynchronizeIO(); delay(10);

    *FloppySWIMIIIRegs.rwGapSize = 0;
    SynchronizeIO(); delay(10);
// dprintf(DEBUG_VERBOSE, "sector=%d side=%d ", DriveStatus->wantedAddress.address.items.sector,DriveStatus->wantedAddress.address.items.side );
    // dprintf(DEBUG_VERBOSE, "rd:%d:%d:%d ",DriveStatus->wantedAddress.address.items.side,DriveStatus->wantedAddress.address.items.track,DriveStatus->wantedAddress.address.items.sector);
    *FloppySWIMIIIRegs.rwFirstSector = DriveStatus->wantedAddress.address.items.sector;
    SynchronizeIO(); delay(10);

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
dprintf(DEBUG_VERBOSE, "HALReadSector:databufferlogical=0x%04x,phys=0x%04x ",
	(unsigned int)DriveStatus->transferAddress.logicalAddress,
	(unsigned int)DriveStatus->transferAddress.physicalAddress);
    errorCode = StartDMAChannel(DriveStatus->unit,
				DriveStatus->transferAddress.physicalAddress,
				DriveStatus->diskFormat.DMASectorSize,
				DMA_READ);

    /*
     * Cancel read mode, turn off sector searching and return our error code.
     */

    if (errorCode != 0)
	dprintf(DEBUG_VERBOSE, "HALRead failed (%d) for %d:%d:%d ", errorCode, DriveStatus->wantedAddress.address.items.side, DriveStatus->wantedAddress.address.items.track, DriveStatus->wantedAddress.address.items.sector);
//     dprintf(DEBUG_VERBOSE, "R:%d ",DriveStatus->wantedAddress.address.items.sector); 
    SwimIIIDisableRWMode();

    *FloppySWIMIIIRegs.rwFirstSector = 0xFF;
    SynchronizeIO(); delay(10);
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
//dprintf(DEBUG_VERBOSE, "HALWrtSec: sec=%d ",DriveStatus->wantedAddress.address.items.sector); 

    /*
     * Start the SWIMBASE looking for the wanted sector.
     */

    *FloppySWIMIIIRegs.rwSectorCount = 1;
    SynchronizeIO(); delay(10);

    *FloppySWIMIIIRegs.rwGapSize = 0;
    SynchronizeIO(); delay(10);

    *FloppySWIMIIIRegs.rwFirstSector = DriveStatus->wantedAddress.address.items.sector;
    SynchronizeIO(); delay(10);

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

    DMACorrectedAddress = (BytePtr) DriveStatus->transferAddress.logicalAddress - DMACorrectionLength;

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

    errorCode = StartDMAChannel(DriveStatus->unit, DMACorrectedAddress,
				DMACorrectionLength,
				DMA_WRITE);
//dprintf(DEBUG_VERBOSE, "wr:%d:%d:%d:%c ",DriveStatus->wantedAddress.address.items.side,DriveStatus->wantedAddress.address.items.track,DriveStatus->wantedAddress.address.items.sector,*(char *)(DMACorrectedAddress+100));
    //dprintf(DEBUG_VERBOSE, "w:%d ",DriveStatus->wantedAddress.address.items.sector);
    /*
     * Cancel read mode, turn off sector searching and return our error code.
     */
    if (errorCode != 0)
	dprintf(DEBUG_VERBOSE, "HALWrite failed for %d:%d:%d ", DriveStatus->wantedAddress.address.items.side, DriveStatus->wantedAddress.address.items.track, DriveStatus->wantedAddress.address.items.sector);
    SwimIIIDisableRWMode();

    *FloppySWIMIIIRegs.rwFirstSector = 0xFF;
    SynchronizeIO(); delay(10);

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
    void *DmaAddr;
    int retrycount=0;

    dprintf(DEBUG_VERBOSE, "in HALFormatTrack\n");

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

    unsigned char save;
	/*
	 * Preset gap1 for the first time through a side loop, plus
	 * set the head and sector number in the sector ID data.
	 */

	gap1Length = DriveStatus->diskFormat.gap1Length;

	DriveStatus->wantedAddress.address.items.side = diskSide;
	// diskSide = DriveStatus->wantedAddress.address.items.side;
	DriveStatus->wantedAddress.address.items.sector = 1;

	/*
	 * The number of sectors per track and other info can vary
	 * between tracks.  Call the format routines to be sure
	 * that we're not laying down junk.
	 */
	save = lastSectorsPerTrack;
	lastSectorsPerTrack=DriveStatus->diskFormat.sectorsPerTrack;
	if (DriveStatus->diskFormat.GCR)
	    FormatGCRCacheSWIMIIIData(DriveStatus);
	else
	    FormatMFMCacheSWIMIIIData(DriveStatus);
	lastSectorsPerTrack = save;

	SynchronizeIO(); delay(10);

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

	SwimIIIHeadSelect(DriveStatus->wantedAddress.address.items.side);
	// FloppyTimedSleep(1);

	/*
	 * Turn on write mode.
	 */
	/*
	 * Not yet.  Do it in the DMA Write.  This will come back if we
	 * ever do IWM, SWIM, or SWIM II support.
	 */

	// SwimIIISetWriteMode();

	/*
	 * Now write out the bytes in gap 4A, the 12 sync bytes and the
	 * address mark that precedes the sector header data, but if we
	 * get an error, return it.
	 */

	if (DriveStatus->wantedAddress.address.items.side == 0)
	    DmaAddr = DriveStatus->tcBuffer.logicalAddress;
	else
	    DmaAddr = DriveStatus->tcBuffer.logicalAddress + track_offset[DriveStatus->unit];

	PrepareCPUCacheForDMAWrite();
	dprintf(DEBUG_VERBOSE,
		"Track offset is %d (0x%x)\n", track_offset[DriveStatus->unit],
		track_offset[DriveStatus->unit]);
	errorCode=StartDMAChannel(DriveStatus->unit, (caddr_t) kvtophys(DmaAddr), 0x8000, DMA_WRITE); /* 0x8000 for large */
	*FloppySWIMIIIRegs.rModewModeZeroes=MS_DO_ACTION;
	SynchronizeIO(); delay(10);
	/*
	 * DAG See note about this bit's double meaning in
	 * SwimIIISetFormatMode
	 */
	*FloppySWIMIIIRegs.rModewModeZeroes=MS_FORMAT_MODE;
	SynchronizeIO(); delay(10);


	/* Done formatting this side.  Check it. */
	if (errorCode == noErr && HALSectorZeroFound(DriveStatus) == noErr) {
		/*
		 * If formatting worked, reset the retry count to zero so that
		 * we get the full number of retries for the second side.
		 */
		retrycount = 0;
		diskSide++;
	} else {
		/* If GCR, we need to keep reading address IDs until we get
		 * sector 0, or if we don't in a reasonable time,
		 * decrease the number of sync patterns between sectors.
		 * Further, if there's too long a gap, we need to increase
		 * the number.
		 * 
		 * This isn't worth doing at the moment, though, so we'll
		 * just retry the track format and hope for the best.
		 */
		if (DriveStatus->diskFormat.GCR) {
#ifdef CHANGE_GAP
		    ++DriveStatus->diskFormat.gap2Length;
#endif
		    if (DriveStatus->diskFormat.gap2Length >
			DriveStatus->diskFormat.gap3Length)
			    return noNybErr;
		    else lastSectorsPerTrack = -1;
#ifndef CHANGE_GAP
		    if (++retrycount > 4) {
			return noNybErr;
		    }
#endif
		} else {
		    if (++retrycount > 4) {
			return noNybErr;
		    }
		}
	}

	// SwimIIIDisableRWMode();
    }

    dprintf(DEBUG_VERBOSE, "left HALFormatTrack, returning %d\n", errorCode);
    return errorCode;

}

#if 0

         // if( ( errorCode = SwimIIIMFMWriteMark( MFM_INDEX_MARK, gap4ALength ) ) == noErr )
	{

	    /*
	     * Write out all of the sectors plus their gaps, starting
	     * with sector number one (MFM numbers sectors starting at
	     * one).
	     */

	    // while ((errorCode == noErr) &&
		   // (DriveStatus->wantedAddress.address.items.sector <= sectorsPerTrack)) {

		/*
		 * Write out the sector ID head field.
		 */

                         // if( ( errorCode = SwimIIIMFMWriteSectorID( &DriveStatus->wantedAddress,
		                                                                                                            // gap1Length ) ) == noErr )
		{

		    /*
		     * Write the next sector data from the dummy sector space.
		     * Exit the loop with any errors. Otherwise, s
		     */

                                 // if( ( errorCode = SwimIIIMFMWriteDataField( DriveStatus->logicalAddress,
		                                                                                                                         // gap2Length ) ) == noErr )
		    {

			/*
			 * Set the sector ID data data pointer for the next sector.
			 */

			// DriveStatus->wantedAddress.address.items.sector++;

                                             // DriveStatus->wantedAddress.address.items.sector += sectorsPerTrack / 2;
			                                             
			                                             // if( DriveStatus->wantedAddress.address.items.sector > sectorsPerTrack )
			                                                     // DriveStatus->wantedAddress.address.items.sector -= sectorsPerTrack - 1;

			// DriveStatus->transferAddress.logicalAddress += DriveStatus->diskFormat.logicalSectorSize;

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

                         // errorCode = SwimIIIMFMWriteGapField( gap3Length );

	    }
	}

	/*
	 * Cancel write mode.
	 */

	// SwimIIIDisableRWMode();

    }

    return errorCode;

}
#endif
#endif // NFD > 0
