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
 * $Log:	machine_routines.h,v $
 * Revision 2.2  91/09/12  16:41:40  bohman
 * 	Created.
 * 	[91/09/11  14:52:23  bohman]
 * 
 */

/*
 * Check for machine dependent
 * kernel IPC interfaces here.
 */
#ifdef MACSERVER
#define MACHINE_SERVER(in, out)	(macserver_server((in), (out)))
#endif
