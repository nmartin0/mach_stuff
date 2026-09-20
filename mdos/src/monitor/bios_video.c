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
 *	V86 BIOS video emulation
 *
 * HISTORY: 
 * $Log:	bios_video.c,v $
 * Revision 2.6  92/05/22  15:59:31  grm
 * 	Removed some extraneous code.  Totally removed all max_rc usgae,
 * 	and replaced with direct access to SPRAM.
 * 	[92/05/21            grm]
 * 
 * Revision 2.5  92/04/14  13:20:26  grm
 * 	Started to clean up the old VGA structure dependencies.  Not
 * 	finished in this release.
 * 	[92/04/14            grm]
 * 	VGA BIOS Emulation code was removed and VGA BIOS Redirection put
 * 	in for all calls except for video mode 3 (tty) related calls.
 * 	[92/03/27            grm]
 * 
 * Revision 2.4  92/03/02  15:47:32  grm
 * 	Moved the check for a valid video mode after the masking of the
 * 	screen clear bit (0x80) in set_video_mode().
 * 	[92/02/27            grm]
 * 
 * Revision 2.3  91/12/06  15:28:56  grm
 * 	Replaced the absolute mon_space code with the relative dosres
 * 	code.
 * 	[91/12/06            grm]
 * 
 * Revision 2.2  91/12/05  16:41:30  grm
 * 	Added cursor box's width and height to mode_info 
 * 	structure.  Put in character_height support so that
 * 	the fonts in graphics mode would be correct.
 * 	Fixed the set_cursor_type for CGA emulation.  Turn
 * 	off hardware cursor correctly now too.
 * 	[91/08/09  19:49:20  grm]
 * 
 * 	Added EGA mouse support (although it is disabled).
 * 	Changed toggle blink to keep PAS on during reset.
 * 	[91/06/28  18:22:41  grm]
 * 
 * 	Changed by rfr to make easier state transitions.
 * 	[91/06/14  11:55:29  grm]
 * 
 * 	New Copyright.  Lots of changes to video code.
 * 	Rewrote the tty output and added on the |0x20
 * 	to the attribute index.
 * 	[91/05/28  15:03:34  grm]
 * 
 * 	Several small changes.
 * 	[91/03/26  19:23:06  grm]
 * 
 * 	Mach3 version.
 * 	[91/02/01  13:28:34  grm]
 * 
 * 	Added color register ops.
 * 	[90/11/09  21:06:22  grm]
 * 
 * 	Added VCLR ops for palette and border ops.
 * 	Changed from Debug to Vdebug.  Set_display_mode
 * 	works.
 * 	[90/11/05  19:29:31  grm]
 * 
 * 	Rewritten set_display_mode middle.
 * 	Checked in for kicks.
 * 	[90/10/18  17:59:05  grm]
 * 
 * 	Moved defines and structs to bios_video.h.
 * 	Removed curses support.  Added direct VGA
 * 	memory support for alpha modes.  Rewrote
 * 	every bios routine from 0 <= ah <= 0x0d.
 * 	Added UNCHANGED return value for no change
 * 	to carry flag.
 * 	[90/10/04  21:03:35  grm]
 * 
 * 	Checkin before major overhall.  Minor
 * 	changes and additions.
 * 	[90/08/28  15:57:02  grm]
 * 
 * 	Fixed curses tty output.
 * 	[90/07/31  17:11:22  grm]
 * 
 * 	Added some curses support.  Doesn't really work well yet.
 * 	[90/07/25  18:36:33  grm]
 * 
 * 	minor changes.
 * 	[90/04/30  15:38:27  grm]
 * 
 * 	Added many constants.  Added video_mode_settings array and
 * 	support.  Fixed set_video_mode to really set any valid vga
 * 	video mode.  Added "advanced_video" support.  Changed fprintfs
 * 	to use dbg_fd instead of stderr.
 * 	[90/04/17  22:38:50  grm]
 * 
 * 	Created. Moved from bios_misc.c and added
 * 	the vga state information.
 * 	[90/04/07  00:33:32  grm]
 * 
 */


#include "base.h"
#include "bios.h"
#include "bios_video.h"
#include "bios_misc.h"

#include <ctype.h>

#define Display_type	0
#define Vga_memory	0x03
#define Feature_bits	0x00
#define Switch_settings	0x09

#define VGA_COLOR	0x08
#define CGA_COLOR	0x02

#define MAX_VIDEO_MODE	0x13

struct video_mode_entry mode_info[0x20] = {
	/* mode 0 */
	{ 0xb8000, 8, 0x800, 0x7d0 , 319, 199, 8, 8},
	{ 0xb8000, 8, 0x800, 0x7d0 , 319, 199, 8, 8},
	{ 0xb8000, 8, 0x1000, 0xfa0, 639, 199, 8, 8},
	{ 0xb8000, 8, 0x1000, 0xfa0, 639, 199, 8, 8},
	{ 0xb8000, 1, 0, 0, 319, 200, 8, 8},
	{ 0xb8000, 1, 0, 0, 319, 200, 8, 8},
	{ 0xb8000, 1, 0, 0, 639, 200, 8, 8},
	{ 0xb0000, 8, 0x1000, 0xfa0, 719, 349, 9, 14},	
	{ 0,0,0,0, 0, 0, 0, 0},
	{ 0,0,0,0, 0, 0, 0, 0},
	{ 0,0,0,0, 0, 0, 0, 0},
	{ 0,0,0,0, 0, 0, 0, 0},
	{ 0,0,0,0, 0, 0, 0, 0},
	{ 0xa0000, 8, 0x2000, 0, 319, 199, 8, 8},
	{ 0xa0000, 4, 0x4000, 0, 639, 199, 8, 8},
	{ 0xa0000, 2, 0, 0, 639, 349, 8, 14},
/*	{ 0xa0000, 2, 0x8000, 0, 319, 349 }, */
	{ 0xa0000, 2, 0x8000, 0, 639, 349, 8, 14},
	{ 0xa0000, 1, 0, 0, 639, 479, 8, 16},
	{ 0xa0000, 1, 0, 0, 639, 479, 8, 16},
/*	{ 0xa0000, 1, 0, 0, 319, 199 } }; */
	{ 0xa0000, 1, 0, 0, 639, 199, 8, 8} };

struct cursor_position_t *cursor_positions = (struct cursor_position_t *)0x450;

/* 
 * The BIOS RAM's variables:
 */
u_char * display_mode = Current_video_mode;
u_short * number_of_columns = Number_of_columns;
u_short * length_of_regen = Length_of_screen_buffer;
u_short * start_of_page = Start_of_current_page;
u_short * cursor_type = Cursor_mode;
u_char * current_page = Current_display_page;
u_short * base_port_addr = Base_address_host_port;
u_char * current_mode = Current_mode;
u_char * current_color = Current_color;
u_char * number_of_rows = Number_of_rows;
u_short * character_height = Character_height;

u_long * int43_ptr = (u_long *)0x10c;

boolean_t last_mode_cleared_p= TRUE;

#ifdef	RUNTIME_FILES
extern u_char vga_color_arrays[0x14][0x100][3];
extern u_long vgafont_info[];
#endif	RUNTIME_FILES

void bios_video_init()
{
	struct vga_state new_state;
	u_short * addr;
	int i;

	if (!vga_state)
		return;

	/*
	 * XXX This procedure assumes that you are
	 *   starting from mode 3
	 */

#ifdef	RUNTIME_FILES
	/* Set the console up in vga mode 3 */

	new_state.control_registers = &vga_init[0x3];
	new_state.color_registers = &default_color_registers;
	restore_vga_state(&new_state, TRUE, 3);

	restore_vga_fonts();
#else
	clear_vga_text();
#endif	RUNTIME_FILES
	
	*display_mode = 3;
	*number_of_columns = 80;
	*length_of_regen = mode_info[3].offset;
#ifdef WRONG
	*start_of_page = (mode_info[3].offset * 0)>>1;
#else
	*start_of_page = 0;
#endif WRONG
	for (i = 0; i < 8; i++) {
		cursor_positions[i].row = 0;
		cursor_positions[i].column = 0;
	}
	*cursor_type = 0x0d0e;
	*current_page = 0;
	*base_port_addr = 0x3d4;
	*number_of_rows = 24;
}

void bios_video_exit()
{
	/* none */
}


/*
 * Manually scroll the vga memory page.
 * Forget curses bullshit!
 */
void tty_scroll(page)
	int page;
{
	int i;
	int rows;
	int columns;
	int line;
	u_long *addr;
	u_short *ptr;
	u_short value;
	u_char * attrp;
	u_char attr;

	if ((*display_mode > 3) && (*display_mode != 7)) {
		fprintf(dbg_fd,"\rbios_video: vga_scroll of graphics mode 0x%x\n",
			*display_mode);
		return;
	}
	addr = (u_long *)(mode_info[*display_mode].start + 
			  mode_info[*display_mode].offset * page);

	attrp = (u_char *)((int)addr + mode_info[*display_mode].size - 1);
	attr = *attrp;

	rows = *number_of_rows;
	line = *number_of_columns * 2;
	columns = line / sizeof(u_long);
	
/*
	for(;rows > 0; rows--) {
		for(i = 0; i < columns; i++) {
			*addr = *(u_long *)((int)addr + line);
			addr++;
		}
	}
*/
	bcopy((char *)((int)addr+line), (char *)addr, rows*columns*4);
	addr += rows*columns;
			
	ptr = (u_short *)addr;
	value = ((attr << 8) | 0x20);
	for(i = 0; i < *number_of_columns; i++)
		*(ptr++) = value;
	
}

/*
 * Place character bitmap directly into vga display memory.
 * Used in graphics modes not in alpha modes.
 */
void place_graphic_char(page,row,column,ch)
	u_char page;
	u_char row;
	u_char column;
	u_char ch;
{
	/* XXX fill in later */
}

/* 
 * Update the vga registers to move the cursor to a certain
 * row and column on the current display page.
 */
void move_cursor(row, column)
	u_char row;
	u_char column;
{
	u_short pos;

	pos = row * *number_of_columns + column;
	outb( CRT_INDEX, CSR_HIGH );
	outb( CRT_DATA, (u_char)(pos>>8) );
	outb( CRT_INDEX, CSR_LOW );
	outb( CRT_DATA, (u_char)(pos&0xff) );
}

/*
 * Do tty output on known 'good' character.  Not special char.
 */
void put_tty_char(ch)
	u_char ch;
{
	u_char crow;
	u_char ccol;
	u_char * addr;

	crow = cursor_positions[*current_page].row;
	ccol = cursor_positions[*current_page].column;
	
	if ((ccol + 1) < *number_of_columns) {
		addr = (u_char *)(mode_info[*display_mode].start+
				  mode_info[*display_mode].offset * 
				  *current_page +
				  (*number_of_columns *
				   crow + ccol) * 2);
		*addr = ch;
		ccol++;
		cursor_positions[*current_page].column = ccol;
		move_cursor(crow, ccol);
	}else if (crow < *number_of_rows) {
		addr = (u_char *)(mode_info[*display_mode].start+
				  mode_info[*display_mode].offset * 
				  *current_page +
				  (*number_of_columns *
				   crow + ccol) * 2);
		*addr = ch;
		crow++;
		cursor_positions[*current_page].row = crow;
		cursor_positions[*current_page].column = 0;
		move_cursor(crow, 0);
	}else{
		addr = (u_char *)(mode_info[*display_mode].start+
				  mode_info[*display_mode].offset * 
				  *current_page +
				  (*number_of_columns *
				   crow + ccol) * 2);
		*addr = ch;
		tty_scroll(*current_page);
		crow = *number_of_rows;
		ccol = 0;
		cursor_positions[*current_page].row = crow;
		cursor_positions[*current_page].column = ccol;
		move_cursor(crow, ccol);
	}
}

/*
 * Do generic tty output.
 */
int do_tty_output(buff, fore_color, count)
	u_char * buff;
	u_char fore_color;
	int count;
{
	u_char crow;
	u_char ccol;
	int i;
	u_char ch;

	Vdebug2((dbg_fd,"\rbios_video: do_tty_out, %c %d\n",buff[0],count));
	
	if (*display_mode != 3) {
		Vdebug2((dbg_fd,"\rbios_video: tty out, not mode 3\n"));
		return(REDIRECT);
	}
	
	for (i=0;i<count;i++) {
		ch = buff[i];
		
		crow = cursor_positions[*current_page].row;
		ccol = cursor_positions[*current_page].column;
		
		/*
		 * Alpha mode switch
		 */
		switch (ch) {
		    case BEL: {
			    /* 
			      beep();
			      */
			    break;
		    }
		    case BS: {
			    if (ccol > 0) {
				    cursor_positions[*current_page].column--;
				    move_cursor(crow,(ccol - 1));
			    }
			    break;
		    }
		    case LF: {
			    if (crow < *number_of_rows) {
				    cursor_positions[*current_page].row++;
				    move_cursor((crow + 1), ccol);
			    }else{
				    tty_scroll(*current_page);
			    }
			    /* so unix files will look right when 'typed' XXX */
			    cursor_positions[*current_page].column = 0;
			    move_cursor(cursor_positions[*current_page].row, 0);
			    break;
		    }
		    case CR: {
			    cursor_positions[*current_page].column = 0;
			    move_cursor(crow, 0);
			    break;
		    }
		    case TAB: {
			    put_tty_char(' ');
			    
			    while((cursor_positions[*current_page].column
				   % 8) != 0)
				    put_tty_char(' ');
			    break;
		    }
		    default: {
			    put_tty_char(ch);
			    break;
		    }
		}
	}
	return(UNCHANGED);
}

boolean_t bios_video_fn (state)
	state_t *state;
{
	if (*display_mode != 3) return(REDIRECT);

	switch (HIGH(state->eax)) {
	    case VIDEO_SCROLL_WIN_UP: {
		    u_char lines = LOW(state->eax);
		    u_char attrib = HIGH(state->ebx);
		    u_char ul_y = HIGH(state->ecx);
		    u_char ul_x = LOW(state->ecx);
		    u_char lr_y = HIGH(state->edx);
		    u_char lr_x = LOW(state->edx);
		    int dx;
		    int dy;
		    int i,j;
		    u_char * to;
		    u_char * from;
		    u_char * tmp1;
		    u_char * tmp2;
		    u_char * end;
		    int line_wrap;

		    Vdebug1((dbg_fd,"\rbios_video: scroll win up, %d\n",lines));

		    dx = lr_x - ul_x;
		    dy = lr_y - ul_y;

		    /* check boundary conditions & vga_state */
		    if ((dx <= 0) || (dy <= 0) || (!vga_state))
			    return(UNCHANGED);

		    line_wrap = *number_of_columns * 2;

		    to = (u_char *)mode_info[*display_mode].start + 
			    (mode_info[*display_mode].offset * 
			     *current_page);
		    
		    to += ((ul_y * *number_of_columns + ul_x) * 2);
		    from = to + (lines * line_wrap);
		    end = to + ((dy+1) * line_wrap);

		    /* Check and see if scrolling is needed. */
		    if ((lines != 0) && (lines <= dy)) {
			    for (i = (dy + 1 - lines);i > 0; i--) {
				    tmp1 = to;
				    tmp2 = from;
				    for (j = 0; j <= dx;j++) {
					    *(tmp1++) = *(tmp2++); /* char */
					    *(tmp1++) = *(tmp2++); /* attr */
				    }
				    to += line_wrap;
				    from += line_wrap;
			    }
		    }
		    
		    /* fill in rest with blanks and attribute */
		    while (to < end) {
			    tmp1 = to;
			    for (j = 0; j <= dx; j++) {
				    *(tmp1++) = SPACE;
				    *(tmp1++) = attrib;
			    }
			    to += line_wrap;
		    }

		    return(UNCHANGED);
	    }
	    case VIDEO_SCROLL_WIN_DOWN: {
		    u_char lines = LOW(state->eax);
		    u_char attrib = HIGH(state->ebx);
		    u_char ul_y = HIGH(state->ecx);
		    u_char ul_x = LOW(state->ecx);
		    u_char lr_y = HIGH(state->edx);
		    u_char lr_x = LOW(state->edx);
		    int dx;
		    int dy;
		    int i,j;
		    u_char * to;
		    u_char * from;
		    u_char * tmp1;
		    u_char * tmp2;
		    u_char * end;
		    int line_wrap;

		    Vdebug1((dbg_fd,"\rbios_video: scroll win down, %d\n",lines));

		    dx = lr_x - ul_x;
		    dy = lr_y - ul_y;

		    /* check boundary conditions & vga_state */
		    if ((dx <= 0) || (dy <= 0) || (!vga_state))
			    return(UNCHANGED);

		    line_wrap = *number_of_columns * 2;

		    to = (u_char *)(mode_info[*display_mode].start + 
				    (mode_info[*display_mode].offset * 
				     *current_page));
		    
		    to += ((lr_y * *number_of_columns + ul_x) * 2);
		    from = to - (lines * line_wrap);
		    end = to - ((dy+1) * line_wrap);

		    /* Check and see if scrolling is needed. */
		    if ((lines != 0) && (lines <= dy)) {
			    for (i = (dy + 1 - lines);i > 0; i--) {
				    tmp1 = to;
				    tmp2 = from;
				    for (j = 0; j <= dx;j++) {
					    *(tmp1++) = *(tmp2++); /* char */
					    *(tmp1++) = *(tmp2++); /* attr */
				    }
				    to -= line_wrap;
				    from -= line_wrap;
			    }
		    }
		    
		    /* fill in rest with blanks and attribute */
		    while (to < end) {
			    tmp1 = to;
			    for (j = 0; j <= dx; j++) {
				    *(tmp1++) = SPACE;
				    *(tmp1++) = attrib;
			    }
			    to -= line_wrap;
		    }

		    return(UNCHANGED);
	    }
	    case VIDEO_READ_ATRB_CHAR: {
		    u_char page = HIGH(state->ebx);
		    u_char * ptr;

		    Vdebug1((dbg_fd,"\rbios_video: read atrb char, %d\n",page));

		    /* Do different things for alpha and graphics modes. */

		    if ((*display_mode < 4) || (*display_mode == 7)) {
			    /*
			     * Alpha mode 
			     */
			    ptr = (u_char *)(mode_info[*display_mode].start + 
				   mode_info[*display_mode].offset * page +
				   (*number_of_columns * 
				    cursor_positions[page].row + 
				    cursor_positions[page].column) * 2);

			    /* al = char */
			    SETLOW(&(state->eax), *(ptr++));
			    /* ah = attribute */
			    SETHIGH(&(state->eax), *(ptr++));

			    return(UNCHANGED);
		    }else{
			    /* 
			     * Graphics mode
			     */
			    /* XXX fill in later */
			    return (REDIRECT);
		    }

		    return(UNCHANGED);
	    }
	    case VIDEO_WRITE_ATRB_CHAR: {
		    u_char page = HIGH(state->ebx);
		    u_char ch = LOW(state->eax);
		    u_char attr = LOW(state->ebx);
		    u_short count = WORD(state->ecx);
		    u_char * ptr;
		    u_char * end;

		    Vdebug1((dbg_fd,"\rbios_video: write atrb char, %d\n",page));

		    /* Do different things for Alpha and Graphics modes. */

		    if ((*display_mode < 4) || (*display_mode == 7)) {
			    /*
			     * Alpha mode.
			     */
			    ptr = (u_char *)(mode_info[*display_mode].start + 
				   mode_info[*display_mode].offset * page +
				   (*number_of_columns * 
				    cursor_positions[page].row + 
				    cursor_positions[page].column) * 2);

			    end = (u_char *)(mode_info[*display_mode].start +
					     mode_info[*display_mode].offset * page +
					     mode_info[*display_mode].size);
			    
			    while ((count-- > 0) && (ptr < end)) {
				    *(ptr++) = ch;
				    *(ptr++) = attr;
			    }
		    }else{
			    /* 
			     * Graphics mode
			     */
			    /* XXX fill in later */
			    return (REDIRECT);
		    }

		    return(UNCHANGED);
	    }
	    case VIDEO_WRITE_CHAR_CURSOR: {
		    u_char page = HIGH(state->ebx);
		    u_char ch = LOW(state->eax);
		    u_char attr = LOW(state->ebx);
		    u_short count = WORD(state->ecx);
		    u_char * ptr;
		    u_char * end;

		    Vdebug1((dbg_fd,"\rbios_video: write char cursor, %d\n",page));


		    /* Do different things for Alpha and Graphics modes. */

		    if ((*display_mode < 4) || (*display_mode == 7)) {
			    /*
			     * Alpha mode.
			     */
			    ptr = (u_char *)(mode_info[*display_mode].start + 
				   mode_info[*display_mode].offset * page +
				   (*number_of_columns * 
				    cursor_positions[page].row + 
				    cursor_positions[page].column) * 2);

			    end = (u_char *)(mode_info[*display_mode].start +
					     mode_info[*display_mode].offset * page +
					     mode_info[*display_mode].size);
			    
			    while ((count-- > 0) && (ptr < end)) {
				    *(ptr++) = ch;
				    ptr++;
			    }
		    }else{
			    /* 
			     * Graphics mode
			     */
			    /* XXX fill in later */
			    return (REDIRECT);
		    }

		    return(UNCHANGED);
	    }
	    case VIDEO_TTY_OUT: {
		    u_char ch = LOW(state->eax);
		    u_char fore_color = LOW(state->ebx);
		    int ret;

		    if(!vga_state) {
			    printf("%c",ch);
			    return(UNCHANGED);
		    }

		    /* modify for graphics characters later. XXX */
		    if ((*display_mode > 3) && (*display_mode != 7)) {
			    Vdebug2((dbg_fd,"\rbios_video: tty out, graphics mode\n"));
			    return (REDIRECT);
			    return(UNCHANGED);
		    }
	

		    ret = do_tty_output(&ch,fore_color, 1);

		    return(ret);
	    }
	    default:
		return(REDIRECT);
	}
	return (TRUE);
}	
