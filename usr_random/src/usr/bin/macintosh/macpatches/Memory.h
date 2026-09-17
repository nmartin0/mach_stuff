/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/************************************************************

Created: Thursday, September 7, 1989 at 4:55 PM
	Memory.h
	C Interface to the Macintosh Libraries


	Copyright Apple Computer, Inc.	1985-1989
	All rights reserved

************************************************************/


#ifndef __MEMORY__
#define __MEMORY__

#define maxSize 0x800000	/*Max data block size is 8 megabytes*/

typedef long	Size;

typedef struct Zone {
    Ptr		bkLim;
    Ptr		purgePtr;
    Ptr		hFstFree;
    long	zcbFree;
    ProcPtr	gzProc;
    short	moreMast;
    short	flags;
    short	cntRel;
    short	maxRel;
    short	cntNRel;
    short	maxNRel;
    short	cntEmpty;
    short	cntHandles;
    long	minCBFree;
    ProcPtr	purgeProc;
    Ptr		sparePtr;
    Ptr		allocPtr;
    short	heapData;
} Zone, *THz;

#endif
