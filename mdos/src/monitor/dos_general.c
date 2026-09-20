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
 *	V86 DOS disk emulation routines
 *
 * HISTORY: 
 * $Log:	dos_general.c,v $
 * Revision 2.5  92/04/29  16:31:24  grm
 * 	Changes for new override vectors code.
 * 	[92/04/29            grm]
 * 
 * Revision 2.4  92/04/14  13:20:35  grm
 * 	Removed some obsolete code.  Fixed some return bugs.  This is the
 * 	NOFILES version.
 * 	[92/03/27            grm]
 * 
 * Revision 2.3  91/12/06  15:29:41  grm
 * 	Fixed han2abshan.  Ifdefed the code so that I could revert to old
 * 	way if needed.  It will get cut soon.
 * 	[91/12/06            grm]
 * 
 * Revision 2.2  91/12/05  16:42:35  grm
 * 	Added a generic handle2abshan procedure which takes a program's
 * 	local file handle and converts it into an sft handle.  This
 * 	procedure is used to check for a redirected console.  Most of the
 * 	code was rewritten to use this procedure.  Some additional code
 * 	was added to debug the sft problem with MS-Write.
 * 	[91/12/04            grm]
 * 	Added some printer support.  Changed the way
 * 	the CONSOLE output was checked.
 * 	[91/08/09  19:56:20  grm]
 * 
 * 	Added routine to speed execution with Dos v5.0
 * 	[91/06/28  18:56:58  grm]
 * 
 * 	Terminate code fixes, and EMM detection added.
 * 	[91/06/14  11:59:23  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  15:17:26  grm]
 * 
 * 	Works with the -s (startup) flag.
 * 	[91/05/02  13:34:50  grm]
 * 
 * 	Fixed the redirection of stdout.
 * 	[91/04/30  13:54:06  grm]
 * 
 * 	Routines that speed up the console output.
 * 	[91/04/30  13:50:57  grm]
 * 
 * 	Obsolete file.
 * 	[91/03/26  19:26:50  grm]
 * 
 * 	Last version before redirector interface.
 * 	[91/03/12  11:19:16  grm]
 * 
 * 	Mkdir and rmdir works along with del.
 * 	Some cleanup.
 * 	[91/03/08  14:15:53  grm]
 * 
 * 	Exec pseudo works.  This is just a back
 * 	up check in.
 * 	[91/03/01  14:41:40  grm]
 * 
 * 	Create and Open work.  Improved functionality.
 * 	[91/02/12  17:11:38  grm]
 * 
 * 	Type works. Interrim version.
 * 	[91/02/11  18:21:04  grm]
 * 
 * 	CD works.
 * 	[91/02/06  18:40:27  grm]
 * 
 * 	Fancy dir.
 * 	[91/02/06  16:58:29  grm]
 * 
 * 	u: dir psuedo works.  not fancy yet.
 * 	[91/02/06  14:30:17  grm]
 * 
 * 	Created.
 * 	[91/02/01  13:30:47  grm]
 * 
 */

#include "base.h"
#include "bios.h"

#include <stdio.h>
#include <machine/asm.h>
#include <machine/psl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>

#include <mach/message.h>
#include <mach/exception.h>

#include "dos.h"

#define DOS_CHARACTER_OUTPUT	0x02
#define DOS_DISPLAY_STRING	0x09
#define DOS_HANDLE_OPEN		0x3d
#define DOS_HANDLE_CLOSE	0x3e
#define	DOS_HANDLE_READ		0x3f
#define DOS_WRITE_FILE_OR_DEV	0x40
#define DOS_SET_FILE_PTR	0x42
#define DOS_IOCTL		0x44
#define DOS_EXEC		0x4b
#define	DOS_TERMINATE		0x00
#define	DOS_TERMINATE_WITH_CODE	0x4c
#define DOS_GET_SET_DATE_TIME	0x57

#define IOCTL_GET_DEV_INFO	0
#define IOCTL_CHECK_OUTPUT_STS	7

#define CONSOLE	1
#define STDPTR	4
#define EMM_FILE_HANDLE 55

extern u_char * display_mode;
extern sda_t sda;
extern lol_t lol;
extern u_char my_drive;
extern boolean_t startup_flag;

FILE * printer_file = NULL;

boolean_t done_override = FALSE;

u_char handle2abshan(handle)
	u_short handle;
{
	u_char * psp;
	u_char * han;
	u_char abshan;

	psp = (u_char *)PSPPTR(sda_cur_psp(sda));
	han = (u_char *)psp_handles(psp);
        abshan = han[handle];

#if 0
	Debug0((dbg_fd,"sda = 0x%x sda_cur_psp(sda) = 0x%x *.. = 0x%x\n",
		sda, sda_cur_psp(sda), *(psp_t)sda_cur_psp(sda)));

	Debug0((dbg_fd,"psp = 0x%x han = 0x%x handle = %x\n",psp,han,handle));
#endif

	Debug0((dbg_fd,"abshan = 0x%x\n",abshan));

	return(han[handle]);
}

boolean_t dos_general_fn(state)
	state_t * state;
{
	switch(HIGH(state->eax)) {
	    case DOS_CHARACTER_OUTPUT: {
		    u_char ch = LOW(state->edx);
		    u_char * psp;
		    u_char * han;
		    u_char dstdin;
		    u_char dstdout;
		    u_char dstderr;
		    int ret;

		    if (!mach_fs_enabled || startup_flag)
			    return(REDIRECT);

		    dstdout = handle2abshan(1);
		    
		    if (dstdout != CONSOLE)
			    return(REDIRECT);

		    ret = do_tty_output(&ch, 0x07, 1);

		    return(ret);
	    }
	    case DOS_DISPLAY_STRING: {
		    u_char * ptr = (u_char *)Addr(state,ds,edx);
		    int len;
		    int ret;
		    u_char * psp;
		    u_char * han;
		    u_char dstdin;
		    u_char dstdout;
		    u_char dstderr;

		    if (!mach_fs_enabled || startup_flag)
			    return(REDIRECT);

		    dstdout = handle2abshan(1);

		    if (dstdout != CONSOLE)
			    return(REDIRECT);

		    for(len=0;ptr[len]!=0x24;len++);

		    if (!vga_state) {
			    int i = 0;
			    while (i < len) {
				    fprintf(stderr,"%c",ptr[i++]);
			    }
			    ret = UNCHANGED;
		    }else{
			    ret = do_tty_output(ptr, 0x07, len);
		    }

		    return(ret);
	    }
#ifdef SFT_DEBUG
	    case DOS_HANDLE_READ: {
		    if (us_debug_level > Debug_Level_0) {
			    fprintf(dbg_fd,"Dos handle read.\n");
			    debug_dump_sft(handle2abshan(WORD(state->ebx)));
		    }
		    return(REDIRECT);
	    }
#endif /* SFT_DEBUG */
	    case DOS_WRITE_FILE_OR_DEV: {
		    u_short handle = WORD(state->ebx);
		    u_short count;
		    u_char * buff;
		    u_char real_desc;
		    int ret = UNCHANGED;

		    if (!mach_fs_enabled || startup_flag)
			    return(REDIRECT);

		    real_desc = handle2abshan(handle);

		    Debug0((dbg_fd,"real_desc = 0x%x\n", real_desc));

		    if (real_desc == CONSOLE) {
			    count = WORD(state->ecx);
			    buff = (u_char *)Addr(state, ds, edx);
			    
			    if (!vga_state) {
				    int i = 0;
				    while (i < count) {
					    fprintf(stderr,"%c",buff[i++]);
				    }
			    }else{
				    ret = do_tty_output(buff, 0x07, count);
				    if (ret != UNCHANGED) {
					    return(ret);
				    }
			    }
			    
			    SETWORD(&(state->eax), count);
			    
			    return(TRUE);
		    }else if (handle == STDPTR) {
			    int i;
			    
			    if (handle != STDPTR) {
				    Kdebug0((dbg_fd,"dos_general: FALSE stdptr\n"));
				    return(REDIRECT);
			    }
			    
			    Kdebug0((dbg_fd,"dos_general: intercepted a printer strings.\n"));
			    buff = (u_char*)Addr(state,ds,edx);
			    count = WORD(state->ecx);
#ifdef DONT_PRINT			    
			    if (key_debug_level > Debug_Level_0) {
				    fprintf(dbg_fd,"dos_general: string = '");
				    for (i = 0; i < count ; i++) {
					    fprintf(dbg_fd,"%c",buff[i]);
				    }
			    }
#else
			    if (printer_file == NULL) {
				    printer_file = fopen("ptr.ses","w+");
			    }
			    
			    for (i = 0; i < count; i++)
				    fprintf(printer_file,"%c",buff[i]);
#endif /* DONT_PRINT */
			    
			    SETWORD(&(state->eax), count);
			    return(TRUE);
#ifdef SFT_DEBUG
		    }else{
			    if (us_debug_level > Debug_Level_0) {
				    fprintf(dbg_fd,"Dos handle write.\n");
				    debug_dump_sft(real_desc);
			    }
			    return(REDIRECT);
#endif /* SFT_DEBUG */
		    }
	    }
#ifdef SFT_DEBUG
	    case DOS_SET_FILE_PTR: {
		    if (us_debug_level > Debug_Level_0) {
			    fprintf(dbg_fd,"Dos handle set file ptr.\n");
			    debug_dump_sft(handle2abshan(WORD(state->ebx)));
		    }
		    return(REDIRECT);
	    }
	    case DOS_GET_SET_DATE_TIME: {
		    if (us_debug_level > Debug_Level_0) {
			    fprintf(dbg_fd,"Dos handle get set date time.\n");
			    debug_dump_sft(handle2abshan(WORD(state->ebx)));
		    }
		    return(REDIRECT);
	    }
#endif /* SFT_DEBUG */
	    case DOS_TERMINATE:
	    case DOS_TERMINATE_WITH_CODE:
	    case DOS_EXEC: {
		    char * ptr = (char *)Addr(state,ds,edx);

		    Debug0((dbg_fd,"exec '%s'\n",ptr));

		    if (done_override || (strncmp(ptr, "\\COMMAND", 8) != 0)) {
			    return(REDIRECT);
		    }

		    Debug1((dbg_fd,"Execing COMMAND.COM, overriding vectors.",ptr));

		    override_interrupt_vectors();

		    done_override = TRUE;

		    return(REDIRECT);
	    }
	    case DOS_HANDLE_OPEN: {
		    char * ptr = (char *)Addr(state,ds,edx);
		    
		    if (strncmp(ptr, "EMMXXXX0", 8) != 0) {
			    return(REDIRECT);
		    }else{
			    Debug0((dbg_fd,"dos_general: opened emm file.\n"));
			    SETWORD(&(state->eax), EMM_FILE_HANDLE);
			    return(TRUE);
		    }
	    }
	    case DOS_HANDLE_CLOSE: {
		    if (WORD(state->ebx) != EMM_FILE_HANDLE) 
#ifdef SFT_DEBUG
			    if (us_debug_level > Debug_Level_0) {
				    fprintf(dbg_fd,"Dos handle close.\n");
				    debug_dump_sft(handle2abshan(WORD(state->ebx)));
			    }
#endif /* SFT_DEBUG */
			    return(REDIRECT);

		    return(TRUE);
	    }
	    case DOS_IOCTL: {
		    
		    if (WORD(state->ebx) != EMM_FILE_HANDLE)
			    return(REDIRECT);

		    switch(LOW(state->eax)) {
			case IOCTL_GET_DEV_INFO:
			    Debug0((dbg_fd,"dos_general: dos_ioctl getdevinfo emm.\n"));
			    SETWORD(&(state->edx), 0x80);
			    return(TRUE);
			case IOCTL_CHECK_OUTPUT_STS:
			    Debug0((dbg_fd,"dos_general: dos_ioctl chkoutsts emm.\n"));
			    SETLOW(&(state->eax), 0xff);
			    return(TRUE);
		    }
		    Debug0((dbg_fd,"dos_general: dos_ioctl shouldn't get here. XXX\n"));
		    return(FALSE);
	    }
	    default:
		return(REDIRECT);
	}
}
