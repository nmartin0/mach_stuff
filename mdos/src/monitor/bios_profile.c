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
 * The Mdos profiling code.
 *
 *
 * HISTORY:
 * $Log:	bios_profile.c,v $
 * Revision 2.2  92/02/14  17:44:41  grm
 * 	Created.
 * 	[92/02/12  16:05:12  grm]
 * 
 *
 */
#include "base.h"
#include "bios.h"
#include "bios_profile.h"
#include <sys/file.h>

char * names21[] = {
	"Terminate Process",			/* 0x00 */
	"Character Input with Echo",
	"Character Output",
	"Auxiliary Input",
	"Auxiliary Output",
	"Printer Output",
	"Direct Console I/O",
	"Unifiltered Char Input w/o Echo",
	"Character Input w/o Echo",		/* 0x08 */
	"Display String",
	"Buffered Keyboard Input",
	"Check Input Status",
	"Flush In Buf and then Input",
	"Disk Reset",
	"Select Disk",
	"Open File F",
	"Close File F",				/* 0x10 */
	"Find First File F",
	"Find Next File F",
	"Delete File F",
	"Sequential Read F",
	"Sequential Write F",
	"Create File F",
	"Rename File F",
	"Reserved",				/* 0x18 */
	"Get Current Disk",
	"Set DTA Address",
	"Get Default Drive Data",
	"Get Drive Data",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",				/* 0x20 */
	"Random Read F",
	"Random Write F",
	"Get File Size F",
	"Set Relative Record Number F",
	"Set Interrupt Vector",
	"Create New PSP",
	"Random Block Read F",
	"Random Block Write F",			/* 0x28 */
	"Parse Filename",
	"Get Date",
	"Set Date",
	"Get Time",
	"Set Time",
	"Set Verify Flag",
	"Get DTA Address",
	"Get MS-DOS Version Number",		/* 0x30 */
	"Terminate and Stay Resident",
	"Reserved",
	"Get/Set Break Flag, Get Boot Drive",
	"Reserved",
	"Get Interrupt Vector",
	"Get Drive Allocation Information",
	"Reserved",
	"Get/Set Country Information",		/* 0x38 */
	"Create Directory",
	"Delete Dircetory",
	"Set Current Directory",
	"Create File H",
	"Open File H",
	"Close File H",
	"Read File or Device H",
	"Write File or Device H",			/* 0x40 */
	"Delete File H",
	"Set File Pointer H",
	"Get/Set File Attributes",
	"IOCTL",
	"Duplicate Handle",
	"Redirect Handle",
	"Get Current Directory",
	"Allocate Memory Block",		/* 0x48 */
	"Release memory Block",	
	"Resize Memory Block",
	"Execute Program",
	"Terminate Process with Return Code",
	"Get Return Code",
	"Find First File H",
	"Find Next File H",
	"Reserved",				/* 0x50 */
	"Reserved",
	"Reserved",
	"Reserved",
	"Get Verify Flag",
	"Reserved",
	"Rename File",
	"Get/Set File Date and Time H",
	"Get/Set Allocation Strategy",		/* 0x58 */
	"Get Extended Error Information",
	"Create Temporary File H",
	"Create New File H",
	"Lock/Unlock File Region H",
	"Reserved",
	"Get Machine Name, G/S Printer Setup",
	"Device Redirection",
	"Reserved",				/* 0x60 */
	"Reserved",
	"Get PSP Address",
	"Get DBCS Lead Byte Table",
	"Reserved",
	"Get Extended Country Information",
	"Get/Set Code Page",
	"Set Handle Count",
	"Commit File H",				/* 0x68 */
	"Reserved",
	"Reserved",
	"Reserved",
	"Extended Open File H"
	};

char * bios_13_names[] = {
	"BIOS: Disk Reset",			/* 0x00 */
	"BIOS: Get Disk System Status",
	"BIOS: Read Sector",
	"BIOS: Write Sector",
	"BIOS: Verify Sector",
	"BIOS: Format Track",
	"BIOS: Format Bad Track",
	"BIOS: Format Drive",
	"BIOS: Get Drive Parameters",		/* 0x08 */
	"BIOS: Initialize Fixed Disk Chars",
	"BIOS: Read Sector Long",
	"BIOS: Write Sector Long",
	"BIOS: Seek",
	"BIOS: Reset Fixed Disk System",
	"BIOS: Read Sector Buffer",
	"BIOS: Write Sector Buffer",
	"BIOS: Get Drive Status",		/* 0x10 */
	"BIOS: Recalibrate Drive",
	"BIOS: Controller RAM Diagnostic",
	"BIOS: Controller Drive Diagnostic",
	"BIOS: Controller Internal Diagnostic",
	"BIOS: Get Disk Type",
	"BIOS: Get Disk Change Status",
	"BIOS: Set Disk Type",
	"BIOS: Set Media Type for Format",	/* 0x18 */
	"BIOS: Park Heads",
	"BIOS: Format ESDI Drive"
	};

#ifdef	PROFILING

boolean_t profiling = FALSE;
u_long total_exceptions = 0;
u_long cli_exceptions = 0;
u_long sti_exceptions = 0;
u_long iret_exceptions = 0;
u_long pushf_exceptions = 0;
u_long popf_exceptions = 0;
u_long in_exceptions = 0;
u_long out_exceptions = 0;
u_long realint_exceptions = 0;

/*
 * Note:  This is a really stupid way to keep track of the interrupt
 * 	  occurrances.  If you want to change it, please be my guest!
 *	  Use a hash table.
 */

struct profile_type profile_info[PROF_INTS][PROF_AX];


void init_prof()
{
	int i,j;

	for(i=0;i<PROF_INTS;i++) {
		for(j=0;j<PROF_AX;j++) {
			profile_info[i][j].invoked = 0;
			profile_info[i][j].max_time = 0;
			profile_info[i][j].ave_time = 0;
		}
	}
}

void dump_prof()
{
	FILE * file;
	int i, j;
	int int_except = 0;
	int accum_except = 0;

	if ((file = fopen ("prof.out", "w+")) == NULL) {
		fprintf(dbg_fd,"Couldn't open prof.out\n");
		return;
	}

	for (i=0;i<PROF_INTS;i++) {
	    for(j=0;j<PROF_AX;j++) {
		if (profile_info[i][j].invoked != 0) {
		    int_except += profile_info[i][j].invoked;
			
		    fprintf(file, "Int 0x%x [0x%x] -- Invoked 0x%x, Max 0x%x, Ave 0x%x",
			    i, j,
			    profile_info[i][j].invoked, profile_info[i][j].max_time,
			    profile_info[i][j].ave_time/profile_info[i][j].invoked);
		    if ((i == 0x21) &&(j < DOS21_NAMES)) {
			    fprintf(file, " '%s'",names21[j]);
		    }else if ((i == 0x13) && (j < BIOS13_NAMES)) {
			    fprintf(file, " '%s'",bios_13_names[j]);
		    }
		    fprintf(file,"\n");
	        }
	    }
        }
	
	accum_except += int_except;
	fprintf(file,"\nInt exceptions %d\n", int_except);

	accum_except += cli_exceptions;
	fprintf(file,"\nCli exceptions %d\n", cli_exceptions);

	accum_except += sti_exceptions;
	fprintf(file,"Sti exceptions %d\n", sti_exceptions);

	accum_except += pushf_exceptions;
	fprintf(file,"Pushf exceptions %d\n", pushf_exceptions);

	accum_except += popf_exceptions;
	fprintf(file,"Popf exceptions %d\n", popf_exceptions);

	accum_except += in_exceptions;
	fprintf(file,"In exceptions %d\n", in_exceptions);

	accum_except += out_exceptions;
	fprintf(file,"Out exceptions %d\n", out_exceptions);

	accum_except += iret_exceptions;
	fprintf(file,"Iret exceptions %d\n", iret_exceptions);

	fprintf(file,"\nTotal number of exceptions: %d\n",
		total_exceptions);

	fprintf(file, "Number of unknown exceptions %d, lost int exceptions %d\n",
	       total_exceptions - accum_except, realint_exceptions - int_except);


	fclose(file);
}

void begin_prof()
{
	profiling = TRUE;
	total_exceptions = 0;
	cli_exceptions = 0;
	sti_exceptions = 0;
	iret_exceptions = 0;
	pushf_exceptions = 0;
	popf_exceptions = 0;
	in_exceptions = 0;
	out_exceptions = 0;
	realint_exceptions = 0;
}

void end_prof()
{
	profiling = FALSE;

	dump_prof();
}

#endif	PROFILING
