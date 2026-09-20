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
 * $Log:	main.c,v $
 * Revision 2.7  94/03/25  18:23:02  mrt
 * 	Added code to parse input args from the kernel and
 * 	to set the root device dynamically.
 * 	Added different type for time() to match
 * 	 __STDC__ include files.
 * 	[93/11/17            mrt]
 * 
 * Revision 2.6  92/02/02  13:02:30  rpd
 * 	Fixed vm_map argument types, time function type.
 * 	Changed to use mapped_time_value_t.
 * 	Removed mach_privileged_host_port, mach_device_server_port.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.5  91/12/19  20:28:30  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  90/12/04  21:55:43  rpd
 * 	Removed reaper thread.
 * 	[90/12/04            rpd]
 * 
 * Revision 2.3  90/09/27  13:55:08  rwd
 * 	Change exit to _exit to not pick up version in libmach_sa.a
 * 	Remove mach_port_allocate_name since library has been fixed.
 * 	[90/09/24            rwd]
 * 	Add setlinebuf for !STANDALONE case.
 * 	[90/09/19            rwd]
 * 	Fix signal code for i386.
 * 	[90/09/08            rwd]
 * 
 * Revision 2.2  90/09/08  00:19:12  rwd
 * 		Define mach_port_allocate_name until bug fix to libmach_sa
 * 		propogates.
 * 	[90/09/03            rwd]
 * 	Added mapped time support.
 * 	[90/08/13            rwd]
 * 
 * 	Added standard way to get ports plus standalone version.
 * 	[90/07/13            rwd]
 * 
 */
/*
 *	File:	./main.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <config.h>
#include <mach.h>
#include <mach/message.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>
#include <ux_user.h>
#include <sys/reboot.h>
#include <device/device_types.h>
#include <machine/spec.h>

extern char version[];
mach_port_t privileged_host_port;
mach_port_t host_port;
mach_port_t device_server_port;
int add_crs = 1;
int boothowto;	/* boot flags, passed in as an arg from the kernel */

dev_t parse_root_device();

get_privileged_ports()
{
	mach_port_t	bootstrap_port;
	mach_port_t	reply_port;
	kern_return_t	result;

	struct imsg {
	    mach_msg_header_t	hdr;
	    mach_msg_type_t	port_desc_1;
	    mach_port_t		port_1;
	    mach_msg_type_t	port_desc_2;
	    mach_port_t		port_2;
	} imsg;

	/*
	 * Get our bootstrap port
	 */
	result = task_get_bootstrap_port(mach_task_self(), &bootstrap_port);
	if (result != KERN_SUCCESS)
	    panic("get bootstrap port %d", result);

	/*
	 * Allocate a reply port
	 */
	reply_port = mig_get_reply_port();
	if (reply_port == MACH_PORT_NULL)
	    panic("allocate reply port");

	/*
	 * Send a message to it, asking for the host and device ports
	 */
	imsg.hdr.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
					    MACH_MSG_TYPE_MAKE_SEND_ONCE);
	imsg.hdr.msgh_size = 0;
	imsg.hdr.msgh_remote_port = bootstrap_port;
	imsg.hdr.msgh_local_port = reply_port;
	imsg.hdr.msgh_id = 999999;

	result = mach_msg(&imsg.hdr, MACH_SEND_MSG|MACH_RCV_MSG,
			  sizeof imsg.hdr, sizeof imsg, reply_port,
			  MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if (result != MACH_MSG_SUCCESS)
	    panic("mach_msg");

	privileged_host_port = imsg.port_1;
	device_server_port = imsg.port_2;
	host_port = mach_host_self();
}

mapped_time_value_t *mtime = 0;

init_mapped_time()
{
        kern_return_t rc;
        mach_port_t device_port, pager = MACH_PORT_NULL;

        rc = device_open(device_server_port,0,"time",&device_port);
        if (rc != D_SUCCESS) panic("unable to open device time");

        rc = device_map(device_port, VM_PROT_READ,
                        0, sizeof(time_value_t), &pager, 0);
        if (rc != D_SUCCESS) panic("unable to map device time");
        if (pager == MACH_PORT_NULL) panic("unable to map device time");

        rc = vm_map(mach_task_self(),
		    (vm_offset_t *) &mtime, sizeof(mapped_time_value_t),
		    0, TRUE, pager, 0, 0, VM_PROT_READ,
                    VM_PROT_READ, VM_INHERIT_SHARE);
        if (rc != D_SUCCESS) panic("unable to vm_map device time");

        rc = mach_port_deallocate(mach_task_self(), pager);
        if (rc != KERN_SUCCESS) panic("unable to deallocate pager");
}

gettimeofday(tp, tzp)
struct timeval *tp;
struct timezone *tzp;
{
	*tp = *(struct timeval *)mtime;
/*XXX*/
	tzp->tz_minuteswest = 300;
	tzp->tz_dsttime = 1;
}

#ifdef __STDC__
time_t time(t)
time_t *t;
{
	time_t nt = mtime->seconds;
#else /* not __STDC__ */

long time(t)
long *t;
{
	long nt = mtime->seconds;
#endif /* __STDC__ */
	if (t) *t = nt;
	return nt;
}


tv_sleep(sec, usec)
	int sec;
	int usec;
{
	mach_msg_header_t	m;
	mach_port_t		sleep_port;
	int			init = 0;
	int			t;

	if (!init) {
	    init = 1;
	    (void) mach_port_allocate(mach_task_self(),
				      MACH_PORT_RIGHT_RECEIVE,
				      &sleep_port);
	    (void) mach_port_insert_right(mach_task_self(),
					  sleep_port, sleep_port,
					  MACH_MSG_TYPE_MAKE_SEND);
	}
	t = usec/1000 + sec*1000;

	(void) mach_msg(&m, MACH_RCV_MSG|MACH_RCV_TIMEOUT,
			0, sizeof m, sleep_port,
			t, MACH_PORT_NULL);

}

server_shutdown_user(ut)
	struct ux_task *ut;
{
	int error;

	if (! ut->ut_dead) {
		error = task_terminate(ut->ut_task);
		printf("shutdown_user(pid=%d) : error %d\r\n",
			ut->ut_pid, error);
	}
}

char bogus_c;

server_shutdown(sig, code)
	int sig;
	int code;
{
	if (sig == SIGINT) {
		bogus_c = 'C'-'@';
		return;
	}
	if (sig == SIGTSTP) {
		bogus_c = 'Z'-'@';
		return;
	}
	if (sig != SIGQUIT && sig != SIGTERM) {
		printf("server_shutdown: sig=%d, code=0x%x\r\n", sig, code);
	}
	foreach_ux_user(server_shutdown_user);
	poe_exit(0, "Server shutdown", 0);
}

#if	!STANDALONE
#ifdef	i386
#define	sc_pc sc_eip
#endif	i386
#include <stdio.h>
got_signal(sig, code, scp)
	int sig;
	int code;
	struct sigcontext *scp;
{
	switch (sig) {
	    default:
		printf("signal(%d) code=%d  pc=x0%x\r\n", sig, code, scp->sc_pc);
	}
	panic("external signal");
}

set_signals()
{
	signal(SIGHUP, got_signal);
	signal(SIGINT, got_signal);
	signal(SIGQUIT, got_signal);
	signal(SIGILL, got_signal);
	signal(SIGTRAP, got_signal);
	signal(SIGIOT, got_signal);
	signal(SIGEMT, got_signal);
	signal(SIGBUS, got_signal);
	signal(SIGSEGV, got_signal);
	signal(SIGTERM, got_signal);
	signal(SIGSTOP, got_signal);
	signal(SIGTSTP, got_signal);
	setlinebuf(stdout);
	setlinebuf(stderr);
}

signal_thread()
{
	cthread_set_kernel_limit(cthread_kernel_limit() + 1);
	cthread_wire();
	set_signals();
	while(1)sleep(10000);
}

#endif	!STANDALONE
main(argc, argv)
	int argc;
	char **argv;
{
/* 
 *    The args to main are: 
 *	0 - program name
 *	1 - bootflags,
 *	2 - root partition name , eg hd0a,
 *	3 - server directory - eg. /dev/hd0a/mach_servers
 */
	char *getenv(), *PATH, *TERM;
	char pathname[1024];
	extern int silent;
	int sig, error;
        extern char emulator_path[];
        extern char emulator_name[];
        extern char init_path[];
        extern char init_name[];
	extern char root_name[];
	extern dev_t root_dev;

	cthread_set_kernel_limit(2);

#if	!STANDALONE
	cthread_detach(cthread_fork(signal_thread, 0));
#endif	!STANDALONE

	bsd_master_init();
	get_privileged_ports();
	printf_init();
	init_mapped_time();

	printf("%s\n\r",version);

/* parse args */

	/*
	 * Arg 0 is program name
	 */
	argv++, argc--;

	/*
	 * Arg 1 should be flags
	 */
	if (argc == 0)
	    return;

	if (argv[0][0] == '-') {
	    register char *cp = argv[0];
	    register char c;

	    printf("bootflags are: %s\n",argv[0]);

	    while ((c = *cp++) != '\0') {
		switch (c) {
		    case 'a':
			boothowto |= RB_ASKNAME;
			break;
		    case 's':
			boothowto |= RB_SINGLE;
			break;
		    case 'd':
			boothowto |= RB_KDB;
			break;
		    case 'n':
			boothowto |= RB_INITNAME;
			break;
		    case 'v':
			break;
		}
	    }
	    argv++, argc--;
	}
	/*
	 * Arg 2 should be root name - hd0a
	 * Arg 3 should be server_dir_name (3.0 style) /dev/hd0a/mach_servers
	 */
	if (argc == 0)
	    return;
	strcpy(root_name,argv[0]);
	printf("root device is %s\n",root_name);
	if (bcmp(argv[1], "/dev/", 5)) {
		dprintf("(startup): server_dir(%s) ignored.  It does not start with /dev/\r\n",
			argv[1]);
		dprintf("(startup): TILT TILT!\r\n");
	} else {
		int len = strlen(argv[0]);

		if (bcmp(argv[1]+5, argv[0], len)) {
			char *cp = argv[1]+5;
			while (*cp++ != '/') ;
			len = cp - (argv[1] + 5) - 1;

			dprintf("(startup): WARNING! specified server_dir(%s) NOT on root(%s)!\r\n",
				argv[1], argv[0]);
			dprintf("(startup): %s and %s will be taken from %s on the localroot(%s)!\r\n",
				init_name, emulator_name, argv[1]+5+len, argv[0]);
		} else {
			dprintf("(startup): server_dir(%s) on root.\r\n",
				argv[1]+5+len);
		}
		strcpy(emulator_path, argv[1]+5+len);
		strcat(emulator_path, emulator_name);
		strcpy(init_path, argv[1]+5+len);
		strcat(init_path, init_name);
	}
	printf("(startup): emulator_path(%s)\r\n", emulator_path);
	printf("(startup): init_path(%s)\r\n", init_path);

	root_dev = parse_root_device(root_name);

	server_init();

	/*
	 * filesystem stuff
	 */

	ufs_devpager_init();
	ufs_pager_init();

	error = ufs_mountroot(root_name,root_dev);
	if (error) {
		poe_exit(1, "ufs_mountroot", error);
	}
	error = rfs_walk();
	if (error) {
		poe_exit(1, "rfs_walk", error);
	}
	
	error = spawn();
	if (error) {
		unix_error("spawn", error);
		poe_exit(0, "unix error spawn", error);
	}

	{
	    struct mutex kill_mutex;
	    struct condition kill_condition;
	    mutex_init(&kill_mutex);
	    condition_init(&kill_condition);
	    mutex_lock(&kill_mutex);
	    condition_wait(&kill_condition, &kill_mutex);
	}
	/* NOTREACHED */
}

poe_exit(i, s, e)
int	i,e;
char	*s;
{
#if	STANDALONE
	host_reboot(privileged_host_port, 0x1004);	/*XXX*/
#else
	printf("%s: error = %d\n", s, e);
	_exit(i);
#endif	STANDALONE
}


/*
 * Parse root device name into a block device number.
 */
dev_t
parse_root_device(str)
	char	*str;
{
	register char c;
	char tmpstr[8];
	register char *cp = tmpstr;
	char *name_end;
	register int partition_num = 0;
	register int minor_num = 0;
	register int major_num;
	register struct special_switch_struct *bdp;

	strcpy(tmpstr,str);
	/*
	 * Find device type name (characters before digit)
	 */
	while ((c = *cp) != '\0' &&
		!(c >= '0' && c <= '9'))
	    cp++;
	name_end = cp;

	if (c != '\0') {
	    /*
	     * Find minor_num number
	     */
	    while ((c = *cp) != '\0' &&
		    c >= '0' && c <= '9') {
		minor_num = minor_num * 10 + (c - '0');
		cp++;
	    }
	}

	if (c >= 'a' && c <= 'h') {
	    /*
	     * Find partition number
	     */
	    partition_num = c - 'a';
	}
	*name_end = 0;	/* terminate string at <name> for lookup */

	for (major_num = 0, bdp = block_spec;
	     major_num < nblkdev;
	     major_num++, bdp++) {
	    if (!strcmp(str, bdp->name))
		break;
	}
	if (major_num == nblkdev)
	    /* not found */
	    return ((dev_t)0);

	/*
	 * Disk minor number is ROOT_DEVICE_DIVISOR*minor_num + partition_num,
	 * where ROOT_DEVICE_DIVISOR is normally 8 but is 16 for an i386
	 */
	minor_num = minor_num * ROOT_DEVICE_DIVISOR + partition_num;

	return (makedev(major_num, minor_num));
}
