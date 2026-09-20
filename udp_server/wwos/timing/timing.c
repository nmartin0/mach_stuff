#include <sys/errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <mach.h>
#include <mach_error.h>
#include <sys/mman.h>
#include <stdio.h>

#include "timing.h"

#ifdef MAPPED_TIMER
#ifndef MMAP_EXISTS
#define	mmap(a,b,c,d,e,f)	syscall(71,a,b,c,d,e,f)
#endif
#define timeofday(tvp)	(*tv = *mach_tv)


struct timeval *mach_tv = 0;
struct timezone *mach_tz = 0;

int
InitTiming()
{
    int fd;
    extern int errno;
    static struct tm {
	struct timeval  t;
	struct timezone tz;
    } *tm;

    if (tm == 0) {
	if (vm_allocate(task_self(), &tm, vm_page_size, TRUE)) {
	    errno = ENOMEM;
	    return -1;
	}
	if ((fd = open("/dev/time", 0)) < 0) {
	    return 0;
	}
	if (mmap(tm, vm_page_size, PROT_READ, MAP_SHARED, fd, 0) < 0) {
	    close(fd);
	    return -1;
	}
	close(fd);
	mach_tv = &tm->t;
	mach_tz = &tm->tz;
    }
    if (tp) *tp = tm->t;
    if (tzp) *tzp = tm->tz;
}
#else
#define timeofday(tvp)  gettimeofday(tvp, 0);
int
InitTiming()
{}
#endif




TimingTick(et)
    ElapsedTimeT	et;
{
    printf("Timing tick not implemented\n");
}



ElapsedTimeT
NewTiming(name, ot)
	char *name;
	ElapsedTimeT 	ot;
{
	ElapsedTimeT    et;
	et = (ElapsedTimeT) malloc(sizeof(*et));
	bzero(et, sizeof(*et));
	et->to_name = ((char *) calloc(1, strlen(name) + 1));
	strcpy(et->to_name, name);
	et->to_count = 1;
	if (ot)
		et->to_next = ot;
	return et;
}

ElapsedTimeT
DeleteTiming(et)
	ElapsedTimeT	et;
{
	ElapsedTimeT	next;
	next = et->to_next;
	free(et->to_name);
	free(et);
	return next;
}

void				
StartTiming(et)
	ElapsedTimeT	et;
{
	getrusage(RUSAGE_SELF, &et->to_rustart);
	timeofday(&et->to_tvstart);
}

void
EndTiming(et, cnt)
    ElapsedTimeT	et;
    int cnt;
{
	getrusage(RUSAGE_SELF, &et->to_ruend);
	timeofday(&et->to_tvend);
	et->to_count = cnt;
}
	

double
GetElapsedTimeInMs(et)
	ElapsedTimeT	et;
{
	struct timeval  tv0;
	struct timeval  tv1;
	double          ms;

	ms = (double) timeDiffInMs(&et->to_tvend, &et->to_tvstart);
	return ms / et->to_count;
}

double
GetSystemTimeInMs(et)
	ElapsedTimeT    et;
{
	double          ms;
	ms = (double) timeDiffInMs(&(et->to_ruend.ru_stime),
				   &(et->to_rustart.ru_stime));
	return ms / et->to_count;				  	
}

double
GetUserTimeInMs(et)
	ElapsedTimeT    et;
{
	double          ms;
	ms = (double)  timeDiffInMs(&(et->to_ruend.ru_utime),
					  &(et->to_rustart.ru_utime));
	return ms / et->to_count;					  
}

double
GetTimeInMs(et, which)
    ElapsedTimeT et;
    int which;
{
    switch (which) {
    case ELAPSED_TIME:
	return GetElapsedTimeInMs(et);
	break;
    case USER_TIME:
	return GetUserTimeInMs(et);
	break;
    case SYSTEM_TIME:
	return GetSystemTimeInMs(et);
	break;
    default:
	return -1;
    }
}


timeDiffInMs(t1, t0)
	struct timeval *t1;
	struct timeval *t0;
{
	int             t0msec;
	int             t1msec;

	t0msec = t0->tv_sec * 1000 + (t0->tv_usec / 1000);
	t1msec = t1->tv_sec * 1000 + (t1->tv_usec / 1000);
	return t1msec - t0msec;
}
ElapsedTimeT 
TimeFunction(f, arg, cnt, name)
    int (*f)();
    char *arg;
    int cnt;
    char *name;
{
    ElapsedTimeT t;

    t = NewTiming(name, 0);
    StartTiming(t);
    (*f)(arg);
    EndTiming(t,cnt);
    return t;
}


void
DumpTiming(et, f)
    ElapsedTimeT et;
    FILE *f;
{
    double          dcnt;

    dcnt = (float) et->to_count;
    fprintf(f, "Event: %-30s  Cnt: %10d E(ms): %5.5f\n",
	    et->to_name,
	    et->to_count,
	    GetElapsedTimeInMs(et));
    return;
}


char *
TimingToString(et)
    ElapsedTimeT et;
{
    static char s[256];
    double          dcnt;

    dcnt = (float) et->to_count;
    sprintf(s, "Event=%-20s  Cnt=%-d E(ms)=%-5.5f",
	    et->to_name,
	    et->to_count,
	    GetElapsedTimeInMs(et));
    return s;
}



	
void
DumpTimingLong(et, f)
    ElapsedTimeT et;
    FILE* f;
{
	double          dcnt;

	dcnt = (float) et->to_count;
	fprintf(f, "Event: %-20s  Cnt: %d E(ms): %5.5f S(ms): %5.5f U(ms): %5.5f\n",
		et->to_name,
		et->to_count,
		GetElapsedTimeInMs(et),
		GetSystemTimeInMs(et),		
		GetUserTimeInMs(et));
}
		
		
	    
void
DumpTimings(et, f)
	ElapsedTimeT et;
        FILE* f;
{
	ElapsedTimeT	curT;
	
	curT = et;
	while (curT)	{
	    DumpTiming(curT,f);
	    curT = curT->to_next;
	}
}
	    
	


