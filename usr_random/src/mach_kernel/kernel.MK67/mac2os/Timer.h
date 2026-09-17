/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	Timer.h,v $
 * Revision 2.2  91/09/12  16:51:30  bohman
 * 	Created.
 * 	[91/09/11  16:34:39  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2os/Timer.h
 */

/************************************************************

Created: Thursday, September 7, 1989 at 8:09 PM
	Timer.h
	C Interface to the Macintosh Libraries


	Copyright Apple Computer, Inc.	1985-1989
	All rights reserved

************************************************************/

#ifndef __TIMER__
#define __TIMER__

typedef struct TMTask {
    Ptr		qLink;
    short	qType;
    ProcPtr	tmAddr;
    long	tmCount;
} TMTask, *TMTaskPtr;

#endif
