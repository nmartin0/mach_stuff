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
 * $Log:	Retrace.h,v $
 * Revision 2.2  91/09/12  16:51:00  bohman
 * 	Created.
 * 	[91/09/11  16:31:35  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2os/Retrace.h
 */

#ifndef _RETRACE_
#define _RETRACE_

typedef struct {
    Ptr			qLink;
    unsigned short	qType;
    void		(*vblAddr)();
    unsigned short	vblCount;
    unsigned short	vblPhase;
} VBLTask, *VBLTaskPtr;

#endif
