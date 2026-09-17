/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/* load the serial port drivers */

#include <Devices.h>
#include <Serial.h>

#include "AlertMsg.h"

#define FALSE 0

#define TWO_STOP_BITS  0xC000
#define EIGHT_BIT_DATA 0x0C00
#define NO_PARITY      0x0000
#define BAUD_2400      0x002E

#define DEFAULT_BITS (BAUD_2400 | TWO_STOP_BITS | EIGHT_BIT_DATA | NO_PARITY)

#define SerReset_Code    8
#define SerHShake_Code  10
#define SerDTRoff_Code  18

/* parameters to SerHShake */
typedef struct {
	unsigned char fXon;
	unsigned char fCTS;
	char xOn;
	char xOff;
	unsigned char errs;
	unsigned char evts;
	unsigned char fInX;
	unsigned char null;
} *SerHShake_t;

/* parameters to SerReset */
typedef struct {
	unsigned short SerConfig;
} *SerReset_t;

/* open a serial port driver, return non-zero if error */
static int open_port(name, flag)
char *name;
int flag;
{
	ParamBlockRec P;
	int ioRefNum;

	/* open the rom serial driver */
	P.ioParam.ioNamePtr = name;
	P.ioParam.ioPermssn = fsCurPerm;
	if (PBOpen(&P, FALSE) != noErr) {
		if (flag) AlertMsg(MSG_LOCALTALK_ACTIVE, 0, 0, 0);
		else AlertMsg(MSG_BAD_PORT_ROMOPEN, name, 0, 0);
		return -1;
	}
	ioRefNum = P.ioParam.ioRefNum;

	/* open the ram serial driver */
	if (ioRefNum == -6) {
		if (RAMSDOpen(sPortA) != noErr) {
			AlertMsg(MSG_BAD_PORT_RAMOPEN, name, 0, 0);
			return -1;
		}
	}
	else if (ioRefNum == -8) {
		if (RAMSDOpen(sPortB) != noErr) {
			AlertMsg(MSG_BAD_PORT_RAMOPEN, name, 0, 0);
			return -1;
		}
	}

	/* reset serial port */
	((SerReset_t)P.cntrlParam.csParam)->SerConfig = DEFAULT_BITS;
	P.cntrlParam.ioCRefNum = ioRefNum;
	P.cntrlParam.csCode = SerReset_Code;
	if (PBControl(&P, FALSE) != noErr) {
		AlertMsg(MSG_BAD_PORT_SERRESET, name, 0, 0);
		return -1;
	}

	/* turn off all handshaking */
	((SerHShake_t)P.cntrlParam.csParam)->fXon = 0;
	((SerHShake_t)P.cntrlParam.csParam)->fCTS = 0;
	((SerHShake_t)P.cntrlParam.csParam)->xOn = 0;
	((SerHShake_t)P.cntrlParam.csParam)->xOff = 0;
	((SerHShake_t)P.cntrlParam.csParam)->errs = 0;
	((SerHShake_t)P.cntrlParam.csParam)->evts = 0;
	((SerHShake_t)P.cntrlParam.csParam)->fInX = 0;
	((SerHShake_t)P.cntrlParam.csParam)->null = 0;
	P.cntrlParam.ioCRefNum = ioRefNum;
	P.cntrlParam.csCode = SerHShake_Code;
	if (PBControl(&P, FALSE) != noErr) {
		AlertMsg(MSG_BAD_PORT_SERHSHAKE, name, 0, 0);
		return -1;
	}

	/* turn of DTR */
	P.cntrlParam.ioCRefNum = ioRefNum;
	P.cntrlParam.csCode = SerDTRoff_Code;
	if (PBControl(&P, FALSE) != noErr) {
		AlertMsg(MSG_BAD_PORT_SERDTROFF, name, 0, 0);
		return -1;
	}

	return 0;

}

/* open all serial port drivers, return non-zero if error */
int SerialPorts()
{
	char n[6];

	/* this sillyness is because an INIT can't have global references */
	n[0] = 4; n[1] = '.'; n[2] = 'A'; n[3] = 'I'; n[4] = 'n';
	if (open_port(n, 0)) return -1;
	n[0] = 5; n[1] = '.'; n[2] = 'A'; n[3] = 'O'; n[4] = 'u'; n[5] = 't';
	if (open_port(n, 0)) return -1;
	n[0] = 4; n[1] = '.'; n[2] = 'B'; n[3] = 'I'; n[4] = 'n';
	if (open_port(n, 1)) return -1;
	n[0] = 5; n[1] = '.'; n[2] = 'B'; n[3] = 'O'; n[4] = 'u'; n[5] = 't';
	if (open_port(n, 0)) return -1;
	return 0;

}