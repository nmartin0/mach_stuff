/*
 * @OSF_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: etap.h,v $
 * Revision 1.1.24.1  1996/09/17  16:34:39  bruel
 * 	fixed types.
 * 	[96/09/17            bruel]
 *
 * Revision 1.1.2.5  1996/02/02  12:19:37  emcmanus
 * 	Replaced the complex definition of etap_time_t by tvalspec_t, which is
 * 	easier and more efficient to operate on.  This breaks client programs
 * 	that know about the internals of struct mbuff_entry and the like, but
 * 	with the grotesque data-sharing between kernel and user modes that we
 * 	have, we can expect that sort of thing.
 * 	[1996/02/01  16:59:31  emcmanus]
 * 
 * Revision 1.1.2.4  1995/10/09  17:20:42  devrcs
 * 	Merged in RT3_SHARED ETAP code.
 * 	[1995/09/13  18:58:34  joe]
 * 
 * 	Merge forward.
 * 	[1995/08/24  22:35:55  watkins]
 * 
 * Revision 1.1.13.1  1995/08/04  17:08:11  watkins
 * 	Declare etap_trace instead of etap_trace_event, until the
 * 	etap functional set is merged in.
 * 	[1995/08/04  15:22:21  watkins]
 * 
 * 	Checkin for rt3 merge.
 * 	[1995/07/26  21:09:28  watkins]
 * 
 * Revision 1.1.2.3  1995/09/18  19:20:19  devrcs
 * 	Merged in RT3_SHARED ETAP code.
 * 	[1995/09/13  18:58:34  joe]
 * 
 * 	Merge forward.
 * 	[1995/08/24  22:35:55  watkins]
 * 
 * Revision 1.1.13.1  1995/08/04  17:08:11  watkins
 * 	Declare etap_trace instead of etap_trace_event, until the
 * 	etap functional set is merged in.
 * 	[1995/08/04  15:22:21  watkins]
 * 
 * 	Checkin for rt3 merge.
 * 	[1995/07/26  21:09:28  watkins]
 * 
 * Revision 1.1.2.2  1995/01/10  05:16:04  devrcs
 * 	mk6 CR801 - new file for mk6_shared from cnmk_shared.
 * 	[1994/12/01  21:11:57  dwm]
 * 
 * Revision 1.1.2.1  1994/10/21  18:45:54  joe
 * 	Initial ETAP submission
 * 	[1994/10/20  19:27:51  joe]
 * 
 * $EndLog$
 */
/*
 * File : etap.h 
 *
 *	  Contains ETAP buffer and table definitions
 *
 */

#ifndef _MACH_ETAP_H_
#define _MACH_ETAP_H_

#include <mach/etap_events.h>
#include <mach/clock_types.h>
#include <mach/time_value.h>
#include <mach/kern_return.h>


#define ETAP_CBUFF_ENTRIES	20000
#define ETAP_CBUFF_IBUCKETS	10
#define	ETAP_CBUFF_WIDTH	80

#define ETAP_MBUFF_ENTRIES	28000
#define ETAP_MBUFF_DATASIZE	4


/* ===================================
 * Event & Subsystem Table Definitions
 * ===================================
 */

#define EVENT_NAME_LENGTH    20                    /* max event name size  */

struct  event_table_entry {
        unsigned short	event;			   /* etap event type      */
        unsigned short	status;                    /* event trace status   */
        char		name [EVENT_NAME_LENGTH];  /* event text name      */
        unsigned short	dynamic;                   /* dynamic ID (0=none)  */
};

struct  subs_table_entry {
        unsigned short	subs;                      /* etap subsystem type  */
        char		name [EVENT_NAME_LENGTH];  /* subsystem text name  */
};

typedef struct event_table_entry*	event_table_t;
typedef struct subs_table_entry*	subs_table_t;
typedef unsigned short			etap_event_t;

#define EVENT_TABLE_NULL		((event_table_t) 0)

/* =========
 * ETAP Time
 * =========
 */

typedef tvalspec_t etap_time_t;

/* =============================
 * Cumulative buffer definitions
 * =============================
 */

/*
 *  The cbuff_data structure contains cumulative lock
 *  statistical information for EITHER hold operations
 *  OR wait operations.
 */

struct cbuff_data {
	unsigned long	triggered;       /* number of event occurances  */
	etap_time_t	time;            /* sum of event durations	*/
	etap_time_t	time_sq;         /* sum of squared durations	*/
	etap_time_t	min_time;        /* min duration of event       */
	etap_time_t	max_time;        /* max duration of event  	*/
};

/*
 *  The cbuff_entry contains all trace data for an event.
 *  The cumulative buffer consists of these entries.
 */

struct cbuff_entry {
	etap_event_t	  event;                     /* event type           */
	unsigned short	  kind;                      /* read,write,or simple */
	unsigned int	  instance;                  /* & of event struct    */
	struct cbuff_data hold;                      /* hold trace data      */
	struct cbuff_data wait;                      /* wait trace data      */
	unsigned long hold_interval[ETAP_CBUFF_IBUCKETS];   /* hold interval array  */
	unsigned long wait_interval[ETAP_CBUFF_IBUCKETS];   /* wait interval array  */
};

typedef struct cbuff_entry* cbuff_entry_t;

#define CBUFF_ENTRY_NULL	((cbuff_entry_t)0)

/* 
 *  The cumulative buffer maintains a header which is used by
 *  both the kernel instrumentation and the ETAP user-utilities.
 */

struct cumulative_buffer {
	unsigned long      next;         	   /* next available entry in buffer */
	unsigned short     static_start;          /* first static entry in buffer   */ 
	struct cbuff_entry  entry [ETAP_CBUFF_ENTRIES];  /* buffer entries   */
};

typedef struct cumulative_buffer* cumulative_buffer_t;


/* ===========================
 * ETAP probe data definitions
 * ===========================
 */

typedef	unsigned int	etap_data_t[ETAP_MBUFF_DATASIZE];

#define ETAP_DATA_ENTRY	sizeof(unsigned int)
#define ETAP_DATA_SIZE	ETAP_DATA_ENTRY * ETAP_MBUFF_DATASIZE
#define ETAP_DATA_NULL	(etap_data_t*) 0

/* ==========================
 * Monitor buffer definitions
 * ==========================
 */

/*
 *  The mbuff_entry structure contains trace event instance data.
 */

struct mbuff_entry {
	unsigned short	event;	    /* event type                             */
	unsigned short	flags;      /* event strain flags		      */
	unsigned int	instance;   /* address of event (lock, thread, etc.)  */
	unsigned int	pc;	    /* program counter			      */
	etap_time_t  	time;	    /* operation time                         */
	etap_data_t	data;	    /* event specific data  		      */
};

typedef struct mbuff_entry* mbuff_entry_t;

/* 
 *  Each circular monitor buffer will contain maintanence 
 *  information and mon_entry records.
 */

struct monitor_buffer {
	unsigned long		free;        /* index of next available record */
	unsigned long 		timestamp;   /* timestamp of last wrap around  */
	struct mbuff_entry entry[1]; /* buffer entries (holder)        */
};

typedef struct monitor_buffer* monitor_buffer_t;


/* ===================
 * Event strains/flags
 * ===================
 */					/* | |t|b|e|k|u|m|s|r|w| | | | | */
					/* ----------------------------- */
#define  WRITE_LOCK	0x10		/* | | | | | | | | | |1| | | | | */
#define  READ_LOCK	0x20		/* | | | | | | | | |1| | | | | | */
#define  COMPLEX_LOCK	0x30		/* | | | | | | | | |1|1| | | | | */
#define  SPIN_LOCK	0x40		/* | | | | | | | |1| | | | | | | */
#define  MUTEX_LOCK	0x80		/* | | | | | | |1| | | | | | | | */
#define	 USER_EVENT	0x100		/* | | | | | |1| | | | | | | | | */
#define  KERNEL_EVENT	0x200		/* | | | | |1| | | | | | | | | | */
#define  EVENT_END	0x400		/* | | | |1| | | | | | | | | | | */
#define  EVENT_BEGIN	0x800		/* | | |1| | | | | | | | | | | | */
#define  SYSCALL_TRAP	0x1000		/* | |1| | | | | | | | | | | | | */


/* =========================
 * Event trace status values
 * =========================
 */					/* | | | | | | | | | | |M|M|C|C| */
					/* | | | | | | | | | | |d|c|d|c| */
					/* ----------------------------- */	
#define CUM_CONTENTION	0x1		/* | | | | | | | | | | | | | |1| */
#define CUM_DURATION	0x2		/* | | | | | | | | | | | | |1| | */
#define MON_CONTENTION	0x4		/* | | | | | | | | | | | |1| | | */
#define MON_DURATION	0x8		/* | | | | | | | | | | |1| | | | */

#define ETAP_TRACE_ON	0xf		/* | | | | | | | | | | |1|1|1|1| */
#define ETAP_TRACE_OFF	0x0		/* | | | | | | | | | | | | | | | */


/* ==================
 * ETAP trace flavors
 * ==================
 */
	
/* Mode */

#define ETAP_CUMULATIVE	0x3		/* | | | | | | | | | | | | |1|1| */
#define ETAP_MONITORED	0xc		/* | | | | | | | | | | |1|1| | | */
#define ETAP_RESET   	0xf0f0

/* Type */

#define ETAP_CONTENTION	0x5		/* | | | | | | | | | | | |1| |1| */
#define ETAP_DURATION	0xa		/* | | | | | | | | | | |1| |1| | */


/* ===============================
 * Buffer/Table flavor definitions
 * ===============================
 */

#define  ETAP_TABLE_EVENT    	 	0
#define  ETAP_TABLE_SUBSYSTEM  		1
#define  ETAP_BUFFER_CUMULATIVE       	2
#define  ETAP_BUFFER_MONITORED       	3

/* ==========================
 * ETAP function declarations
 * ==========================
 */

extern
kern_return_t		etap_trace_event(
				   unsigned short 	mode,
				   unsigned short 	type,
				   boolean_t		enable,
				   unsigned int 	nargs,
				   unsigned short 	args[]);

extern
kern_return_t		etap_probe(
				   unsigned short	event_id,
				   unsigned int		data_size,
				   etap_data_t		*data);

#endif  /* _MACH_ETAP_H_ */
