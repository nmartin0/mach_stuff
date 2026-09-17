/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/************************************************************

Created: Thursday, September 7, 1989 at 3:36 PM
	Devices.h
	C Interface to the Macintosh Libraries


	Copyright Apple Computer, Inc.	 1985-1989
	All rights reserved

************************************************************/


#ifndef __DEVICES__
#define __DEVICES__

#define newSelMsg 12
#define fillListMsg 13
#define getSelMsg 14
#define selectMsg 15
#define deselectMsg 16
#define terminateMsg 17
#define buttonMsg 19
#define chooserID 1
#define initDev 0		/*Time for cdev to initialize itself*/
#define hitDev 1		/*Hit on one of my items*/
#define closeDev 2		/*Close yourself*/
#define nulDev 3		/*Null event*/
#define updateDev 4 	/*Update event*/
#define activDev 5		/*Activate event*/
#define deactivDev 6	/*Deactivate event*/
#define keyEvtDev 7 	/*Key down/auto key*/
#define macDev 8		/*Decide whether or not to show up*/
#define undoDev 9
#define cutDev 10
#define copyDev 11
#define pasteDev 12
#define clearDev 13
#define cdevGenErr -1	/*General error; gray cdev w/o alert*/
#define cdevMemErr 0	/*Memory shortfall; alert user please*/
#define cdevResErr 1	/*Couldn't get a needed resource; alert*/
#define cdevUnset 3 	/* cdevValue is initialized to this*/

struct DCtlEntry {
    Ptr		dCtlDriver;
    short	dCtlFlags;
    QHdr	dCtlQHdr;
    long	dCtlPosition;
    Handle	dCtlStorage;
    short	dCtlRefNum;
    long	dCtlCurTicks;
    Ptr		dCtlWindow;
    short	dCtlDelay;
    short	dCtlEMask;
    short	dCtlMenu;
};

typedef struct DCtlEntry DCtlEntry;
typedef DCtlEntry *DCtlPtr, **DCtlHandle;

struct AuxDCE {
    Ptr		dCtlDriver;
    short	dCtlFlags;
    QHdr	dCtlQHdr;
    long	dCtlPosition;
    Handle	dCtlStorage;
    short	dCtlRefNum;
    long	dCtlCurTicks;
    Ptr		dCtlWindow;
    short	dCtlDelay;
    short	dCtlEMask;
    short	dCtlMenu;
    char	dCtlSlot;
    char	dCtlSlotId;
    long	dCtlDevBase;
    Ptr		dCtlOwner;
    char	dCtlExtDev;
    char	fillByte;
};

typedef struct AuxDCE AuxDCE;
typedef AuxDCE *AuxDCEPtr, **AuxDCEHandle;

#endif
