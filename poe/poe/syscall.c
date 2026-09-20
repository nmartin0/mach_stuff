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
 * $Log:	syscall.c,v $
 * Revision 2.4  92/02/02  13:02:50  rpd
 * 	Removed old IPC vestiges.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.3  91/12/19  20:29:38  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:21:03  rwd
 * 	Syscall tracing now prints cthread_self also.
 * 	[90/09/04            rwd]
 * 	Fully convert to new IPC structures.
 * 	[90/07/19            rwd]
 * 
 */
/*
 *	File:	./syscall.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <mach/message.h>
#include <mig_errors.h>
#include <bsd_msg.h>
#include <sysent.h>
#include <ux_user.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <errno.h>

extern int		add_crs;
extern int		silent;
extern int		Bsd_nothing();
extern struct sysent	sysent[];
extern int		nsysent;

static struct sysent bad_sysent = { Bsd_nothing, 0, 0 };

copyin_things(ut, syscode, se, argv, stringsizev)
	struct ux_task *ut;
	int syscode;
	struct sysent *se;
	int *argv;
	int *stringsizev;
{
	int i, error;
	char *s;

	if (se == &bad_sysent) {
		return EINVAL; /*XXX*/
	}
	for (i = 0, s = se->fmt; s && *s; i++, s++) {
		if (*s == 's') {
			error = copyin_string(ut, argv[i], &argv[i]);
			if (error) {
				return error;
			}
		}
	}
	return 0;
}

uncopyin_things(argv, stringsizev)
	int *argv;
	int *stringsizev;
{
	int i;

	for (i = 0; i < 6; i++) {
		if (stringsizev[i]) {
			vm_deallocate(mach_task_self(), trunc_page(argv[i]),
				      stringsizev[i]);
		}
	}
	return 0;
}

print_syscall(ut, syscode, se, argv)
	struct ux_task *ut;
	int syscode;
	struct sysent *se;
	int *argv;
{
	int i;
	char *s;

	if (silent) {
		return;
	}
	printf("[%x][%d]", cthread_self(), ut->ut_pid);
	if (se == &bad_sysent) {
		printf("[%d](0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			syscode,
			argv[0], argv[1], argv[2], 
			argv[3], argv[4], argv[5]);
		return;
	}
	printf("%s(", se->name);
	for (i = 0, s = se->fmt; s && *s; i++, s++) {
		if (*s == 'x') {
			printf("0x%x", argv[i]);
		} else if (*s == 'd') {
			printf("%d", argv[i]);
		} else if (*s == 'o') {
			printf("0%o", argv[i]);
		} else if (*s == 'I') {
			/* ioctl request */
			unsigned long rw = argv[i] & IOC_INOUT;
			unsigned short size = (argv[i] >> 16) & IOCPARM_MASK;
			unsigned short type = (argv[i] & 0xff00) >> 8;
			unsigned short command = (argv[i] & 0xff);

			if ((argv[i] >> 16) == 0) {
				printf("[OLD,");
			} else {
				char *rws;
				if (rw == IOC_IN) {
					rws = "r";
				} else if (rw == IOC_OUT) {
					rws = "w";
				} else if (rw == IOC_INOUT) {
					rws = "rw";
				} else {
					rws = "void";
				}
				printf("[%s,size=%d,'", rws, size);
			}
			if (type >= ' ' && type < 127) {
				printf("%c", type);
			} else {
				printf("\0%o", type);
			}
			printf("',cmd=%d]", command);
		} else if (*s == 's') {
			printf("\"%s\"", argv[i]);
		} else {
			printf("%%?");
		}
		if (s[1]) {
			printf(", ");
		}
	}
	printf(")");
}

print_syscall_value(error, rval1)
	int error;
	int rval1;
{
	if (silent) {
		return;
	}
	if (error) {
		unix_error(" ", error);
	} else {
		printf(" = %d\n", rval1);
	}
}

/*
 * Generic UX server message handler.
 * Generic system call.
 */
boolean_t
ux_generic_server(ut, InHeadP, OutHeadP)
	struct ux_task *ut;
	mach_msg_header_t *InHeadP, *OutHeadP;
{
	register struct bsd_request	*req = (struct bsd_request *)InHeadP;
	register struct bsd_reply	*rep = (struct bsd_reply *)OutHeadP;
	struct sysent *se;
	int ssv[6];

	/*
	 * Fix the reply message.
	 */
	static mach_msg_type_t bsd_rep_int_type = {
	    /* msgt_name */		MACH_MSG_TYPE_INTEGER_32,
	    /* msgt_size */		32,
	    /* msgt_number */		4,
	    /* msgt_inline */		TRUE,
	    /* msgt_longform */		FALSE,
	    /* msgt_deallocate */	FALSE,
	    /* msgt_unused */		0
	};

	/*
	 * Set up standard reply.
	 */
	rep->hdr.msgh_bits =
		MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(InHeadP->msgh_bits), 0);
	rep->hdr.msgh_remote_port = InHeadP->msgh_remote_port;
	rep->hdr.msgh_local_port = MACH_PORT_NULL;
	rep->hdr.msgh_id = InHeadP->msgh_id + 100;

	if (InHeadP->msgh_id != BSD_REQ_MSG_ID) {
	    static mach_msg_type_t RetCodeType = {
	        /* msgt_name */			MACH_MSG_TYPE_INTEGER_32,
	     	/* msgt_size */			32,
		/* msgt_number */		1,
	    	/* msgt_inline */		TRUE,
	    	/* msgt_longform */		FALSE,
	    	/* msgt_deallocate */		FALSE,
		/* msgt_unused */		0
	    };
	    mig_reply_header_t *OutP = (mig_reply_header_t *)OutHeadP;
	    OutP->RetCodeType = RetCodeType;
	    OutP->RetCode = MIG_BAD_ID;
	    OutP->Head.msgh_size = sizeof *OutP;
	    return (FALSE);
	}

	rep->int_type = bsd_rep_int_type;
	rep->hdr.msgh_size = sizeof(struct bsd_reply);

	if (ut->ut_sigtake) {
		dprintf("[%d]syscall gets sigtake(%d)\n",
			ut->ut_pid, ut->ut_sigtake);
		rep->retcode = ERESTART;
		return TRUE;
	}

	if (req->syscode >= 0 && req->syscode < nsysent) {
		se = &sysent[req->syscode];
	} else {
		se = &bad_sysent;
	}

	bzero(ssv, sizeof(ssv));
	rep->retcode = copyin_things(ut, req->syscode, se, req->arg, ssv);
	if (rep->retcode) {
		dprintf("[?]copyin_things.error %d %d\n", rep->retcode,
			req->syscode);
		return TRUE;
	}
	print_syscall(ut, req->syscode, se, req->arg);
	rep->rval[0] = 0;
	rep->rval[1] = req->rval2;
	rep->retcode = (*se->func)(ut, rep->rval,
				   req->arg[0], req->arg[1], req->arg[2],
				   req->arg[3], req->arg[4], req->arg[5]);
	print_syscall_value(rep->retcode, rep->rval[0]);
	rep->interrupt = (ut->ut_sigtake != 0);
	uncopyin_things(req->arg, ssv);
	return (TRUE);
}

ut_lookup(routine_name, proc_port, utp, interrupt)
	char *routine_name;
	mach_port_t proc_port;
	struct ux_task **utp;
	boolean_t *interrupt;
{
	struct ux_task *ut;
	int error;

	if (interrupt) {
		*interrupt = FALSE;
	}
	ut = lookup_ux_task(proc_port);
	if (! ut) {
		dprintf("[?]%s()\n", routine_name);
		return EINVAL;
	}
	if (interrupt && ut->ut_sigtake) {
		dprintf("[%d]%s() gets sigtake(%d)\n",
			ut->ut_pid, routine_name, ut->ut_sigtake);
		*interrupt = TRUE; /* XXX necessary? */
		return ERESTART;
	}
	dprintf("[%d]%s", ut->ut_pid, routine_name); /* it prints rest */
	*utp = ut;
	return 0;
}
