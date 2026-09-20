/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	emulator_base.c,v $
 * Revision 2.4  94/03/25  18:21:23  mrt
 * 	Added -Xlinker switch to get the -D value passed to gcc linker.
 * 	[94/03/24            mrt]
 * 
 * Revision 2.3  91/12/19  20:36:53  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:40:42  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:21:03  rwd]
 * 
 * Revision 2.3  90/05/21  13:45:57  dbg
 * 	Return 0 for success.
 * 	[90/03/14            dbg]
 * 
 * Revision 2.2  89/11/29  15:26:49  af
 * 	Added mips, RCS-ed.
 * 	[89/11/16            af]
 * 
 */
#include <machine/vmparam.h>
#include <sys/exec.h>

main()
{
#ifdef sun
	printf("%x\n", EMULATOR_BASE + sizeof (struct exec));
#else
#ifdef mips
#ifdef  __GNUC__
        printf("%x -Xlinker -D -Xlinker %x\n", EMULATOR_BASE, EMULATOR_BASE + (1024*1024));
#else
	printf("%x -D %x\n", EMULATOR_BASE, EMULATOR_BASE + (1024*1024));
#endif /* __GNUC__ */
#else
	printf("%x\n", EMULATOR_BASE);
#endif
#endif
	return (0);
}
