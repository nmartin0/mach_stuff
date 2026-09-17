// #define OLD_DBDMA
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

#include <types.h>
#include <sys/ioctl.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppysal.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "swimiii.h"

#ifdef OLD_DBDMA
#include "dbdma_maxwell.h"
#else
#include <ppc/POWERMAC/dbdma.h>
#include "floppy_dbdma.h"
#endif

#include "floppy_amic.h"
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#include <ppc/proc_reg.h>

#include "swimiiicommonhal.h"

#if (!MACH_DEBUG)
#define printf donone

#else
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
	printf("\n\r%s\n\r",string);
	}
}
#endif

/************************************************************************
*
* Global data for driver.
*
************************************************************************/

extern
SwimIIIRegs FloppySWIMIIIRegs;	/* Floppy controller register sets */

#ifdef OLD_DBDMA
DBDMAChannelRegisters *GRCFloppyDMARegs;	/* Floppy DMA register sets */
DBDMAChannelConnectionPtr GRCFloppyDMAChannel;

#else // (!OLD_DBDMA)

dbdma_regmap_t *GRCFloppyDMAChannel;	/* Floppy DMA register sets */
typedef dbdma_regmap_t DBDMAChannelRegisters;

void *GRCFloppyDMARegs;

#define ResetDBDMA dbdma_reset
#define PrepDBDMA donone
#define StopDBDMA dbdma_stop
#define SetDBDMAPhysicalAddress floppy_dbdma_setup
// #define OpenDBDMAChannel donone

#endif // (OLD_DBDMA)

extern dma_softc_t floppy_amic_softc[];

LogicalAddress ccCommandsLogicalAddr = NULL;
PhysicalAddress ccCommandsPhysicalAddr = NULL;


/************************************************************************
*
* Private data for HAL.
*
************************************************************************/

extern unsigned char lastErrorsPending;

extern UInt32 *driveOSEventIDptr;


/************************************************************************
*
*	FUNCTION : ResetDMAChannel
*
************************************************************************/

OSStatus
ResetDMAChannel(void)
{
    int info;

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
	    ResetDBDMA(GRCFloppyDMAChannel);
#ifdef OLD_DBDMA
	    PrepDBDMA(GRCFloppyDMAChannel);
#else
	    floppy_dbdma_prep(GRCFloppyDMAChannel, ccCommandsLogicalAddr);
#endif
	    break;
	}

    return noErr;

}


/************************************************************************
*
*	FUNCTION : StopDMAChannel
*
************************************************************************/

OSStatus
StopDMAChannel(void)
{
    int info;

    info = powermac_info.class;
    switch(info) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    floppy_amic_end();
	    break;
	case POWERMAC_CLASS_PCI:
	    StopDBDMA(GRCFloppyDMAChannel);
	    break;
	}

    return noErr;

}


/************************************************************************
*
*	FUNCTION : StartDMAChannel
*
************************************************************************/

OSStatus
StartDMAChannel(void *DMAAddress,
		unsigned long DMACount,
		unsigned short direction)
{
    int info1, info2;
    OSStatus errorCode;


    /*
     * Assume success.
     */

    errorCode = noErr;

    /*
     * Reset the DMA channel and wait for the DMA channel to freeze
     * before we change either the DMA address or count.
     */

    info1 = powermac_info.class;
    switch(info1) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    floppy_amic_init();
	    // floppy_amic_end();
	    break;
	case POWERMAC_CLASS_PCI:
	    ResetDMAChannel();
	    break;
	}

    /*
     * Set the DMA address and count, then put the DMA register set
     * into the proper read/write mode and enable it. Then fire off
     * the SWIMIII.
     */

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
	    printf("Grcswim.c:StartDMAChannel:setting physaddr=0x%x, cnt=%d ", DMAAddress, DMACount);
	    if (direction == DMA_READ) {
#ifdef OLD_DBDMA
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel,
					true,
					(PhysicalAddress) DMAAddress,
					(uint_t) DMACount);
#else
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel,
					true,
					(LogicalAddress) DMAAddress,
					(LogicalAddress) ccCommandsLogicalAddr,
					(uint_t) DMACount);
#endif
		SwimIIISetReadMode();

	    } else {
#ifdef OLD_DBDMA
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel,
					false,
					(PhysicalAddress) DMAAddress,
					(uint_t) DMACount);
#else
		SetDBDMAPhysicalAddress(GRCFloppyDMAChannel,
					false,
					(LogicalAddress) DMAAddress,
					(LogicalAddress) ccCommandsLogicalAddr,
					(uint_t) DMACount);
#endif
		SwimIIISetWriteMode();

	    }
	}

    /*
     * Make sure that we don't get any false triggers by
     * preclearing the events we're waiting for.
     */

    CancelOSEvent(driveOSEventIDptr,
		  IT_SECTOR_TRANSFER_DONE);

#if MACH_DEBUG
	// pdm_dma_print();
#endif

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
#ifdef OLD_DBDMA
	    StartDBDMA(GRCFloppyDMAChannel);
#else
	    dbdma_start(GRCFloppyDMAChannel, ccCommandsLogicalAddr);
#endif
	}

    /*
     * Wait for the DMA activity to reach terminal count, or five
     * diskette rotations (1 second) to make sure that the entire
     * sector, plus any CRC bytes, have been read in and checked.
     */
// PrintDMA();
// PrintRegs();
    errorCode = WaitForEvent(4000, MS_DO_ACTION, IT_SECTOR_TRANSFER_DONE);	/* changed 1000 to 4000 */

#if MACH_DEBUG
	// pdm_dma_print();
#endif
    if (errorCode != noErr) {
	printf("DMA Status: ");
	printf("0x%x\n", GRCFloppyDMAChannel->d_status);
    }

    /*
     * Pause the DMA and disable interrupts from the DMA register set so that
     * we're sure that no more DMA activity occurs. Then clear the pending interrupt.
     */
// PrintDMA();
// PrintRegs();
    StopDMAChannel();

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


    GRCSwimIIIRegs *GRCSwimIIIRegsPtr;
    PDMSwimIIIRegs *PDMSwimIIIRegsPtr;

    /*
     * Initialize any local variables.
     */

    lastErrorsPending = 0;

    /*
     * Remember which OSEventID our events will come in on.
     */

    driveOSEventIDptr = &DriveStatus->OSEventID;

    /*
     * Get the base address of the SWIMIII controller chip and fill in
     * pointers to all of the SwimIII registers to abstract out register
     * memory map gaps.
     */
    switch (powermac_info.class) {
    case POWERMAC_CLASS_PDM:
    case POWERMAC_CLASS_POWERBOOK:
    case POWERMAC_CLASS_PERFORMA:
	PDMSwimIIIRegsPtr = (PDMSwimIIIRegs *) floppyControllerBaseAddr;

	FloppySWIMIIIRegs.rwData = &PDMSwimIIIRegsPtr->rwData;
	FloppySWIMIIIRegs.rwTimer = &PDMSwimIIIRegsPtr->rwTimer;
	FloppySWIMIIIRegs.rError = &PDMSwimIIIRegsPtr->rError;
	FloppySWIMIIIRegs.rwParams = &PDMSwimIIIRegsPtr->rwParams;
	FloppySWIMIIIRegs.rwPhase = &PDMSwimIIIRegsPtr->rwPhase;
	FloppySWIMIIIRegs.rwSetup = &PDMSwimIIIRegsPtr->rwSetup;
	FloppySWIMIIIRegs.rModewModeZeroes = &PDMSwimIIIRegsPtr->rModewModeZeroes;
	FloppySWIMIIIRegs.rHandshakewModeOnes = &PDMSwimIIIRegsPtr->rHandshakewModeOnes;
	FloppySWIMIIIRegs.rInterrupt = &PDMSwimIIIRegsPtr->rInterrupt;
	FloppySWIMIIIRegs.rwStepCount = &PDMSwimIIIRegsPtr->rwStepCount;
	FloppySWIMIIIRegs.rCurrentTrack = &PDMSwimIIIRegsPtr->rCurrentTrack;
	FloppySWIMIIIRegs.rCurrentSector = &PDMSwimIIIRegsPtr->rCurrentSector;
	FloppySWIMIIIRegs.rwGapSize = &PDMSwimIIIRegsPtr->rwGapSize;
	FloppySWIMIIIRegs.rwFirstSector = &PDMSwimIIIRegsPtr->rwFirstSector;
	FloppySWIMIIIRegs.rwSectorCount = &PDMSwimIIIRegsPtr->rwSectorCount;
	FloppySWIMIIIRegs.rwInterruptMask = &PDMSwimIIIRegsPtr->rwInterruptMask;
	break;
    case POWERMAC_CLASS_PCI:
    default:

	GRCSwimIIIRegsPtr = (GRCSwimIIIRegs *) floppyControllerBaseAddr;

	FloppySWIMIIIRegs.rwData = &GRCSwimIIIRegsPtr->rwData;
	FloppySWIMIIIRegs.rwTimer = &GRCSwimIIIRegsPtr->rwTimer;
	FloppySWIMIIIRegs.rError = &GRCSwimIIIRegsPtr->rError;
	FloppySWIMIIIRegs.rwParams = &GRCSwimIIIRegsPtr->rwParams;
	FloppySWIMIIIRegs.rwPhase = &GRCSwimIIIRegsPtr->rwPhase;
	FloppySWIMIIIRegs.rwSetup = &GRCSwimIIIRegsPtr->rwSetup;
	FloppySWIMIIIRegs.rModewModeZeroes = &GRCSwimIIIRegsPtr->rModewModeZeroes;
	FloppySWIMIIIRegs.rHandshakewModeOnes = &GRCSwimIIIRegsPtr->rHandshakewModeOnes;
	FloppySWIMIIIRegs.rInterrupt = &GRCSwimIIIRegsPtr->rInterrupt;
	FloppySWIMIIIRegs.rwStepCount = &GRCSwimIIIRegsPtr->rwStepCount;
	FloppySWIMIIIRegs.rCurrentTrack = &GRCSwimIIIRegsPtr->rCurrentTrack;
	FloppySWIMIIIRegs.rCurrentSector = &GRCSwimIIIRegsPtr->rCurrentSector;
	FloppySWIMIIIRegs.rwGapSize = &GRCSwimIIIRegsPtr->rwGapSize;
	FloppySWIMIIIRegs.rwFirstSector = &GRCSwimIIIRegsPtr->rwFirstSector;
	FloppySWIMIIIRegs.rwSectorCount = &GRCSwimIIIRegsPtr->rwSectorCount;
	FloppySWIMIIIRegs.rwInterruptMask = &GRCSwimIIIRegsPtr->rwInterruptMask;
    }

    /*
     * Set the eight byte DMA alignment restrictions for PDM machines.
     */

    // DriveStatus->DMAByteBoundryAlignment = 1;
    DriveStatus->DMAByteBoundryAlignment = 8;

    /*
     * Get the base address of AMIC DMA channel registers for the floppy.
     */

#ifdef OLD_DBDMA
    GRCFloppyDMARegs = (DBDMAChannelRegisters *) DMAReadControllerBaseAddr;
#else
    GRCFloppyDMAChannel = (dbdma_regmap_t *) DMAReadControllerBaseAddr;
#endif

    /*
     * Create a DBDMA Channel Command memory space that is one
     * DBDMA Channel Command long in size. Keep the logical and
     * physical addresses of this space around for later use.
     *
     * Also, tell the upper layers that we don't have a fixed
     * hardware track cache buffer, and to allocate one out
     * of system memory.
     */
    switch(powermac_info.class) {
	case POWERMAC_CLASS_PDM:
	case POWERMAC_CLASS_PERFORMA:
	case POWERMAC_CLASS_POWERBOOK:
	    // Don't do anything here, I don't think....
	    break;
	case POWERMAC_CLASS_PCI:
#ifdef OLD_DBDMA
	    OpenDBDMAChannel(GRCFloppyDMARegs,
			     &GRCFloppyDMAChannel,
			     1,
			     &ccCommandsLogicalAddr,
			     &ccCommandsPhysicalAddr);
#else
	    if (!ccCommandsLogicalAddr) {
		ccCommandsLogicalAddr = (char *)dbdma_alloc(1);
		ccCommandsPhysicalAddr = (char *)
		    (kvtophys((vm_offset_t)(ccCommandsLogicalAddr)));
	    }
#endif
	    break;
	}

    /*
     * If a track cache buffer isn't already defined, return the default
     * 7.5 addresses.
     */

    if (DriveStatus->tcBuffer.logicalAddress == NULL) {

	DriveStatus->tcBuffer.physicalAddress = (Ptr) 0;
	DriveStatus->tcBuffer.logicalAddress = (Ptr) 0;
	DriveStatus->tcBufferLen = 0;

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
    SynchronizeIO();

    *FloppySWIMIIIRegs.rwSetup = RS_IBM_NOT_APPLE_DRIVE;
    SynchronizeIO();
    return noErr;
}

#endif // NFD > 0
