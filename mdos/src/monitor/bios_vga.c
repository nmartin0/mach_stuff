/*
 * Copyright (c) 1992,1991 Carnegie Mellon University
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
 * The V86 Mode screen handling routines.
 *
 *
 * HISTORY:
 * $Log:	bios_vga.c,v $
 * Revision 2.7  92/05/22  15:59:27  grm
 * 	Finished the groundwork for SuperVGA.  Tseng Labs ET4000 works at
 * 	this point.  Changed the register and video ram save/restore
 * 	state routines.  Added some constants.
 * 	[92/05/21            grm]
 * 
 * Revision 2.5  92/04/14  13:20:22  grm
 * 	Started adding in the SuperVGA code.  It's ifdefed out for this
 * 	release.
 * 	[92/04/14            grm]
 * 	No runtime files.  Mach3 Version.
 * 	[92/03/27            grm]
 * 
 * Revision 2.4  92/02/14  17:44:48  grm
 * 	Conditionalized the vga.info and font.info.vga loading code to
 * 	use the DOSPREFIX environment variable when OSF_SVR is defined.
 * 	[92/02/14            grm]
 * 	Added predeclaration of fopenp so that compiler won't complain.
 * 	[92/02/11            grm]
 * 
 * Revision 2.3  92/02/03  14:24:54  rvb
 * 	Clean Up
 * 
 * Revision 2.2  91/12/05  16:41:10  grm
 * 	Modified the initialization code so that it will search the LPATH
 * 	for the initialization files.
 * 	[91/12/04            grm]
 * 	Updated code to use setup.exe version 2.0.
 * 	Now load in font information form setup3 aka vga.info.
 * 	[91/08/09  19:46:06  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  15:01:03  grm]
 * 
 * 	Rewrote so that works correctly on Dells and Toshibas.
 * 	Fixed the font stuff correctly.  
 * 	[91/05/02  13:33:11  grm]
 * 
 * 	Does resets right on sequencer register.
 * 	[91/04/03  14:57:18  grm]
 * 
 * 	Put in code to save and restore the vga's ram.
 * 	[91/03/26  19:01:32  grm]
 * 
 * 	Switched sequencer register init so that works right on
 * 	toshiba.
 * 	[91/02/01  13:41:13  grm]
 * 
 * 	Added font reloading support.  Added color registers
 * 	to vga_ports.
 * 	[90/11/09  21:01:22  grm]
 * 
 * 	Extensive changes.  Restructured header
 * 	info with bios_video.c.  Removed register
 * 	code and wrote it from scratch.
 * 	[90/11/05  19:32:53  grm]
 * 
 * 	Rewritten middle stage.
 * 	Checked in just for kicks.
 * 	[90/10/18  17:58:18  grm]
 * 
 * 	corrections.
 * 	[90/04/30  15:37:36  grm]
 * 
 * 	Added vga_ports array for use with the PORT_FOO macros.
 * 	Put in initialization code to load in the vga_ports into
 * 	io_ports bitmap. Changed fprintfs to use dbg_fd instead
 * 	of stderr.
 * 	[90/04/17  22:35:52  grm]
 * 
 * 	Moved enable_iopl to v86.c, added bios_vga_fn.
 * 	Changed bios_vga_init.
 * 	[90/04/05  21:19:20  grm]
 * 
 * 	Started editing as grm.  Moved to v86 branch.
 * 	[90/03/28  18:36:22  grm]
 * 
 * Revision 2.1.1.1  90/03/22  21:50:00  dorr
 * 	Code to use vga from dos.
 * 
 */

#include "base.h"
#include "bios.h"
#include "bios_video_common.h"
#include <sys/file.h>

#define VGA_PORTS	0x30

#ifdef ALL_VGA_PORTS
unsigned short vga_ports[VGA_PORTS] = {
	0x3b0, 0x3b1, 0x3b2, 0x3b3, 0x3b4, 0x3b5, 0x3b6, 0x3b7,
	0x3b8, 0x3b9, 0x3ba, 0x3bb, 0x3bc, 0x3bd, 0x3be, 0x3bf,
	0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4, 0x3c5, 0x3c6, 0x3c7,
	0x3c8, 0x3c9, 0x3ca, 0x3cb, 0x3cc, 0x3cd, 0x3ce, 0x3cf,
	0x3d0, 0x3d1, 0x3d2, 0x3d3, 0x3d4, 0x3d5, 0x3d6, 0x3d7,
	0x3d8, 0x3d9, 0x3da, 0x3db, 0x3dc, 0x3dd, 0x3de, 0x3df };
#else
unsigned short vga_ports[] = {
	0x3c2, 0x3cc, 0x3da, 0x3ca, 0x3c4, 0x3c5, 0x3d4, 0x3d5,
	0x3ce, 0x3cf, 0x3c0, 0x3c1, 0x3c8, 0x3c7, 0x3c9, 0x3c6, 0x00};
#endif

#ifdef	RUNTIME_FILES
struct vga_color_register_state default_color_registers;
struct vga_control_register_state vga_init[0x14];
u_char vga_color_arrays[0x14][0x100][3];
u_long vgafont_info[6];
#endif	RUNTIME_FILES

extern struct video_mode_entry mode_info[];
extern int exit_index;

#ifdef	OSF_SVR
extern char osfdir[];
extern int prefix_len;
#else	/* BSD 4.3 UX Server */
extern char * lpath;
FILE * fopenp();
#endif	OSF_SVR

struct vga_state before_dos_state;
struct vga_state hardware_vga_state;
struct vga_control_register_state crstate0;
struct vga_control_register_state crstate1;
struct vga_color_register_state clstate0;
struct vga_color_register_state clstate1;
u_char color_palette0[3*256];
u_char color_palette1[3*256];

#define FONT_SIZE	(128 * 1024)
u_char fonts[FONT_SIZE];

/* these constants are for any VGA with up to 256K ram */
#define VGA_DATA_AREA	0xa0000
#define VGA_DATA_LENGTH	0x10000
#define MAX_BIT_PLANES	4

vm_address_t mach_planes;
vm_address_t dos_planes;
int dos_planes_mode;

extern boolean_t (*save_supervga_state)();
extern boolean_t (*restore_supervga_state)();
extern boolean_t (*save_supervga_ram)();
extern boolean_t (*restore_supervga_ram)();

extern u_char * display_mode;

/*
 * Save the vga hardware state.
 */
void save_standard_vga_state( video_state )
	struct vga_state * video_state;
{
	u_char Target_Reg;
	int i,j;
	struct vga_control_register_state * VS = video_state->control_registers;
	struct vga_color_register_state * CS = video_state->color_registers;

	Vdebug0((dbg_fd,"bios_vga: save vga CS: 0x%x\n",CS));

	VS->Feature_Control = inb( 0x3ca );
	VS->Video_Enable = inb( 0x3c3 );
	VS->Misc_Output_Reg = inb( 0x3cc );
	VS->Input_Status_0 = inb( 0x3c2 );
	/* Initialize Flip Flop */
	VS->Input_Status_1 = inb( 0x3da );

	/* Save Attribute Registers */
	VS->attr_index = inb( 0x3c0 );

	for ( Target_Reg = 0 ; Target_Reg < ATTR_REG_COUNT ; Target_Reg++ ) {
		outb( 0x3C0, Target_Reg ) ;
		/* Read It, Save It, Then Write It Back */
		outb( 0x3c0, ( VS->attr_regs[Target_Reg] = inb( 0x3C1 ) ) );
	}
	outb ( 0x3c0, (VS->attr_index | 0x20) );

	/* Save Crtc Regs */
	VS->crtc_index = inb( 0x3d4 );
	for ( Target_Reg = 0 ; Target_Reg < CRTC_REG_COUNT ; Target_Reg++ ) {
		outb( 0x3d4, Target_Reg ) ;
		VS->crtc_regs[Target_Reg] = inb( 0x3d5 );
	}
	outb ( 0x3d4, VS->crtc_index );
	
	/* Save Sequencer Registers */
	VS->seq_index = inb( 0x3c4 );
	for ( Target_Reg = 0 ; Target_Reg < SEQ_REG_COUNT ; Target_Reg++ ) {
		outb( 0x3c4, Target_Reg ) ;
		VS->seq_regs[Target_Reg] = inb( 0x3c5 );
	}
	outb ( 0x3c4, VS->seq_index );
	
	/* Save Graphics Registers */
	VS->graf_index = inb( 0x3ce );
	for ( Target_Reg = 0 ; Target_Reg < GRAF_REG_COUNT ; Target_Reg++ ) {
		outb( 0x3ce, Target_Reg ) ;
		VS->graf_regs[Target_Reg] = inb( 0x3cf );
	}
	outb ( 0x3ce, VS->graf_index );

	if (CS != NULL) {
		Vdebug0((dbg_fd,"bios_vga: saving color_regs %d\n",CS->colors_used));
		/* Save Color Registers */
		outb ( 0x3c7, 0x00 );
		for ( i=0; i < CS->colors_used ; i++ ) {
			for ( j=0; j<3; j++ ) {
				CS->color_array[i*3+j] = inb ( 0x3c9 );
				Vdebug0((dbg_fd,"%x ",CS->color_array[i*3+j]));
			}
		}
		Vdebug0((dbg_fd,"\n"));
	}
}

void save_vga_state( video_state )
	struct vga_state * video_state;
{
	if (save_supervga_state) {
		save_supervga_state_preproc( video_state );
	}else{
		save_standard_vga_state( video_state );
	}
}

/*
 * Restore the vga hardware state.
 */
void restore_standard_vga_state( video_state , clear_p , mode)
	struct vga_state * video_state;
	boolean_t clear_p;
	int mode;
{
	u_char Target_Reg;
	int i,j;
	struct vga_control_register_state * VS = video_state->control_registers;
	struct vga_color_register_state * CS = video_state->color_registers;

	Vdebug0((dbg_fd,"bios_vga: restore vga CS: 0x%x\n",CS));

	/* RESET sequencer register zero */
	outb ( 0x3c4, 0 );
	outb ( 0x3c5, 1 );
	outb ( 0x3c5, 3 );

	/* Write the General Registers */
	outb ( 0x3c2, VS->Misc_Output_Reg );
	outb ( 0x3da, VS->Feature_Control );

	/* disable screen memory */
	/* turn off display for memory access */
	outb ( 0x3C4, 0x01 );
	Target_Reg = inb ( 0x3C5 );
	outb ( 0x3C5, (Target_Reg | 0x20) );

	outb( 0x3c4, 0 );
	outb( 0x3c5, VS->seq_regs[0] );
	outb( 0x3c4, 2 );
	outb( 0x3c5, VS->seq_regs[2] );
	outb( 0x3c4, 3 );
	outb( 0x3c5, VS->seq_regs[3] );
	outb( 0x3c4, 4 );
	outb( 0x3c5, VS->seq_regs[4] );

	/* Restore Graphics Registers */
	for ( Target_Reg = 0 ; Target_Reg < 0x09 ; Target_Reg++ ) {
		outb( 0x3ce, Target_Reg ) ;
		outb( 0x3cf, VS->graf_regs[Target_Reg] );
	}
	VS->graf_index = inb( 0x3ce );

	if (clear_p) {
		/* Clear screen page zero if clear_p set */
		if ((mode < 0x04) || (mode == 0x07)) {
			u_short * addr;
			addr = (u_short *)(mode_info[mode].start +
					   (mode_info[mode].size));
			while ((u_short *)mode_info[mode].start <= addr) {
				/* black white space */
				*addr = 0x0720;
				addr--;
			}
		}else{
			u_long * addr;
			
			if (mode < MAX_VIDEO_MODES) {
				addr = (u_long *)mode_info[mode].start;
			}else{
				addr = (u_long *)0xa0000;
			}

			while(addr < (u_long *)MAX_VGA_MEM) {
				*(addr++) = 0;
			}
		}
	}

	outb( 0x3c4, 1 );
	outb( 0x3c5, VS->seq_regs[1] );
	outb( 0x3c4, VS->seq_index );

	/* Initialize Flip Flop */
	Target_Reg = inb( 0x3da );

	/* Restore Attribute Registers */
	for ( Target_Reg = 0 ; Target_Reg < ATTR_REG_COUNT ; Target_Reg++ ) {
		outb( 0x3C0, Target_Reg ) ;
		outb( 0x3c0, VS->attr_regs[Target_Reg] );
	}
	outb ( 0x3c0, (VS->attr_index | 0x20) );

	/*
	 * Make sure that the crtc regs can be read and written to.
	 */
	outb( 0x3d4, 0x11 );
	Target_Reg = inb( 0x3d5 );
	outb( 0x3d5, Target_Reg & 0x7f );

	outb( 0x3d4, 0x03 );
	Target_Reg = inb( 0x3d5 );
	outb( 0x3d5, Target_Reg | 0x80 );

	/* Restore Crtc Regs */
	for ( Target_Reg = 0 ; Target_Reg < 0x19 ; Target_Reg++ ) {
		outb( 0x3d4, Target_Reg ) ;
		outb( 0x3d5, VS->crtc_regs[Target_Reg] );
	}
	outb ( 0x3d4, VS->crtc_index );

	if (video_debug_level > Debug_Level_0) {
		u_char * ptr = (u_char *) VS;
		fprintf(dbg_fd,"VGA Control Register State:\n");
		for(i=0;i< sizeof(struct vga_control_register_state); i++) {
			fprintf(dbg_fd,"%x ", ptr[i]);
		}
		fprintf(dbg_fd,"\n");
	}
	
	if (CS != NULL) {
		if (CS->load_color_regs) {
			Vdebug0((dbg_fd,"bios_vga: restoring color regs %d\n",
				 CS->colors_used));
			/* Restore Color Registers */
			outb ( 0x3c8, 0x00 );
			for ( i=0; i < CS->colors_used ; i++ ) {
				for ( j=0; j<3; j++ ) {
					outb( 0x3c9, CS->color_array[i*3+j] );
					Vdebug0((dbg_fd,"%x ",
						 CS->color_array[i*3+j]));
				}
			}
			Vdebug0((dbg_fd,"\n"));
		}
	}
}

void restore_vga_state( video_state , clear_p , mode )
	struct vga_state * video_state;
	boolean_t clear_p;
	int mode;
{
	if (restore_supervga_state) {
		restore_supervga_state_preproc( video_state, clear_p, mode );
	}else{
		restore_standard_vga_state( video_state, clear_p, mode );
	}
}

void restore_vga_fonts ()
{
	u_char graphic_index;
	u_char sequencer_index;
	u_char mode_value;
	u_char memory_mode_value;
	u_char map_mask_value;
	u_char bit_mask_value;
	u_char enable_set_reset_value;
	u_char data_rotate_value;
	u_char misc_value;
	u_char * bit_plane;
	int i;
	u_long * vga_ptr;
	u_long * font_ptr;

	/* RESET */
	sequencer_index = inb ( 0x3C4 );
	outb (0x3c4, 0 );
	outb (0x3c5, 1 );
	outb (0x3c5, 3 );

	/* turn on write mode #0 and turn off o/e */
	graphic_index = inb ( 0x3CE );
	outb ( 0x3CE, 0x05 );
	mode_value = inb ( 0x3CF );
	outb ( 0x3CF, (mode_value & ~0x13));
	
	/* make sure that memory mode register has o/e compliment */
	outb ( 0x3C4, 0x04 );
	memory_mode_value = inb ( 0x3C5 );
	outb ( 0x3C5, (memory_mode_value | 0x04) );

	/* turn off chaining */
	/* set mm = 0 --> start of mem is 0xa0000 and length is 128K */
	outb ( 0x3CE, 0x06 );
	misc_value = inb ( 0x3CF );
	outb ( 0x3CF, (misc_value & 0xf1) );  /* ~(2|4|8) */

	/* enable host to write bit plane #2 only */
	outb ( 0x3C4, 0x02 );
	map_mask_value = inb ( 0x3C5 );
	outb ( 0x3C5, 0x04 );

	/* make sure bit mask register value lets all bits go through */
	outb ( 0x3CE, 0x08 );
	bit_mask_value = inb ( 0x3CF );
	outb ( 0x3CF, 0xFF );

	/* make sure that no set/reset planes are set */
	outb ( 0x3CE, 0x01 );
	enable_set_reset_value = inb ( 0x3CF );
	outb ( 0x3CF, 0x00 );

	/* disable the data rotate and functions */
	outb ( 0x3CE, 0x03 );
	data_rotate_value = inb ( 0x3CF );
	outb ( 0x3CF, 0x00 );

#ifdef CANT_DO_ANYMORE
	/* copy fonts into vga memory */
	bcopy(mach_planes + (VGA_DATA_LENGTH * 2), VGA_DATA_AREA, VGA_DATA_LENGTH);
#else
	bcopy(fonts, 0xa0000, FONT_SIZE);
#endif

	/* restore video registers */
	outb ( 0x3CE, 0x01 );
	outb ( 0x3CF, enable_set_reset_value );
	outb ( 0x3CE, 0x03 );
	outb ( 0x3CF, data_rotate_value );
	outb ( 0x3CE, 0x05 );
	outb ( 0x3CF, mode_value );
	outb ( 0x3CE, 0x06 );
	outb ( 0x3CF, misc_value );
	outb ( 0x3CE, 0x08 );
	outb ( 0x3CF, bit_mask_value );
	outb ( 0x3CE, graphic_index );
	outb ( 0x3C4, 0x02 );
	outb ( 0x3C5, map_mask_value );
	outb ( 0x3C4, 0x04 );
	outb ( 0x3C5, memory_mode_value );

	outb (0x3c4, 0 );
	outb (0x3c5, 3 );
	outb ( 0x3C4, sequencer_index );
}

/*
 * Save the video ram 
 */
void save_video_ram(addr)
	u_char * addr;
{
	u_char graphic_index;
	u_char sequencer_index;
	u_char mode_value;
	u_char memory_mode_value;
	u_char read_mask_value;
	u_char misc_value;
	int i;

	/* RESET */
	sequencer_index = inb ( 0x3C4 );
	outb (0x3c4, 0);
	outb (0x3c5, 1);
	outb (0x3c5, 3);
	 
	/* turn on read mode #0 */
	graphic_index = inb ( 0x3CE );
	outb ( 0x3CE, 0x05 );
	mode_value = inb ( 0x3CF );
	Vdebug2((dbg_fd,"mode_value = 0x%x\n", mode_value));
	outb ( 0x3CF, (mode_value & ~0x18));
	
	/* make sure that memory mode register has o/e compliment */
	outb ( 0x3C4, 0x04 );
	memory_mode_value = inb ( 0x3C5 );
	Vdebug2((dbg_fd,"memory_mode_value = 0x%x\n", memory_mode_value));
	outb ( 0x3C5, (memory_mode_value | 0x04) );

	/* turn off chaining */
	/* set mm = 0 --> start of mem is 0xa0000 and length is 128K */
	outb ( 0x3CE, 0x06 );
	misc_value = inb ( 0x3CF );
	outb ( 0x3CF, (misc_value & 0xf1) );  /* ~(2|4|8) */

	/* get original bit plane enabled info */
	outb ( 0x3CE, 0x04 );
	read_mask_value = inb ( 0x3CF );
	Vdebug2((dbg_fd,"read_mask_value = 0x%x\n", read_mask_value));
	
	/* do all 4 bit planes 0-3 */

	/* 
	 * I know that this solution for saving and restoring vga and 
	 * supervga memory is NOT elegant, but I'm really really really
	 * tired of writing SuperVGA code!!!!  If anyone doesn't like
	 * the way it looks, change it.
	 */
	if (addr == (u_char *)dos_planes) {
		dos_planes_mode = *display_mode;
	}

	if (save_supervga_ram && (*display_mode > 0x13)) {
		save_supervga_ram( addr );
	}else{
		/* standard vga only */
		for(i=0; i < MAX_BIT_PLANES; i++) {
			outb ( 0x3CE, 0x04 );
			outb ( 0x3CF, i );
			bcopy(VGA_DATA_AREA, addr, VGA_DATA_LENGTH);
			addr += VGA_DATA_LENGTH;
		}
	}

	/* restore video registers */

	Vdebug2((dbg_fd,"Beginning register restoration\n"));
	outb ( 0x3CE, 0x06 );
	outb ( 0x3CF, misc_value );
	outb ( 0x3CE, 0x05 );
	outb ( 0x3CF, mode_value );
	outb ( 0x3CE, 0x04 );
	outb ( 0x3CF, read_mask_value );
	outb ( 0x3CE, graphic_index );
	outb ( 0x3C4, 0x04 );
	outb ( 0x3C5, memory_mode_value );

	outb (0x3c4, 0);
	outb (0x3c5, 3);
	outb ( 0x3C4, sequencer_index );
	Vdebug2((dbg_fd,"Finished register restoration\n"));

}

/*
 * Restore the video ram
 */
void restore_video_ram(addr)
	u_char * addr;
{
	u_char graphic_index;
	u_char sequencer_index;
	u_char mode_value;
	u_char memory_mode_value;
        u_char clocking_mode_value;
	u_char map_mask_value;
	u_char bit_mask_value;
	u_char enable_set_reset_value;
	u_char data_rotate_value;
	u_char misc_value;
	int i;
	
	/* RESET */
	sequencer_index = inb ( 0x3C4 );
	outb (0x3c4, 0);
	outb (0x3c5, 1);
	outb (0x3c5, 3);

	/* turn on write mode #0 and turn off o/e */
	graphic_index = inb ( 0x3CE );
	outb ( 0x3CE, 0x05 );
	mode_value = inb ( 0x3CF );
	outb ( 0x3CF, (mode_value & ~0x13));
	
	/* make sure that memory mode register has o/e compliment */
	outb ( 0x3C4, 0x04 );
	memory_mode_value = inb ( 0x3C5 );
	outb ( 0x3C5, (memory_mode_value | 0x04) );

	/* turn off display for memory access */
	outb ( 0x3C4, 0x01 );
	clocking_mode_value = inb ( 0x3C5 );
	outb ( 0x3C5, (clocking_mode_value | 0x20) );

	/* turn off chaining */
	/* set mm = 0 --> start of mem is 0xa0000 and length is 128K */
	outb ( 0x3CE, 0x06 );
	misc_value = inb ( 0x3CF );
	outb ( 0x3CF, (misc_value & 0xf1) );  /* ~(2|4|8) */

	/* make sure bit mask register value lets all bits go through */
	outb ( 0x3CE, 0x08 );
	bit_mask_value = inb ( 0x3CF );
	outb ( 0x3CF, 0xFF );

	/* make sure that no set/reset planes are set */
	outb ( 0x3CE, 0x01 );
	enable_set_reset_value = inb ( 0x3CF );
	outb ( 0x3CF, 0x00 );

	/* disable the data rotate and functions */
	outb ( 0x3CE, 0x03 );
	data_rotate_value = inb ( 0x3CF );
	outb ( 0x3CF, 0x00 );

	/* write the display planes one at a time */
	outb ( 0x3C4, 0x02 );
	map_mask_value = inb ( 0x3C5 );

	/*
	 * Only use the supervga restore routine if the mode to restore
	 * coming back into Mach DOS after a pause is a supervga mode.
	 * We assume Mach is always standard text mode, so never use 
	 * supervga restore with mach_planes.  This is a bug (XXX) if
	 * you plan to start this from X (whenever X is made friendly
	 * with the devices.
	 */
	if (restore_supervga_ram && (addr == (u_char *)dos_planes) && 
	    (dos_planes_mode > 0x13)) {
		restore_supervga_ram( addr );
	}else{
		/* standard vga only */
		for (i=0; i < MAX_BIT_PLANES; i++) {
			outb ( 0x3C4, 0x02 );
			outb ( 0x3C5, (1<<i) );
			bcopy(addr, VGA_DATA_AREA, VGA_DATA_LENGTH);
			addr += VGA_DATA_LENGTH;
		}
	}

	/* restore video registers */
	outb ( 0x3CE, 0x01 );
	outb ( 0x3CF, enable_set_reset_value );
	outb ( 0x3CE, 0x03 );
	outb ( 0x3CF, data_rotate_value );
	outb ( 0x3CE, 0x05 );
	outb ( 0x3CF, mode_value );
	outb ( 0x3CE, 0x06 );
	outb ( 0x3CF, misc_value );
	outb ( 0x3CE, 0x08 );
	outb ( 0x3CF, bit_mask_value );
	outb ( 0x3CE, graphic_index );
	outb ( 0x3C4, 0x02 );
	outb ( 0x3C5, map_mask_value );
	outb ( 0x3C4, 0x04 );
	outb ( 0x3C5, memory_mode_value );
	outb ( 0x3C4, 0x01 );
	outb ( 0x3C5, clocking_mode_value );

	outb (0x3c4, 0);
	outb (0x3c5, 3);
	outb ( 0x3C4, sequencer_index );
}

/* 
 * Take the vga in and out of the 'safe' state.
 */
boolean_t bios_vga_fn(state)
	onoff_t state;
{
	int i;

	switch (state) {
	    case VGA_ENABLED:
		if (vga_state != VGA_DISABLED) {
			Debug1((dbg_fd,"\rmon: vga enable while enabled.\n"));
			return(FALSE);
		}
		for (i = 0; vga_ports[i] != 0x00 ; i++)
			PORT_ENABLE(vga_ports[i]);
		restore_vga_state(&hardware_vga_state, FALSE, NULL);
		vga_state = VGA_ENABLED;
		break;
	    case VGA_DISABLED:
		if (vga_state != VGA_ENABLED) {
			Debug1((dbg_fd,"\rmon: vga disable while dis.\n"));
			return(FALSE);
		}
		save_vga_state(&hardware_vga_state);
		restore_vga_state(&before_dos_state, FALSE, NULL);
#ifdef	TSENG4000
		restore_vga_state(&before_dos_state, FALSE, NULL);
#endif	TSENG4000
		vga_state = VGA_DISABLED;
		break;
	    default:
		Debug1((dbg_fd,"\rmon: Bad Vga Command.\n"));
		return(FALSE);
		break;
	}
	return(TRUE);
}

fix_colors() 
{
	u_char Target_Reg;
	struct vga_control_register_state crstate;
	struct vga_control_register_state * VS = &crstate;

	/* Initialize Flip Flop */
	VS->Input_Status_1 = inb( 0x3da );
	/* Save Attribute Registers */
	VS->attr_index = inb( 0x3c0 );

	VS->Input_Status_1 = inb( 0x3da );
	outb( 0x3C0, 0x13) ;
	outb( 0x3c0, 0);
	VS->Input_Status_1 = inb( 0x3da );
	outb ( 0x3c0, VS->attr_index );
}



/*
 *
 * Save the initial state of the vga card so that upon exit of the
 * monitor, we can make sure that we go back into text mode.  
 */
void bios_vga_init()
{
	int i,j;
	char buf[1024];
	int file_fd;
	FILE * file;

	/*
	 * Figure out what kind of supervga we have.
	 */
	bios_supervga_init();

	/* set up permanent vars */
	clstate0.colors_used = 256;
	clstate0.color_array = color_palette0;
	clstate0.load_color_regs = TRUE;

	before_dos_state.control_registers = &crstate0;
	before_dos_state.color_registers = &clstate0;

	clstate1.colors_used = 256;
	clstate1.color_array = color_palette1;
	clstate1.load_color_regs = TRUE;

	hardware_vga_state.control_registers = &crstate1;
	hardware_vga_state.color_registers = &clstate1;

#ifdef	RUNTIME_FILES
	default_color_registers.load_color_regs = TRUE;
	default_color_registers.colors_used = 256;
	default_color_registers.color_array = color_palette0;
#endif	RUNTIME_FILES

	if(save_supervga_state) {
		supervga_state_init(&before_dos_state);
		supervga_state_init(&hardware_vga_state);
	}

	save_vga_state(&before_dos_state);
	save_vga_state(&hardware_vga_state);


	/*
	 * enable ports 
	 */
	bios_vga_fn(VGA_ENABLED);

	/*
	 * Figure out what kind of VGA we have.
	 */
	MACH_CALL((vm_allocate(mach_task_self(), &mach_planes, (vm_size_t)0x80000, 
		TRUE)), "vm_allocate of video save address space.");

	MACH_CALL((vm_allocate(mach_task_self(), &dos_planes, (vm_size_t)0x80000, 
		TRUE)), "vm_allocate of video save address space.");

	save_video_ram(mach_planes);

#ifdef	RUNTIME_FILES
	/*
	 * Load the Vga information into memory.
	 */
#ifdef	OSF_SVR

	strcpy((char *)osfdir+prefix_len, "vga.info");
	if ((file = fopen(osfdir, "r")) == NULL) {
		exit_index = 1;
		exit_dos();
	}else{
	        Debug0((dbg_fd,"found vga.info in %s\n",osfdir));
	}
#else	/* BSD 4.3 UX Server */	

	if ((file = fopenp(lpath, "vga.info", buf, "r")) == NULL) {
		exit_index = 1;
		exit_dos();
	}else{
	        Debug0((dbg_fd,"found vga.info in %s\n",buf));
	}
#endif	OSF_SVR

	fscanf(file, "%s\n", buf);
	if (strncmp(buf, "sv20", 4) != 0) {
		exit_index = 2;
		exit_dos();
	}

	for (i = 0; i < 6; i++) {
		fscanf(file, "%x\n", &vgafont_info[i]);
	}

	for (i = 0; i < 0x14; i++) {
		/* read in the mode */
		fscanf(file, "%*s %*s %*s %*s\n");
		
		fscanf(file, "%x\n", &(vga_init[i].Misc_Output_Reg));
		fscanf(file, "%x\n", &(vga_init[i].Input_Status_0));
		fscanf(file, "%x\n", &(vga_init[i].Input_Status_1));
		fscanf(file, "%x\n", &(vga_init[i].Feature_Control));
		fscanf(file, "%x\n", &(vga_init[i].Video_Enable));

		fscanf(file, "%x\n", &(vga_init[i].attr_index));
		for (j = 0; j < ATTR_REG_COUNT; j++) {
			fscanf(file, "%x\n", &(vga_init[i].attr_regs[j]));
		}

		fscanf(file, "%x\n", &(vga_init[i].crtc_index));
		for (j = 0; j < CRTC_REG_COUNT; j++) {
			fscanf(file, "%x\n", &(vga_init[i].crtc_regs[j]));
		}

		fscanf(file, "%x\n", &(vga_init[i].seq_index));
		for (j = 0; j < SEQ_REG_COUNT; j++) {
			fscanf(file, "%x\n", &(vga_init[i].seq_regs[j]));
		}

		fscanf(file, "%x\n", &(vga_init[i].graf_index));
		for (j = 0; j < GRAF_REG_COUNT; j++) {
			fscanf(file, "%x\n", &(vga_init[i].graf_regs[j]));
		}

		for (j = 0; j < 0x100; j++) {
			fscanf(file, "%x %x %x\n", &(vga_color_arrays[i][j][0]),
			       &(vga_color_arrays[i][j][1]),
			       &(vga_color_arrays[i][j][2]));
		}
	}

	fclose(file);

	/*
	 * Load the fonts into memory from a file.  This is necessary
	 * since might now be started from a state with intact fonts.
	 */
#ifdef	OSF_SVR
	strcpy((char *)osfdir+prefix_len, "font.info.vga");
	if ((file_fd = open(osfdir, O_RDONLY, 0)) < 0) {
		exit_index = 3;
		exit_dos();
	}else{
		Debug0((dbg_fd,"found font.info.vga in %s\n",osfdir));
	}

#else	/* BSD 4.3 UX Server */

	if ((file_fd = openp(lpath, "font.info.vga", buf, O_RDONLY, 0)) < 0) {
		exit_index = 3;
		exit_dos();
	}else{
		Debug0((dbg_fd,"found font.info.vga in %s\n",buf));
	}
#endif	OSF_SVR

	if ((i = read(file_fd, fonts, FONT_SIZE)) != FONT_SIZE) {
		exit_index = 4;
		exit_dos();
	}
	close(file_fd);
#endif	RUNTIME_FILES
}

#ifdef	RUNTIME_FILES
#else
clear_vga_text()
{
	u_short * addr;
	
	addr = (u_short *)(0xb8000 + 0xfa0);
	while ((u_short *)0xb8000 <= addr) {
		/* black white space */
		*addr = 0x0720;
		addr--;
	}
}
#endif	RUNTIME_FILES
