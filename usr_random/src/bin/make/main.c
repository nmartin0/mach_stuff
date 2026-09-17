#ifndef lint
static char *RCSid = "$Id: main.c,v 1.16 91/01/03 13:00:58 bww Exp $";
#endif

/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 ************************************************************************
 * HISTORY
 * $Log:	main.c,v $
 * Revision 5.4  91/10/14  13:41:55  berman
 * 	Added support for ${MAKEDCLS} and -E/-X switches.
 * 	From STUMP "[91/01/03  12:53:42  bww]".
 * 	Some rudimentary support for timing statistics.
 * 	From STUMP "[90/10/11  17:44:10  bww]".
 * 	Also restored tahoe style definition for MACHINE,
 * 	and added dynamic setting of ${MAKE} from argv[0].
 * 	[91/10/10            bww]
 * 
 * Revision 5.3  90/08/06  18:22:01  mrt
 * 	Remove tahoe style definition of MACHINE for cmu compatibility
 * 	[90/07/13            mrt]
 * 
 * Revision 1.14  90/07/11  17:23:28  bww
 * 	Treat ${MACHINE} as a built-in macro.  Added support for
 * 	${MAKEDEFS} which holds the command line macro definitions.
 * 	[90/07/11  17:22:35  bww]
 * 
 * Revision 1.13  90/05/07  17:20:51  bww
 * 	Added support for the "-b" (botch) flag.  Being
 * 	a Murray Hill derivative, we come pre-botched.
 * 	[90/05/07  17:10:35  bww]
 * 
 * Revision 1.12  90/05/07  16:00:31  bww
 * 	fatal() now accepts printf-style arguments.
 * 	[90/05/07  15:52:59  bww]
 * 
 * Revision 1.11  90/05/07  12:41:35  bww
 * 	Purged "unix" conditionals.  Archives are now "precious"
 * 	default.  Reworked intrupt() to check modification dates.
 * 	[90/05/07  12:38:33  bww]
 * 
 * Revision 1.10  90/03/03  18:29:05  bww
 * 	Only enable INOBJECTDIR processing if
 * 	we believe we are in the object tree.
 * 	[90/03/03  18:27:01  bww]
 * 
 * Revision 1.9  90/02/22  17:38:52  bww
 * 	Added forward declarations for static functions.
 * 	[90/02/22  17:33:44  bww]
 * 
 * Revision 1.8  90/02/11  18:24:43  bww
 * 	Also do dynamic dollar substitution on .INOBJECTDIR
 * 	so that a `...` rule can be passed things like VPATH,
 * 	MAKEMCH, and MAKECPP.
 * 	[90/02/11  18:24:13  bww]
 * 
 * Revision 1.7  90/02/01  20:48:33  bww
 * 	Modifications for new doname() and docom().  Delinted.
 * 	[90/02/01  20:42:00  bww]
 * 
 * Revision 1.6  89/12/19  16:05:21  bww
 * 	Enable tahoe-style ${MACHINE}.  isdependent()
 * 	now returns the matching lineblock.
 * 	[89/12/19  16:03:18  bww]
 * 
 * Revision 1.5  89/12/02  22:54:15  bww
 * 	Use defined value of MACHINE to initialize ${MACHINE}.
 * 	Currently disabled during transition period.
 * 	[89/12/02  22:48:19  bww]
 * 
 * Revision 1.4  89/11/20  14:18:14  bww
 * 	Fixed to not try to modify literal strings.
 * 	[89/11/20  14:15:21  bww]
 * 
 * Revision 1.3  89/08/16  16:18:37  bww
 * 	Changed printing of entry banner to be sensitive to
 * 	"dbgflag" in $MAKEFLAGS as well as the command line.
 * 	[89/08/16  16:18:16  bww]
 * 
 * Revision 1.2  89/05/26  10:06:10  bww
 * 	CMU CS as of 89/05/15
 * 	[89/05/26  09:46:50  bww]
 * 
 * Revision 5.2  89/05/13  16:34:31  bww
 * 	Added entry and exit banners under -d to clearly delimit
 * 	debug output in recursive makes.
 * 	[89/05/13  16:34:13  bww]
 * 
 * Revision 5.1  89/04/13  20:45:57  bww
 * 	Added support for reading ".depend" after [Mm]akefile.
 * 	[89/04/13  20:44:32  bww]
 * 
 * Revision 5.0  89/03/01  01:39:55  bww
 * 	Version 5.
 * 	[89/03/01  01:31:33  bww]
 * 
 */

#ifndef lint
static	char *sccsid = "@(#)main.c	4.10 (Berkeley) 87/11/15";
#endif

/*
 * command make to update programs.
 *
 * Flags:
 *	'd'  print out debugging comments
 *	'p'  print out a version of the input graph
 *	'r'  do not use the default rules
 *	's'  silent mode--don't print out commands
 *	'f'  the next argument is the name of the description file;
 *	     "makefile" is the default
 *	'F'  a description file is compulsory
 *	'i'  ignore error codes from the shell
 *	'S'  stop after any command fails (default)
 *	'k'  continue work on other branches when a command fails
 *	'n'  don't issue, just print, commands
 *	't'  touch (update time of) files but don't issue command
 *	'q'  don't do anything, but check if object is up to date;
 *	     returns exit code 0 if up to date, -1 if not
 *	'e'  environment variable override makefile assignments
 *	'E'  the next argument is a variable assignment which should
 *           be "exported" to sub-makes
 *	'X'  treat all command line definitions as if a -E were given
 *	'c'  don't check out RCS files
 *	'C'  check out RCS files (default)
 *	'u'  don't unlink RCS working files
 *	'U'  unlink RCS working files automatically checked out (default)
 *	'x'  only check out (and don't unlink) RCS files
 *	'y'  check working file/RCS file dependencies
 *	'm'  look in machine specific subdirectory first
 *	'N'  don't do and "Makeconf" processing
 */

#include "defs.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#ifdef lint
#undef SIG_IGN
SIG_IGN(){}
#endif

FSTATIC int intrupt();
FSTATIC int rddescf();
FSTATIC setcocmd();
FSTATIC printdesc();
FSTATIC printmac();
FSTATIC setmflgs();
FSTATIC setflags();
FSTATIC optswitch();
FSTATIC getmflgs();
FSTATIC getmdefs();
FSTATIC setmdefs();
FSTATIC getmdcls();
FSTATIC setmdcls();

struct chain *rmchain		= 0;
struct chain *deschain		= 0;
struct nameblock *mainname	= 0;
struct nameblock *firstname	= 0;
struct nameblock *inobjdir	= 0;
struct lineblock *co_cmd	= 0;
struct lineblock *rcstime_cmd	= 0;
struct lineblock *sufflist	= 0;
struct varblock *firstvar	= 0;
struct dirhdr *firstod		= 0;

int rmflag	= YES;
int coflag	= YES;
int nocmds	= NO;
int rcsdep	= NO;
char *RCSdir	= RCS;
char *RCSsuf	= RCS_SUF;

int sighvalue	= 0;
int sigivalue	= 0;
int sigqvalue	= 0;
int sigtvalue	= 0;

int envover	= NO;
int dbgflag	= NO;
int prtrflag	= NO;
int silflag	= NO;
int noexflag	= NO;
int keepgoing	= NO;
int descflag	= NO;
int noruleflag	= NO;
int noconfig	= NO;
int touchflag	= NO;
int questflag	= NO;
int machdep	= NO;
int ignerr	= NO;	/* default is to stop on error */
int xportflag	= NO;	/* don't automatically export command line defs */
int botchflag	= YES;	/* we come pre-botched */
int okdel	= YES;
int ndocoms	= 0;
int inarglist	= 0;
int ininternal	= 0;
char *prompt	= "";	/* other systems -- pick what you want */
char *curfname	= "no description file";
char funny[128];
char options[26 + 1] = { '-' };
char Makeflags[] = "MAKEFLAGS";
char Makedefs[] = "MAKEDEFS";
char Makedcls[] = "MAKEDCLS";


int
main(argc, argv)
	int argc;
	char **argv;
{
	register struct nameblock *p;
	int i, descset, nfargs;
	time_t td;
	struct chain *ch;
	char *s;
	struct varblock *v;
	struct rusage ru1, ru2, kids;
	extern char *ctime();

	i = getdtablesize();
	while (i > 3)
		(void) close(--i);
#ifdef METERFILE
	meter(METERFILE);
#endif

	descset = 0;

	funny[0] = (META | TERMINAL);
	for (s = "=|^();&<>*?[]:$`'\"\\\n"; *s; ++s)
		funny[*s] |= META;
	for (s = "\n\t :;&>|()"; *s; ++s)
		funny[*s] |= TERMINAL;

	getmflgs();		/* Init $(MAKEFLAGS) variable */
	for (i = 1; i < argc; ++i)
		if (argv[i][0] == '-' && index(argv[i], 'd'))
			break;
	if (dbgflag || i < argc)
		entrybanner(argc, argv);
	getmdefs();
	getmdcls();
	setflags(argc, argv);

	inarglist = 1;
	for (i = 1; i < argc; ++i) {
		if (argv[i] == 0)
			continue;
		if (strcmp(argv[i], "-f") == 0) {
			++i;
		} else if (strcmp(argv[i], "-E") == 0) {
			argv[i] = 0;
			if (++i >= argc || argv[i] == 0)
				fatal("No definition after -E flag");
			if ((v = eqsign(argv[i], (char *) 0, 0)) == 0)
				fatal("Bad definition after -E flag");
			setmdefs(v);
			setmdcls(v);
			argv[i] = 0;
		} else if ((v = eqsign(argv[i], (char *) 0, 0)) != 0) {
			setmdefs(v);
			if (xportflag)
				setmdcls(v);
			argv[i] = 0;
		}
	}
	ininternal = 1;
	(void) setvar("$", "$", 0);
	(void) setvar("MACHINE", MACHINE, 0);
	(void) setvar("MAKE", argc ? argv[0] : "make", 0);
	ininternal = 0;
	inarglist = 0;

	/*
	 * Summary of Assigning Macros and Variables
	 * from lower to higher priority.
	 *
	 *	-e not specified	-e specified
	 *	--------------------------------------------
	 *	internal definitions	internal definintions
	 *	environment		makefile(s)
	 *	makefile(s)		environment
	 *	command line		command line
	 */

	/*
	 * Read internal definitions and rules
	 */
	if (!noruleflag) {
		if (dbgflag)
			printf("Reading internal rules.\n");
		ininternal = 1;
		rdd1((FILE *) 0, "internal rules");
		ininternal = 0;
	}

	/*
	 * Read environment args.  Let file args which follow override
	 * unless "envover" variable is set.
	 */
	inarglist = envover;
	readenv();
	inarglist = 0;

	/*
	 * export MAKEFLAGS and MAKEDCLS
	 */
	setenviron(Makeflags, varptr(Makeflags)->varval, 1);
	if ((v = srchvar(Makedcls)) != 0 && v->varval)
		setenviron(v->varname, v->varval, 1);

	/*
	 * MFLAGS=options to make
	 */
	if (strcmp(options, "-") == 0)
		*options = 0;
	(void) setvar("MFLAGS", options, 0);

	if (!noconfig)
		makemove("Makeconf");

	setcocmd();  /* to possibly check-out description file */
	setvpath();
	mainname = 0;

	/*
	 * Read command line "-f" arguments.
	 */
	for (i = 1; i < argc; i++)
		if (argv[i] && strcmp(argv[i], "-f") == 0) {
			argv[i] = 0;
			if (++i >= argc || argv[i] == 0)
				fatal("No description argument after -f flag");
			if (rddescf(argv[i]))
				fatal("Cannot open %s", argv[i]);
			argv[i] = 0;
			++descset;
		}

	if (!descset) {
		if (rddescf("makefile") == 0 || rddescf("Makefile") == 0)
			(void) rddescf(".depend");
		else if (descflag)
			fatal("No description file");
	}

	setvpath();
	setcocmd();  /* may have been redefined in description file */

	addincdirs();

	if (prtrflag)
		printdesc(NO);

	if (srchname(".IGNORE"))
		++ignerr;
	if (srchname(".SILENT"))
		silflag = 1;
	if (p = srchname(".SUFFIXES"))
		sufflist = p->linep;
	if (!sufflist)
		fprintf(stderr, "No suffix list.\n");
	if (p = srchname(".EXPORT"))
		export(p);
	if (didchdir() && (inobjdir = srchname(".INOBJECTDIR")) != 0) {
		doruntime(1, inobjdir);
		doruntime(2, inobjdir);
	}

	sighvalue = (signal(SIGHUP, SIG_IGN) == SIG_IGN);
	sigivalue = (signal(SIGINT, SIG_IGN) == SIG_IGN);
	sigqvalue = (signal(SIGQUIT, SIG_IGN) == SIG_IGN);
	sigtvalue = (signal(SIGTERM, SIG_IGN) == SIG_IGN);
	enbint(intrupt);
	if (dbgflag)
		(void) getrusage(RUSAGE_SELF, &ru1);

	if (p = srchname(".INIT")) {
		ch = 0;
		(void) doname(p, 0, &td, &ch);
	}

	nfargs = 0;

	for (i = 1; i < argc; ++i)
		if (s = argv[i]) {
			p = makename(s);
			ch = 0;
			(void) doname(p, 0, &td, &ch);
			if (prtrflag)
				printdesc(YES);
			argv[i] = 0;
			++nfargs;
		}

	/*
	 * If no file arguments have been encountered, make the first
	 * name encountered that doesn't start with a dot
	 */

	if (nfargs == 0) {
		if (mainname == 0)
			fatal("No arguments or description file");
		ch = 0;
		(void) doname(mainname, 0, &td, &ch);
		if (prtrflag)
			printdesc(YES);
	}

	if (dbgflag) {
		(void) getrusage(RUSAGE_SELF, &ru2);
		(void) getrusage(RUSAGE_CHILDREN, &kids);
		usagesumm(&ru1, &ru2, &kids);
	}
	quit(0);
	/*NOTREACHED*/
}


struct lineblock *
isdependent(p, np)
	register char *p;
	register struct nameblock *np;
{
	register struct lineblock *lp;
	register struct depblock *dp;

	if (np == 0)
		return 0;
	for (lp = np->linep; lp; lp = lp->nxtlineblock)
		for (dp = lp->depp; dp; dp = dp->nxtdepblock)
			if (dp->depname && !unequal(p, dp->depname->namep))
				return lp;
	return 0;
}


int
isprecious(p)
	struct nameblock *p;
{
	static struct nameblock *prec = 0;

	if (prec == 0)
		prec = srchname(".PRECIOUS");
	return (p && (p->isarch || isdependent(p->namep, prec)));
}


FSTATIC int
intrupt(s)
	int s;
{
	char *p;
	struct nameblock *q;
	struct stat sbuf;

	if (!questflag && !touchflag && !noexflag
	&& (p = varptr("@")->varval) && stat(p, &sbuf) == 0
	&& (sbuf.st_mode & S_IFMT) == S_IFREG
	&& ((q = srchname(p)) == 0 || q->modtime != sbuf.st_mtime)) {
		if (!okdel || isprecious(q))
			printf("*** `%s' may be inconsistent\n", p);
		else {
			printf("*** `%s' removed\n", p);
			(void) fflush(stdout);
			(void) unlink(p);
		}
	}
	quit(s);
}


enbint(k)
	int (*k)();
{
	if (sighvalue == 0)
		(void) signal(SIGHUP, k);
	if (sigivalue == 0)
		(void) signal(SIGINT, k);
	if (sigqvalue == 0)
		(void) signal(SIGQUIT, k);
	if (sigtvalue == 0)
		(void) signal(SIGTERM, k);
}


extern FILE *fin;

FSTATIC int
rddescf(descfile)
	char *descfile;
{
	register struct nameblock *np;
	FILE *k;

	/*
	 * read and parse description
	 */
	if (!unequal(descfile, "-")) {
		rdd1(stdin, "standard input");
		return 0;
	}

	np = rcsco(descfile);
	if ((k = fopen(np->alias ? np->alias : descfile, "r")) != NULL) {
		rdd1(k, descfile);
		(void) fclose(k);
		return 0;
	}

	return 1;
}


FSTATIC char MAKEFILE[] = "MAKEFILE";

rdd1(k, fn)
	FILE *k;
	char *fn;
{
	extern int yylineno;
	extern char *zznextc;

	(void) setvar(MAKEFILE, copys(fn), 1);
	curfname = fn;
	fin = k;
	yylineno = 0;
	zznextc = 0;

	if (yyparse())
		fatal("%s: Description file error", fn);
}


FSTATIC
setcocmd()
{
	struct nameblock *p;
	register struct lineblock *lp;

	if (p = srchname(".CO")) {
		for (lp = p->linep; lp; lp = lp->nxtlineblock)
			if (lp->shp) {
				co_cmd = lp;
				break;
			}
		if (co_cmd == 0)
			coflag = NO;
		else if (p = srchname(".RCSTIME"))
			for (lp = p->linep; lp; lp = lp->nxtlineblock)
				if (lp->shp) {
					rcstime_cmd = lp;
					break;
				}
	} else
		coflag = NO;
}


entrybanner(ac, av)
	int ac;
	char **av;
{
	register int i;
	time_t now;
	extern time_t prestime();
	extern char *ctime();

	now = prestime();
	printf("MAKE ENTRY (%s", av[0]);
	for (i = 1; i < ac; ++i)
		printf(" %s", av[i]);
	printf(") at %s", ctime(&now));
}


exitbanner(cc)
	int cc;
{
	time_t now;
	extern time_t prestime();
	extern char *ctime();

	now = prestime();
	printf("MAKE EXIT (%d) at %s", cc, ctime(&now));
}


usagesumm(ru1, ru2, kids)
	struct rusage *ru1, *ru2, *kids;
{
	long init, work, execs, total;

#define ticks(tvp)	(((tvp)->tv_sec*1000)+((tvp)->tv_usec/1000))
#ifndef lint
#define percent(n,d)	((long)(((100.0*(n))/(d))+0.5))
#else
#define percent(n,d)	((long)((100*(n))/(d)))
#endif
	init = ticks(&ru1->ru_utime) + ticks(&ru1->ru_stime);
	work = ticks(&ru2->ru_utime) + ticks(&ru2->ru_stime) - init;
	execs = ticks(&kids->ru_utime) + ticks(&kids->ru_stime);
	total = init + work + execs;
	printf("MAKE TIME %ld%% init, %ld%% work, %ld%% execs\n",
				percent(init,total),
				percent(work,total),
				percent(execs,total));
}


FSTATIC
printdesc(prntflag)
	int prntflag;
{
	struct nameblock *p;
	struct depblock *dp;
	struct dirhdr *od;
	struct shblock *sp;
	struct lineblock *lp;

	if (prntflag) {
		printf("Open directories:\n");
		for (od = firstod; od; od = od->nxtopendir)
			printf("\t%d: %s\n", dirfd(od->dirfc), od->dirn);
		printf("\n");
	}

	if (firstvar) {
		printf("Macros:\n");
		printmac(firstvar);
	}

	for (p = firstname; p; p = p->nxtnameblock) {
		printf("\n%s", p->namep);
		if (p->linep)
			printf(":");
		if (prntflag)
			printf("  done=%u", p->done);
		if (p == mainname)
			printf("  (MAIN NAME)");
		for (lp = p->linep; lp; lp = lp->nxtlineblock) {
			if (dp = lp->depp) {
				printf("\n depends on:");
				for (; dp; dp = dp->nxtdepblock)
					if (dp->depname)
						printf(" %s ", dp->depname->namep);
			}

			if (sp = lp->shp) {
				printf("\n commands:");
				for (; sp; sp = sp->nxtshblock)
					printf("\n\t%s", sp->shbp);
			}
		}
	}
	printf("\n");
}


FSTATIC
printmac(vp)
	register struct varblock *vp;
{
	if (vp->lftvarblock)
		printmac(vp->lftvarblock);
	printf(" %9s = %s\n" , vp->varname , vp->varval);
	if (vp->rgtvarblock)
		printmac(vp->rgtvarblock);
}


/*
 * Handle options and MAKEFLAGS and MFLAGS.
 */
FSTATIC
setflags(ac, av)
	int ac;
	char **av;
{
	register int i, j;
	register char c;
	int edflg = 0;		/* flag to note `-E' option. */
	int flflg = 0;		/* flag to note `-f' option. */

	for (i = 1; i < ac; ++i) {
		if (edflg || flflg) {
			edflg = flflg = 0;
			continue;
		}
		if (av[i] && av[i][0] == '-') {
			if (index(av[i], 'E'))
				edflg++;
			if (index(av[i], 'f'))
				flflg++;
			for (j = 1; c = av[i][j]; ++j)
				optswitch(c);
			if (edflg && flflg)
				fatal("-E and -f flags cannot be grouped");
			av[i] = edflg ? "-E" : flflg ? "-f" : 0;
		}
	}
}


/*
 * Handle a single char option.
 */
FSTATIC
optswitch(c)
	register char c;
{
	switch (c) {
	case 'c':
		coflag = NO;
		setmflgs(c);
		break;
	case 'C':
		coflag = YES;
		setmflgs(c);
		break;
	case 'e':
		envover = YES;
		setmflgs(c);
		break;
	case 'd':
		dbgflag = YES;
		setmflgs(c);
		break;
	case 'p':
		prtrflag = YES;
		break;
	case 's':
		silflag = YES;
		setmflgs(c);
		break;
	case 'i':
		ignerr = YES;
		setmflgs(c);
		break;
	case 'S':
		keepgoing = NO;
		setmflgs(c);
		break;
	case 'k':
		keepgoing = YES;
		setmflgs(c);
		break;
	case 'F':
		descflag = YES;
		setmflgs(c);
		break;
	case 'n':
		noexflag = YES;
		setmflgs(c);
		break;
	case 'r':
		noruleflag = YES;
		break;
	case 'N':
		noconfig = YES;
		setmflgs(c);
		break;
	case 't':
		touchflag = YES;
		setmflgs(c);
		break;
	case 'q':
		questflag = YES;
		setmflgs(c);
		break;
	case 'm':
		machdep = YES;
		setmflgs(c);
		break;
	case 'u':
		rmflag = NO;
		setmflgs(c);
		break;
	case 'U':
		rmflag = YES;
		setmflgs(c);
		break;
	case 'x':
		nocmds = YES;
		setmflgs(c);
		break;
	case 'y':
		rcsdep = YES;
		setmflgs(c);
		break;
	case 'E':
		/*
		 * Exported definition; already handled by setflags().
		 */
		break;
	case 'f':
		/*
		 * Named makefile; already handled by setflags().
		 */
		break;
	case 'b':
		botchflag = YES;
		setmflgs(c);
		break;
	case 'X':
		xportflag = YES;
		setmflgs(c);
		break;
	default:
		fatal("Unknown flag argument %c", c);
	}
}


/*
 * getmflgs() set the cmd line flags into an EXPORTED variable
 * for future invocations of make to read.
 */
FSTATIC
getmflgs()
{
	register struct varblock *vpr;
	register char *p;
	static char mflags[64];  /* XXX */
	extern char *getenv();

	vpr = varptr(Makeflags);
	vpr->varval = mflags;
	vpr->varval[0] = 0;
	vpr->noreset = 1;
	vpr->builtin = 1;
	vpr->used = 1;
	if ((p = getenv(Makeflags)) != 0)
		while (*p)
			optswitch(*p++);
}


/*
 * setmflgs(c) sets up the cmd line input flags for EXPORT.
 * Also adds them to "MFLAGS" for compatibility with 4.3 make.
 */
FSTATIC
setmflgs(c)
	register char c;
{
	register struct varblock *vpr;
	register char *p;

	for (p = options; *p; p++)
		if (*p == c)
			break;
	if (*p == 0) {
		*p++ = c;
		*p = 0;
	}
	vpr = varptr(Makeflags);
	for (p = vpr->varval; *p; p++)
		if (*p == c)
			break;
	if (*p == 0) {
		*p++ = c;
		*p = 0;
	}
}


/*
 * getmdefs() allocate space for command line definitions
 * which may then be passed on to recursive makes.  Note
 * that they are not automatically imported/exported.
 */
FSTATIC
getmdefs()
{
	register struct varblock *vpr;
	static char mdefs[BUFSIZ];  /* XXX */

	vpr = varptr(Makedefs);
	vpr->varval = mdefs;
	vpr->varval[0] = 0;
	vpr->noreset = 1;
	vpr->builtin = 1;
	vpr->used = 0;
}


/*
 * setmdefs(v) adds the command line definition to $(MAKEDEFS).
 */
FSTATIC
setmdefs(v)
	register struct varblock *v;
{
	register struct varblock *vpr;
	register char *p, *q;

	vpr = varptr(Makedefs);
	for (p = vpr->varval; *p; p++)
		;
	if (p != vpr->varval)
		*p++ = ' ';
	*p++ = '\'';
	for (q = v->varname; *q; q++) {
		if (*q == '\'')
			*p++ = *q, *p++ = '\\', *p++ = *q;
		*p++ = *q;
	}
	*p++ = '=';
	for (q = v->varval; *q; q++) {
		if (*q == '\'')
			*p++ = *q, *p++ = '\\', *p++ = *q;
		*p++ = *q;
	}
	*p++ = '\'';
	*p = 0;
}


static char mdcls[BUFSIZ];  /* XXX */

/*
 * getmdcls() allocate space for automatically imported/exported
 * variable names, and mark them as command line arguments.
 */
FSTATIC
getmdcls()
{
	register struct varblock *vpr;
	register char *p, *q, c;
	extern char *getenv();

	vpr = varptr(Makedcls);
	vpr->noreset = 1;
	vpr->builtin = 1;
	vpr->used = 1;
	if ((q = getenv(Makedcls)) != 0) {
		p = strcpy(vpr->varval = mdcls, q);
		for (;;) {
			q = p;
			while (*p && *p != '=')
				p++;
			c = *p;
			*p = 0;
			vpr = setvar(q, (char *) 0, 0);
			vpr->envover = 1;
			if (c == 0)
				break;
			*p++ = c;
		}
	}
}


/*
 * setmdcls(v) adds the exported variable name to $(MAKEDCLS) and its
 * value to the environment, where they can be picked up by sub-makes.
 */
FSTATIC
setmdcls(v)
	register struct varblock *v;
{
	register struct varblock *vpr;
	register char *p, *q;
	register int l = v->varlen;

	setenviron(v->varname, v->varval, 1);
	vpr = varptr(Makedcls);
	if ((p = vpr->varval) == 0)
		(void) strcpy(vpr->varval = mdcls, v->varname);
	else {
		for (q = p;; q = ++p) {
			while (*p && *p != '=')
				p++;
			if ((p - q) == l && strncmp(q, v->varname, l) == 0)
				break;
			if (*p == 0) {
				*p++ = '=';
				(void) strcpy(p, v->varname);
				break;
			}
		}
	}
}
