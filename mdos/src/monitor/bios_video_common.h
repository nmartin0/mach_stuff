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
 *	V86 BIOS vga emulation header file
 *
 * HISTORY: 
 * $Log:	bios_video_common.h,v $
 * Revision 2.4  92/05/22  15:59:36  grm
 * 	Added some constants for the number of indices in VGA registers.
 * 	Added MAX_VIDEO_MODES constant.
 * 	[92/05/21            grm]
 * 
 * Revision 2.2  91/12/05  16:41:54  grm
 * 	Added the width and height fields to the mode_info
 * 	structure.
 * 	[91/08/09  19:52:10  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  15:10:57  grm]
 * 
 * 	Changed declarations for more intuitive
 * 	usage in bios_video.c and bios_vga.c.
 * 	[90/11/05  19:36:12  grm]
 * 
 * 	Created.
 * 	[90/10/18  18:02:03  grm]
 * 
 */

#ifndef _BIOS_VIDEO_COMMON_H
#define _BIOS_VIDEO_COMMON_H 1

#define MAX_VGA_MEM		0xc0000

#define	ATTR_REG_COUNT	0x15
#define	CRTC_REG_COUNT	0x19
#define	SEQ_REG_COUNT	0x5
#define	GRAF_REG_COUNT	0x9

struct vga_control_register_state {
	/* General Registers */
	u_char Misc_Output_Reg;
	u_char Input_Status_0;
	u_char Input_Status_1;
	u_char Feature_Control;
	u_char Video_Enable;

	/* Attribute control registers */
	u_char attr_index;
	u_char attr_regs[ATTR_REG_COUNT];

	/* Crt control registers */
	u_char crtc_index;
	u_char crtc_regs[CRTC_REG_COUNT];

	/* Sequencer registers */
	u_char seq_index;
	u_char seq_regs[SEQ_REG_COUNT];

	/* Graphics control registers */
	u_char graf_index;
	u_char graf_regs[GRAF_REG_COUNT];
};

struct vga_color_register_state {
	boolean_t load_color_regs;
	int colors_used;
	u_char * color_array;
};

struct vga_state {
	struct	vga_control_register_state * control_registers;
	struct	vga_color_register_state * color_registers;
	int	supervga_state_index;
};

struct video_mode_entry {
	u_long	start;
	u_char	pages;
	u_short offset;
	u_short size;
	u_short max_x;
	u_short max_y;
	u_char	width;
	u_char	height;
};
#define	MAX_VIDEO_MODES	20
		
#endif _BIOS_VIDEO_COMMON_H
