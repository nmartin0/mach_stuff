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
 *	V86 BIOS keyboard routine header file.
 *
 * HISTORY: 
 * $Log:	bios_kbd.h,v $
 * Revision 2.3  92/02/02  23:02:24  rvb
 * 	Changed kd.h inclusion for first alpha release.
 * 	[92/02/02  14:51:51  grm]
 * 
 * Revision 2.2  91/12/05  16:40:35  grm
 * 	Moved the queue structure definitions from bios_kbd.c
 * 	so that bios_misc could use easier.
 * 	[91/07/16  17:39:23  grm]
 * 
 * 	New Copyright.
 * 	[91/05/28  14:50:13  grm]
 * 
 * 	Corrected constants for the enhanced keyboard ops.
 * 	[90/11/09  21:04:28  grm]
 * 
 * 	Created.
 * 	[90/10/11  20:26:55  grm]
 * 
 */
#include <sys/time.h>
#include "kd.h"

/* 	int 16 bios kbd operations	*/
#define KBD_READ		0x00
#define KBD_GET_KEYBOARD_STATUS	0x01
#define KBD_GET_KEYBOARD_FLAGS	0x02
#define KBD_SET_REPEAT_RATE	0x03
#define KBD_SET_KEYCLICK	0x04
#define KBD_PUSH_C_AND_S	0x05
#define KBD_READ_EN_CHAR	0x10
#define KBD_GET_EN_STATUS	0x11
#define KBD_GET_EN_FLAGS	0x12
#define	ERR_RESET_FAILED	0x05

/* 
 * Scratch Pad BIOS Ram's keyboard conrtol byte bit values:
 */
/* control byte 1:  0x417 */
#define Insert_active		0x80
#define Caps_lock_active	0x40
#define Num_lock_active		0x20
#define Scroll_lock_active	0x10
#define Alt_key_down		0x08
#define Cntrl_key_down		0x04
#define L_Shift_key_down	0x02
#define R_Shift_key_down	0x01

/* control byte 2:  0x418 */
#define Insert_key_down		0x80
#define Caps_lock_down		0x40
#define Num_lock_down		0x20
#define Scroll_lock_down	0x10
#define Pause_mode_active	0x08
#define System_Request_down	0x04
#define L_Alt_key_down		0x02
#define R_Alt_key_down		0x01

/*
 * scan code to ascii mapping for ibm keyboard
 */

#define NoSymbol	     0x0
#define Extended	 0x10000
#define Supressed	 0x20000
#define Cntrl		 0x40000
#define Alt		 0x80000
#define L_Shift		0x100000
#define R_Shift		0x200000

/* scan codes for these keys */
#define K_Cntrl		0x1d
#define K_Alt		0x38
#define K_L_Shift	0x2a
#define K_R_Shift	0x36
#define K_Caps_lock	0x3a
#define K_Del		0x53
#define K_Scroll_lock	0x46
#define K_Insert	0x52
#define K_Num_lock	0x45


/* dos ascii character set */
#define D_SOH	0x1
#define D_STX	0x2
#define D_ETX	0x3
#define D_EOT	0x4
#define D_ENQ	0x5
#define D_ACK	0x6
#define D_BEL	0x7
#define D_BS	0x8
#define D_HT	0x9
#define D_LF	0xa
#define D_VT	0xb
#define D_FF	0xc
#define D_CR	0xd
#define D_SO	0xe
#define D_SI	0xf
#define D_DLE	0x10
#define D_DC1	0x11
#define D_DC2	0x12
#define D_DC3	0x13
#define D_DC4	0x14
#define D_NAK	0x15
#define D_SYN	0x16
#define D_ETB	0x17
#define D_CAN	0x18
#define D_EM	0x19
#define D_SUB	0x1a
#define D_ESC	0x1b
#define D_FS	0x1c
#define D_GS	0x1d
#define D_RS	0x1e
#define D_US	0x1f
#define D_DEL	0x7f

#define Asc_a	'a'
#define Asc_b	'b'
#define Asc_c	'c'
#define Asc_d	'd'
#define Asc_e	'e'
#define Asc_f	'f'
#define Asc_g	'g'
#define Asc_h	'h'
#define Asc_i	'i'
#define Asc_j	'j'
#define Asc_k	'k'
#define Asc_l	'l'
#define Asc_m	'm'
#define Asc_n	'n'
#define Asc_o	'o'
#define Asc_p	'p'
#define Asc_q	'q'
#define Asc_r	'r'
#define Asc_s	's'
#define Asc_t	't'
#define Asc_u	'u'
#define Asc_v	'v'
#define Asc_w	'w'
#define Asc_x	'x'
#define Asc_y	'y'
#define Asc_z	'z'

#define Alt_a	0x1e00
#define Alt_b	0x3000
#define Alt_c	0x2e00
#define Alt_d	0x2000
#define Alt_e	0x1200
#define Alt_f	0x2100
#define Alt_g	0x2200
#define Alt_h	0x2300
#define Alt_i	0x1700
#define Alt_j	0x2400
#define Alt_k	0x2500
#define Alt_l	0x2600
#define Alt_m	0x3200
#define Alt_n	0x3100
#define Alt_o	0x1800
#define Alt_p	0x1900
#define Alt_q	0x1000
#define Alt_r	0x1300
#define Alt_s	0x1f00
#define Alt_t	0x1400
#define Alt_u	0x1600
#define Alt_v	0x2f00
#define Alt_w	0x1100
#define Alt_x	0x2d00
#define Alt_y	0x1500
#define Alt_z	0x2c00

#define Asc_A	'A'
#define Asc_B	'B'
#define Asc_C	'C'
#define Asc_D	'D'
#define Asc_E	'E'
#define Asc_F	'F'
#define Asc_G	'G'
#define Asc_H	'H'
#define Asc_I	'I'
#define Asc_J	'J'
#define Asc_K	'K'
#define Asc_L	'L'
#define Asc_M	'M'
#define Asc_N	'N'
#define Asc_O	'O'
#define Asc_P	'P'
#define Asc_Q	'Q'
#define Asc_R	'R'
#define Asc_S	'S'
#define Asc_T	'T'
#define Asc_U	'U'
#define Asc_V	'V'
#define Asc_W	'W'
#define Asc_X	'X'
#define Asc_Y	'Y'
#define Asc_Z	'Z'

#define Asc_0	'0'
#define Asc_1	'1'
#define Asc_2	'2'
#define Asc_3	'3'
#define Asc_4	'4'
#define Asc_5	'5'
#define Asc_6	'6'
#define Asc_7	'7'
#define Asc_8	'8'
#define Asc_9	'9'

#define Alt_0	0x7800
#define Alt_1	0x7900
#define Alt_2	0x7a00
#define Alt_3	0x7b00
#define Alt_4	0x7c00
#define Alt_5	0x7d00
#define Alt_6	0x7e00
#define Alt_7	0x7f00
#define Alt_8	0x8000
#define Alt_9	0x8100

#define Asc_Excl	'!'
#define Asc_Atsgn	'@'
#define Asc_Pnd		'#'
#define Asc_Dlr		'$'
#define Asc_Pcnt	'%'
#define Asc_Crt		'^'
#define Asc_Ampr	'&'
#define Asc_Astr	'*'
#define Asc_L_Paren	'('
#define Asc_R_Paren	')'
#define Asc_Pipe	'|'
#define Asc_BkSlsh	'\134'
#define Asc_Minus	'-'
#define Asc_UndrScr	'_'
#define Asc_L_Brc	'{'
#define Asc_R_Brc	'}'
#define Asc_L_Brk	'['
#define Asc_R_Brk	']'
#define Asc_Semi	';'
#define Asc_Colon	':'
#define Asc_SQte	'\047'
#define Asc_DQte	'"'
#define Asc_BQte	'\140'
#define Asc_Slsh	'/'
#define Asc_Quest	'?'
#define Asc_Period	'.'
#define Asc_Comma	','
#define Asc_GrtrTh	'>'
#define Asc_LessTh	'<'
#define Asc_Plus	'+'
#define Asc_Equal	'='
#define Asc_Tilde	'~'
#define Asc_Space	' '

#define Alt_Minus	0x8200
#define Alt_Equal	0x8300

#define Asc_BS		'\010'
#define Asc_DEL		'\177'
#define Asc_CR		'\015'
#define Asc_LF		'\012'
#define Asc_TAB		'\011'

#define Fn_1	0x3b00
#define Fn_2	0x3c00
#define Fn_3	0x3d00
#define Fn_4	0x3e00
#define Fn_5	0x3f00
#define Fn_6	0x4000
#define Fn_7	0x4100
#define Fn_8	0x4200
#define Fn_9	0x4300
#define Fn_10	0x4400
#define Fn_11	0xd900
#define Fn_12	0xda00

#define Sh_Fn_1		0x5400
#define Sh_Fn_2		0x5500
#define Sh_Fn_3		0x5600
#define Sh_Fn_4		0x5700
#define Sh_Fn_5		0x5800
#define Sh_Fn_6		0x5900
#define Sh_Fn_7		0x5a00
#define Sh_Fn_8		0x5b00
#define Sh_Fn_9		0x5c00
#define Sh_Fn_10	0x5d00
#define Sh_Fn_11	0x0000
#define Sh_Fn_12	0x0000

#define Ct_Fn_1		0x5e00
#define Ct_Fn_2		0x5f00
#define Ct_Fn_3		0x6000
#define Ct_Fn_4		0x6100
#define Ct_Fn_5		0x6200
#define Ct_Fn_6		0x6300
#define Ct_Fn_7		0x6400
#define Ct_Fn_8		0x6500
#define Ct_Fn_9		0x6600
#define Ct_Fn_10	0x6700
#define Ct_Fn_11	0x0000
#define Ct_Fn_12	0x0000

#define Alt_Fn_1	0x6800
#define Alt_Fn_2	0x6900
#define Alt_Fn_3	0x6a00
#define Alt_Fn_4	0x6b00
#define Alt_Fn_5	0x6c00
#define Alt_Fn_6	0x6d00
#define Alt_Fn_7	0x6e00
#define Alt_Fn_8	0x6f00
#define Alt_Fn_9	0x7000
#define Alt_Fn_10	0x7100
#define Alt_Fn_11	0x0000
#define Alt_Fn_12	0x0000

#define Home		0x4700
#define Up_Arrow	0x4800
#define Page_Up		0x4900
#define Left_Arrow	0x4b00
#define Right_Arrow	0x4d00
#define End		0x4f00
#define Down_Arrow	0x5000
#define Page_Down	0x5100
#define Insert		0x5200
#define Delete		0x5300

#define Ct_Home		0x7700
#define Ct_Page_Up	0x8400
#define Ct_Left_Arrow	0x7300
#define Ct_Right_Arrow	0x7400
#define Ct_End		0x7500
#define Ct_Page_Down	0x7600

typedef struct {
	u_int base;
	u_int shift;
	u_int cntrl;
	u_int alt;
#ifdef FINISHED_CODING
	u_int shalt;
#endif FINISHED_CODING
} keymap_t;

keymap_t pc_keymap[] = {

NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 0x00 */
D_ESC,		D_ESC,		D_ESC,		Supressed,	/* 0x01 - Escape */
Asc_1,		Asc_Excl,	NoSymbol,	Alt_1,		/* 0x02 */
Asc_2,		Asc_Atsgn,	Supressed,	Alt_2,		/* 0x03 */
Asc_3,		Asc_Pnd,	Supressed,	Alt_3,		/* 0x04 */
Asc_4,		Asc_Dlr,	Supressed,	Alt_4,		/* 0x05 */
Asc_5,		Asc_Pcnt,	Supressed,	Alt_5,		/* 0x06 */
Asc_6,		Asc_Crt,	D_RS,		Alt_6,		/* 0x07 */
Asc_7,		Asc_Ampr,	Supressed,	Alt_7,		/* 0x08 */
Asc_8,		Asc_Astr,	Supressed,	Alt_8,		/* 0x09 */
Asc_9,		Asc_L_Paren,	Supressed,	Alt_9,		/* 0x0a */
Asc_0,		Asc_R_Paren,	Supressed,	Alt_0,		/* 0x0b */
Asc_Minus,	Asc_UndrScr,	D_US,		Alt_Minus,	/* 0x0c */
Asc_Equal,	Asc_Plus,	Supressed,	Alt_Equal,	/* 0x0d */
Asc_BS,		Asc_BS,		D_DEL,		Supressed,	/* 0x0e */
Asc_TAB,	Extended,	Supressed,	Supressed,	/* 0x0f */
Asc_q,		Asc_Q,		D_DC1,		Alt_q,		/* 0x10 */
Asc_w,		Asc_W,		D_ETB,		Alt_w,		/* 0x11 */
Asc_e,		Asc_E,		D_ENQ,		Alt_e,		/* 0x12 */
Asc_r,		Asc_R,		D_DC2,		Alt_r,		/* 0x13 */
Asc_t,		Asc_T,		D_DC4,		Alt_t,		/* 0x14 */
Asc_y,		Asc_Y,		D_EM,		Alt_y,		/* 0x15 */
Asc_u,		Asc_U,		D_NAK,		Alt_u,		/* 0x16 */
Asc_i,		Asc_I,		D_HT,		Alt_i,		/* 0x17 */
Asc_o,		Asc_O,		D_SI,		Alt_o,		/* 0x18 */
Asc_p,		Asc_P,		D_DLE,		Alt_p,		/* 0x19 */
Asc_L_Brk,	Asc_L_Brc,	D_ESC,		Supressed,	/* 0x1a */
Asc_R_Brk,	Asc_R_Brc,	D_GS,		Supressed,	/* 0x1b */
Asc_CR,		Asc_CR,		D_LF,		Supressed,	/* 0x1c */
Cntrl,		Supressed,	Supressed,	Supressed,	/* 0x1d */
Asc_a,		Asc_A,		D_SOH,		Alt_a,		/* 0x1e */
Asc_s,		Asc_S,		D_DC3,		Alt_s,		/* 0x1f */
Asc_d,		Asc_D,		D_EOT,		Alt_d,		/* 0x20 */
Asc_f,		Asc_F,		D_ACK,		Alt_f,		/* 0x21 */
Asc_g,		Asc_G,		D_BEL,		Alt_g,		/* 0x22 */
Asc_h,		Asc_H,		D_BS,		Alt_h,		/* 0x23 */
Asc_j,		Asc_J,		D_LF,		Alt_j,		/* 0x24 */
Asc_k,		Asc_K,		D_VT,		Alt_k,		/* 0x25 */
Asc_l,		Asc_L,		D_FF,		Alt_l,		/* 0x26 */
Asc_Semi,	Asc_Colon,	Supressed,	Supressed,	/* 0x27 */
Asc_SQte,	Asc_DQte,	Supressed,	Supressed,	/* 0x28 */
Asc_BQte,	Asc_Tilde,	Supressed,	Supressed,	/* 0x29 */
L_Shift,	Supressed,	Supressed,	Supressed,	/* 0x2a */
Asc_BkSlsh,	Asc_Pipe,	D_FS,		Supressed,	/* 0x2b */
Asc_z,		Asc_Z,		D_SUB,		Alt_z,		/* 0x2c */
Asc_x,		Asc_X,		D_CAN,		Alt_x,		/* 0x2d */
Asc_c,		Asc_C,		D_ETX,		Alt_c,		/* 0x2e */
Asc_v,		Asc_V,		D_SYN,		Alt_v,		/* 0x2f */
Asc_b,		Asc_B,		D_STX,		Alt_b,		/* 0x30 */
Asc_n,		Asc_N,		D_SO,		Alt_n,		/* 0x31 */
Asc_m,		Asc_M,		D_CR,		Alt_m,		/* 0x32 */
Asc_Comma,	Asc_LessTh,	Supressed,	Supressed,	/* 0x33 */
Asc_Period,	Asc_GrtrTh,	Supressed,	Supressed,	/* 0x34 */
Asc_Slsh,	Asc_Quest,	Supressed,	Supressed,	/* 0x35 */
R_Shift,	Supressed,	Supressed,	Supressed,	/* 0x36 */
Asc_Astr,	NoSymbol,       NoSymbol,	Supressed,	/* 0x37 - Print Screen */
Alt,		Supressed,	Supressed,	Supressed,	/* 0x38 */
Asc_Space,	Asc_Space,	Asc_Space,	Asc_Space,	/* 0x39 */
NoSymbol,	NoSymbol,	NoSymbol,	Supressed,	/* 0x3a - Caps Lock */
Fn_1,		Sh_Fn_1,	Ct_Fn_1,	Alt_Fn_1,	/* 0x3b */
Fn_2,		Sh_Fn_2,	Ct_Fn_2,	Alt_Fn_2,	/* 0x3c */
Fn_3,		Sh_Fn_3,	Ct_Fn_3,	Alt_Fn_3,	/* 0x3d */
Fn_4,		Sh_Fn_4,	Ct_Fn_4,	Alt_Fn_4,	/* 0x3e */
Fn_5,		Sh_Fn_5,	Ct_Fn_5,	Alt_Fn_5,	/* 0x3f */
Fn_6,		Sh_Fn_6,	Ct_Fn_6,	Alt_Fn_6,	/* 0x40 */
Fn_7,		Sh_Fn_7,	Ct_Fn_7,	Alt_Fn_7,	/* 0x41 */
Fn_8,		Sh_Fn_8,	Ct_Fn_8,	Alt_Fn_8,	/* 0x42 */
Fn_9,		Sh_Fn_9,	Ct_Fn_9,	Alt_Fn_9,	/* 0x43 */
Fn_10,		Sh_Fn_10,	Ct_Fn_10,	Alt_Fn_10,	/* 0x44 */
NoSymbol,	NoSymbol,	NoSymbol,	Supressed,	/* 0x45 - Num Lock */
NoSymbol,	NoSymbol,	NoSymbol,	Supressed,	/* 0x46 - Scroll Lock */
Home,		NoSymbol,	Ct_Home,	Supressed,	/* 0x47 */
Up_Arrow,	NoSymbol,	Supressed,	Supressed,	/* 0x48 */
Page_Up,	NoSymbol,	Ct_Page_Up,	Supressed,	/* 0x49 */
Asc_Minus,	NoSymbol,	Supressed,	Supressed,	/* 0x4a */
Left_Arrow,	NoSymbol,	Ct_Left_Arrow,	Supressed,	/* 0x4b */
Asc_5,		NoSymbol,	Supressed,	Supressed,	/* 0x4c */
Right_Arrow,	NoSymbol,	Ct_Right_Arrow,	Supressed,	/* 0x4d */
Asc_Plus,	NoSymbol,	Supressed,	Supressed,	/* 0x4e */
End,		NoSymbol,	Ct_End,		Supressed,	/* 0x4f */
Down_Arrow,	NoSymbol,	Supressed,	Supressed,	/* 0x50 */
Page_Down,	NoSymbol,	Ct_Page_Down,	Supressed,	/* 0x51 */
Insert,		NoSymbol,	Supressed,	Supressed,	/* 0x52 */
Delete,		NoSymbol,	NoSymbol,	Supressed,	/* 0x53 */

NoSymbol,	NoSymbol,	NoSymbol,	Supressed,	/* 0x54 */
NoSymbol,	NoSymbol,	NoSymbol,	Supressed,	/* 0x55 */
NoSymbol,	NoSymbol,	NoSymbol,	Supressed,	/* 0x56 */
Fn_11,		NoSymbol,	NoSymbol,	NoSymbol,	/* 0x57 */
Fn_12,		NoSymbol,	NoSymbol,	NoSymbol	/* 0x58 */

};

#define Numkeys		(sizeof(pc_keymap)/sizeof(keymap_t))

#define KS_NORM		0x01
#define KS_CNTRL	0x02
#define KS_SHIFT	0x03
#define KS_ALT		0x04
#define KS_SHALT	0x05

struct queue_entry {
	int	entry;
	long	time;
	struct queue_entry *next;
};

struct queue_type {
	struct queue_entry * head;
	struct queue_entry * tail;
};

