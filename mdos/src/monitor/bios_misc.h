/*
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 *
 * Purpose:
 *	V86 BIOS emulation
 *
 * HISTORY: 
 * $Log:	bios_misc.h,v $
 * Revision 2.4  92/03/02  15:47:23  grm
 * 	Added the constant declarations for the interrupt table indices.
 * 	[92/02/27            grm]
 * 
 * Revision 2.3  91/12/06  15:28:38  grm
 * 	Replaced the defines for the mon_space code with the externs for
 * 	the relative dosres code.
 * 	[91/12/06            grm]
 * 
 * Revision 2.2  91/12/05  16:40:54  grm
 * 	Put in the constants for the keyisr.
 * 	[91/07/16  17:43:44  grm]
 * 
 * 	Created.
 * 	[91/06/28  18:13:09  grm]
 * 
 */

#define RESERVE_SPACE	0x2000

extern u_long FAKE_MOUSE_DRIVER;
extern u_long VDISK_HEADER;
extern u_long MOUSE_HELPER_2;
extern u_long MOUSE_HELPER_1;
extern u_long INT16_ISR;
extern u_long XMS_DRIVER;
extern u_long SYS_CONFIG;
extern u_long KEYISR;
extern u_long IRET_LOCATION;
extern u_long EMM_LOCATION;

#define	KEYBOARD_INT_VEC	0
#define TIMER1_INT_VEC		1
#define	TIMER2_INT_VEC		2
#define COM1_INT_VEC		3
#define COM2_INT_VEC		4
#define MOUSE_INT_VEC		5
#define BREAK_INT_VEC		6
