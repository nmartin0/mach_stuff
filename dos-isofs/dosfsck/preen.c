/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * HISTORY
 * $Log:	preen.c,v $
 * Revision 2.1.1.1  93/08/27  11:51:03  af
 * 	Created.
 * 	[93/07/26            af]
 * 
 */
/*
 * This is compat with old fsck
 */
#include <strings.h>
#include <stdio.h>
#include <fstab.h>
#include <sys/wait.h>
#ifndef	WEXITSTATUS
#define	WEXITSTATUS(x)	x.w_retcode
#define	WTERMSIG(x)	x.w_termsig
#endif
#include <sys/stat.h>

char	*mayberaw( struct fstab *);
char	NoFstab[] = "Can't open checklist file: %s\n";
char	BadDisk[] = "BAD DISK NAME %s\n";
char	NoMem[] = "Out of memory\n";
char	NotMine[] = "Got status for unknown process %d\n";
char	SigExit[] = "%s (%s): EXITED WITH SIGNAL %d\n";
char	BadNews[] = "THE FOLLOWING FILE SYSTEMS HAD AN UNEXPECTED INCONSISTENCY:\n\t";
char	NoStat[] = "Can't stat %s\n";
char	NotChardev[] = "%s is not a character device\n";

void	wait_children(int how_many);
char *	savestring(char *str);
void *	malloc();


int
checkfstab(
	int	preen,
	int	maxprocs,
	int	(*test)(struct fstab *),
	int	(*fsck)(char *))
{
	struct fstab	*fsp;
	int		status = 0;
	char		*fname;

	/*
	 * First pass, sequentially
	 */
	if (setfsent() == 0) {
		fprintf(stderr,NoFstab, FSTAB);
		return (8);
	}
	while ((fsp = getfsent()) != NULL) {
		if (((*test)(fsp) == 0) || fsp->fs_passno > 1)
			continue;
		fname = mayberaw(fsp);
		if (fname) {
			status = (*fsck)( fname );
			if (status)
				goto badexit;
		} else if (preen) {
			status = 8;
			goto badexit;
		}
	}

	/*
	 * Second pass, possibly parallel
	 */
	(void) setfsent();
	while ((fsp = getfsent()) != 0) {
		if (((*test)(fsp) == 0) || fsp->fs_passno < 2)
			continue;
		fname = mayberaw(fsp);
		if (fname)
			status |= schedule( preen, maxprocs, fsck,
					fname, savestring(fsp->fs_file));
		else {
			status |= 8;
			fprintf(stderr,BadDisk, fsp->fs_spec);
		}
		if (!preen && status)
			goto badexit;
	}
	endfsent();

	if (!preen)
		return status;

	/*
	 * Now wait for children to be done
	 */
	wait_children(maxprocs);

	/*
	 * Look at what they did
	 */
	return check_results();
badexit:
	endfsent();
	return (status);
}

/*
 * Start a parallel fsck
 */
struct worker {
	struct worker *next;
	char	*fname;
	char	*mname;
	int	id;
	int	result;
} *children;
int nchildren, nbad;

int
schedule(
	int	parallel,
	int	max_active,
	int	(*fsck)(char *),
	char	*fname,
	char	*mname)
{
	struct worker	*this;

	if (!parallel)
		return (*fsck)(fname);

	while (nchildren >= max_active)
		wait_children(1);

	this = (struct worker *)malloc(sizeof(*this));
	if (this == NULL) {
		fprintf(stderr,NoMem);
		return (8);
	}

	/*
	 * Start it
	 */
	this->next = children;
	this->fname = fname;
	this->mname = mname;
	this->result = 0;
	if ((this->id = fork()) == 0)
		exit((*fsck)(fname));
	if (this->id < 0) {
		perror("fork");
		return (8);
	}

	/*
	 * In active list
	 */
	children = this;
	nchildren++;
	/*
	 * Old fsck used to sleep for 10 secs
	 * in between forks. We wont.
	 */
}

/*
 * Wait for one or more children
 * to be done fscking
 */
void
wait_children(
	int	how_many)
{
	register struct worker	*this;

	while (nchildren > 0 && how_many > 0) {
		union wait	result;
		int		who;

		who = wait(&result);
		if (who == -1)
			break;

		for (this = children; this; this = this->next)
			if (this->id == who)
				break;
		if (!this) {
			printf(NotMine, who);
			continue;
		}

		this->result = (WIFEXITED(result)) ? WEXITSTATUS(result) : 0;
		if (WIFSIGNALED(result)) {
			this->result = 8;
			printf(SigExit,this->fname, this->mname, WTERMSIG(result));
		}

		nchildren--;
		how_many--;

		if (this->result != 0)
			nbad++;
	}
}

/*
 * Warn user if anything wrong
 */
int
check_results()
{
	struct worker	*this, *next;
	int		status = 0;

	this = children;
	children = NULL;

	if (nbad)
		fprintf(stderr,BadNews);

	for (; this; this = next) {
		next = this->next;

		status |= this->result;

		if (this->result)
			fprintf(stderr,"%s (%s)%s",
				this->fname, this->mname,
				(nbad-- > 1) ? ", " : "\n");

		free(this->fname);
		free(this->mname);
		free(this);
	}
	return status;
}

/*
 * See if we can use the raw
 * device instead of the block
 * device one.
 */
char *
mayberaw(
	struct fstab	*fsp)
{
	struct stat	fstat, mstat, rstat;
	char		*rname = NULL;
	register int	len;

	if (stat(fsp->fs_spec, &fstat) < 0){
		perror(fsp->fs_spec);
		printf(NoStat, fsp->fs_spec);
		return 0;
	}
	if ((fstat.st_mode & S_IFMT) != S_IFBLK)
		/*
		 * Old fsck insisted this was wrong (sure) and
		 * then looked up the block device instead and
		 * then.. make it raw again. Silly.
		 */
		goto nope;

	/*
	 * Make up raw name
	 */
	len = strlen(fsp->fs_spec);
	rname = (char *)malloc(len + 2);
	strcpy(rname,fsp->fs_spec);
	{
		register char *p, *q;

		rname[len+1] = 0;
		p = &rname[len];
		q = p - 1;
		while (q >= rname) {
			if (*q == '/')
				break;
			*p-- = *q--;
		}
		q[1] = 'r';
	}
	/*
	 * See if it exists, and is a char dev
	 */
	if (((len = stat(rname, &rstat)) < 0) ||
	    ((rstat.st_mode & S_IFMT) != S_IFCHR)) {
		if (len < 0)
			perror(rname);
		else
			printf(NotChardev,rname);
		goto nope;
	}
#if 0
/* kernel must be fixed not to hash on block dev vnode */
	/*
	 * Check if mounted. If mount point bad assume ok.
	 */
	if (stat(fsp->fs_file, &mstat) < 0) {
		perror(fsp->fs_file);
		printf(NoStat, fsp->fs_file);
		return rname;
	}
	/*
	 * check the block device dev against the mount point real dev
	 */
	if (fstat.st_rdev == mstat.st_dev)
		goto nope;
#endif
	return rname;
nope:
	if (rname)
		free(rname);
	return savestring(fsp->fs_spec);
}

/*
 * Copy a string
 */
char *
savestring(
	char	*str)
{
	char	*ret;

	ret = (char *)malloc(strlen(str + 1));
	if (ret == NULL) {
		fprintf(stderr,NoMem);
		return NULL;
	}
	strcpy(ret,str);
	return ret;
}

/*
 * My fstab handling cuz mja is slow.
 * man page sez these are static, so they are.
 */
static FILE *ffstab;
static struct fstab fstab;
static char line[1024]; /* big nuf */

int
setfsent()
{
	if (ffstab)
		rewind(ffstab);
	else
		ffstab = fopen(FSTAB, "r");
	return (ffstab != NULL);
}

static char *
gcolon(
	register char	*str)
{
	register int	c;
	do {
		c = *str++;
	} while (c && (c != ':') && (c != '\n'));
	return (c == ':') ? (str-1) : NULL;
}

struct fstab *
getfsent()
{
	register char	*p;

	if (!ffstab)
		return (NULL);
	p = line;
	do {
		if (fgets(line, sizeof line, ffstab) == NULL)
			return (NULL);
	} while (*p == '#');

	fstab.fs_spec = p;
	if ((p = gcolon(line)) == NULL)
		return NULL;
	*p++ = 0;

	fstab.fs_file = p;
	if ((p = gcolon(p)) == NULL)
		return NULL;
	*p++ = 0;

	fstab.fs_type = p;
	if ((p = gcolon(p)) == NULL)
		return NULL;
	*p++ = 0;

	fstab.fs_freq = atoi(p);
	if ((p = gcolon(p)) == NULL)
		return NULL;
	*p++ = 0;

	fstab.fs_passno = atoi(p);
	if ((p = gcolon(p)) == NULL)
		return NULL;
	*p++ = 0;

	fstab.fs_vfstype = p;
	if ((p = gcolon(p)) == NULL)
		return NULL;
	*p++ = 0;

	fstab.fs_options = p;
	if ((p = gcolon(p)) == NULL)
		fstab.fs_options = NULL;
	else
		*p++ = 0;
	return (&fstab);
}

int
endfsent()
{
	int ret = 0;
	if (ffstab) {
		ret = fclose(ffstab);
		ffstab = NULL;
	}
	return (ret);
}

