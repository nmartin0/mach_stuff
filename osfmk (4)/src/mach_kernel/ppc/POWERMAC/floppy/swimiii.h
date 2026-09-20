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
*	File:		SwimIII.h
*
*	Contains:	Hardware interface structures for PDM class machines.
*
************************************************************************/

#ifndef __SWIMIII_H__
#define __SWIMIII_H__


/************************************************************************
*
* Constants for reading the rError register.
*
************************************************************************/

#define ER_UNDERRUN					0x01
#define ER_UNUSED1					0x02
#define ER_OVERRUN					0x04
#define ER_UNUSED2					0x08
#define ER_UNUSED3					0x10
#define ER_UNUSED4					0x20
#define ER_CRC_ON_ADDRESS_FIELD		0x40
#define ER_CRC_ON_DATA_FIELD		0x80


/************************************************************************
*
* Constants for reading and writing the rwSetup register.
*
************************************************************************/

#define RS_INVERT_NOT_INVERT_WRITE_DATA	0x01
#define RS_COPY_PROTECT_MODE			0x02
#define RS_GCR_NOT_MFM					0x04
#define RS_FCLK_DIV_2_NOT_FCLK			0x08
#define RS_TEST_MODE					0x10
#define RS_IBM_NOT_APPLE_DRIVE			0x20
#define RS_GCR_WRITE_NOT_MFM_WRITE		0x40
#define RS_SOFTWARE_RESET				0x80


/************************************************************************
*
* Constants for reading and writing the rModewModeZeroes register and for
* writing the rHandshakewModeOnes register.
*
************************************************************************/

#define MS_ENABLE_INTS		0x01
#define MS_ENABLE_DRIVE_1	0x02
#define MS_ENABLE_DRIVE_2	0x04
#define MS_DO_ACTION		0x08
#define MS_NOT_READ_WRITE	0x10
#define MS_SIDE1_NOT_SIDE0	0x20
#define MS_FORMAT_MODE		0x40
#define MS_DO_STEPPING		0x80


/************************************************************************
*
* Constants for reading the rHandshakewModeOnes register.
*
************************************************************************/

#define HR_MARK_BYTE		0x01
#define HR_INT_PENDING		0x02
#define HR_RDDATA_IN		0x04
#define HR_SENSE_IN			0x08
#define HR_UNUSED1			0x10
#define HR_ERROR_OCCURED	0x20
#define HR_FIFO_HAS_TWO		0x40
#define HR_FIFO_HAS_ONE		0x80


/************************************************************************
*
* Constants for reading the rInterrupt register and for reading and writing
* rwInterruptMask register.
*
************************************************************************/

#define IT_TIMER_DONE			0x01
#define IT_STEPPING_DONE		0x02
#define IT_ID_HEADER_READ		0x04
#define IT_SECTOR_TRANSFER_DONE	0x08
#define IT_SENSE_INPUT_CHANGE	0x10
#define IT_ERROR_OCCURRED		0x20
#define IT_UNUSED2				0x40
#define IT_TIMEOUT				0x80	// This is not really a hardware bit, its just easy to group it here.


/************************************************************************
*
* Data escape patterns for controlling and writing special data to
* a diskette.
*
************************************************************************/

#define SWIM3_DATA_ESCAPE_VALUE			0x99
#define GCR_SYNC_BYTE_EXCEPTION			0x3F
#define GCR_SYNC_EXCEPTION_CONVERSION	0x80
#define GCR_FIRST_MARK_EXCEPTION		0xD5
#define GCR_SECOND_MARK_EXCEPTION		0xAA

#define SWIM3_GCR_ADDRESS_MARK			"\xD5\xAA\x00"
#define SWIM3_GCR_DATA_MARK				"\xD5\xAA\x0B"
#define SWIM3_GCR_SLIP_BYTE				"\x27\xAA"
#define SWIM3_GCR_SELF_SYNC				"\x3F\xBF\x1E\x34\x3C\x3F"

#define SWIM3_LITERAL_99				"\x99\x99"
#define SWIM3_MFM_ADDRESS_MARK			"\x99\xA1\x99\xA1\x99\xA1\x99\xFE"
#define SWIM3_MFM_DATA_MARK				"\x99\xA1\x99\xA1\x99\xA1\x99\xFB"
#define SWIM3_MFM_INDEX_MARK			"\x99\xC2\x99\xC2\x99\xC2\x99\xFC"
#define SWIM3_WRITE_MFM_CRC				"\x99\x04"
#define SWIM3_DISABLE_DATA_ESCAPING		"\x99\x0F"
#define SWIM3_TERMINATE_DMA_TRANSFER	"\x99\x08"

#define SWIM3_MFM_MARK_SYNC_GAP_LEN		12

// Clear data out of the SwimIII FIFO including SWIM3_WRITE_MFM_CRC command
// for MFM mode, and SWIM3_GCR_SLIP_BYTE for GCR mode.

#define SWIM3_FIFO_CLEARANCE_LEN		6


/* Drive command signal line masks. */

#define	M_CA0		1	/* Really the SwimIII PH0 signal. */
#define	M_CA1		2	/* Really the SwimIII PH1 signal. */
#define	M_CA2		4	/* Really the SwimIII PH1 signal. */
#define	M_SEL		8	/* Really the SwimIII HDSEL signal. */

#define	M_LSTRB		8	/* Really the Swim PH3 signal. */

#define	M_PHxOUTPUT	0xF0	/* Mask for the Phase register to make any PHx setting an output. */


/* Drive commands. Properly flipped for POSITIVE logic. */

						/* M_SEL   M_CA2   M_CA1   M_CA0 */
#define	C_STEPFROMZERO	(   0x00 |  0x00 |  0x00 |  0x00 ) | M_PHxOUTPUT
#define	C_STEPTOZERO	(   0x00 | M_CA2 |  0x00 |  0x00 ) | M_PHxOUTPUT
#define	C_STEP			(   0x00 |  0x00 |  0x00 | M_CA0 ) | M_PHxOUTPUT
#define	C_MOTORON		(   0x00 |  0x00 | M_CA1 |  0x00 ) | M_PHxOUTPUT
#define	C_MOTOROFF		(   0x00 | M_CA2 | M_CA1 |  0x00 ) | M_PHxOUTPUT
#define	C_EJECT			(   0x00 | M_CA2 | M_CA1 | M_CA0 ) | M_PHxOUTPUT
#define	C_SETMFM		(  M_SEL |  0x00 |  0x00 | M_CA0 ) | M_PHxOUTPUT
#define	C_SETGCR		(  M_SEL | M_CA2 |  0x00 | M_CA0 ) | M_PHxOUTPUT
#define	C_INDEX			(   0x00 |  0x00 | M_CA1 | M_CA0 ) | M_PHxOUTPUT
#define	C_2MBMEDIACHECK	(   0x00 | M_CA2 | M_CA1 |  0x00 ) | M_PHxOUTPUT


/* Drive status commands. Properly flipped for POSITIVE logic. */

					/* M_SEL   M_CA2   M_CA1   M_CA0 */
#define S_OUTWARD	(   0x00 |  0x00 |  0x00 |  0x00 ) | M_PHxOUTPUT	/* seek direction toward track 0 */
#define	S_STEPOK	(   0x00 |  0x00 |  0x00 | M_CA0 ) | M_PHxOUTPUT	/* ok to issue Step command */
#define	S_MOTOROFF	(   0x00 |  0x00 | M_CA1 |  0x00 ) | M_PHxOUTPUT	/* motor is off */
#define	S_EJECTING	(   0x00 |  0x00 | M_CA1 | M_CA0 ) | M_PHxOUTPUT	/* disk eject in progress */
#define S_RDDATA0	(   0x00 | M_CA2 |  0x00 |  0x00 ) | M_PHxOUTPUT	/* read data from head 0 */
#define	S_2MBDRIVE	(   0x00 | M_CA2 |  0x00 | M_CA0 ) | M_PHxOUTPUT	/* drive is an FDHD or Typhoon */
#define	S_2MBDRVDSK	(   0x00 | M_CA2 | M_CA1 |  0x00 ) | M_PHxOUTPUT	/* drive or diskette is not a 4MB type */
#define S_NODRIVE	(   0x00 | M_CA2 | M_CA1 | M_CA0 ) | M_PHxOUTPUT	/* no drive present */
#define S_NODISK	(  M_SEL |  0x00 |  0x00 |  0x00 ) | M_PHxOUTPUT	/* no disk inserted */
#define S_WRTENAB	(  M_SEL |  0x00 |  0x00 | M_CA0 ) | M_PHxOUTPUT	/* write enabled disk inserted */
#define	S_NOTTRK0	(  M_SEL |  0x00 | M_CA1 |  0x00 ) | M_PHxOUTPUT	/* not on cyl zero (recal sensor) */
#define S_TACH		(  M_SEL |  0x00 | M_CA1 | M_CA0 ) | M_PHxOUTPUT	/* tach pulse in GCR mode */
#define S_INDEX		(  M_SEL |  0x00 | M_CA1 | M_CA0 ) | M_PHxOUTPUT	/* index pulse in MFM mode */
#define S_RDDATA1	(  M_SEL | M_CA2 |  0x00 |  0x00 ) | M_PHxOUTPUT	/* read data from head 1 */
#define S_MFM		(  M_SEL | M_CA2 |  0x00 | M_CA0 ) | M_PHxOUTPUT	/* drive is in MFM mode */
#define	S_NOTREADY	(  M_SEL | M_CA2 | M_CA1 |  0x00 ) | M_PHxOUTPUT	/* drive is NOT ready */
#define	S_1MBMEDIA	(  M_SEL | M_CA2 | M_CA1 | M_CA0 ) | M_PHxOUTPUT	/* 1MB media inserted */

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif

/************************************************************************
*
* Structure for accessing the PDM SWIMIII.
*
************************************************************************/

typedef struct PDMSwimIIIRegs {

    volatile unsigned char rwData;	/* 00 read/write a data byte */
    unsigned char pad00[0x1FF];

    volatile unsigned char rwTimer;	/* 02 read/write the timer register */
    unsigned char pad01[0x1FF];

    volatile unsigned char rError;	/* 04 read the error reason bits */
    unsigned char pad02[0x1FF];

    volatile unsigned char rwParams;	/* 06 read/write the param regs */
    unsigned char pad03[0x1FF];

    volatile unsigned char rwPhase;	/* 08 read/write the phase states & dirs */
    unsigned char pad04[0x1FF];

    volatile unsigned char rwSetup;	/* 0a read/write the setup register */
    unsigned char pad05[0x1FF];

    volatile unsigned char rModewModeZeroes;	/* 0c read mode register: write 1's clr mode bits */
    unsigned char pad06[0x1FF];

    volatile unsigned char rHandshakewModeOnes;		/* 0e read handshake reg: write 1's set mode bits */
    unsigned char pad07[0x1FF];

    volatile unsigned char rInterrupt;	/* 10 read the interrupt reason bits */
    unsigned char pad08[0x1FF];

    volatile unsigned char rwStepCount;		/* 12 read/write the head step count */
    unsigned char pad09[0x1FF];

    volatile unsigned char rCurrentTrack;	/* 14 read the current track position */
    unsigned char pad10[0x1FF];

    volatile unsigned char rCurrentSector;	/* 16 read the current sector position */
    unsigned char pad11[0x1FF];

    volatile unsigned char rwGapSize;	/* 18 read/write the gap3 size */
    unsigned char pad12[0x1FF];

    volatile unsigned char rwFirstSector;	/* 1a read/write the first sector to read/write */
    unsigned char pad13[0x1FF];

    volatile unsigned char rwSectorCount;	/* 1c read/write the number of sectors to read/write */
    unsigned char pad14[0x1FF];

    volatile unsigned char rwInterruptMask;	/* 1e read/write the interrupt enable mask bits */
    unsigned char pad15[0x1FF];

} PDMSwimIIIRegs;


/************************************************************************
*
* SWIMIII memory map for Grand Central machines.
*
************************************************************************/

typedef struct GRCSwimIIIRegs {

    volatile unsigned char rwData;	/* 00 read/write a data byte */
    unsigned char pad00[0xF];

    volatile unsigned char rwTimer;	/* 02 read/write the timer register */
    unsigned char pad01[0xF];

    volatile unsigned char rError;	/* 04 read the error reason bits */
    unsigned char pad02[0xF];

    volatile unsigned char rwParams;	/* 06 read/write the param regs */
    unsigned char pad03[0xF];

    volatile unsigned char rwPhase;	/* 08 read/write the phase states & dirs */
    unsigned char pad04[0xF];

    volatile unsigned char rwSetup;	/* 0a read/write the setup register */
    unsigned char pad05[0xF];

    volatile unsigned char rModewModeZeroes;	/* 0c read mode register: write 1's clr mode bits */
    unsigned char pad06[0xF];

    volatile unsigned char rHandshakewModeOnes;		/* 0e read handshake reg: write 1's set mode bits */
    unsigned char pad07[0xF];

    volatile unsigned char rInterrupt;	/* 10 read the interrupt reason bits */
    unsigned char pad08[0xF];

    volatile unsigned char rwStepCount;		/* 12 read/write the head step count */
    unsigned char pad09[0xF];

    volatile unsigned char rCurrentTrack;	/* 14 read the current track position */
    unsigned char pad10[0xF];

    volatile unsigned char rCurrentSector;	/* 16 read the current sector position */
    unsigned char pad11[0xF];

    volatile unsigned char rwGapSize;	/* 18 read/write the gap3 size */
    unsigned char pad12[0xF];

    volatile unsigned char rwFirstSector;	/* 1a read/write the first sector to read/write */
    unsigned char pad13[0xF];

    volatile unsigned char rwSectorCount;	/* 1c read/write the number of sectors to read/write */
    unsigned char pad14[0xF];

    volatile unsigned char rwInterruptMask;	/* 1e read/write the interrupt enable mask bits */
    unsigned char pad15[0xF];

} GRCSwimIIIRegs;


/************************************************************************
*
* Abstract memory map structure for SWIMIII.
*
************************************************************************/

typedef struct SwimIIIRegs {

    volatile unsigned char *rwData;	/* 00 read/write a data byte */

    volatile unsigned char *rwTimer;	/* 02 read/write the timer register */

    volatile unsigned char *rError;	/* 04 read the error reason bits */

    volatile unsigned char *rwParams;	/* 06 read/write the param regs */

    volatile unsigned char *rwPhase;	/* 08 read/write the phase states & dirs */

    volatile unsigned char *rwSetup;	/* 0a read/write the setup register */

    volatile unsigned char *rModewModeZeroes;	/* 0c read mode register: write 1's clr mode bits */

    volatile unsigned char *rHandshakewModeOnes;	/* 0e read handshake reg: write 1's set mode bits */

    volatile unsigned char *rInterrupt;		/* 10 read the interrupt reason bits */

    volatile unsigned char *rwStepCount;	/* 12 read/write the head step count */

    volatile unsigned char *rCurrentTrack;	/* 14 read the current track position */

    volatile unsigned char *rCurrentSector;	/* 16 read the current sector position */

    volatile unsigned char *rwGapSize;	/* 18 read/write the gap3 size */

    volatile unsigned char *rwFirstSector;	/* 1a read/write the first sector to read/write */

    volatile unsigned char *rwSectorCount;	/* 1c read/write the number of sectors to read/write */

    volatile unsigned char *rwInterruptMask;	/* 1e read/write the interrupt enable mask bits */

} SwimIIIRegs;


/************************************************************************
*
* Structure for a SWIMIII GCR sector.
*
************************************************************************/

typedef struct rawSwimIIIGCRSectorFormat {

    unsigned char addressMarkSync[sizeof(SWIM3_GCR_SELF_SYNC) - 1];	// 6 bytes

    unsigned char addressMark[sizeof(SWIM3_GCR_ADDRESS_MARK) - 1];	// 3 bytes

    unsigned char track;	// 1 byte

    unsigned char sector;	// 1 byte

    unsigned char side;		// 1 byte

    unsigned char format;	// 1 byte

    unsigned char checkSum;	// 1 byte

    unsigned char addressMarkTerminator[sizeof(SWIM3_GCR_SLIP_BYTE) - 1];	// 2 bytes

    unsigned char writeSpliceGap[sizeof(SWIM3_GCR_SELF_SYNC) - 1];	// 6 bytes

    unsigned char dataMarkSync[sizeof(SWIM3_GCR_SELF_SYNC) - 1];	// 6 bytes

    unsigned char dataMark[sizeof(SWIM3_GCR_DATA_MARK) - 1];	// 3 bytes

    unsigned char sectorNumber;	// 1 byte

    unsigned char tagData[GCRNibblizedTagDataSize];	// 16 bytes

    unsigned char dataField[GCRNibblizedSectorsSize];	// 683 bytes

    unsigned char dataFieldCheckSum[GCRNibblizedSectorCRCSize];		// 4 bytes

    unsigned char dataFieldTerminator[sizeof(SWIM3_GCR_SLIP_BYTE) - 1];		// 2 bytes

    unsigned char interSectorGap[(5 * (sizeof(SWIM3_GCR_SELF_SYNC) - 1)) - 1];	// 29 bytes (to make it even)

}
rawSwimIIIGCRSectorFormat;

#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

#endif				// if already included
