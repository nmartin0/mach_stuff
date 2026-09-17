/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: host_reboot.h,v $
 * Revision 1.1.5.1  1995/01/06  19:50:20  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	new file for mk6
 * 	[1994/10/12  22:25:10  dwm]
 *
 * Revision 1.1.2.2  1993/10/20  19:09:29  gm
 * 	CR9913: Replacement for <sys/reboot.h> flags used in host_reboot()
 * 	calls.
 * 	[1993/10/13  17:21:14  gm]
 * 
 * $EndLog$
 */

#define HOST_REBOOT_HALT	0x8
#define	HOST_REBOOT_DEBUGGER	0x1000
