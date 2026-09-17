/* utmp.h for Linux, by poe@daimi.aau.dk */

#ifndef UTMP_H
#define UTMP_H

#include <features.h>
#include <sys/types.h>
#include <time.h>
#include <paths.h>

#define UTMP_FILE	_PATH_UTMP
#define WTMP_FILE	_PATH_WTMP

#define UTMP_FILENAME	UTMP_FILE
#define WTMP_FILENAME	WTMP_FILE

/* defines for the ut_type field */
#define UT_UNKNOWN	0

/* size of user name */
#if 1
#define UT_LINESIZE	12
#define UT_NAMESIZE	8
#define UT_HOSTSIZE	16
#else
#define UT_LINESIZE	16
#define UT_NAMESIZE	16
#define UT_HOSTSIZE	256
#endif

#define RUN_LVL		1
#define BOOT_TIME	2
#define NEW_TIME	3
#define OLD_TIME	4

#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8


struct utmp {
	short	ut_type;	/* type of login */
	pid_t	ut_pid;		/* pid of login-process */
	char	ut_line[UT_LINESIZE];	/* devicename of tty -"/dev/", null-term */
	char	ut_id[4];	/* abbrev. ttyname, as 01, s1 etc. */
	time_t	ut_time;	/* logintime */
	char	ut_user[UT_NAMESIZE];	/* username, not null-term */
	char	ut_host[UT_HOSTSIZE];	/* hostname for remote login... */
	long	ut_addr;	/* IP addr of remote host */
};

#define ut_name ut_user

__BEGIN_DECLS

extern void		setutent __P ((void));
extern void		utmpname __P ((__const char *));
extern struct utmp	*getutent __P ((void));
extern struct utmp	*getutid __P ((struct utmp *));
extern struct utmp 	*getutline __P ((struct utmp *));
extern void		pututline __P ((struct utmp *));
extern struct utmp	*_getutline __P ((struct utmp *));
extern void		endutent __P ((void));

__END_DECLS

#endif	
