/*
 * HISTORY
 * $Log: machverstest.s,v $
# Revision 1.1.1.1  1995/05/04  06:56:39  sclawson
# New files.
#
 * Revision 2.2  92/11/13  17:32:35  mrt
 * 	Created.
 * 
 * 
 * Revision 2.3  92/11/09  16:40:48  mrt
 * 	Switched to use Mach kernel trap mechanism instead of Unix
 * 	SYSCALL, since the Unix one was not setting a register that the
 * 	Mach kernel was using. Also switced to mach_task_self, since an
 * 	extra reference to the task port seemed more innocouous than one
 * 	to a thread port.
 * 	[92/11/09            mrt]
 * 
 * Revision 2.2  92/07/23  13:59:39  mrt
 * 	Test to see if we are running Mach 2.5 or 3.0. -27 is
 * 	mach_thread_self implemented only by Mach 3.0. When called on 
 * 	Mach 2.5 it returns 4, KERN_INVALID_ARG.
 * 	[92/07/17            mrt]
 * 
 */
#include <mach/machine/syscall_sw.h>

kernel_trap(mach_vers_test,-28,0)
