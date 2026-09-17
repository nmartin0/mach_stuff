/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/************************************************************

Created: Tuesday, September 19, 1989 at 11:22 AM
	OSUtils.h
	C Interface to the Macintosh Libraries


	Copyright Apple Computer, Inc.	1985-1989
	All rights reserved
	
	Dont use:
	-SetUpA5
	-RestoreA5
	Instead:
	SetUpA5 --> myA5 = SetCurrentA5();
	RestoreA5 --> oldA5 = SetA5(myA5);

************************************************************/


#ifndef __OSUTILS__
#define __OSUTILS__

#define useFree 0
#define useATalk 1
#define useAsync 2
#define useExtClk 3 		/*Externally clocked*/
#define useMIDI 4
#define curSysEnvVers 2 	/*Updated to equal latest SysEnvirons version*/
#define macXLMachine 0
#define macMachine 1
#define envMac -1			/*Environs Equates*/
#define envXL -2
#define envMachUnknown 0
#define env512KE 1
#define envMacPlus 2
#define envSE 3
#define envMacII 4
#define envMacIIx 5
#define envMacIIcx 6
#define envSE30 7
#define envPortable 8
#define envMacIIci 9
#define envCPUUnknown 0 	/*CPU types*/
#define env68000 1
#define env68010 2
#define env68020 3
#define env68030 4
#define envUnknownKbd 0 	/*Keyboard types*/
#define envMacKbd 1
#define envMacAndPad 2
#define envMacPlusKbd 3
#define envAExtendKbd 4
#define envStandADBKbd 5
#define envPortADBKbd 6
#define envPortISOADBKbd 7
#define envStdISOADBKbd 8
#define envExtISOADBKbd 9
#define false32b 0			/*24 bit addressing error*/
#define true32b 1			/*32 bit addressing error*/

/* result types for RelString Call */

#define sortsBefore -1		/*first string < second string*/
#define sortsEqual 0		/*first string = second string*/
#define sortsAfter 1		/*first string > second string*/

enum {dummyType,vType,ioQType,drvQType,evType,fsQType };
typedef unsigned short QTypes;

enum {OSTrap,ToolTrap};
typedef unsigned char TrapType;

struct SysParmType {
    char valid;
    char aTalkA;
    char aTalkB;
    char config;
    short portA;
    short portB;
    long alarm;
    short font;
    short kbdPrint;
    short volClik;
    short misc;
};

typedef struct SysParmType SysParmType;
typedef SysParmType *SysPPtr;

typedef struct QElem {
    struct QElem	*qLink;
    unsigned short	qType;
} QElem, *QElemPtr;

typedef struct QHdr {
    unsigned short	qFlags;
    QElemPtr		qHead;
    QElemPtr		qTail;
} QHdr, *QHdrPtr;

struct DateTimeRec {
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
	short dayOfWeek;
};

typedef struct DateTimeRec DateTimeRec;
struct SysEnvRec {
    short environsVersion;
    short machineType;
    short systemVersion;
    short processor;
    Boolean hasFPU;
    Boolean hasColorQD;
    short keyBoardType;
    short atDrvrVersNum;
    short sysVRefNum;
};

typedef struct SysEnvRec SysEnvRec;

#endif
