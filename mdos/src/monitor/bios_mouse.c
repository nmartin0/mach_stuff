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
 *	Mach DOS mouse driver support
 *
 * HISTORY: 
 * $Log:	bios_mouse.c,v $
 * Revision 2.8  92/07/01  14:24:53  grm
 * 	Replaced kev_type declarations to be int as in bios_kbd.c.
 * 	[92/06/03            grm]
 * 
 * Revision 2.7  92/04/14  13:20:15  grm
 * 	Started removing old vga structure dependencies.  This is not
 * 	finished yet.
 * 	[92/04/14            grm]
 * 
 * 	 Changed device interface back to using /dev/iopl for release.
 * 	[92/04/06            grm]
 * 	Mach3 Version.  Devicemap instead of open /dev/iopl.  This will
 * 	revert and then change back.  Archive only.
 * 	[92/03/27            grm]
 * 
 * Revision 2.6  92/03/02  15:47:25  grm
 * 	Moved the interrupt table index declaration to bios_misc.h.
 * 	[92/02/27            grm]
 * 	Removed the sleep and replaced it with a millisecond_wait in
 * 	mouse_loop.
 * 	[92/02/18            grm]
 * 
 * Revision 2.5  92/02/03  14:24:47  rvb
 * 	Clean Up
 * 
 * Revision 2.4  92/02/02  23:02:30  rvb
 * 	Changed kd.h inclusion for first alpha release.
 * 	[92/02/02  14:51:21  grm]
 * 
 * 	Replaced the exit(0)'s with exit_dos()'s.  Made the mouse loop wait
 * 	until the vectors have been redirected before accepting input.
 * 	[92/01/27            grm]
 * 
 * Revision 2.3  91/12/06  15:28:44  grm
 * 	Replaced the absolute mon_space code with the relative dosres
 * 	code.
 * 	[91/12/06            grm]
 * 
 * Revision 2.2  91/12/05  16:40:59  grm
 * 	Fixed the read_dev_mouse routine so that mouse thread will
 * 	timeout on a read and will reset itself after mdos has been
 * 	paused.
 * 	[91/12/04            grm]
 * 	Changed mouse ranges to depend on the mode's limits.
 * 	[91/08/09  19:44:23  grm]
 * 
 * 	Corrected the MICKEYS code.
 * 	[91/07/16  17:45:39  grm]
 * 
 * 	Added Mouse pointer support.  Rewrote some routines.
 * 	[91/06/28  18:15:14  grm]
 * 
 * 	Modified to use dbg's v86 kernel support.
 * 	[91/06/14  11:53:17  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  14:59:01  grm]
 * 
 * 	Rick's new self adjusting timer changes.
 * 	Started adding advanced pausing feature.
 * 	[91/04/30  13:35:01  grm]
 * 
 * 	Works in MS Windows 3.0.
 * 	[91/03/26  19:19:48  grm]
 * 
 * 	Created.
 * 	[91/02/01  13:27:31  grm]
 * 
 */

#include "base.h"
#include "bios.h"

#include <sys/file.h>
#include <sys/ioctl.h>
#include "kd.h"

#include <sys/time.h>

#include "bios_video.h"
#include "bios_video_common.h"
#include "bios_misc.h"

/* from bios_video.c */
extern struct video_mode_entry mode_info[];
extern u_char * display_mode;
extern u_char * number_of_rows;
extern u_short * number_of_columns;
extern int vectors_overridden;

/*
 * Contents of eax when INT 33 is called
 * determines the function to be performed.  Function
 * values are listed below:
 */
#define	MOUSE_INIT			0x00
#define	MOUSE_SHOW			0x01
#define MOUSE_HIDE			0x02
#define MOUSE_GET_STATUS		0x03
#define	MOUSE_SET_POS			0x04
#define	MOUSE_GET_BUTTON_PRESS		0x05
#define MOUSE_GET_BUTTON_RELEASE 	0x06
#define	MOUSE_SET_HORIZONTAL_LIMITS	0x07
#define	MOUSE_SET_VERTICAL_LIMITS	0x08
#define MOUSE_SET_GRAPHICS_SHAPE	0x09
#define MOUSE_SET_TEXT_SHAPE		0x0a
#define	MOUSE_READ_MOTION_COUNTERS	0x0b
#define	MOUSE_SET_EVENT_HANDLER		0x0c
#define MOUSE_LIGHT_PEN_ON		0x0d
#define MOUSE_LIGHT_PEN_OFF		0x0e
#define	MOUSE_SET_MICKEYS_TO_PIXELS	0x0f
#define	MOUSE_SET_POINTER_EXCLUSION	0x10
#define	MOUSE_SET_DOUBLE_SPEED_THRESH	0x13
#define	MOUSE_SWAP_USER_DEFINED_HANDLER	0x14
#define MOUSE_GET_SAVE_BUFFER_SIZE	0x15
#define MOUSE_SAVE_STATE		0x16
#define	MOUSE_RESTORE_STATE		0x17
#define MOUSE_SET_ALTERNATE_HANDLER	0x18
#define	MOUSE_GET_ALT_HANDLER_ADDRESS	0x19
#define	MOUSE_SET_SENSITIVITY		0x1a
#define	MOUSE_GET_SENSITIVITY		0x1b
#define	MOUSE_SET_INTERRUPT_RATE	0x1c
#define	MOUSE_SELECT_POINTER_PAGE	0x1d
#define	MOUSE_GET_POINTER_PAGE		0x1e
#define	MOUSE_DISABLE_MOUSE_DRIVER	0x1f
#define	MOUSE_ENABLE_MOUSE_DRIVER	0x20
#define	MOUSE_RESET			0x21
#define	MOUSE_SET_LANGUAGE		0x22
#define MOUSE_GET_LANGUAGE		0x23
#define	MOUSE_GET_INFORMATION		0x24
#define	MOUSE_INTERRUPT_DONE		0xffff


#define	LEFT_BUTTON	0
#define	RIGHT_BUTTON	1
#define	MIDDLE_BUTTON	2

#define DEFAULT_X_MICKS	8
#define DEFAULT_Y_MICKS	16
#define DEFAULT_TEXT_AND	0x77ff
#define DEFAULT_TEXT_XOR	0x7700

#define SEQUENCER_INDEX	0x3c4
#define SEQUENCER_DATA	0x3c5
#define GRAPHICS_INDEX	0x3ce
#define GRAPHICS_DATA	0x3cf

#define MOUSE_MAX_USE_MODE	-1

struct mutex mouse_lock_data = { 0, 0};
mutex_t mouse_lock = &mouse_lock_data;

int	mouse_ptr_display_page = 0;
boolean_t pointer_visible = FALSE;

u_short one_zero_array[] = {
	0x0000, 0x8000, 0xc000, 0xe000,
	0xf000,	0xf800, 0xfc00, 0xfe00 };

u_char	boz[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };

u_short zero_one_array[] = {
	0xffff, 0x7fff, 0x3fff, 0x1fff,
	0x0fff, 0x07ff, 0x03ff, 0x01ff };

u_char	bzo[] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };

u_short	default_bitmap[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			     0xffff, 0xffff, 0xffff, 0xffff,
			     0xffff, 0xffff, 0xffff, 0xffff,
			     0xffff, 0xffff, 0xffff, 0xffff,
			     0xffff, 0xffff, 0xffff, 0xffff };
u_char	*graphics_crsr = (u_char *)default_bitmap;
/* sixteen u_shorts, two to allow for shifting, four bit planes */
u_long bitmap_save[16][4];
int	bitmap_x = 320;
int	bitmap_y = 100;


u_short	text_crsr_and = DEFAULT_TEXT_AND;
u_short	text_crsr_xor = DEFAULT_TEXT_XOR;

int 	last_column = 12;
int 	last_row = 40;
int	last_value = -1;

int	mouse_x = 320;
int	mouse_y = 100;
int	button_status = 0;

int	mouse_x_movement = 0;
int	mouse_y_movement = 0;

int	mouse_min_x = 0;
int	mouse_min_y = 0;
int	mouse_max_x = MOUSE_MAX_USE_MODE;
int	mouse_max_y = MOUSE_MAX_USE_MODE;

int	x_mickeys_to_pixels = DEFAULT_X_MICKS;
int	y_mickeys_to_pixels = DEFAULT_Y_MICKS;

int	left_button_pressed = 0;
int	left_x_when_pressed = 0;
int	left_y_when_pressed = 0;

int	middle_button_pressed = 0;
int	middle_x_when_pressed = 0;
int	middle_y_when_pressed = 0;

int	right_button_pressed = 0;
int	right_x_when_pressed = 0;
int	right_y_when_pressed = 0;


int	left_button_released = 0;
int	left_x_when_released = 0;
int	left_y_when_released = 0;

int	middle_button_released = 0;
int	middle_x_when_released = 0;
int	middle_y_when_released = 0;

int	right_button_released = 0;
int	right_x_when_released = 0;
int	right_y_when_released = 0;

unsigned int mouse_cs = 0;
unsigned int mouse_eip = 0;
unsigned int mouse_mask = 0;
unsigned int mouse_action = 0;

#define	INTERRUPT_ON_MOVEMENT 		0x1
#define	INTERRUPT_ON_LEFT_PRESSED 	0x2
#define	INTERRUPT_ON_RIGHT_PRESSED 	0x8
#define	INTERRUPT_ON_MIDDLE_PRESSED 	0x20
#define	INTERRUPT_ON_LEFT_RELEASED 	0x4
#define	INTERRUPT_ON_RIGHT_RELEASED 	0x10
#define	INTERRUPT_ON_MIDDLE_RELEASED 	0x40

int horiz_sense = 1;
int vert_sense = 2;

boolean_t reset_mouse_dev = FALSE;

void replace_last()
{
	int i,j;
	int bytes_x;
	int bits_x;
	u_char * ptr;
	u_char * ptr2;
	u_char * starting_addr;
	u_short * addr;
	u_long * laddr;
	u_char graphics_index;
	u_char old_mode;
	u_char data[] = { 0x1, 0x2, 0x4, 0x8 };

	if (last_value == -1)
		return;

	if ((*display_mode > 3) && (*display_mode != 7)) {

		bytes_x = bitmap_x/8;
		bits_x = bitmap_x%8;

		starting_addr  = (u_char *)mode_info[*display_mode].start +
			mode_info[*display_mode].offset * mouse_ptr_display_page;
		ptr = starting_addr + 
			((bitmap_y * (*number_of_columns))+ bytes_x);

		laddr = (u_long *)ptr;

		lock_cs();
		MACH_CALL((thread_suspend(v86_thread)), "t_sus  for mouse update");

		graphics_index = inb(GRAPHICS_INDEX);
		outb (GRAPHICS_INDEX, 5);
		old_mode = inb(GRAPHICS_DATA);
		outb (GRAPHICS_DATA, (old_mode & 0xfc) | 0x1);

		ptr2 = (u_char *)(starting_addr +
				  ((*number_of_columns+2) *
				   (mode_info[*display_mode].max_y + 1)));

		Kdebug1((dbg_fd,"restore_pointer: ptr2 = 0x%x\n",ptr2));

		for(i=0; i < 16; i++) {
			*ptr++ = *ptr2++;
			*ptr++ = *ptr2++;
			*ptr++ = *ptr2++;
			ptr += *number_of_columns - 3;
		}

		outb (GRAPHICS_DATA, old_mode);
		outb (GRAPHICS_INDEX, graphics_index);

		MACH_CALL((thread_resume(v86_thread)), 
			  "t_res  for interrupt");

		unlock_cs();

	}else{
		addr = (u_short *)mode_info[*display_mode].start +
			mode_info[*display_mode].offset * mouse_ptr_display_page;
		addr += (((last_row * (*number_of_columns))+last_column));
		*addr = last_value;
	}
}

void update_pointer()
{
	int i,j;
	int index;
	int new_column;
	int new_row;
	int new_x;
	int new_y;
	u_char graphics_index;
	u_char sequencer_index;
	u_short * addr;
	u_char * ptr;
	int bytes_x;
	int bits_x;
	u_char val, tmp1, tmp2;
	u_char left, right, lo_left, lo_right, over;
	u_char old_index;
	u_char old_map;
	u_char old_mode;
	u_char * ptr2;
	u_char * starting_addr;

	if (!pointer_visible)
		return;

	if ((*display_mode > 3) && (*display_mode != 7)) {
		new_x = mouse_x;
		new_y = mouse_y;

		if ((new_x == bitmap_x) && (new_y == bitmap_y))
			return;

		replace_last();
		
		bytes_x = new_x/8;
		bits_x = new_x%8;

		Kdebug1((dbg_fd,"update_pointer old x,y = %d, %d, new x,y = %d, %d.\n",
			 bitmap_x, bitmap_y, new_x, new_y));
		Kdebug1((dbg_fd,"update_pointer bytes_x = %d, bits_x = %d\n",
			 bytes_x, bits_x));

		starting_addr = (u_char *)mode_info[*display_mode].start +
			mode_info[*display_mode].offset * mouse_ptr_display_page;
		ptr = starting_addr +
			((new_y * (*number_of_columns))+ bytes_x);

		bitmap_x = new_x;
		bitmap_y = new_y;
		last_value = 1;

		lock_cs();
		MACH_CALL((thread_suspend(v86_thread)), "t_sus  for mouse update");

		graphics_index = inb(GRAPHICS_INDEX);

#ifdef USE_SEQUENCER_REGS
		sequencer_index = inb(SEQUENCER_INDEX);
		outb (SEQUENCER_INDEX, 2);
		old_map = inb(SEQUENCER_DATA);
		outb (SEQUENCER_DATA, 0x0f);
#endif USE_SEQUENCER_REGS

			/* save old data */
		{
			u_char * tmp_ptr = ptr;

			outb (GRAPHICS_INDEX, 5);
			old_mode = inb (GRAPHICS_DATA);
			/* turn on write mode #1 (display mem to display mem) */
			outb (GRAPHICS_DATA, (old_mode & 0xfc) | 0x1);

			ptr2 = (u_char *)(starting_addr +
					  ((*number_of_columns+2) *
					   (mode_info[*display_mode].max_y + 1)));
			Kdebug1((dbg_fd,"update_pointer: ptr2 = 0x%x\n",ptr2));

			for(i=0; i< 16; i++) {
				*ptr2++ = *ptr++;
				*ptr2++ = *ptr++;
				*ptr2++ = *ptr++;
				ptr += *number_of_columns - 3;
			}
			
			outb (GRAPHICS_DATA, (old_mode));
			
			ptr = tmp_ptr;
		}

		/* point index to FC */
		outb (GRAPHICS_INDEX, 3);

		for(i=0; i < 16; i++) {

			/* Do bitmap AND */

			left = graphics_crsr[(i*2)+1];
			right = graphics_crsr[i*2];

			lo_left = left << (8 - bits_x);
			lo_right = right << (8 - bits_x);

			left >>= bits_x;
			right >>= bits_x;

			left |= boz[bits_x];
			right |= lo_left;
			over = lo_right | bzo[bits_x];

			vga_and_write(ptr, left, right, over);

			/* Do bitmap XOR */
			
			left = graphics_crsr[(i*2)+33];
			right = graphics_crsr[(i*2)+32];

			lo_left = left << (8 - bits_x);
			lo_right = right << (8 - bits_x);

			left >>= bits_x;
			right >>= bits_x;

			right |= lo_left;
			over = lo_right;

			vga_xor_write(ptr, left, right, over);

			ptr += (*number_of_columns);
		}

		outb (GRAPHICS_DATA, 0);
		outb (GRAPHICS_INDEX, graphics_index);
#ifdef USE_SEQUENCER_REGS
		outb (SEQUENCER_DATA, old_map);
		outb (SEQUENCER_INDEX, sequencer_index);
#endif USE_SEQUENCER_REGS

		MACH_CALL((thread_resume(v86_thread)), 
			  "t_res  for interrupt");

		unlock_cs();
		
	}else{
		new_column = mouse_x/8;
		new_row = mouse_y/8;
		
		if ((new_column == last_column) && (new_row == last_row))
			return;
		
		replace_last();
		
		addr = (u_short *)mode_info[*display_mode].start +
			mode_info[*display_mode].offset * mouse_ptr_display_page;
		addr += (((new_row * (*number_of_columns))+new_column));
		last_value = *addr;
		last_column = new_column;
		last_row = new_row;
		
		*addr &= text_crsr_and;
		*addr ^= text_crsr_xor;
	}
}

void hide_pointer()
{
	replace_last();
	last_value = -1;
	pointer_visible = FALSE;
}

void show_pointer()
{
	pointer_visible = TRUE;
	update_pointer();
}

struct saved_mouse_record {
	struct saved_mouse_record * next;
	int	saved_mouse_min_x;
	int	saved_mouse_min_y;
	int	saved_mouse_max_x;
	int	saved_mouse_max_y;

	int	saved_x_mickeys_to_pixels;
	int	saved_y_mickeys_to_pixels;
	
	unsigned int saved_mouse_cs;
	unsigned int saved_mouse_eip;
	unsigned int saved_mouse_mask;
} * saved_mouse = NULL;

void 
save_mouse_state()
{
	struct saved_mouse_record * mouse;
	mouse = (struct saved_mouse_record *)
			   malloc(sizeof (struct saved_mouse_record));

	Debug0((dbg_fd,"save_mouse_state\n"));
	mouse->saved_mouse_min_x = mouse_min_x;
	mouse->saved_mouse_min_y = mouse_min_y;
	mouse->saved_mouse_max_x = mouse_max_x;
	mouse->saved_mouse_max_y = mouse_max_y;

	mouse->saved_x_mickeys_to_pixels = x_mickeys_to_pixels;
	mouse->saved_y_mickeys_to_pixels = y_mickeys_to_pixels;

	mouse->saved_mouse_cs = mouse_cs;
	mouse->saved_mouse_eip = mouse_eip;
	mouse->saved_mouse_mask = mouse_mask;
	mouse->next = saved_mouse;
	saved_mouse = mouse;
}

void 
restore_mouse_state()
{
	struct saved_mouse_record * mouse = saved_mouse;
	if (mouse == NULL) {
		Debug0((dbg_fd,"unable to restore_mouse_state\n"));
		return;
	} 
	Debug0((dbg_fd,"restore_mouse_state\n"));
	mouse_min_x = mouse->saved_mouse_min_x;
	mouse_min_y = mouse->saved_mouse_min_y;
	mouse_max_x = mouse->saved_mouse_max_x;
	mouse_max_y = mouse->saved_mouse_max_y;

	x_mickeys_to_pixels = mouse->saved_x_mickeys_to_pixels;
	y_mickeys_to_pixels = mouse->saved_y_mickeys_to_pixels;

	mouse_cs = mouse->saved_mouse_cs;
	mouse_eip = mouse->saved_mouse_eip;
	mouse_mask = mouse->saved_mouse_mask;
	saved_mouse = mouse->next;
	free(mouse);
}

void cause_mouse_intr(main_thread)
boolean_t main_thread;
{
	if ((mouse_cs == 0) && (mouse_eip == 0)) return;
	queue_interrupt(MOUSE_INT_VEC, 1, main_thread);
	check_interrupt(main_thread);
}

/*
 * The call the Int 0xfd makes happen.  This calls the user
 * mouse handler.
 */
boolean_t
do_mouse_interrupt(state)
state_t * state;
{
	u_short * sp;
	extern u_long milliseconds_since_boot;
			
	sp = (u_short *)Addr(state, ss, uesp);
	Kdebug2((dbg_fd, "mouse cs 0x%x eip 0x%x efl\n",
			state->cs,state->eip, state->efl));
	*--sp = WORD(state->es);
	*--sp = WORD(state->ds);
	*--sp = WORD(state->ebp);
	*--sp = WORD(state->esi);
	*--sp = WORD(state->edi);
	*--sp = WORD(state->edx);
	*--sp = WORD(state->ecx);
	*--sp = WORD(state->ebx);
	*--sp = WORD(state->eax);
	*--sp = Segment(MOUSE_HELPER_2);
	*--sp = Offset(MOUSE_HELPER_2);
	state->uesp -= 22;

	SETWORD(&(state->eax),mouse_action);
	SETWORD(&(state->ebx),button_status);
	SETWORD(&(state->ecx),mouse_x);
	SETWORD(&(state->edx),mouse_y);

	SETWORD(&(state->edi),mouse_y_movement);
	SETWORD(&(state->esi),mouse_x_movement);
		
	Kdebug1((dbg_fd, "Calling mouse intr = %x, %x\n",
					mouse_cs, mouse_eip));

	Kdebug2((dbg_fd, "mouse interrupt called at %d\n",
			      		milliseconds_since_boot));
	state->cs = mouse_cs;
	state->eip = mouse_eip;
	MACH_CALL(( thread_set_state ( v86_thread, i386_THREAD_STATE,
				     state, i386_THREAD_STATE_COUNT)),
			         "thread_set_state do_bad_instruction");
	return (LEAVE_EIP_ALONE);
}


void
set_mouse_position(x,y)
int x,y;
{
	int max_x, max_y;
	Kdebug1((dbg_fd, "set_mouse x=%d, y=%d\n", x, y));

	max_x = (mouse_max_x == MOUSE_MAX_USE_MODE ? 
		 mode_info[*display_mode].max_x :
		 mouse_max_x);
	max_y = (mouse_max_y == MOUSE_MAX_USE_MODE ? 
		 mode_info[*display_mode].max_y :
		 mouse_max_y);

	if (x < mouse_min_x) x = mouse_min_x;
	if (x > max_x) x = max_x;
	if (y < mouse_min_y) y = mouse_min_y;
	if (y > max_y) y = max_y;

	mouse_x = x;
	mouse_y = y;

	Kdebug1((dbg_fd, "set_mouse x=%d, y=%d\n", x, y));

	if (pointer_visible)
		update_pointer();
}


void
set_button(button, up)
int button;
boolean_t up;
{
	boolean_t interrupt = FALSE;
	lock_mouse();
	if (up) {
		button_status &= ~(1<<button);
	} else {
		button_status |= (1<<button);
	}
	if (pointer_visible) {
		replace_last();
		last_value = -1;
	}
	switch(button) {
		case LEFT_BUTTON:
			if (!up) {
				left_button_pressed++;
				left_x_when_pressed = mouse_x;
				left_y_when_pressed = mouse_y;
				if (mouse_action = 
				    (mouse_mask&INTERRUPT_ON_LEFT_PRESSED))
					interrupt = TRUE;
			} else {
				left_button_released++;
				left_x_when_released = mouse_x;
				left_y_when_released = mouse_y;
				if (mouse_action = 
				    (mouse_mask&INTERRUPT_ON_LEFT_RELEASED))
					interrupt = TRUE;
			}
			break;			
		case MIDDLE_BUTTON:
			if (!up) {
				middle_button_pressed++;
				middle_x_when_pressed = mouse_x;
				middle_y_when_pressed = mouse_y;
				if (mouse_action = 
				    (mouse_mask&INTERRUPT_ON_MIDDLE_PRESSED))
					interrupt = TRUE;
			} else {
				middle_button_released++;
				middle_y_when_released = mouse_x;
				middle_y_when_released = mouse_y;
				if (mouse_action = 
				    (mouse_mask&INTERRUPT_ON_MIDDLE_RELEASED))
					interrupt = TRUE;
			}
			break;			
		case RIGHT_BUTTON:
			if (!up) {
				right_button_pressed++;
				right_x_when_pressed = mouse_x;
				right_y_when_pressed = mouse_y;
				if (mouse_action = 
				    (mouse_mask&INTERRUPT_ON_RIGHT_PRESSED))
					interrupt = TRUE;
			} else {
				right_button_released++;
				right_x_when_released = mouse_x;
				right_y_when_released = mouse_y;
				if (mouse_action = 
				    (mouse_mask&INTERRUPT_ON_RIGHT_RELEASED))
					interrupt = TRUE;
			}
			break;			
	}
	unlock_mouse();
	if (interrupt) cause_mouse_intr(FALSE);
	com_mouse_event(button_status, 0, 0);
}

void
mouse_delta(dx, dy)
int dx;
int dy;
{
	lock_mouse();
	Kdebug1((dbg_fd, "set_mouse_delta x=%d, y=%d\n", dx, dy));
	set_mouse_position(mouse_x+dx, mouse_y-dy);
	mouse_x_movement += ((dx*8)/x_mickeys_to_pixels);
	mouse_y_movement -= ((dy*8)/y_mickeys_to_pixels);
	unlock_mouse();
	if (mouse_action = 
	    (mouse_mask&INTERRUPT_ON_MOVEMENT)) cause_mouse_intr(FALSE);
	com_mouse_event(button_status, dx, dy);
}

boolean_t bios_mouse_fn(state)
	state_t *state;
{
	boolean_t ret = UNCHANGED;

	lock_mouse();
	
	Kdebug1((dbg_fd,"Mouse driver call: %x\n",WORD(state->eax)));
	switch (WORD(state->eax)) {
	    case MOUSE_INIT:
		if (pointer_visible)
			hide_pointer();

		/* center mouse */
		mouse_x = mode_info[*display_mode].max_x/2;
		mouse_y = mode_info[*display_mode].max_y/2;
		mouse_ptr_display_page = 0;

		/* disable user mouse event handler */
		mouse_cs = 0;
		mouse_eip = 0;
		mouse_mask = 0;

		x_mickeys_to_pixels = DEFAULT_X_MICKS;
		y_mickeys_to_pixels = DEFAULT_Y_MICKS;

		mouse_min_x = 0;
		mouse_max_x = MOUSE_MAX_USE_MODE;
		mouse_min_y = 0;
		mouse_max_y = MOUSE_MAX_USE_MODE;

		graphics_crsr = (u_char *)default_bitmap;

		text_crsr_and = DEFAULT_TEXT_AND;
		text_crsr_xor = DEFAULT_TEXT_XOR;

		SETWORD(&(state->eax),0xffff);
		SETWORD(&(state->ebx),0x2);
		break;
	    case MOUSE_SHOW:
		if (!pointer_visible) {
			show_pointer();
		}
		break;
	    case MOUSE_HIDE:
		if (pointer_visible) {
			hide_pointer();
		}
		break;
	    case MOUSE_GET_STATUS:
		SETWORD(&(state->ebx),button_status);
		SETWORD(&(state->ecx),mouse_x);
		SETWORD(&(state->edx),mouse_y);
		break;
	    case MOUSE_SET_POS:
		set_mouse_position(WORD(state->ecx),WORD(state->edx));
		break;
	    case MOUSE_GET_BUTTON_PRESS:
		SETWORD(&(state->eax),button_status);
		switch (WORD(state->ebx)) {
		    case LEFT_BUTTON:
			SETWORD(&(state->ebx),
				left_button_pressed);
			left_button_pressed = 0;
			SETWORD(&(state->ecx),
				left_x_when_pressed);
			SETWORD(&(state->edx),
				left_y_when_pressed);
			break;
		    case RIGHT_BUTTON:
			SETWORD(&(state->ebx),
				right_button_pressed);
			right_button_pressed = 0;
			SETWORD(&(state->ecx),
				right_x_when_pressed);
			SETWORD(&(state->edx),
				right_y_when_pressed);
			break;
		    case MIDDLE_BUTTON:
			SETWORD(&(state->ebx),
				middle_button_pressed);
			middle_button_pressed = 0;
			SETWORD(&(state->ecx),
				middle_x_when_pressed);
			SETWORD(&(state->edx),
				middle_y_when_pressed);
			break;
		}
		break;
	    case MOUSE_GET_BUTTON_RELEASE:
		SETWORD(&(state->eax),button_status);
		switch (WORD(state->ebx)) {
		    case LEFT_BUTTON:
			SETWORD(&(state->ebx),
				left_button_released);
			left_button_released = 0;
			SETWORD(&(state->ecx),
				left_x_when_released);
			SETWORD(&(state->edx),
				left_y_when_released);
			break;
		    case RIGHT_BUTTON:
			SETWORD(&(state->ebx),
				right_button_released);
			right_button_released = 0;
			SETWORD(&(state->ecx),
				right_x_when_released);
			SETWORD(&(state->edx),
				right_y_when_released);
			break;
		    case MIDDLE_BUTTON:
			SETWORD(&(state->ebx),
				middle_button_released);
			middle_button_released = 0;
			SETWORD(&(state->ecx),
				middle_x_when_released);
			SETWORD(&(state->edx),
				middle_y_when_released);
			break;
		}
		break;
		
	    case MOUSE_SET_HORIZONTAL_LIMITS:
		{ 
			int min_x, max_x;
			min_x = WORD(state->ecx);
			max_x = WORD(state->edx);
			if (min_x > max_x) {
				int tmp;	
				tmp = max_x;
				max_x = min_x;
				min_x = tmp;
			}
			
			mouse_min_x = min_x;
			mouse_max_x = max_x;
			set_mouse_position(mouse_x, mouse_y);
		}		
		break;
	    case MOUSE_SET_VERTICAL_LIMITS:
		{ 
			int min_y, max_y;
			min_y = WORD(state->ecx);
			max_y = WORD(state->edx);
			if (min_y > max_y) {
				int tmp;	
				tmp = max_y;
				max_y = min_y;
				min_y = tmp;
			}
			
			mouse_min_y = min_y;
			mouse_max_y = max_y;
			set_mouse_position(mouse_x, mouse_y);
		}		
		break;
	    case MOUSE_SET_GRAPHICS_SHAPE: {
		    u_char * ptr = (u_char *)Addr(state,es, edx);

		    graphics_crsr = ptr;

		    break;
	    }
	    case MOUSE_SET_TEXT_SHAPE: {
		    u_short type = WORD(state->ebx);

		    if (type != 0) {
			    Kdebug0((dbg_fd,"bios_mouse: hardware cursor! not implemented.\n"));
		    }else{
#ifndef NOTDEF
			    text_crsr_and = WORD(state->ecx);
			    text_crsr_xor = WORD(state->edx);
#endif
		    }
		    break;
	    }
	    case MOUSE_READ_MOTION_COUNTERS:
		SETWORD(&(state->edx),mouse_y_movement);
		SETWORD(&(state->ecx),mouse_x_movement);
		mouse_x_movement = 0;
		mouse_y_movement = 0;
		break;			
	    case MOUSE_SET_EVENT_HANDLER:
		{	int es, dx, mask;
			es = WORD(state->es);
			dx = WORD(state->edx);
			mask = WORD(state->ecx);
			Kdebug1((dbg_fd, "Set mouse intr = %x, %x, %x\n",
				 es, dx, mask));
			mouse_cs = es;
			mouse_eip = dx;
			mouse_mask =mask;
			break;
		}
	    case MOUSE_SET_MICKEYS_TO_PIXELS:
		x_mickeys_to_pixels = WORD(state->ecx);
		y_mickeys_to_pixels = WORD(state->edx);
		break;
	    case MOUSE_LIGHT_PEN_ON:
	    case MOUSE_LIGHT_PEN_OFF:
	    case MOUSE_SET_POINTER_EXCLUSION:
	    case MOUSE_SET_DOUBLE_SPEED_THRESH:
		break;
	    case MOUSE_SWAP_USER_DEFINED_HANDLER:
		{	int es, dx, mask;
			es = WORD(state->es);
			dx = WORD(state->edx);
			mask = WORD(state->ecx);
			SETWORD(&(state->es),mouse_cs);
			SETWORD(&(state->edx),mouse_eip);
			SETWORD(&(state->ecx),mouse_mask);
			mouse_cs = es;
			mouse_eip = dx;
			mouse_mask =mask;
			break;
		}
	    case MOUSE_GET_SAVE_BUFFER_SIZE:
		SETWORD(&(state->ebx),0x4);
		break;
	    case MOUSE_SAVE_STATE:
	    case MOUSE_RESTORE_STATE:
	    case MOUSE_SET_ALTERNATE_HANDLER:
	    case MOUSE_GET_ALT_HANDLER_ADDRESS:
	    case MOUSE_SET_SENSITIVITY:
	    case MOUSE_GET_SENSITIVITY:
	    case MOUSE_SET_INTERRUPT_RATE:
	    case MOUSE_SELECT_POINTER_PAGE:
	    case MOUSE_GET_POINTER_PAGE:
	    case MOUSE_DISABLE_MOUSE_DRIVER:
	    case MOUSE_ENABLE_MOUSE_DRIVER:
		SETWORD(&(state->eax),0x0);
		break;
	    case MOUSE_RESET:
		if (pointer_visible)
			hide_pointer();

		/* center mouse */
		mouse_x = mode_info[*display_mode].max_x/2;
		mouse_y = mode_info[*display_mode].max_y/2;
		mouse_ptr_display_page = 0;

		/* disable user mouse event handler */
		mouse_cs = 0;
		mouse_eip = 0;
		mouse_mask = 0;

		x_mickeys_to_pixels = DEFAULT_X_MICKS;
		y_mickeys_to_pixels = DEFAULT_Y_MICKS;

		mouse_min_x = 0;
		mouse_max_x = MOUSE_MAX_USE_MODE;
		mouse_min_y = 0;
		mouse_max_y = MOUSE_MAX_USE_MODE;

		graphics_crsr = (u_char *)default_bitmap;
		text_crsr_and = DEFAULT_TEXT_AND;
		text_crsr_xor = DEFAULT_TEXT_XOR;

		SETWORD(&(state->eax),0xffff);
		SETWORD(&(state->ebx),2);
		break;
	    case MOUSE_SET_LANGUAGE:
		break;
	    case MOUSE_GET_LANGUAGE:
		SETWORD(&(state->ebx),0);
		break;
	    case MOUSE_GET_INFORMATION:
		/* bx = major minor version of 'driver' */
		SETWORD(&(state->ebx),0x600);
		/* ch = serial mouse */
		SETHIGH(&(state->ecx),0x2);
		/* cl = irq 4 */
		SETLOW(&(state->ecx),0x4);
		break;
	}
	
	unlock_mouse();
	return (ret);
}

int mouseFd;
#define MAXMOUSEEVENTS	32
kd_event mevents[MAXMOUSEEVENTS];

void dev_mouse_init()
{
	int mode;

	Kdebug1((dbg_fd,"bios_mouse: dev_mouse_init()\n"));

	if ((mouseFd = open("/dev/mouse",O_RDONLY)) <= 0) {
		fprintf(dbg_fd,"error opening /dev/mouse\n");
		fprintf(dbg_fd,"Shutting mouse thread down.\n");
		while (1)
			sleep(100);
		/* Never get to here. */
		exit(0);
	}
}


int read_dev_mouse()
{
	int nbytes;
	int ncodes;

	int nfound;
	fd_set readfds;
	struct timeval timev;

	Kdebug1((dbg_fd,"bios_mouse: read_dev_mouse()\n"));

	FD_ZERO(&readfds);
	FD_SET(mouseFd, &readfds);	

	timev.tv_sec = 5;
	timev.tv_usec = 0;

	while ((nfound = select(mouseFd+1, &readfds, 0, 0, &timev)) <= 0) {
		if (nfound == 0 && reset_mouse_dev) {
			close(mouseFd);
			dev_mouse_init();
			reset_mouse_dev = FALSE;
		}

		Kdebug1((dbg_fd,"bios_mouse: read_dev_mouse, while interation\n"));

		FD_ZERO(&readfds);
		FD_SET(mouseFd, &readfds);
	};

	nbytes = read (mouseFd, mevents, sizeof(mevents));

	if (nbytes < 0) {
		fprintf(dbg_fd,"\rError reading /dev/mouse.\n");
		fprintf(dbg_fd,"Shutting mouse thread down.\n");
		while (1)
			sleep(100);
		/* Never gets here */
		exit(1);
	}

	ncodes = nbytes/sizeof(kd_event);

	Kdebug1((dbg_fd,"bios_mouse: read_dev_kbd() ncodes = 0x%x\n",ncodes));

	return(ncodes);
}

extern mach_port_t device_server_port;
extern int exit_index;

void mouse_loop()
{
	int klioplfd;
	int num_sc;
	int cur_sc;

	if ((klioplfd = open("/dev/iopl", O_RDWR)) < 0) {
		fprintf(dbg_fd,"\rmon_mouse: Error in iopl fd open. fd = %d\n",klioplfd);
		fprintf(dbg_fd,"Shutting mouse thread down.\n");
		while (1)
			sleep(100);
		/* Never gets here */
		exit(1);
	}

	Kdebug1((dbg_fd,"bios_mouse:  mouse_loop() before vector check\n"));

	while (!vectors_overridden)
		millisecond_wait(5000);

	Kdebug1((dbg_fd,"bios_mouse:  mouse_loop() after vector check\n"));

	dev_mouse_init();

	Kdebug1((dbg_fd,"bios_mouse:  mouse_loop() after dev_mouse_init\n"));

	while(TRUE) {
		kd_event * event;
		Kdebug1((dbg_fd, "About to read mouse code\n"));
		millisecond_wait(10);
		num_sc = read_dev_mouse();
		Kdebug1((dbg_fd, "Num mouse codes = %x\n",num_sc));

		for(cur_sc = 0; cur_sc < num_sc; cur_sc++) {
			event = &(mevents[cur_sc]);
			switch (event->type) {			
			    case MOUSE_LEFT:
				set_button(LEFT_BUTTON,event->value.up);
				break;
			    case MOUSE_MIDDLE:
				set_button(MIDDLE_BUTTON,event->value.up);
				break;
			    case MOUSE_RIGHT:
				set_button(RIGHT_BUTTON,event->value.up);
				break;
			    case MOUSE_MOTION:
				mouse_delta(((event->value.m_deltaX*8)/
					     x_mickeys_to_pixels),
					    ((event->value.m_deltaY*8)/
					     y_mickeys_to_pixels));
				break;
			}
		}
	}
}
