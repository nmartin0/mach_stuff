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
 *	V86 BIOS video emulation header file
 *
 * HISTORY: 
 * $Log:	bios_video.h,v $
 * Revision 2.2  91/12/05  16:41:44  grm
 * 	Added Character_height definition.
 * 	[91/08/09  19:50:46  grm]
 * 
 * 	Added EGA mouse support.
 * 	[91/06/28  18:24:07  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  15:06:42  grm]
 * 
 * 	Minor changes dealing with restructuring
 * 	of bios_vga.h declarations.
 * 	[90/11/05  19:34:49  grm]
 * 
 * 	Moved stuff into bios_vga.h and bios_video_common.h
 * 	[90/10/18  18:01:28  grm]
 * 
 * 	Added some constants
 * 	[90/10/16  16:50:31  grm]
 * 
 * 	Changed display_page to mode_info with more
 * 	general information on the video modes.
 * 	[90/10/02  19:59:19  grm]
 * 
 * 	Created.
 * 	[90/09/27  20:29:43  grm]
 * 
 */
#include "bios_video_common.h"

#define VIDEO_SET_DISPLAY_MODE	0x00
#define VIDEO_SET_CURSOR_TYPE	0x01
#define VIDEO_SET_CSR_POSITION	0x02
#define VIDEO_GET_CSR_POSITION	0x03
#define VIDEO_SET_DISPLAY_PAGE	0x05
#define VIDEO_SCROLL_WIN_UP	0x06
#define VIDEO_SCROLL_WIN_DOWN	0x07
#define VIDEO_READ_ATRB_CHAR	0x08
#define VIDEO_WRITE_ATRB_CHAR	0x09
#define VIDEO_WRITE_CHAR_CURSOR	0x0a
#define VIDEO_SET_PALETTE	0x0b
#define VIDEO_WRITE_PIXEL	0x0c
#define VIDEO_READ_PIXEL	0x0d
#define VIDEO_TTY_OUT		0x0e
#define VIDEO_GET_DISPLAY_MODE	0x0f
#define VIDEO_COLOR_OPS		0x10
#define VIDEO_FONT_OPS		0x11
#define VIDEO_ADVANCED_OPS	0x12
#define VIDEO_WRITE_STRING	0x13
#define VIDEO_SET_COMBIN_MODE	0x1a
#define VIDEO_GET_STATE_INFO	0x1b
#define VIDEO_SAVE_RESTORE	0x1c

#define VMOUSE_READ_1_REG	0xf0
#define VMOUSE_WRITE_1_REG	0xf1
#define VMOUSE_READ_REG_RANGE	0xf2
#define VMOUSE_WRITE_REG_RANGE	0xf3
#define VMOUSE_READ_REG_SET	0xf4
#define VMOUSE_WRITE_REG_SET	0xf5
#define VMOUSE_REVERT_TO_DEF	0xf6
#define VMOUSE_DEFINE_DEF	0xf7
#define VMOUSE_INTERROGATE_DVR	0xfa

/* VIDEO_COLOR_OPS */

#define VCLR_SET_PALETTE_REG	0x00
#define VCLR_SET_BORDER_CLR	0x01
#define VCLR_SET_PAL_BORDER	0x02
#define VCLR_TOGGLE_BLINK_BIT	0x03
#define VCLR_GET_PALETTE_REG	0x07
#define VCLR_GET_BORDER_CLR	0x08
#define VCLR_GET_PAL_BORDER	0x09
#define VCLR_SET_COLOR_REG	0x10
#define VCLR_SET_BLOCK_CLR_REG	0x12
#define VCLR_SET_CLR_PAGE_STATE	0x13
#define VCLR_GET_COLOR_REG	0x15
#define VCLR_GET_BLOCK_CLR_REG	0x17
#define VCLR_GET_CLR_PAGE_STATE	0x1a
#define VCLR_SET_GRAY_SCALE_VAL	0x1b

/* VIDEO_FONT_OPS */

#define VFNT_LOAD_USER_FONT	0x00
#define VFNT_LOAD_ROM_8_X_14	0x01
#define VFNT_LOAD_ROM_8 X_8	0x02
#define VFNT_SET_BLOCK_SPEC	0x03
#define VFNT_LOAD_ROM_8_X_16	0x04
#define VFNT_LOAD_USER_FONT_RC	0x10
#define VFNT_LOAD_ROM_8_X_14_RC	0x11
#define VFNT_LOAD_ROM_8_X_8_RC	0x12
#define VFNT_LOAD_ROM_8_X_16_RC	0x14
#define VFNT_SET_INT_1F_PTR	0x20
#define VFNT_SET_INT_43_USR_FNT	0x21
#define VFNT_SET_INT_43_8_X_14	0x22
#define VFNT_SET_INT_43_8_X_8	0x23
#define VFNT_SET_INT_43_8_X_16	0x24
#define VFNT_GET_FONT_INFO	0x30

/* VIDEO_ADVANCED_OPS */

#define VADV_GET_CONFIG_INFO	0x10
#define VADV_ALT_PRINTSCRN	0x20
#define VADV_SET_SCAN_LINES	0x30
#define VADV_PALETTE_LOADING	0x31
#define VADV_EN_DIS_VIDEO	0x32
#define VADV_EN_DIS_GS_SUM	0x33
#define VADV_EN_DIS_CSR_EMUL	0x34
#define VADV_SWTC_ACV_DISPLAY	0x35
#define VADV_EN_DIS_REFRESH	0x36

/* Memory locations for bios video info */

#define Current_video_mode	(u_char *)0x449
#define Number_of_columns	(u_short *)0x44a
#define Length_of_screen_buffer	(u_short *)0x44c
#define Start_of_current_page	(u_short *)0x44e
#define Cursor_positions	0x450
#define Cursor_mode		(u_short *)0x460
#define Current_display_page	(u_char *)0x462
#define Base_address_host_port	(u_short *)0x463
#define Current_mode		(u_char *)0x465
#define Current_color		(u_char *)0x466
#define Number_of_rows		(u_char *)0x484
#define Character_height	(u_short *)0x485

#define DISPLAY_PAGE_BASE_ADDR	0xb8000

#define CRT_INDEX	0x3d4
#define CRT_DATA	0x3d5
#define CSR_START	0x0a
#define CSR_END		0x0b
#define SAH		0x0c
#define SAL		0x0d
#define CSR_HIGH	0x0e
#define CSR_LOW		0x0f

#define OVERSCAN	0x11

#define BEL		0x07
#define BS		0x08
#define TAB		0x09
#define LF		0x0a
#define CR		0x0d
#define SPACE		0x20

#define GROUP_CRTC	0x00
#define GROUP_SEQUENCER	0x08
#define GROUP_GRAPHICS	0x10
#define GROUP_ATTRIBUTE	0x18

struct cursor_position_t {
	u_char column;
	u_char row;
};

extern struct vga_color_register_state default_color_registers;
extern struct vga_control_register_state vga_init[];
