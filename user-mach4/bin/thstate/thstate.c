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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: thstate.c,v $
 * Revision 1.2  1995/08/11  15:29:16  mike
 * add parisc support
 *
 * Revision 1.1.1.1  1995/05/04  06:56:46  sclawson
 * New files.
 *
 * Revision 2.4  93/04/14  11:45:14  mrt
 * 	Added sun4 changes from dlc.
 * 	[93/01/08            berman]
 * 
 * Revision 2.3  93/04/09  14:04:18  mrt
 * 	Added alpha.
 * 	[92/12/02            af]
 * 
 * Revision 2.2  92/01/22  23:07:51  rpd
 * 	Moved to a separate directory.
 * 	[92/01/22            rpd]
 * 
 * Revision 2.6  92/01/17  14:24:57  rpd
 * 	Updated for CountInOut.
 * 	[92/01/17  14:13:03  rpd]
 * 
 * Revision 2.5  91/08/29  15:49:31  rpd
 * 	Moved machid include files into the standard include directory.
 * 	[91/08/29            rpd]
 * 
 * Revision 2.4  91/03/27  17:27:51  mrt
 * 	Changed mach.h include
 * 
 * Revision 2.3  91/03/19  12:32:37  mrt
 * 	Changed to new copyright
 * 
 * Revision 2.2  90/09/12  16:33:04  rpd
 * 	Enhanced to do the right thing with tasks and multiple ids.
 * 	[90/09/12            rpd]
 * 
 * 	Created.
 * 	[90/06/18            rpd]
 * 
 */

#include <stdio.h>
#include <mach.h>
#include <mach/message.h>
#include <mach_error.h>
#include <servers/machid.h>
#include <servers/machid_types.h>
#include <servers/machid_lib.h>

#define streql(a, b)	(strcmp((a), (b)) == 0)

mach_port_t machid_server_port;
mach_port_t machid_auth_port;

static void machine_thread_state();
static void mips_thread_state();
static void sun3_thread_state();
static void sparc_thread_state();
static void vax_thread_state();
static void i386_thread_state();

static void
usage()
{
    quit(1, "usage: thstate [-host machine] ids...\n");
}

main(argc, argv)
    int argc;
    char *argv[];
{
    char *hostname = "";
    int i, j;
    kern_return_t kr;

    for (i = 1; i < argc; i++)
	if (streql(argv[i], "-host") && (i < argc-1))
	    hostname = argv[++i];
	else if (streql(argv[i], "--")) {
	    i++;
	    break;
	} else if (argv[i][0] == '-')
	    usage();
	else
	    break;

    argc -= i;
    argv += i;

    kr = netname_look_up(name_server_port, hostname, "MachID",
			 &machid_server_port);
    if (kr != KERN_SUCCESS)
	quit(1, "thstate: netname_lookup_up(MachID): %s\n",
	     mach_error_string(kr));

    machid_auth_port = mach_host_priv_self();
    if (machid_auth_port == MACH_PORT_NULL)
	machid_auth_port = mach_task_self();

    for (i = 0; i < argc; i++) {
	mach_id_t id = atoi(argv[i]), origid = id;
	mach_type_t type;
	mthread_t threads_buf[1024];
	mthread_t *threads = threads_buf;
	natural_t threadCnt = sizeof threads_buf/sizeof threads_buf[0];

	kr = machid_mach_type(machid_server_port, machid_auth_port, id, &type);
	if (kr != KERN_SUCCESS)
	    quit(1, "thstate: machid_mach_type: %s\n", mach_error_string(kr));

	switch (type) {
	  case MACH_TYPE_NONE:
	    continue;

	  default:
	    kr = machid_mach_lookup(machid_server_port, machid_auth_port,
				    id, MACH_TYPE_THREAD, &id);
	    if (kr != KERN_SUCCESS)
		quit(1, "ms: machid_mach_lookup: %s\n", mach_error_string(kr));

	    if (id == 0)
		goto badtype;
	    /* fall-through */

	  case MACH_TYPE_THREAD:
	    machine_thread_state(id);
	    continue;

	  case MACH_TYPE_TASK:
	    kr = machid_task_threads(machid_server_port, machid_auth_port,
				     id, &threads, &threadCnt);
	    break;

	  case MACH_TYPE_PROCESSOR_SET_NAME:
	    kr = machid_mach_lookup(machid_server_port, machid_auth_port,
				    id, MACH_TYPE_PROCESSOR_SET, &id);
	    if (kr != KERN_SUCCESS)
		quit(1, "ms: machid_mach_lookup: %s\n", mach_error_string(kr));

	    if (id == 0)
		goto badtype;
	    /* fall-through */

	  case MACH_TYPE_PROCESSOR_SET:
	    kr = machid_processor_set_threads(machid_server_port,
					      machid_auth_port,
					      id, &threads, &threadCnt);
	    break;

	  case MACH_TYPE_HOST:
	    kr = machid_mach_lookup(machid_server_port, machid_auth_port,
				    id, MACH_TYPE_HOST_PRIV, &id);
	    if (kr != KERN_SUCCESS)
		quit(1, "ms: machid_mach_lookup: %s\n", mach_error_string(kr));

	    if (id == 0)
		goto badtype;
	    /* fall-through */

	  case MACH_TYPE_HOST_PRIV:
	    kr = machid_host_threads(machid_server_port,machid_auth_port,
				     id, &threads, &threadCnt);
	    break;

	  badtype:
	    fprintf(stderr, "ms: %u has type %s\n",
		    origid, mach_type_string(type));
	    continue;
	}
	if (kr != KERN_SUCCESS)
	    continue;

	for (j = 0; j < threadCnt; j++)
	    machine_thread_state(threads[j]);

	if ((threads != threads_buf) && (threadCnt != 0)) {
	    kr = vm_deallocate(mach_task_self(), (vm_offset_t) threads,
			       (vm_size_t) (threadCnt * sizeof *threads));
	    if (kr != KERN_SUCCESS)
		quit(1, "ms: vm_deallocate: %s\n", mach_error_string(kr));
	}
    }

    exit(0);
}

static void
machine_thread_state(thread)
    mthread_t thread;
{
#ifdef	mips
    static void mips_thread_state(mthread_t thread);
    mips_thread_state(thread);
#endif
#ifdef	sun3
    static void sun3_thread_state(mthread_t thread);
#endif	sun3
#ifdef	sun4
    static void sparc_thread_state(mthread_t thread);
#endif	sun4
#ifdef	vax
    static void vax_thread_state(mthread_t thread);
    vax_thread_state(thread);
#endif
#ifdef	i386
    static void i386_thread_state(mthread_t thread);
    i386_thread_state(thread);
#endif
#ifdef	alpha
    static void alpha_thread_state(mthread_t thread);
    alpha_thread_state(thread);
#endif
#ifdef	parisc
    static void parisc_thread_state(mthread_t thread);
    parisc_thread_state(thread);
#endif
}

#ifdef	mips
static void
mips_thread_state(thread)
    mthread_t thread;
{
    mips_thread_state_t state;
    kern_return_t kr;

    kr = machid_mips_thread_state(machid_server_port, machid_auth_port,
				  thread, &state);
    if (kr != KERN_SUCCESS)
	return;

    printf("Thread %d:\n", thread);
    printf("at = %08x\tv0 = %08x\tv1 = %08x\ta0 = %08x\n",
	   state.r1, state.r2, state.r3, state.r4);
    printf("a1 = %08x\ta2 = %08x\ta3 = %08x\tt0 = %08x\n",
	   state.r5, state.r6, state.r7, state.r8);
    printf("t1 = %08x\tt2 = %08x\tt3 = %08x\tt4 = %08x\n",
	   state.r9, state.r10, state.r11, state.r12);
    printf("t5 = %08x\tt6 = %08x\tt7 = %08x\ts0 = %08x\n",
	   state.r13, state.r14, state.r15, state.r16);
    printf("s1 = %08x\ts2 = %08x\ts3 = %08x\ts4 = %08x\n",
	   state.r17, state.r18, state.r19, state.r20);
    printf("s5 = %08x\ts6 = %08x\ts7 = %08x\tt8 = %08x\n",
	   state.r21, state.r22, state.r23, state.r24);
    printf("t9 = %08x\tk0 = %08x\tk1 = %08x\tgp = %08x\n",
	   state.r25, state.r26, state.r27, state.r28);
    printf("sp = %08x\tfp = %08x\tra = %08x\tlo = %08x\n",
	   state.r29, state.r30, state.r31, state.mdlo);
    printf("hi = %08x\tpc = %08x\n", state.mdhi, state.pc);
}
#endif

#ifdef	sun3
static void
sun3_thread_state(thread)
    mthread_t thread;
{
    sun3_thread_state_t state;
    kern_return_t kr;

    kr = machid_sun3_thread_state(machid_server_port, machid_auth_port,
				  thread, &state);
    if (kr != KERN_SUCCESS)
	return;

    printf("Thread %d:\n", thread);
    printf("d0 = %08x\td1 = %08x\td2 = %08x\td3 = %08x\n",
	   state.d0, state.d1, state.d2, state.d3);
    printf("d4 = %08x\td5 = %08x\td6 = %08x\td7 = %08x\n",
	   state.d4, state.d5, state.d6, state.d7);
    printf("a0 = %08x\ta1 = %08x\ta2 = %08x\ta3 = %08x\n",
	   state.a0, state.a1, state.a2, state.a3);
    printf("a4 = %08x\ta5 = %08x\ta6 = %08x\tsp = %08x\n",
	   state.a4, state.a5, state.a6, state.sp);
    printf("pc = %08x\tsr = %08x\n",
	   state.pc, state.sr);
}
#endif	sun3

#ifdef	sun4
static void
sparc_thread_state(thread)
    mthread_t thread;
{
    sparc_thread_state_t state;
    kern_return_t kr;

    kr = machid_sparc_thread_state(machid_server_port, machid_auth_port,
				  thread, &state);
    if (kr != KERN_SUCCESS)
	return;

    printf("Thread %d:\n", thread);
    printf("y = %08x\tg1 = %08x\tg2 = %08x\tg3 = %08x\n",
	   state.y, state.g1, state.g2, state.g3);
    printf("g4 = %08x\tg5 = %08x\tg6 = %08x\tg7 = %08x\n",
	   state.g4, state.g5, state.g6, state.g7);
    printf("i0 = %08x\ti1 = %08x\ti2 = %08x\ti3 = %08x\n",
	   state.i0, state.i1, state.i2, state.i3);
    printf("i4 = %08x\ti5 = %08x\ti6 = %08x\ti7 = %08x\n",
	   state.i4, state.i5, state.i6, state.i7);
    printf("l0 = %08x\tl1 = %08x\tl2 = %08x\tl3 = %08x\n",
	   state.l0, state.l1, state.l2, state.l3);
    printf("l4 = %08x\tl5 = %08x\tl6 = %08x\tl7 = %08x\n",
	   state.l4, state.l5, state.l6, state.l7);
    printf("o0 = %08x\to1 = %08x\to2 = %08x\to3 = %08x\n",
	   state.o0, state.o1, state.o2, state.o3);
    printf("o4 = %08x\to5 = %08x\to6 = %08x\to7 = %08x\n",
	   state.o4, state.o5, state.o6, state.o7);
    printf("psr = %08x\tpc = %08x\tnpc = %08x\tsp = %08x\n",
	   state.psr, state.pc, state.npc, state.o6);

    /* No FP state displayed for now */
}
#endif	sun4

#ifdef	vax
static void
vax_thread_state(thread)
    mthread_t thread;
{
    vax_thread_state_t state;
    kern_return_t kr;

    kr = machid_vax_thread_state(machid_server_port, machid_auth_port,
				 thread, &state);
    if (kr != KERN_SUCCESS)
	return;

    printf("Thread %d:\n", thread);
    printf("r0  = %08x\tr1  = %08x\tr2  = %08x\tr3  = %08x\n",
	   state.r0, state.r1, state.r2, state.r3);
    printf("r4  = %08x\tr5  = %08x\tr6  = %08x\tr7  = %08x\n",
	   state.r4, state.r5, state.r6, state.r7);
    printf("r8  = %08x\tr9  = %08x\tr10 = %08x\tr11 = %08x\n",
	   state.r8, state.r9, state.r10, state.r11);
    printf("ap  = %08x\tfp  = %08x\tsp  = %08x\tpc  = %08x\n",
	   state.ap, state.fp, state.sp, state.pc);
    printf("ps  = %08x\n", state.ps);
}
#endif

#ifdef	i386
static void
i386_thread_state(thread)
    mthread_t thread;
{
    i386_thread_state_t state;
    kern_return_t kr;

    kr = machid_i386_thread_state(machid_server_port, machid_auth_port,
				  thread, &state);
    if (kr != KERN_SUCCESS)
	return;

    printf("Thread %d:\n", thread);
    printf("gs   = %08x\tfs   = %08x\tes   = %08x\tds   = %08x\n",
	   state.gs, state.fs, state.es, state.ds);
    printf("edi  = %08x\tesi  = %08x\tebp  = %08x\tesp  = %08x\n",
	   state.edi, state.esi, state.ebp, state.esp);
    printf("ebx  = %08x\tedx  = %08x\tecx  = %08x\teax  = %08x\n",
	   state.ebx, state.edx, state.ecx, state.eax);
    printf("eip  = %08x\tcs   = %08x\tefl  = %08x\tuesp = %08x\n",
	   state.eip, state.cs, state.efl, state.uesp);
    printf("ss   = %08x\n", state.ss);
}
#endif

#ifdef	alpha
static void
alpha_thread_state(thread)
    mthread_t thread;
{
    alpha_thread_state_t state;
    kern_return_t kr;

    kr = machid_alpha_thread_state(machid_server_port, machid_auth_port,
				  thread, &state);
    if (kr != KERN_SUCCESS)
	return;

    printf("Thread %d:\n", thread);
    printf("v0 = %016X\tt0 = %016X\n",
	   state.r0, state.r1);
    printf("t1 = %016X\tt2 = %016X\n",
	   state.r2, state.r3);
    printf("t3 = %016X\tt4 = %016X\n",
	   state.r4, state.r5);
    printf("t5 = %016X\tt6 = %016X\n",
	   state.r6, state.r7);
    printf("t7 = %016X\ts0 = %016X\n",
	   state.r8, state.r9);
    printf("s1 = %016X\ts2 = %016X\n",
	   state.r10, state.r11);
    printf("s3 = %016X\ts4 = %016X\n",
	   state.r12, state.r13);
    printf("s5 = %016X\ts6 = %016X\n",
	   state.r14, state.r15);
    printf("a0 = %016X\ta1 = %016X\n",
	   state.r16, state.r17);
    printf("a2 = %016X\ta3 = %016X\n",
	   state.r18, state.r19);
    printf("a4 = %016X\ta5 = %016X\n",
	   state.r20, state.r21);
    printf("t8 = %016X\tt9 = %016X\n",
	   state.r22, state.r23);
    printf("t10 = %016X\tt11 = %016X\n",
	   state.r24, state.r25);
    printf("ra = %016X\tt12 = %016X\n",
	   state.r26, state.r27);
    printf("at = %016X\tgp = %016X\n",
	   state.r28, state.r29);
    printf("sp = %016X\tpc = %016X\n",
	   state.r30, state.pc);
}
#endif

#ifdef	parisc
static void
parisc_thread_state(thread)
    mthread_t thread;
{
    parisc_thread_state_t state;
    kern_return_t kr;

    kr = machid_parisc_thread_state(machid_server_port, machid_auth_port,
				    thread, &state);
    if (kr != KERN_SUCCESS)
	return;

    printf("Thread %d:\n", thread);
    printf("iioqh= %08x\tiioqt= %08x\tipsw = %08x\tsar  = %08x\n",
	   state.iioq_head, state.iioq_tail, state.cr22, state.cr11);
    printf("flags= %08x\tr1   = %08x\trp   = %08x\tr3   = %08x\n",
	   state.flags, state.r1, state.r2, state.r3);
    printf("r4   = %08x\tr5   = %08x\tr6   = %08x\tr7   = %08x\n",
	   state.r4, state.r5, state.r6, state.r7);
    printf("r8   = %08x\tr9   = %08x\tr10  = %08x\tr11  = %08x\n",
	   state.r8, state.r9, state.r10, state.r11);
    printf("r12  = %08x\tr13  = %08x\tr14  = %08x\tr15  = %08x\n",
	   state.r12, state.r13, state.r14, state.r15);
    printf("r16  = %08x\tr17  = %08x\tr18  = %08x\tr19  = %08x\n",
	   state.r16, state.r17, state.r18, state.r19);
    printf("r20  = %08x\tr21  = %08x\tr22  = %08x\targ3 = %08x\n",
	   state.r20, state.r21, state.r22, state.r23);
    printf("arg2 = %08x\targ1 = %08x\targ0 = %08x\tdp   = %08x\n",
	   state.r24, state.r25, state.r26, state.r27);
    printf("ret0 = %08x\tret1 = %08x\tsp   = %08x\tr31  = %08x\n",
	   state.r28, state.r29, state.r30, state.r31);
    printf("sr0  = %08x\tsr1  = %08x\tsr2  = %08x\tsr3  = %08x\n",
	   state.sr0, state.sr1, state.sr2, state.sr3);
    printf("sr4  = %08x\tsr5  = %08x\tsr6  = %08x\tsr7  = %08x\n",
	   state.sr4, state.sr5, state.sr6, state.sr7);
}
#endif
