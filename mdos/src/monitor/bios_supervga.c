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
 * The V86 Mode SuperVGA state handling routines.
 *
 *
 * HISTORY:
 * $Log:	bios_supervga.c,v $
 * Revision 2.2  92/05/22  16:13:14  grm
 * 	Stupid stupid stupid....
 * 	[92/05/22  16:10:22  grm]
 * 
 * Revision 2.1.2.1  92/05/21  16:45:11  grm
 * 	Finished groundwork for generic SuperVGA support.  Implemented
 * 	support code for the Tseng Lab's ET4000 SuperVGA chip set.
 * 	[92/05/21            grm]
 * 
 * Revision 2.1.1.1  92/04/24  12:38:15  grm
 * 	Initial Version.  Not in working state.
 * 
 *
 */
#include "base.h"
#include "bios.h"

#include "bios_video_common.h"

boolean_t (*save_supervga_state)();
boolean_t (*restore_supervga_state)();
boolean_t (*save_supervga_ram)();
boolean_t (*restore_supervga_ram)();

vm_address_t * start_vga_memory;
vm_address_t * pause_vga_memory;

extern vm_address_t * mach_planes;

#define	SUPERVGA_ARRAYS		4
#define	SUPERVGA_REGS_MAX	64
u_char	supervga_states[SUPERVGA_ARRAYS][SUPERVGA_REGS_MAX];
u_char	* supervga_state;
int	supervga_array_index = 0;

#define	STANDARD_VGA	0
#define	TSENG_ET3000	1
#define	TSENG_ET4000	2
#define VIDEO7_V7VGA	3

#define	outval(index,val)	((val << 8) | index)

int	supervga_type;
u_short	sbase;		/* Base Register for SuperVGA, Color or Mono? */

/*
 * Test for Video 7 Super VGA card
 */
boolean_t video7_p()
{
	u_char new, old, current, id;
	u_char old_extension;

	/* Enable Video 7 Extensions */
	outb( 0x3c4, 6);
	old_extension = inb( 0x3c5 );

	outb( 0x3c5, 0xea );	/* enable extensions */

	/* determine if mono or color */
	
	outb( sbase+4, 0x0c );
	old = inb( sbase+5 );
	outb( sbase+5, 0x55 );
	new = inb( sbase+5 );

	outb( sbase+4, 0x1f );
	id = inb( sbase+5 );

	outb( sbase+4, 0x0c );
	outb( sbase+5, old );

	/* Disable Video 7 Extensions */
	outb( 0x3c4, 6 );
	outb( 0x3c5, 0xae );

	if ((0x55 ^ 0xea) == id) 
		return(1);

	outb( 0x3c4, 6 );
	outb( 0x3c5, old_extension );

	return(0);
}

/*
 * Test for the Tseng Labs SuperVGA Chip sets.
 */
boolean_t tseng_p()
{
	u_char	temp;
	u_char	old_val, check_val, new_val;


	outb( 0x3bf, 3 );
	if (inb( 0x3cc ) & 1) {
		outb( 0x3d8, 0xa0 );
	}else{
		outb( 0x3b8, 0xa0 );
	}

	temp = inb( 0x3da );
	outb( 0x3c0, 0x16 );
	old_val = inb( 0x3c1 );

	temp = inb( 0x3da );
	outb( 0x3c0, 0x16 );
	new_val = old_val ^0x10;
	outb( 0x3c0, new_val );

	temp = inb( 0x3da );
	outb( 0x3c0, 0x16 );
	check_val = inb( 0x3c1 );

	temp = inb( 0x3da );
	outb( 0x3c0, 0x16 );
	outb( 0x3c0, old_val );

	/* enable video access to memory. */
	outb( 0x3c0, (0x16 | 0x20) );

	return(check_val == new_val);
}

/*
 * Figure out which Tseng Chip we have, ET3000 or ET4000
 * 
 * returns 1 if 4000, 0 if 3000.
 */
int tseng_4000_p()
{
	u_char new, old;

	outb( sbase+4, 0x33 );
	old = inb( sbase+5 );

	outb( sbase+5, old ^ 0xf );
	new = inb( sbase+5 );

	outb( sbase+5, old );

	return(new == old ^ 0xf);
}

int tseng_4000_save_state( video_state )
	struct vga_state * video_state;
{
	int tmp;

	/* Do some preliminary stuff to soften up the ET4000 :-) */

	outb( 0x3bf, 3 );
	if (inb( 0x3cc ) & 1) {
		outb( 0x3d8, 0xa0 );
	}else{
		outb( 0x3b8, 0xa0 );
	}

	/* Save the Standard VGA Register State */
	save_standard_vga_state( video_state );
	
	/* Save the ET4000 Extended Register State */
	outb( 0x3d4, 0x32); supervga_state[3] = inb( 0x3d5 );	/* rasconfig */
	outb( 0x3d4, 0x33); supervga_state[9] = inb( 0x3d5 );	/* extendstrt*/
	outb( 0x3d4, 0x34); supervga_state[4] = inb( 0x3d5 );	/* compat */
	outb( 0x3d4, 0x35); supervga_state[10] = inb( 0x3d5 );	/* overflow */
	outb( 0x3d4, 0x36); supervga_state[1] = inb( 0x3d5 );	/* config1 */
	outb( 0x3d4, 0x37); supervga_state[2] = inb( 0x3d5 );	/* config2 */
	outb( 0x3d4, video_state->control_registers->crtc_index );

#if	0
	Vdebug0((dbg_fd,"video_state->control_registers->seq_index = 0x%x\n",video_state->control_registers->seq_index));

	outb( 0x3c4, 7); supervga_state[6] = inb( 0x3c5 );	/* aux mode */
	outb( 0x3c4, video_state->control_registers->seq_index );
#endif

	supervga_state[7] = inb( 0x3c3 );	/* video subsystem */
	supervga_state[9] = inb( 0x3cd );	/* segment select */

	/* Initialize Flip Flop */
	tmp = inb( 0x3da );
	outb( 0x3c0, 0x16 );
	outb( 0x3c0, (supervga_state[8] = inb( 0x3c1 ) ) );
	outb( 0x3c0, video_state->control_registers->attr_index | 0x20 );
}

int tseng_4000_restore_state( video_state, clear_p, mode )
	struct vga_state * video_state;
	boolean_t clear_p;
	int mode;
{
	int tmp;

	/* Restore the Standard VGA Registers */
	restore_standard_vga_state( video_state, clear_p, mode );

	/* Restore the ET4000 Extended Registers */

	outb( 0x3c3, supervga_state[7] ); /* video_subsystem */
	outb( 0x3cd, supervga_state[9] ); /* segment select */

#if	0
	outb( 0x3c4, 7); outb( 0x3c5, supervga_state[6] );	/* aux mode */
	outb( 0x3c4, video_state->control_registers->seq_index );
#endif

	outb( 0x3d4, 0x32); outb( 0x3d5, supervga_state[3] );	/* rasconfig */
	outb( 0x3d4, 0x33); outb( 0x3d5, supervga_state[9] );	/* extendstrt*/
	outb( 0x3d4, 0x34); outb( 0x3d5, supervga_state[4] ); 	/* compat */
	outb( 0x3d4, 0x35); outb( 0x3d5, supervga_state[10] );	/* overflow */
	outb( 0x3d4, 0x36); outb( 0x3d5, supervga_state[1] );	/* config1 */
	outb( 0x3d4, 0x37); outb( 0x3d5, supervga_state[2] );	/* config2 */

	supervga_state[7] = inb( 0x3c3 );	/* video subsystem */
	supervga_state[9] = inb( 0x3cd );	/* segment select */

	/* Initialize Flip Flop */
	tmp = inb( 0x3da );
	outb( 0x3c0, 0x16 );
	outb( 0x3c0, supervga_state[8] );
	outb( 0x3c0, video_state->control_registers->attr_index | 0x20 );
}

tseng_4000_save_ram( addr )
	vm_address_t * addr;
{
	int i,j,k;
	int segment_reg;
	int crtc_index, config1;
	u_char * rptr;
	u_char * wptr;

	Vdebug0((dbg_fd,"tseng_4000_save_ram()\n"));

	wptr = ((addr == mach_planes) ? (u_char *)start_vga_memory :
		(u_char *)pause_vga_memory);

	segment_reg = inb( 0x3cd );

	/* i loop is the segment register loop */
	for(i=0; i < 16; i++) {

		outb( 0x3cd, ((segment_reg & 0xf) | ((i)<<4)) );

		bcopy(0xa0000, wptr, 0x10000);
		wptr += 0x10000;
	}

	outb( 0x3cd, segment_reg );
}

tseng_4000_restore_ram( addr )
	vm_address_t * addr;
{
	int i,j;
	int segment_reg;
	int crtc_index, config1;
	u_char * rptr;
	u_char * wptr;

	Vdebug0((dbg_fd,"tseng_4000_restore_ram()\n"));

	rptr = ((addr == mach_planes) ? (u_char *)start_vga_memory :
		(u_char *)pause_vga_memory);

	segment_reg = inb( 0x3cd );

	for(i=0; i < 16; i++) {

		outb( 0x3cd, ((segment_reg & 0xf0) | (i)) );

		bcopy(rptr, 0xa0000, 0x10000);
		rptr += 0x10000;
	}

	outb( 0x3cd, segment_reg );
}

boolean_t save_supervga_state_preproc( video_state )
	struct vga_state * video_state;
{
	int index = video_state->supervga_state_index;

	Vdebug0((dbg_fd,"save_supervga_state_preproc (%x)\n",index));
	supervga_state = (u_char *)&(supervga_states[index][0]);

	save_supervga_state( video_state );
}

boolean_t restore_supervga_state_preproc( video_state, clear_p, mode )
	struct vga_state * video_state;
	boolean_t clear_p;
	int mode;
{
	int index = video_state->supervga_state_index;

	Vdebug0((dbg_fd,"restore_supervga_state_preproc (%x)\n",index));
	supervga_state = (u_char *)&(supervga_states[index][0]);

	restore_supervga_state( video_state, clear_p, mode );
}

int tseng_4000_save_memory() {}
int tseng_4000_restore_memory() {}

boolean_t (*supervga_p[])() = {tseng_p, NULL};
char * vga_names[] = {"Tseng Labs SuperVGA", "Standard VGA"};

boolean_t (*save_supervga_reg_procs[])() = {tseng_4000_save_state, NULL};
boolean_t (*restore_supervga_reg_procs[])() = {tseng_4000_restore_state, NULL};
boolean_t (*save_supervga_ram_procs[])() = {tseng_4000_save_ram, NULL};
boolean_t (*restore_supervga_ram_procs[])() = {tseng_4000_restore_ram, NULL};

/*
 * Allocate supervga state array to vga state.
 */
void supervga_state_init(vga_state)
	struct vga_state * vga_state;
{
	int index;

	Vdebug0((dbg_fd,"bios_supervga: supervga_state_init\n"));

	if (supervga_array_index == SUPERVGA_ARRAYS) {
		Vdebug0((dbg_fd,"Ran out of supervga state arrays.\n"));
		index = -1;
	}else{
		index = supervga_array_index++;
	}
	
	vga_state->supervga_state_index = index;
}

/*
 * Determine what kind, if any, SuperVGA card
 * is installed.
 */
void bios_supervga_init()
{
	int i,j;
	
	Vdebug0((dbg_fd,"bios_supervga: bios_supervga_init()\n"));

	sbase = ((inb( 0x3cc ) & 1) ? 0x3d0 : 0x3b0 );

	for (i=0; supervga_p[i] != NULL; i++) {
		if (supervga_p[i]()) {
			Vdebug0((dbg_fd,"%s\n",vga_names[i]));
			break;
		}
	}

	if (i == 0) {
		if (tseng_4000_p()) {
			Vdebug0((dbg_fd,"ET 4000 Chip Set\n"));
			supervga_type = TSENG_ET4000;
		}else{
			Vdebug0((dbg_fd,"ET 3000 Chip Set\n"));
			supervga_type = TSENG_ET3000;
		}
	}else{
		Vdebug0((dbg_fd,"Standard VGA\n"));
		supervga_type = STANDARD_VGA;
	}
	
	if (supervga_type == TSENG_ET4000) {
		Vdebug0((dbg_fd,"Setting up supervga_save/restore_state.\n"));
		save_supervga_state =  save_supervga_reg_procs[0];
		restore_supervga_state = restore_supervga_reg_procs[0];
		save_supervga_ram = save_supervga_ram_procs[0];
		restore_supervga_ram = restore_supervga_ram_procs[0];

		MACH_CALL(vm_allocate(mach_task_self(), &start_vga_memory, 
				      0x100000, TRUE),
			  "Couldn't allocate supervga ram memory\n");

		MACH_CALL(vm_allocate(mach_task_self(), &pause_vga_memory, 
				      0x100000, TRUE),
			  "Couldn't allocate supervga ram memory\n");

		Vdebug0((dbg_fd,"save = 0x%x, restore = 0x%x\n",
			 save_supervga_state,
			 restore_supervga_state));

		Vdebug0((dbg_fd,"start_vga_memory = 0x%x pause_vga_mem = 0x%x\n",
			 start_vga_memory, pause_vga_memory));
	}else{
		save_supervga_state = NULL;
		restore_supervga_state = NULL;
		save_supervga_ram = NULL;
		restore_supervga_ram = NULL;
	}
	supervga_state = (u_char *)&(supervga_states[0][0]);
}
