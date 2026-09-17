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
 * $Log: vminfo.c,v $
 * Revision 1.1.1.1  1995/05/04  06:57:05  sclawson
 * New files.
 *
 * Revision 2.2  92/01/22  23:08:35  rpd
 * 	Moved to a separate directory.
 * 	[92/01/22            rpd]
 * 
 * Revision 2.8  92/01/17  14:25:02  rpd
 * 	Updated for CountInOut.
 * 	Added new vm debugging options.
 * 	[92/01/17  14:14:27  rpd]
 * 
 * Revision 2.7  91/08/30  15:41:35  rpd
 * 	Updated from vio_single_use to vio_use_old_pageout.
 * 	[91/08/30            rpd]
 * 
 * Revision 2.6  91/08/29  15:49:37  rpd
 * 	Moved machid include files into the standard include directory.
 * 	[91/08/29            rpd]
 * 
 * Revision 2.5  91/03/27  17:27:57  mrt
 * 	Changed mach.h include
 * 
 * Revision 2.4  91/03/19  12:32:44  mrt
 * 	Changed to new copyright
 * 
 * Revision 2.3  90/11/05  23:34:20  rpd
 * 	Removed vir_copy_on_write.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.2  90/09/12  16:33:13  rpd
 * 	Created.
 * 	[90/06/18            rpd]
 * 
 */

#include <stdio.h>
#include <strings.h>
#include <mach.h>
#include <mach_debug/mach_debug.h>
#include <mach_error.h>
#include <servers/netname.h>
#include <servers/machid.h>
#include <servers/machid_debug.h>
#include <servers/machid_dpager.h>
#include <servers/machid_types.h>
#include <servers/machid_lib.h>

#define streql(a, b)	(strcmp((a), (b)) == 0)

static void print_vm_info();
static vm_offset_t print_vm_region(), print_vm_region_info();
static void print_object_chain();
static void print_default_pager_info();

mach_port_t machid_server_port;
mach_port_t machid_auth_port;

static boolean_t TaskHold = FALSE;
static boolean_t Debug = FALSE;
static boolean_t Verbose = FALSE;
static boolean_t Pages = FALSE;

static void
usage()
{
    fprintf(stderr, "usage: vminfo [-host machine] [-z] [taskid]\n");
    fprintf(stderr, "usage: vminfo [-host machine] -d [-z] [-v] [-p] [taskid]\n");
    fprintf(stderr, "usage: vminfo [-host machine] -d [-p] objectid\n");
    fprintf(stderr, "usage: vminfo [-host machine] -d [-v] [-p] defaultpagerid\n");
    exit(1);
}

static void
parse_args(argc, argv, idp)
    int argc;
    char *argv[];
    mach_id_t *idp;
{
    char *hostname = "";
    mach_id_t id;
    kern_return_t kr;
    int i;

    for (i = 1; i < argc; i++)
	if (streql(argv[i], "-host") && (i < argc-1))
	    hostname = argv[++i];
	else if (streql(argv[i], "-z"))
	    TaskHold = TRUE;
	else if (streql(argv[i], "-Z"))
	    TaskHold = FALSE;
	else if (streql(argv[i], "-d"))
	    Debug = TRUE;
	else if (streql(argv[i], "-D"))
	    Debug = FALSE;
	else if (streql(argv[i], "-v"))
	    Verbose = TRUE;
	else if (streql(argv[i], "-V"))
	    Verbose = FALSE;
	else if (streql(argv[i], "-p"))
	    Pages = TRUE;
	else if (streql(argv[i], "-P"))
	    Pages = FALSE;
	else if (streql(argv[i], "--")) {
	    i++;
	    break;
	} else if (argv[i][0] == '-')
	    usage();
	else
	    break;

    kr = netname_look_up(name_server_port, hostname, "MachID",
			 &machid_server_port);
    if (kr != KERN_SUCCESS)
	quit(1, "vminfo: netname_lookup_up(MachID): %s\n",
	     mach_error_string(kr));

    machid_auth_port = mach_host_priv_self();
    if (machid_auth_port == MACH_PORT_NULL)
	machid_auth_port = mach_task_self();

    switch (argc - i) {
      case 0: {
	mhost_t host;

	/* default to our own task port */

	kr = machid_mach_register(machid_server_port, machid_auth_port,
				  mach_task_self(), MACH_TYPE_TASK, &id);
	if (kr != KERN_SUCCESS)
	    quit(1, "vminfo: machid_mach_register: %s\n",
		 mach_error_string(kr));

	/* we don't need the host here, but we want to set up
	   the association between the task and the host */

	kr = machid_task_host(machid_server_port, machid_auth_port, id, &host);
	if (kr != KERN_SUCCESS)
	    quit(1, "vminfo: machid_task_host: %s\n",
		 mach_error_string(kr));

	break;
      }

      case 1:
	id = atoi(argv[i]);
	break;

      default:
	usage();
    }

    *idp = id;
}

void
main(argc, argv)
    int argc;
    char *argv[];
{
    mach_id_t id;
    mach_type_t type;
    kern_return_t kr;

    parse_args(argc, argv, &id);

    kr = machid_mach_type(machid_server_port, machid_auth_port, id, &type);
    if (kr != KERN_SUCCESS)
	quit(1, "vminfo: machid_mach_type: %s\n",
	     mach_error_string(kr));

    switch (type) {
      case MACH_TYPE_NONE:
	/* exit silently */
	break;

      case MACH_TYPE_TASK:
	if (!Debug && (Verbose || Pages))
	    usage();

	if (TaskHold) {
	    kr = machid_task_suspend(machid_server_port, machid_auth_port, id);
	    if (kr != KERN_SUCCESS)
		quit(1, "vminfo: machid_task_suspend: %s\n",
		     mach_error_string(kr));
	}

	print_vm_info((mtask_t) id,
		      Debug ? print_vm_region_info : print_vm_region);

	if (TaskHold) {
	    kr = machid_task_resume(machid_server_port, machid_auth_port, id);
	    if (kr != KERN_SUCCESS)
		quit(1, "vminfo: machid_task_resume: %s\n",
		     mach_error_string(kr));
	}
	break;

      case MACH_TYPE_OBJECT_NAME:
	if (!Debug || TaskHold || Verbose)
	    usage();

	print_object_chain((mobject_name_t) id);
	break;

      case MACH_TYPE_DEFAULT_PAGER:
	if (!Debug || TaskHold)
	    usage();

	print_default_pager_info((mdefault_pager_t) id);
	break;

      default:
	usage();
    }

    exit(0);
}

static char *inherit_name[] =
	{ "both", "copy", "none", "move" };
static char *prot_name[] =
	{ "   ", "R  ", " W ", "RW ", "  X", "R X", " WX", "RWX" };
static char *copy_name[] =
	{ "none", "call", "delay" };

static vm_offset_t
print_vm_region(task, addr, pagesize)
    mtask_t task;
    vm_offset_t addr;
    vm_size_t pagesize;
{
    vm_region_t info;
    kern_return_t kr;

    kr = machid_vm_region(machid_server_port, machid_auth_port,
			  task, addr, &info);
    if (kr == KERN_NO_SPACE)
	return 0;
    else if (kr != KERN_SUCCESS)
	quit(1, "vminfo: machid_vm_region: %s\n",
	     mach_error_string(kr));

    printf("Region %08x-%08x: ", info.vr_address,
	   info.vr_address + info.vr_size);
    printf("prot=%s/%s/%s, ",
	   prot_name[info.vr_prot],
	   prot_name[info.vr_max_prot],
	   inherit_name[info.vr_inherit]);
    printf("object=%u, offset=%x\n", info.vr_name, info.vr_offset);

    return info.vr_address + info.vr_size;
}

static void
print_region(info)
    vm_region_info_t *info;
{
    printf("Region %08x-%08x: ",
	   info->vri_start, info->vri_end);
    printf("prot=%s/%s/%s",
	   prot_name[info->vri_protection],
	   prot_name[info->vri_max_protection],
	   inherit_name[info->vri_inheritance]);
    printf(", object=%u, offset=%x", info->vri_object, info->vri_offset);
    if ((info->vri_wired_count != 0) ||
	(info->vri_user_wired_count != 0))
	printf(", wired=%u/%u",
	       info->vri_wired_count, info->vri_user_wired_count);
    if (info->vri_needs_copy)
	printf(", needs copy");
    if (info->vri_sharing != 0)
	printf(", sharing=%u", info->vri_sharing);
    printf("\n");
}

static void
print_object(info)
    vm_object_info_t *info;
{
    printf("    Object %u: size = %x, %u refs, %u pages",
	   info->voi_object, info->voi_size,
	   info->voi_ref_count, info->voi_resident_page_count);
    printf(", %u absent, %u paging ops\n",
	   info->voi_absent_count, info->voi_paging_in_progress);

    if (info->voi_state & VOI_STATE_INTERNAL) {
	printf("\tinternal");
	if (!(info->voi_state & VOI_STATE_TEMPORARY))
	    printf(", permanent");
	if (info->voi_state & VOI_STATE_CAN_PERSIST)
	    printf(", cacheable");
	if (info->voi_copy_strategy != MEMORY_OBJECT_COPY_NONE)
	    printf(", copy %s", copy_name[info->voi_copy_strategy]);
	if (info->voi_state & VOI_STATE_PAGER_CREATED) {
	    printf(", created");
	    if (!(info->voi_state & VOI_STATE_PAGER_READY))
		printf(", not ready");
	    if (!(info->voi_state & VOI_STATE_PAGER_INITIALIZED))
		printf(", uninitialized");
	} else {
	    if (info->voi_state & VOI_STATE_PAGER_READY)
		printf(", ready");
	    if (info->voi_state & VOI_STATE_PAGER_INITIALIZED)
		printf(", initialized");
	}
    } else {
	printf("\texternal");
	if (info->voi_state & VOI_STATE_TEMPORARY)
	    printf(", temporary");
	if (!(info->voi_state & VOI_STATE_CAN_PERSIST))
	    printf(", not cacheable");
	if (info->voi_copy_strategy != MEMORY_OBJECT_COPY_DELAY)
	    printf(", copy %s", copy_name[info->voi_copy_strategy]);
	if (!(info->voi_state & VOI_STATE_PAGER_CREATED))
	    printf(", not created");
	if (!(info->voi_state & VOI_STATE_PAGER_READY))
	    printf(", not ready");
	if (!(info->voi_state & VOI_STATE_PAGER_INITIALIZED))
	    printf(", uninitialized");
    }

    if (info->voi_state & VOI_STATE_LOCK_IN_PROGRESS)
	printf(", locking");
    if (info->voi_state & VOI_STATE_LOCK_RESTART)
	printf(", restart");
    if (!(info->voi_state & VOI_STATE_USE_OLD_PAGEOUT))
	printf(", new pageout");
    if (!(info->voi_state & VOI_STATE_ALIVE))
	printf(", dead");

    if (info->voi_paging_offset != 0)
	printf(", paging offset = %x", info->voi_paging_offset);
    if (info->voi_shadow != 0)
	printf(", shadow = %u", info->voi_shadow);
    if (info->voi_shadow_offset != 0)
	printf(", shadow offset = %x", info->voi_shadow_offset);
    if (info->voi_copy != 0)
	printf(", copy = %u", info->voi_copy);
    printf(", last alloc = %x\n", info->voi_last_alloc);
}

static void
print_page(info)
    vm_page_info_t *info;
{
    if (info->vpi_wire_count != 0)
	printf("wired (%u)", info->vpi_wire_count);
    else if (info->vpi_state & VPI_STATE_ACTIVE)
	printf("active");
    else if (info->vpi_state & VPI_STATE_INACTIVE) {
	printf("inactive");
	if (info->vpi_state & VPI_STATE_REFERENCE)
	    printf("/referenced");
    } else {
	if (info->vpi_state & VPI_STATE_PAGER)
	    printf("paged out");
	else
	    printf("not queued");
	if (info->vpi_state & VPI_STATE_TABLED)
	    printf(", tabled");
	goto skip_pager;
    }

    if (info->vpi_state & VPI_STATE_PAGER)
	printf(", paged out");
    if (!(info->vpi_state & VPI_STATE_TABLED))
	printf(", not tabled");

  skip_pager:

    if (info->vpi_state & VPI_STATE_BUSY)
	printf(", busy");
    if (info->vpi_state & VPI_STATE_WANTED)
	printf(", wanted");
    if (info->vpi_state & VPI_STATE_FICTITIOUS)
	printf(", fictitious");
    if (info->vpi_state & VPI_STATE_PRIVATE)
	printf(", private");
    if (info->vpi_state & VPI_STATE_ABSENT)
	printf(", absent");
    if (info->vpi_state & VPI_STATE_ERROR)
	printf(", error");
    if (info->vpi_state & VPI_STATE_DIRTY)
	printf(", dirty");
    if (info->vpi_state & VPI_STATE_PRECIOUS)
	printf(", precious");
    if (info->vpi_state & VPI_STATE_OVERWRITING)
	printf(", overwriting");
    if (info->vpi_state & VPI_STATE_LAUNDRY)
	printf(", laundry");
    if (info->vpi_state & VPI_STATE_FREE)
	printf(", free");

    if (info->vpi_page_lock != VM_PROT_NONE)
	printf(", page lock = %s", prot_name[info->vpi_page_lock]);
    if (info->vpi_unlock_request != VM_PROT_NONE)
	printf(", unlock request = %s", prot_name[info->vpi_unlock_request]);

    printf("\n");
}

static void
print_pages(start, end, pagesize, chain, offset, lookup, print)
    vm_offset_t start, end;
    vm_size_t pagesize;
    object_t *chain;
    vm_offset_t offset;
    void (*lookup)();
    void (*print)();
{
    vm_offset_t page, prevpage;
    object_t *object, *prevobject;
    vm_page_info_t *info, *previnfo;

    for (page = start; page <= end; page += pagesize) {
	if (page == end)
	    goto end_subregion;

	(*lookup)(chain, page + offset, &object, &info);

	if (page == start)
	    goto start_subregion;

	if ((object != prevobject) ||
	    ((info != 0) && (previnfo == 0)) ||
	    ((info == 0) && (previnfo != 0)) ||
	    ((info != 0) && (previnfo != 0) &&
	     ((info->vpi_state != previnfo->vpi_state) ||
	      (info->vpi_wire_count != previnfo->vpi_wire_count) ||
	      (info->vpi_page_lock != previnfo->vpi_page_lock) ||
	      (info->vpi_unlock_request != previnfo->vpi_unlock_request)))) {

	  end_subregion:
	    (*print)(prevpage, page, prevobject, previnfo);

	  start_subregion:
	    prevpage = page;
	    prevobject = object;
	    previnfo = info;
	}
    }
}

static void
print_region_page_range(start, end, object, info)
    vm_offset_t start, end;
    object_t *object;
    vm_page_info_t *info;
{
    printf("    Pages %08x-%08x: ", start, end);
    if (info == 0) {
	printf("not present\n");
    } else {
	printf("object %u, ", object->o_info.voi_object);
	print_page(info);
    }
}

static void
print_region_pages(info, pagesize)
    vm_region_info_t *info;
    vm_size_t pagesize;
{
    print_pages(info->vri_start, info->vri_end, pagesize,
		get_object(info->vri_object, TRUE, TRUE),
		info->vri_offset - info->vri_start,
		lookup_page_chain, print_region_page_range);
}

static void
print_object_page_range(start, end, object, info)
    vm_offset_t start, end;
    object_t *object;
    vm_page_info_t *info;
{
    printf("        Pages %08x-%08x: ", start, end);
    if (info == 0)
	printf("not present\n");
    else
	print_page(info);
}

static void
print_object_pages(o)
    object_t *o;
{
    vm_offset_t start, end;

    get_object_bounds(o, &start, &end);

    print_pages(start, end, o->o_info.voi_pagesize, o, 0,
		lookup_page_object, print_object_page_range);
}

static void
print_object_chain(object)
    mobject_name_t object;
{
    object_t *o;

    for (o = get_object(object, Pages, Pages); o != 0; o = o->o_shadow) {
	if (o->o_flag) {
	    /* only show objects once */

	    printf("    Object %u: ...\n", o->o_info.voi_object);
	    break;
	}

	print_object(&o->o_info);
	if (Pages)
	    print_object_pages(o);

	o->o_flag = 1;
    }
}

static vm_offset_t
print_vm_region_info(task, addr, pagesize)
    mtask_t task;
    vm_offset_t addr;
    vm_size_t pagesize;
{
    vm_region_info_t info;
    kern_return_t kr;

    kr = machid_vm_region_info(machid_server_port, machid_auth_port,
			       task, addr, &info);
    if (kr == KERN_NO_SPACE)
	return 0;
    else if (kr != KERN_SUCCESS)
	quit(1, "vminfo: machid_vm_region_info: %s\n",
	     mach_error_string(kr));

    print_region(&info);

    if (Verbose)
	print_object_chain(info.vri_object);

    if (Pages)
	print_region_pages(&info, pagesize);

    return info.vri_end;
}

static void
print_vm_info(task, func)
    mtask_t task;
    vm_offset_t (*func)();
{
    vm_statistics_data_t stats;
    vm_offset_t addr;
    kern_return_t kr;

    kr = machid_vm_statistics(machid_server_port, machid_auth_port,
			      task, &stats);
    if (kr != KERN_SUCCESS)
	quit(1, "vminfo:  machid_vm_statistics: %s\n",
	     mach_error_string(kr));

    for (addr = (*func)(task, 0, stats.pagesize);
	 addr != 0;
	 addr = (*func)(task, addr, stats.pagesize))
	continue;
}

static void
digits(buffer, number)
    char *buffer;
    double number;
{
    if (number < 10.0)
	sprintf(buffer, "%4.2f", number);
    else if (number < 100.0)
	sprintf(buffer, "%4.1f", number);
    else
	sprintf(buffer, "%4.0f", number);
}

static void
mem_to_string(buffer, size)
    char *buffer;
    vm_size_t size;
{
    char digitsbuf[100];

    if (size > 1024*1024*1024) {
	digits(digitsbuf, size/(1024.0*1024.0*1024.0));
	sprintf(buffer, "%sG", digitsbuf);
    } else if (size > 1024*1024) {
	digits(digitsbuf, size/(1024.0*1024.0));
	sprintf(buffer, "%sM", digitsbuf);
    } else
	sprintf(buffer, "%uK", size/1024);
}

static void
print_default_pager_object(dpager, pagesize, info)
    mdefault_pager_t dpager;
    vm_size_t pagesize;
    default_pager_object_t *info;
{
    printf("    Object %u: size = %x (%u pages)",
	   info->dpo_object, info->dpo_size, info->dpo_size/pagesize);

    if (Pages) {
	default_pager_page_t pages_buf[1024];
	default_pager_page_t *pages = pages_buf;
	unsigned int count = sizeof pages_buf/sizeof pages_buf[0];
	unsigned int i;
	kern_return_t kr;

	kr = machid_default_pager_object_pages(machid_server_port,
					       machid_auth_port,
					       dpager, info->dpo_object,
					       &pages, &count);
	if (kr != KERN_SUCCESS)
	    quit(1, "vminfo: machid_default_pager_object_pages: %s\n",
		 mach_error_string(kr));

	printf(", page offsets =");
	for (i = 0; i < count; i++) {
	    if (i % 8 == 0)
		printf("\n");
	    printf("\t%06x", pages[i].dpp_offset);
	}

	if ((pages != pages_buf) && (count != 0)) {
	    kr = vm_deallocate(mach_task_self(), (vm_offset_t) pages,
			       (vm_size_t) (count * sizeof *pages));
	    if (kr != KERN_SUCCESS)
		quit(1, "vminfo: vm_deallocate: %s\n",
		     mach_error_string(kr));
	}
    }

    printf("\n");
}

static void
print_default_pager_info(dpager)
    mdefault_pager_t dpager;
{
    default_pager_object_t *objects;
    unsigned int count, i;
    default_pager_info_t info;
    vm_size_t total;
    char total_buffer[100], free_buffer[100];
    kern_return_t kr;

    kr = machid_default_pager_info(machid_server_port, machid_auth_port,
				   dpager, &info);
    if (kr != KERN_SUCCESS)
	quit(1, "vminfo: machid_default_pager_info: %s\n",
	     mach_error_string(kr));

    get_dpager_objects(dpager, &objects, &count);

    mem_to_string(total_buffer, info.dpi_total_space);
    mem_to_string(free_buffer, info.dpi_free_space);

    printf("Default pager %u has %s free out of %s total space.\n",
	   dpager, free_buffer, total_buffer);

    total = 0;
    for (i = 0; i < count; i++)
	total += objects[i].dpo_size;
    mem_to_string(total_buffer, total);
    printf("Total allocated size is %s (%u pages).\n",
	   total_buffer, total/info.dpi_page_size);

    if (Verbose) {
	for (i = 0; i < count; i++)
	    print_default_pager_object(dpager,
			info.dpi_page_size, &objects[i]);
    }
}
