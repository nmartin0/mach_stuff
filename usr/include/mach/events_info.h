/*
 * @OSF_FREE_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: events_info.h,v $
 * Revision 1.1.8.3  1995/01/26  22:15:39  ezf
 * 	removed extraneous CMU CR
 * 	[1995/01/26  20:24:56  ezf]
 *
 * Revision 1.1.8.2  1995/01/06  19:50:12  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	64bit cleanup
 * 	[1994/10/14  03:42:32  dwm]
 * 
 * Revision 1.1.4.3  1993/09/17  21:35:27  robert
 * 	change marker to OSF_FREE_COPYRIGHT
 * 	[1993/09/17  21:28:46  robert]
 * 
 * Revision 1.1.4.2  1993/06/04  15:13:47  jeffc
 * 	CR9193 - MK5.0 merge.
 * 	[1993/05/18  02:37:52  gm]
 * 
 * Revision 3.0  92/12/31  22:12:17  ede
 * 	Initial revision for OSF/1 R1.3
 * 
 * Revision 1.2  1991/06/20  12:13:09  devrcs
 * 	Created from mach/task_info.h.
 * 	[91/06/04  08:53:02  jeffc]
 * 
 * $EndLog$
 */
/*
 *	Machine-independent event information structures and definitions.
 *
 *	The definitions in this file are exported to the user.  The kernel
 *	will translate its internal data structures to these structures
 *	as appropriate.
 *
 *	This data structure is used to track events that occur during
 *	thread execution, and to summarize this information for tasks.
 */

#ifndef	_MACH_EVENTS_INFO_H_
#define _MACH_EVENTS_INFO_H_

struct events_info {
	long		faults;		/* number of page faults */
	long		zero_fills;	/* number of zero fill pages */
	long		reactivations;	/* number of reactivated pages */
	long		pageins;	/* number of actual pageins */
	long		cow_faults;	/* number of copy-on-write faults */
	long		messages_sent;	/* number of messages sent */
	long		messages_received; /* number of messages received */
};
typedef struct events_info		events_info_data_t;
typedef struct events_info		*events_info_t;
#define EVENTS_INFO_COUNT	\
		(sizeof(events_info_data_t) / sizeof(long))

#endif	/*_MACH_EVENTS_INFO_H_*/
