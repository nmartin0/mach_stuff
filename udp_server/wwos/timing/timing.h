
#ifndef _TIMING_H
#define _TIMING_H
#include <sys/time.h>
#include <sys/resource.h>
#ifndef MULTIMAX
#include <mach/time_stamp.h>
#endif

typedef struct ElapsedTime *ElapsedTimeT;

struct ElapsedTime {
	char           *to_name;
	int		to_count;	/* how many events occurred */
	struct rusage   to_rustart;
	struct rusage   to_ruend;
	struct timeval  to_tvstart;
	struct timeval  to_tvend;	/* also used for last tick */
	struct timeval  to_tvrunning;
	ElapsedTimeT    to_next;
};


#ifdef __STDC__
ElapsedTimeT	NewTiming(char*, ElapsedTimeT);
ElapsedTimeT	DeleteTiming(ElapsedTimeT);
void	        StartTiming(ElapsedTimeT);
void	        EndTiming(ElapsedTimeT, int);


double		GetTimeInMs(ElapsedTimeT, int);
void		DumpTiming(ElapsedTimeT, FILE*);
void		SDumpTiming(ElapsedTimeT, char*);
void		DumpTimings(ElapsedTimeT, FILE*);
#else

ElapsedTimeT	NewTiming();
ElapsedTimeT	DeleteTiming();
ElapsedTimeT	TimeFunction();
void	        StartTiming();
void	        EndTiming();


double		GetTimeInMs();
void		SDumpTiming();
void		DumpTiming();
void		DumpTimings();
#endif


#define		ELAPSED_TIME 0
#define		USER_TIME   1
#define		SYSTEM_TIME 2

#define NULL_TIMING ((ElapsedTimeT)0)

#endif _TIMING_H
