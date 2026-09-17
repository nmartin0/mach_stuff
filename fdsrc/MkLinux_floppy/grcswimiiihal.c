// #define OLD_DBDMA
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
*	File:		GRCSwimIIIHAL.c
*
*	Contains:	Floppy driver HAL routines for SWIMIII on PowerSurge machines.
*
************************************************************************/

#include <fd.h>
#if NFD > 0

//#include <DriverSupport.h>

#include <machine/types.h>
#include <sys/ioctl.h>
#if 0
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#endif
#include <kern/lock.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppysal.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "swimiii.h"

// #ifdef NOTYETDAG
#ifdef OLD_DBDMA
#include "dbdma_maxwell.h"
#else
#if 0
#include <ppc/POWERMAC/dbdma.h>
#else
#include <floppy_darwin_dbdma.h>
#endif
#include "floppy_dbdma.h"
#endif
// #endif

#if 0
#include "floppy_amic.h"

#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#endif
#include <ppc/proc_reg.h>

#include "swimiiicommonhal.h"
#include <sys/systm.h> // for printf
#include "dprintf.h"

// extern caddr_t org_mklinux_iokit_swim3_cmdbase[NFD];

#if (!MACH_DEBUG)
// #define printf donone

#else
#if (SERIAL_DEBUG)
#define printf kprintf
#endif
#ifdef PDMSUPPORT
void pdm_dma_print(void);

void pdm_dma_print()
{
int val;
char string[1024], tag[5];

string[0]=0;
if (powermac_info.class == POWERMAC_CLASS_PDM) {
	vm_offset_t addra, addrb;
	for (addra = PDM_IO_BASE_ADDR+0x2a000 ; 
	     addra < PDM_IO_BASE_ADDR+0x2a020 ;
	     addra++) {
		addrb=(vm_offset_t) POWERMAC_IO(addra);
		val=via_reg((caddr_t) addrb); eieio();
		sprintf(tag,"%02x ", val);
		strcat(string,tag);
		}
	dprintf(DEBUG_VERBOSE, "\n\r%s\n\r",string);
	}
}
#else
void pdm_dma_print()
{
dprintf(DEBUG_VERBOSE, "PDM DMA PRINT....\n");

}
#endif
#endif

/************************************************************************
*
* Global data for driver.
*
************************************************************************/

extern
SwimIIIRegs FloppySWIMIIIRegs;	/* Floppy controller register sets */
SwimIIIRegs FloppySWIMIIIRegsList[NFD];	/* Floppy controller register sets */

// #ifdef NOTYETDAG
#ifdef OLD_DBDMA
DBDMAChannelRegisters *GRCFloppyDMARegs[NFD];	/* Floppy DMA register sets */
DBDMAChannelConnectionPtr GRCFloppyDMAChannel[NFD];

#else // (!OLD_DBDMA)

dbdma_regmap_t *GRCFloppyDMAChannel[NFD];	/* Floppy DMA register sets */
typedef dbdma_regmap_t DBDMAChannelRegisters[NFD];

void *GRCFloppyDMARegs[NFD];

#define ResetDBDMA dbdma_reset
#define PrepDBDMA donone
#define StopDBDMA dbdma_stop
#define SetDBDMAPhysicalAddress floppy_dbdma_setup
// #define OpenDBDMAChannel donone

#endif // (OLD_DBDMA)
// #endif

#if 0
extern dma_softc_t floppy_amic_softc[];
#endif

LogicalAddress ccCommandsLogicalAddr[NFD] = {NULL, NULL};
PhysicalAddress ccCommandsPhysicalAddr[NFD] = {NULL, NULL};


/************************************************************************
*
* Private data for HAL.
*
************************************************************************/

extern unsigned char lastErrorsPending;

extern UInt32 *driveOSEventIDptr;

void SWIMIIICopyRegs(SwimIIIRegs *dest, SwimIIIRegs *src)
{
	dest->rwData = src->rwData;
	dest->rwTimer = src->rwTimer;
	dest->rError = src->rError;
	dest->rwParams = src->rwParams;
	dest->rwPhase = src->rwPhase;
	dest->rwSetup = src->rwSetup;
	dest->rModewModeZeroes = src->rModewModeZeroes;
	dest->rHandshakewModeOnes = src->rHandshakewModeOnes;
	dest->rInterrupt = src->rInterrupt;
	dest->rwStepCount = src->rwStepCount;
	dest->rCurrentTrack = src->rCurrentTrack;
	dest->rCurrentSector = src->rCurrentSector;
	dest->rwGapSize = src->rwGapSize;
	dest->rwFirstSector = src->rwFirstSector;
	dest->rwSectorCount = src->rwSectorCount;
	dest->rwInterruptMask = src->rwInterruptMask;

}

void SWIMIIIFixGlobals(DriveStatusType *ds)
{
    dprintf(DEBUG_VERBOSE, "SWIMIIIFixGlobals: THIS IS UNIT %d\n", ds->unit);
    SWIMIIICopyRegs(&FloppySWIMIIIRegs, &FloppySWIMIIIRegsList[ds->unit]);
}

/************************************************************************
*
*	FUNCTION : ResetDMAChannel
*
************************************************************************/

OSStatus
ResetDMAChannel(int unit)
{
#if 0
    int info;
#endif

// #ifdef NOTYETDAG
#if 0
    info = powermac_info.class;
    switch(info) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    floppy_amic_init();
	    floppy_amic_end();
	    // floppy_amic_setup((vm_offset_t) floppy_amic_softc[0].base, 0);
	    // replaced by:
	    // floppy_amic_ack();
	    // replaced by:
	    floppy_amic_setup(0,0);
	    break;
	case POWERMAC_CLASS_PCI:
#endif
	    ResetDBDMA(GRCFloppyDMAChannel[unit]);
#ifdef OLD_DBDMA
	    PrepDBDMA(GRCFloppyDMAChannel[unit]);
#else
	    floppy_dbdma_prep(GRCFloppyDMAChannel[unit], ccCommandsLogicalAddr[unit]);
#endif
#if 0
	    break;
	}

#endif
    return noErr;

}


/************************************************************************
*
*	FUNCTION : StopDMAChannel
*
************************************************************************/

OSStatus
StopDMAChannel(int unit)
{
#if 0
    int info;
#endif

// #ifdef NOTYETDAG
#if 0
    info = powermac_info.class;
    switch(info) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    floppy_amic_end();
	    break;
	case POWERMAC_CLASS_PCI:
#endif
	    StopDBDMA(GRCFloppyDMAChannel[unit]);
#if 0
	    break;
	}
#endif

    return noErr;

}


/************************************************************************
*
*	FUNCTION : StartDMAChannel
*
************************************************************************/

OSStatus
StartDMAChannel(int unit, void *DMAAddress,
		unsigned long DMACount,
		unsigned short direction)
{
#if 0
    int info1, info2;
#endif
    OSStatus errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Reset the DMA channel and wait for the DMA channel to freeze
     * before we change either the DMA address or count.
     */

// #ifdef NOTYETDAG
#if 0

    info1 = powermac_info.class;
    switch(info1) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    floppy_amic_init();
	    // floppy_amic_end();
	    break;
	case POWERMAC_CLASS_PCI:
#endif
	    ResetDMAChannel(unit);
#if 0
	    break;
	}
#endif

    /*
     * Set the DMA address and count, then put the DMA register set
     * into the proper read/write mode and enable it. Then fire off
     * the SWIMIII.
     */

#if 0
    info2 = powermac_info.class;
    switch(info2) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    floppy_amic_setup((vm_offset_t) DMAAddress, (unsigned int)DMACount);
	    if (direction == DMA_READ)
		SwimIIISetReadMode();
	    else
		SwimIIISetWriteMode();
	    break;
	case POWERMAC_CLASS_PCI:
#endif
	    dprintf(DEBUG_VERBOSE, "Grcswim.c:StartDMAChannel:setting physaddr=0x%lx, cnt=%ld ", (unsigned long)DMAAddress, DMACount);
	    if (direction == DMA_READ) {
#ifdef OLD_DBDMA
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel[unit],
					true,
					(PhysicalAddress) DMAAddress,
					(uint_t) DMACount);
#else
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel[unit],
					true,
					(LogicalAddress) DMAAddress,
					(LogicalAddress) ccCommandsLogicalAddr[unit],
					(uint_t) DMACount);
#endif
		SwimIIISetReadMode();

		PrintRegs(0);
		PrintRegs(1);
	    } else {
#ifdef OLD_DBDMA
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel[unit],
					false,
					(PhysicalAddress) DMAAddress,
					(uint_t) DMACount);
#else
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel[unit],
					false,
					(LogicalAddress) DMAAddress,
					(LogicalAddress) ccCommandsLogicalAddr[unit],
					(uint_t) DMACount);
#endif
		PrintRegs(0);
		PrintRegs(1);
		if(DMACount == 0x8000) SwimIIISetFormatMode();
		SwimIIISetWriteMode();

		PrintRegs(0);
		PrintRegs(1);

	    }
#if 0
	}
#endif

    /*
     * Make sure that we don't get any false triggers by
     * preclearing the events we're waiting for.
     */

    CancelOSEvent(driveOSEventIDptr,
		  IT_SECTOR_TRANSFER_DONE);

#if MACH_DEBUG
	// pdm_dma_print();
#endif

#if 0
    info1 = powermac_info.class;
    switch(info1) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    /* !?!?!?! */
	    floppy_amic_start((direction == DMA_READ));
	    // floppy_amic_start((direction != DMA_READ));
	    break;
	case POWERMAC_CLASS_PCI:
#endif
#ifdef OLD_DBDMA
	    StartDBDMA(GRCFloppyDMAChannel[unit]);
#else
        /* @@@ GETS A WARNING @@@ */
	/* dbdma_command_t *commands */
	eieio();
	    dbdma_start(GRCFloppyDMAChannel[unit], ccCommandsLogicalAddr[unit]);
#endif
#if 0
	}
#endif

    /*
     * Wait for the DMA activity to reach terminal count, or five
     * diskette rotations (1 second) to make sure that the entire
     * sector, plus any CRC bytes, have been read in and checked.
     */
PrintDMA();
PrintRegs(0);
PrintRegs(1);
    if(DMACount == 0x8000) {
	errorCode = WaitForEvent(4000, MS_DO_ACTION, IT_SECTOR_TRANSFER_DONE);	/* changed 1000 to 4000 */
    } else {
	errorCode = WaitForEvent(1000, MS_DO_ACTION, IT_SECTOR_TRANSFER_DONE);
    }

#if MACH_DEBUG
	// pdm_dma_print();
#endif
#ifndef OLD_DBDMA
    if (errorCode != noErr) {
	dprintf(DEBUG_VERBOSE, "DMA Status: ");
	dprintf(DEBUG_VERBOSE, "0x%lx\n", GRCFloppyDMAChannel[unit]->d_status);
    }
#endif

    /*
     * Pause the DMA and disable interrupts from the DMA register set so that
     * we're sure that no more DMA activity occurs. Then clear the pending interrupt.
     */
PrintDMA();
// PrintRegs();
    StopDMAChannel(unit);
    
// #endif

    return RecordError(errorCode);

}


/************************************************************************
*
*	FUNCTION : HALReset
*
*	Reset the SWIMBASE and set it up for doing MFM.
*
************************************************************************/

OSStatus
HALReset(DriveStatusType * DriveStatus,
	 LogicalAddress floppyControllerBaseAddr,
	 LogicalAddress DMAReadControllerBaseAddr,
	 LogicalAddress DMAWriteControllerBaseAddr)
{

    int unit;
    GRCSwimIIIRegs *GRCSwimIIIRegsPtr;
#ifdef PDMSUPPORT
    PDMSwimIIIRegs *PDMSwimIIIRegsPtr;
#endif

    /*
     * Initialize any local variables.
     */

    lastErrorsPending = 0;

    /*
     * Remember which OSEventID our events will come in on.
     */

    driveOSEventIDptr = &DriveStatus->OSEventID;

    unit = DriveStatus->unit;

    /*
     * Get the base address of the SWIMIII controller chip and fill in
     * pointers to all of the SwimIII registers to abstract out register
     * memory map gaps.
     */
#ifdef PDMSUPPORT
    switch (powermac_info.class) {
    case POWERMAC_CLASS_PDM:
    case POWERMAC_CLASS_POWERBOOK:
    case POWERMAC_CLASS_PERFORMA:
	PDMSwimIIIRegsPtr = (PDMSwimIIIRegs *) floppyControllerBaseAddr;

	FloppySWIMIIIRegsList[unit].rwData = &PDMSwimIIIRegsPtr->rwData;
	FloppySWIMIIIRegsList[unit].rwTimer = &PDMSwimIIIRegsPtr->rwTimer;
	FloppySWIMIIIRegsList[unit].rError = &PDMSwimIIIRegsPtr->rError;
	FloppySWIMIIIRegsList[unit].rwParams = &PDMSwimIIIRegsPtr->rwParams;
	FloppySWIMIIIRegsList[unit].rwPhase = &PDMSwimIIIRegsPtr->rwPhase;
	FloppySWIMIIIRegsList[unit].rwSetup = &PDMSwimIIIRegsPtr->rwSetup;
	FloppySWIMIIIRegsList[unit].rModewModeZeroes = &PDMSwimIIIRegsPtr->rModewModeZeroes;
	FloppySWIMIIIRegsList[unit].rHandshakewModeOnes = &PDMSwimIIIRegsPtr->rHandshakewModeOnes;
	FloppySWIMIIIRegsList[unit].rInterrupt = &PDMSwimIIIRegsPtr->rInterrupt;
	FloppySWIMIIIRegsList[unit].rwStepCount = &PDMSwimIIIRegsPtr->rwStepCount;
	FloppySWIMIIIRegsList[unit].rCurrentTrack = &PDMSwimIIIRegsPtr->rCurrentTrack;
	FloppySWIMIIIRegsList[unit].rCurrentSector = &PDMSwimIIIRegsPtr->rCurrentSector;
	FloppySWIMIIIRegsList[unit].rwGapSize = &PDMSwimIIIRegsPtr->rwGapSize;
	FloppySWIMIIIRegsList[unit].rwFirstSector = &PDMSwimIIIRegsPtr->rwFirstSector;
	FloppySWIMIIIRegsList[unit].rwSectorCount = &PDMSwimIIIRegsPtr->rwSectorCount;
	FloppySWIMIIIRegsList[unit].rwInterruptMask = &PDMSwimIIIRegsPtr->rwInterruptMask;
	break;
    case POWERMAC_CLASS_PCI:
    default:
#endif

	GRCSwimIIIRegsPtr = (GRCSwimIIIRegs *) floppyControllerBaseAddr;

	FloppySWIMIIIRegsList[unit].rwData = &GRCSwimIIIRegsPtr->rwData;
	FloppySWIMIIIRegsList[unit].rwTimer = &GRCSwimIIIRegsPtr->rwTimer;
	FloppySWIMIIIRegsList[unit].rError = &GRCSwimIIIRegsPtr->rError;
	FloppySWIMIIIRegsList[unit].rwParams = &GRCSwimIIIRegsPtr->rwParams;
	FloppySWIMIIIRegsList[unit].rwPhase = &GRCSwimIIIRegsPtr->rwPhase;
	FloppySWIMIIIRegsList[unit].rwSetup = &GRCSwimIIIRegsPtr->rwSetup;
	FloppySWIMIIIRegsList[unit].rModewModeZeroes = &GRCSwimIIIRegsPtr->rModewModeZeroes;
	FloppySWIMIIIRegsList[unit].rHandshakewModeOnes = &GRCSwimIIIRegsPtr->rHandshakewModeOnes;
	FloppySWIMIIIRegsList[unit].rInterrupt = &GRCSwimIIIRegsPtr->rInterrupt;
	FloppySWIMIIIRegsList[unit].rwStepCount = &GRCSwimIIIRegsPtr->rwStepCount;
	FloppySWIMIIIRegsList[unit].rCurrentTrack = &GRCSwimIIIRegsPtr->rCurrentTrack;
	FloppySWIMIIIRegsList[unit].rCurrentSector = &GRCSwimIIIRegsPtr->rCurrentSector;
	FloppySWIMIIIRegsList[unit].rwGapSize = &GRCSwimIIIRegsPtr->rwGapSize;
	FloppySWIMIIIRegsList[unit].rwFirstSector = &GRCSwimIIIRegsPtr->rwFirstSector;
	FloppySWIMIIIRegsList[unit].rwSectorCount = &GRCSwimIIIRegsPtr->rwSectorCount;
	FloppySWIMIIIRegsList[unit].rwInterruptMask = &GRCSwimIIIRegsPtr->rwInterruptMask;
#ifdef PDMSUPPORT
    }

#endif

    SWIMIIICopyRegs(&FloppySWIMIIIRegs, &FloppySWIMIIIRegsList[unit]);
    /*
     * Set the eight byte DMA alignment restrictions for PDM machines.
     */

    DriveStatus->DMAByteBoundryAlignment = 8;
    // DriveStatus->DMAByteBoundryAlignment = 1;

    /*
     * Get the base address of AMIC DMA channel registers for the floppy.
     */

// #ifdef NOTYETDAG
#ifdef OLD_DBDMA
    GRCFloppyDMARegs[unit] = (DBDMAChannelRegisters *) DMAReadControllerBaseAddr;
#else
    GRCFloppyDMAChannel[unit] = (dbdma_regmap_t *) DMAReadControllerBaseAddr;
#endif
// #endif

    /*
     * Create a DBDMA Channel Command memory space that is one
     * DBDMA Channel Command long in size. Keep the logical and
     * physical addresses of this space around for later use.
     *
     * Also, tell the upper layers that we don't have a fixed
     * hardware track cache buffer, and to allocate one out
     * of system memory.
     */
// #ifdef NOTYETDAG
#ifdef PDMSUPPORT
    switch(powermac_info.class) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    // Don't do anything here, I don't think....
	    break;
	case POWERMAC_CLASS_PCI:
#endif
#ifdef OLD_DBDMA
	    OpenDBDMAChannel(GRCFloppyDMARegs[unit],
			     &GRCFloppyDMAChannel[unit],
			     1,
			     &ccCommandsLogicalAddr[unit],
			     &ccCommandsPhysicalAddr[unit]);
#else
	    if (!ccCommandsLogicalAddr[unit]) {
		ccCommandsLogicalAddr[unit] = (char *)dbdma_alloc(1);
		org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "dbdma_alloc\n");
// org_mklinux_iokit_swim3_cmdbase[DriveStatus->unit];
		ccCommandsPhysicalAddr[unit] = (char *)
		    (kvtophys((vm_offset_t)(ccCommandsLogicalAddr[unit])));
	    }
#endif
#ifdef PDMSUPPORT
	    break;
	}
#endif

// #endif

    /*
     * If a track cache buffer isn't already defined, return the default
     * 7.5 addresses.
     */

    if (DriveStatus->tcBuffer.logicalAddress == NULL) {

	DriveStatus->tcBuffer.physicalAddress = (Ptr) 0;
	DriveStatus->tcBuffer.logicalAddress = (Ptr) 0;
	DriveStatus->tcBufferLen = 0;
	printf("SWIM3: WE'RE HOSED!\n");
	return notEnoughMemoryErr;

    }
    DriveStatus->tcIsFixedMemory = false;

    /*
     * Now setup the ISM registers.
     *
     *    1. Disable MotorOn, put it in read mode, turn off head select,
     *       clear the action bit, disable drive 1 & 2.
     *
     *    2. Motor timer off, use Trans-Space machine, read/write data
     *       as pulses, disable Error Correction machine, don't divide
     *               the clock by 2, MFM mode, reset 3.5SEL pin, Q3/HDSEL pin
     *               is an input.
     */

    *FloppySWIMIIIRegs.rModewModeZeroes = ~*FloppySWIMIIIRegs.rModewModeZeroes;
    SynchronizeIO(); delay(10);

    *FloppySWIMIIIRegs.rwSetup = RS_IBM_NOT_APPLE_DRIVE;
    SynchronizeIO(); delay(10);
    return noErr;
}

#endif // NFD > 0
