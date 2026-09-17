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

#ifndef __FLOPPYPRIV_H__
#define __FLOPPYPRIV_H__
//#include <Devices.h>
//#include <Retrace.h>
//#include <Timer.h>
//#include <Power.h>
#include <machine/spl.h>
typedef char *Ptr;
/************************************************************************
*
* Some macros for use with bit arrays.
*
************************************************************************/

typedef unsigned char BitArrayByte;

// Number of bytes in sector bit arrays rounded up to the nearest whole uint_t size.

#define BitArrayByteSize( size )	( ( (size) / 8 ) + sizeof( uint_t ) )

#define TestBit( array, bitNum )	( ( *( (unsigned char *)(array) + ( bitNum >> 3 ) ) & (unsigned char)( 1 << ( bitNum & 7 ) ) ) != 0 )

#define SetBit( array, bitNum )		( *( (unsigned char *)(array) + ( bitNum >> 3 ) ) |= (unsigned char)( 1 << ( bitNum & 7 ) ) )

#define ClearBit( array, bitNum )	( *( (unsigned char *)(array) + ( bitNum >> 3 ) ) &= ~(unsigned char)( 1 << ( bitNum & 7 ) ) )

void
 ResetBitArray(BitArrayByte * bitArray,
	       uint_t bitArraySize);

boolean_t
TestBitArray(BitArrayByte * bitArray,
	     uint_t bitArraySize);

/************************************************************************
*
*	Global constants.
*
************************************************************************/

#define	SonyRfN		-5

#define	HD20RfN		-2

#define	SonyVersion	3

#define maxDrvNum 	2

#define GCRAddrMarkDataSize				5	// size of a denibblized GCR contents address mark.
#define GCRSectorbyteSize				1	// size of a denibblized GCR sector byte.
#define GCRTagDataSize					12	// size of a denibblized GCR tag bytes.
#define GCRSectorsSize 					512	// size of a GCR sector without tag bytes.
#define GCRSectorCRCSize				3	// size of a denibblized GCR sector checksum.

												// size of a denibblized GCR sector checksum.
#define GCRDenibblizedSectorSize		( GCRSectorbyteSize + \
										  GCRTagDataSize +    \
										  GCRSectorsSize +    \
										  GCRSectorCRCSize )

#define GCRNibblizedAddrMarkDataSize	5	// size of a nibblized GCR contents address mark.
#define GCRNibblizedSectorbyteSize		1	// size of a nibblized GCR sector byte.
#define GCRNibblizedTagDataSize			16	// size of a nibblized GCR tag bytes.
#define GCRNibblizedSectorsSize			683	// size of a nibblized GCR sector without tag bytes.
#define GCRNibblizedSectorCRCSize		4	// size of a nibblized GCR sector checksum.

												// size of a nibblized GCR sector checksum.
#define GCRNibblizedSectorSize			( GCRNibblizedSectorbyteSize + \
										  GCRNibblizedTagDataSize +    \
										  GCRNibblizedSectorsSize +    \
										  GCRNibblizedSectorCRCSize )

#define MFMSectorsSize 	512	// size of a MFM sector.

#define maxMFMSectors 	36	// 36 MFM sectors/track side (@2880K)
#define maxGCRSectors 	12	// 12 GCR sectors/track side (max)
#define minGCRSectors 	 8	//  8 GCR sectors/track side (min)
#define maxGCRHeads 	 2	//  GCR disks only will have two R/W heads

/*
 * Define the size of the track cache plus a little for those
 * machines that need to have it aligned on 4 byte boundries.
 */

#define trackCacheSize		( maxMFMSectors * GCRSectorsSize * 2 ) + sizeof( long )

/*
 * Floppy data stream mark constants.
 */

#define MFM_ADDRESS_MARK	"\xA1\xA1\xA1\xFE"
#define MFM_DATA_MARK		"\xA1\xA1\xA1\xFB"
#define MFM_INDEX_MARK		"\xC2\xC2\xC2\xFC"

#define GCR_ADDRESS_MARK	"\xD5\xAA\x96"
#define GCR_DATA_MARK		"\xD5\xAA\xAD"
#define GCR_SLIP_BYTE		"\xDE\xAA"
#define GCR_SELF_SYNC		"\xFF\x3F\xCF\xF3\xFC\xFF"

/*
 * Nibblizing Table to convert 6 bits into 8-bit code word.
 */

#define GCR_NIBBLIZE_TBL	"\x96\x97\x9A\x9B\x9D\x9E\x9F\xA6\xA7\xAB\xAC\xAD\xAE\xAF\xB2\xB3\xB4\xB5\xB6\xB7\xB9\xBA\xBB\xBC\xBD\xBE\xBF\xCB\xCD\xCE\xCF\xD3\xD6\xD7\xD9\xDA\xDB\xDC\xDD\xDE\xDF\xE5\xE6\xE7\xE9\xEA\xEB\xEC\xED\xEE\xEF\xF2\xF3\xF4\xF5\xF6\xF7\xF9\xFA\xFB\xFC\xFD\xFE\xFF"

#define GCR_NIBBLIZE( byteValue )		GCR_NIBBLIZE_TBL[ (byteValue) ]

/*
 * Denibblizing Table (denibbles $96 thru $FF converted to $00 thru $3F)
 * $FF means illegal nibble. Subtract $96 first, then do the lookup with
 * the result.
 */

#define GCR_DENIBBLIZE_TBL	"\x00\x01\xFF\xFF\x02\x03\xFF\x04\x05\x06\xFF\xFF\xFF\xFF\xFF\xFF\x07\x08\xFF\xFF\xFF\x09\x0A\x0B\x0C\x0D\xFF\xFF\x0E\x0F\x10\x11\x12\x13\xFF\x14\x15\x16\x17\x18\x19\x1A\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x1B\xFF\x1C\x1D\x1E\xFF\xFF\xFF\x1F\xFF\xFF\x20\x21\xFF\x22\x23\x24\x25\x26\x27\x28\xFF\xFF\xFF\xFF\xFF\x29\x2A\x2B\xFF\x2C\x2D\x2E\x2F\x30\x31\x32\xFF\xFF\x33\x34\x35\x36\x37\x38\xFF\x39\x3A\x3B\x3C\x3D\x3E\x3F"

#define GCR_DENIBBLIZE( byteValue )		GCR_DENIBBLIZE_TBL[ (byteValue) - 0x96 ]


/************************************************************************
*
*	Structures for GCR sector interleave, next sector lookup tables.
*
*	These tables are organized as 12 by 5 byte tables. Subtracting
*	the base number of sectors per track from the number of sectors
*	per track in the current track zone, using that number as a index
*	to multipling times the maximum number of sectors per track yields
*	an offset from the begining of the table to the first entry for the
*	current track zone. Add in the current sector number and the base
*	address of the table, and use that address to lookup the next physical
*	sector number that will rotate bye the read/write head. In algorithmic
*	terms:
*
*	table_index = ( current_sectors_per_track - minGCRSectors ) * maxGCRSectors;
*
*	next_sector = TWO_TO_ONE_NEXT_SECTORS_TBL[ table_index + current_sector ];
*
************************************************************************/

#define TWO_TO_ONE_NEXT_SECTORS_TBL		"\x04\x05\x06\x07\x01\x02\x03\x00\xFF\xFF\xFF\xFF\x05\x06\x07\x08\x00\x01\x02\x03\x04\xFF\xFF\xFF\x05\x06\x07\x08\x09\x01\x02\x03\x04\x00\xFF\xFF\x06\x07\x08\x09\x0A\x00\x01\x02\x03\x04\x05\xFF\x06\x07\x08\x09\x0A\x0B\x01\x02\x03\x04\x05\x00"

#define FOUR_TO_ONE_NEXT_SECTORS_TBL	"\x02\x03\x04\x05\x06\x07\x01\x00\xFF\xFF\xFF\xFF\x07\x08\x00\x01\x02\x03\x04\x05\x06\xFF\xFF\xFF\x07\x08\x09\x01\x02\x03\x04\x05\x06\x00\xFF\xFF\x03\x04\x05\x06\x07\x08\x09\x0A\x00\x01\x02\xFF\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x01\x02\x00"


/************************************************************************
*
* So that the low memory global "TagData" can be refered to as the simple
* data structure.
*
************************************************************************/

typedef union TagDataType {
    unsigned char TagBytes[GCRTagDataSize];	// the raw tag byte values

    struct			// the individual tag data components
     {
	unsigned long BufTgFNum;	// file number [long]

	unsigned short BufTgFFlg;	// flags [word]

	unsigned short BufTgFBkNum;	// logical block number [word]

	unsigned long BufTgDate;	// time stamp [long]

    } TagItems;
} TagDataType;

typedef struct lowMemTagDataType {
    unsigned short sectorNum;
    TagDataType defaultTagData;
} lowMemTagDataType;


/************************************************************************
*
*	Structures to define icons and icon tables.
*
************************************************************************/

typedef struct iconTableType {

    char *logicalIcon;
    char *physicalIcon;
    short driveInfoBits;

} iconTableType;


/************************************************************************
*
* Structure for diskette sector address.
*
************************************************************************/

typedef struct diskAddressType {

    union {

	unsigned long blockAddress;	// Long version for fast comparisons.

	struct {
	    //naga changed unsigned char to char    
	    char track;		// Desired track.

	    unsigned char side;	// Desired side.

	    unsigned char sector;	// Desired sector.

	    unsigned char blockSize;	// Index of sector size.

	} items;

    } address;

} diskAddressType;


/************************************************************************
*
* Definitions and structures for diskette format tables.
*
************************************************************************/

#define GCR400K				0	// Format table index for 400K GCR format.
#define GCR800K				1	// Format table index for 800K GCR format.
#define MFM720K				2	// Format table index for 720K MFM format.
#define MFM1440K			3	// Format table index for 1440K MFM format.
#define MFM1680K			4	// Format table index for 1680K DMF MFM format.
#define MFM2880K			5	// Format table index for 2880K MFM format.

#define maxFormatNum		6	// Maximum number formats in the table.

#define FmtTSSValid			0x80	// Track, Sides, Sectors are valid bit flag.
#define FmtCurrentFormat	0x40	// Current diskette format bit flag.
#define FmtReserved			0x20
#define FmtDblDensity		0x10	// Double density bit flag.

#define FmtSidesMask		0x0F	// Mask for isolating the number of sides value.


typedef struct formatTableEntry {

    long drvBlockMax;
    unsigned char flagsNSides;
    unsigned char sectorsPerTrack;
    short drvTrackMax;
    short logicalSectorSize;
    short DMASectorSize;
    unsigned char firstSectorInTrack;
    unsigned char sectorInterleave;
    unsigned char GCR;
    unsigned int gap1Length;
    unsigned int gap2Length;
    unsigned int gap3Length;
    unsigned int gap4ALength;
    unsigned int gap4BLength;

} formatTableEntry;

typedef struct formatTableEntry formatTableType[maxFormatNum];


/************************************************************************
*
* Structure for returning diskette format tables.
*
************************************************************************/


typedef struct formatReturnType {

    long drvBlockMax;
    unsigned char flagsNSides;
    unsigned char sectorsPerTrack;
    short drvTrackMax;

} formatReturnType;


/************************************************************************
*
*	State values for the "diskInPlace" field.
*
************************************************************************/

#define NO_DISKETTE			0
#define DISK_PRESENT		1
#define DISK_WAS_ACCESSED	2
#define DISK_WEIRD_FORMAT	3
#define DISK_AUTO_EJECTING	0xFF


/************************************************************************
*
*	State values for the "motorOnState" field.
*
************************************************************************/

#define MOTOR_IS_OFF	0
#define MOTOR_WAS_ON	1
#define MOTOR_WAS_OFF	2


/************************************************************************
*
*	State values for the "installed" field.
*
************************************************************************/

#define DRV_UNSURE_INSTALLED	0
#define DRV_IS_INSTALLED		1
#define DRV_NOT_INSTALLED		2


/************************************************************************
*
* Definitions for drive types in the "driveType" field.
*
************************************************************************/

#define DRIVE_1MEG_800k		0
#define DRIVE_2MEG_FDHD		0xFF	// SuperDrive
#define DRIVE_4MEG_TYPHOON	0xFE	// Typhoon


/************************************************************************
*
* Definitions for format types in the "formatType" field.
*
************************************************************************/

#define GCR_DISKETTE		0
#define MFM_DISKETTE		0xFF


/************************************************************************
*
* Definitions for diskette density types in the "disketteType" field.
*
************************************************************************/

#define DISKETTE_1MEG	0
#define DISKETTE_2MEG	0xFF
#define DISKETTE_4MEG	0xFE


/************************************************************************
*
* Definitions for controller types/modes in the "FDCType" field.
*
************************************************************************/

#define isIWM		0
#define isSWIM1		0xFF
#define isSWIM2		0xFE
#define isSWIM3		0xFD
#define isNewAge	0xFC
#define isNewAgeStd	0xFB	//        NewAge used as standard controller              GEW
#define isIntelStd	0xFA	//        Intel 82078 64 pin controller                   GEW


/************************************************************************
*
* Buffer addressing specific structures.
*
************************************************************************/

typedef struct TransferAddressType {
    Ptr logicalAddress;		// Logical address of a block data transfer

    Ptr physicalAddress;	// Physical address of a block data transfer

} TransferAddressType;

/************************************************************************
*
* Drive-specific status.
*
************************************************************************/

typedef struct DriveStatusType {

    //void *                                        OSHardwareLockID;                       // non-zero when driver is accessing the controller
#ifdef MKLINUX_SOURCE
    spl_t OSHardwareLockID;	// non-zero when driver is accessing the controller
#else
    mutex_t *OSHardwareLockID;
#endif

    void *OSDiskID;		// Typeless identifier for the OS to refer to a specific disk drive.

    UInt32 OSEventID;		// Typeless identifier for the OS to refer to a specific event resource.

    short DRVROperation;	// Current DRVR command being executed.

    short currentCommand;	// Current sub command, for the DRVR Operation, being executed.

    short commandModifier;	// Current sub command modifier bits.

    long firstBlockNumber;	// The first block number in the current transfer.

    long blockCount;		// The number of blocks in the current transfer.

    diskAddressType currentAddress;	// last access location the head was over

    diskAddressType wantedAddress;	// desired access location

    void *transferSGList;	// Address to use for block data transfers

    TransferAddressType transferAddress;	// Address to use for block data transfers

    Ptr tagDataAddress;		// Address to use for tag data transfers

    unsigned char formatByte;	// byte value to blank sector to when formating.

    /*
     * These fields and the drive queue element must stay together
     * in this order because getting to these four fields ahead of
     * the drive queue element is published in IM IV page 181.
     */

    boolean_t writeProt;	// bit7 = 1 = write-protected

    unsigned char diskInPlace;	// See the definitions above.

    unsigned char installed;	// 0 = don't know, 1 = inst., $FF = not inst.

    boolean_t doubleSided;	// 0 = single sided diskette, $FF = double sided diskette
    //naga  DrvQEl                                  driveQEl;                                       // the drive queue entry for this drive

    unsigned char motorOnState;	// Non-zero if the motor is on, zero if not.

    unsigned char FDCType;	// 0 = IWM, $FF = SWIM I, $FE = SWIM II, $FD = SWIM III

    unsigned char driveNumber;	// number of drive this struct is for

    unsigned char driveType;	// $FF=SuperDrive, $00=400K or 800K GCR

    unsigned char formatType;	// $FF=MFM, $00=GCR or unformatted

    unsigned char disketteType;	// $00=1 meg (720K), $FF=2 meg MFM (1440K), $FE=4 meg MFM (2880K)

    short driveErrs;		// drive soft errs

    formatTableEntry diskFormat;	// Current format of diskette from format table

    unsigned char sectorInterleaveLookup[maxMFMSectors + 1];

    iconTableType iconTable;	// Table of pointers to drive icons.

    // True bit flag if sector data in track cache buffer is denibblized, false if nibblized.

    BitArrayByte sectorDenibblized[maxGCRHeads][BitArrayByteSize(maxMFMSectors+1)];

    // True bit flag if sector data in track cache buffer is "write" dirty.

    BitArrayByte sectorDirty[maxGCRHeads][BitArrayByteSize(maxMFMSectors+1)];

    char cachedTrack[maxGCRHeads];	// (byte) track of side cached blocks 

    TransferAddressType tcBuffer;	// (long) pointer to track cache buffer (512+12)*12

    unsigned long tcBufferLen;	// (long) lenght of track cache buffer (512+12)*12

    boolean_t tcIsFixedMemory;	// (byte) non-zero when track cache buffer is fixed memory provided by HAL

    TransferAddressType intraCacheSectorAddress[maxGCRHeads][maxMFMSectors + 1];

    short preFirstSectorCachePadding;	// Bytes of space before the first sector in track cache.

    short preSectorCachePadding;	// Bytes of space before sector data in track cache.

    short postSectorCachePadding;	// Bytes of space after sector data in track cache.

    short postLastSectorCachePadding;	// Bytes of space after last sector in track cache.

    short DMAByteBoundryAlignment;	// Byte alignment restrictions for DMA to/from the track cache.

    int isFormatted;

    int unit;			// Necessary, if ugly.  --dg

} DriveStatusType;

/************************************************************************
*
* Definitions and structures for MFM status requests.
*
************************************************************************/

typedef struct MFMStatusType {

    unsigned char mfmDrive;	// $FF=SuperDrive, $00=400K or 800K GCR

    unsigned char mfmDisk;	// $FF=MFM, $00=GCR or unformatted

    unsigned char twoMegFmt;	// $FD=4 meg MFM (2880K), $FE=2 meg DMF MFM (1680K), $FF=2 meg MFM (1440K), $00=1 meg (720K)

    unsigned char isSWIM;	// FDC type: $FF=SWIM, $00=IWM

} MFMStatusType;


/************************************************************************
*
* DOS boot sector definition (used for DMF disk authentication)
*
************************************************************************/

#define DOSsecsPerCluster		4
#define DOShdMediaType			0xF0
#define DOSdmfTotalSectors		0x200D	// little-endian ($0D20=3360)
#define DOSdmfSecsPerTrack		0x1500	// little-endian ($0015=21)

typedef struct DOSBootSector {

    unsigned char bsJump[3];	// [00] jump instruction to bootstrap routine

    unsigned char bsOemName[8];	// [03] OEM name and version

    unsigned short bsBytesPerSec;	// [0B] bytes per sector

    unsigned char bsSecPerClust;	// [0D] sectors per cluster

    unsigned short bsResSectors;	// [0E] number of reserved sectors at the begining (1 for diskettes)

    unsigned char bsFATs;	// [10] number of file allocation tables

    unsigned short bsRootDirEnts;	// [11] number of root-directory entries

    unsigned short bsSectors;	// [13] total number of sectors

    unsigned char bsMedia;	// [15] media descriptor

    unsigned short bsFATSecs;	// [16] number of sectors per FAT

    unsigned short bsSecPerTrack;	// [18] sectors per track

    unsigned short bsHeads;	// [1A] number of heads

    unsigned short bsSpecResSectors;	// [1C] number of special reserved sectors

} DOSBootSector;


/************************************************************************
*
*	Driver local vars type.
*
************************************************************************/


typedef struct SonyVarsType {
    unsigned char debuggingEnabled;	// non-zero when driver is active

    DriveStatusType driveStatus[maxDrvNum];	// internal and external drive variables

    unsigned char *tagBufPtr;	// if non-zero, pointer to separate buffer for file tags . . .

    short tcDrive;		// (word) drive number of cache blks (zero to inval)

    short timeOut;		// power time out

    DriveStatusType *timeOutDrive;	// Points to the status table of the drive powering down 

    formatTableType formatTable;	// Diskette format lookup tables.

    unsigned char lastGCRFormatByte;	// saved GCR format byte for duplicator

} SonyVarsType;


/************************************************************************
*
* Driver command codes.
*
************************************************************************/

#define DRVROpenCC  	0	// 'Driver Open' control code
#define DRVRPrimeCC  	1	// 'Driver Prime' control code
#define DRVRControlCC  	2	// 'Driver Control' control code
#define DRVRStatusCC  	3	// 'Driver Status' control code
#define DRVRCloseCC  	4	// 'Driver Close' control code

#define killCC  		1	// 'kill I/O' control code
#define verifyCC  		5	// 'verify' control code
#define formatCC  		6	// 'format' control code
#define ejectCC  		7	// 'eject disk' control code
#define tagBufCC  		8	// 'set tag buffer' control code
#define tCacheCC  		9	// 'track cache' control
#define iconCC			21	// 'get icon' control code
#define iconLogCC 		22	// 'get logical icon' code
#define infoCC			23	// 'get drive info' code
#define FmtCopyCC		0x5343	// one-pass format/copy/verify for disk duplicator
#define GetRawDataCC	0x4744	// 'get raw track data' code
#define DiagnosticCall	0x99	// Added to help test HAL (GEW)
										// Operation is defined by a selector
										// passed in PBlock->cntrlParam.csCode.


// (2) Driver Status codes.

#define fmtLstCode		6	// Returns a list of disk formats
#define drvStsCode		8	// Get status info on a drive
#define mfmStsCode		10	// 'Get MFM status' status code
#define dupVerSts		0x4456	// disk duplicator version supported (to match features)
#define fmtByteSts		0x5343	// return address header format byte


// (2) Driver Prime codes.

#define prmReadCode		2	// Read blocks of data.
#define prmWriteCode	3	// Writes blocks of data.
#define prmStatCode 4

#define prmRdVerifyCode	0x40	// Bit mask for read-verify flag.
#define prmWrVerifyCode	0x20	// Bit mask for write-verify flag.

#endif
