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
 * $Log:	ufs_pager.c,v $
 * Revision 2.5  92/02/02  13:03:22  rpd
 * 	Removed old IPC vestiges.
 * 	Fixed cthread_fork argument types.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.4  91/12/19  20:30:11  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:56:03  rwd
 * 	Fix problem of writing to end of file first.
 * 	[90/09/27            rwd]
 * 
 * Revision 2.2  90/09/08  00:22:08  rwd
 * 	Include cthread_self in debugging output.
 * 	[90/09/04            rwd]
 * 	Use port_allocate_name.
 * 	[90/09/03            rwd]
 * 	Converty to new cthread semantics.
 * 	[90/08/16            rwd]
 * 
 * 	Correctly terminate object.
 * 	[90/08/16            rwd]
 * 
 * 	For now, wire cthreads.
 * 	[90/07/16            rwd]
 * 
 */
/*
 *	File:	./ufs_pager.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <ufs_fops.h>
#include <device/device_types.h>
#include <sys/types.h>
#include <sys/stat.h>

extern void ufs_pager_cthread_busy();
extern void ufs_pager_cthread_active();

/*
 *  Rename to avoid name conflict with other pagers in same task.
 */
#define	memory_object_init		ufs_memory_object_init
#define	memory_object_terminate		ufs_memory_object_terminate
#define	memory_object_copy		ufs_memory_object_copy
#define	memory_object_data_request	ufs_memory_object_data_request
#define	memory_object_data_unlock	ufs_memory_object_data_unlock
#define	memory_object_data_write	ufs_memory_object_data_write
#define	memory_object_lock_completed	ufs_memory_object_lock_completed
#define	memory_object_server		ufs_memory_object_server

extern struct unode *un_by_memory_object();

int ufs_pager_debug=0;
#define DP(flag,format) if (flag&ufs_pager_debug) {printf("[%x]",cthread_self());printf/**/format/**/;} else

memory_object_init(memory_object, memory_control, memory_object_name,
		   page_size)
	mach_port_t memory_object;
	mach_port_t memory_control;
	mach_port_t memory_object_name;
	vm_size_t page_size;
{
	struct unode *un;
	int error;

	DP(1,("memory_object_init(%x, %x, %x, %x)\n",memory_object, memory_control, memory_object_name, page_size));
	if (memory_object_name != MACH_PORT_NULL) {
		mach_port_deallocate(mach_task_self(), memory_object_name);
	}
	if (memory_control == MACH_PORT_NULL) {
		printf("memory_object_init: invalid memory_control\n");
	    	return;
	}
	if (page_size != vm_page_size) {
		printf("memory_object_init: invalid page size\n");
		return;
	}
	un = un_by_memory_object(memory_object);
	if (! un) {
		printf("memory_object_init: invalid memory_object\n");
		return;
	}
	un->un_pager_refs++; /* should now be 1 */
	error = memory_object_set_attributes(memory_control, TRUE, TRUE,
					     MEMORY_OBJECT_COPY_DELAY);
	if (error) {
		printf("memory_object_init: set_attributes: %d\n", error);
	}
}

static
fill_page(un, offset, buffer, len)
	struct unode *un;
	vm_offset_t offset;
	vm_offset_t buffer;
	vm_size_t len;
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	lbn;
	daddr_t			d;
	vm_size_t		block_size;
	vm_offset_t		dev_offset;
	int error;

	DP(2,("fill_page(%x, %x, %x, %x)\n",un, offset, buffer, len));
	fs = &un->un_ud->ud_fs;
	for (;;) {
		off = blkoff(fs, offset);
		lbn = lblkno(fs, offset);
		block_size = blksize(fs, un, lbn);
		error = sbmap(un, lbn, &d);
		if (error) {
			printf("fill_page: sbmap error %d?\n", error);
			return error;
		}
		if (d == 0) {
			return -1; /* data unavailable */
		}
		dev_offset = 512 * fsbtodb(fs, d) + off; /* 512 ?! XXX */
		if (dev_offset + block_size > un->un_ud->ud_size) {
			printf("fill_page: read past end of device!\n");
			printf("dev_offset=0x%x, bsize=0x%x, ud_size=0x%x\n",
			       dev_offset, block_size, un->un_ud->ud_size);
			printf("offset=0x%x, lbn=%x0x, off=0x%x\n",offset,lbn,off);
			return error;
		}
		if (len <= block_size) {
			return map_file_read(&un->un_ud->ud_map_info,
					     buffer, dev_offset, len);
		}
		error =  map_file_read(&un->un_ud->ud_map_info, buffer,
				       dev_offset, block_size);
		if (error) {
			printf("fill_page: map_file_read error = %d\n",error);
			return error;
		}
		offset += block_size;
		len -= block_size;
	}
}

memory_object_data_request(memory_object, memory_control, offset, length,
			   desired_access)
	mach_port_t memory_object, memory_control;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	struct unode *un;
	int error;
	vm_offset_t buffer;
	vm_size_t bsize;
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	lbn;
	daddr_t			d;
	vm_size_t		block_size;
	vm_offset_t		dev_offset;
struct uhack *uhk;

	DP(4,("memory_object_data_request(%x, %x, %x, %x, %x)\n",memory_object, memory_control, offset, length, desired_access));
	/*
	 * Boring parameter munging.
	 */
	un = un_by_memory_object(memory_object);
	if (! un) {
		printf("data_request: invalid memory_object\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
#if 0
printf("%d [0x%x .. 0x%x]\n", un->un_ino, offset, offset+length);
#endif
	if ((offset % vm_page_size) != 0) {
		printf("data_request: unaligned offset\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	if (length != vm_page_size) {
		printf("data_request: size not vm_page_size\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
#if 0
	if ((desired_access & VM_PROT_WRITE) != 0) {
		printf("data_request: requesting write!\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
#endif
	/*
	 * The actual work.
	 */
	/*
	 * The reader should check file size both before and after
	 * reading, since it may change. We return data_unavailable
	 * since it is easier to ignore null data than to recover
	 * from a invalid memory reference (which would be the result
	 * of doing a memory_object_data_error).
	 */
	if (offset >= un->i_size) {
	DP(128,("memory_object_data_request: data unavailable: %x %x\n",offset,un->i_size));
		memory_object_data_unavailable(memory_control, offset, length);
		return;
	}


/* xxx look up in our hacky linked list of things that have been paged out */
for (uhk=un->un_hack; uhk; uhk = uhk->uh_next){
 if (uhk->uh_offset == offset) {
  DP(128,("memory_object_data_request: data provided (old written)\n"));
  memory_object_data_provided(memory_control, offset, uhk->uh_data,
			      vm_page_size, VM_PROT_NONE);
  return;
 }
}


	/*
	 * vm_page_size < block_size needs some thought, but we'll
	 * ignore it for now.
	 *
	 * vm_page_size > block_size occurs either from
	 *	. vm_page_size > fs_bsize
	 *	. looking at a fragment
	 * The first case is a real pain and requires some thought.
	 * The second case requires some copying and zeroing, which is
	 * unfortunate but probably dominated by other costs.
	 * (There are tricks we can play here as well, such as keeping around
	 * pages that we know have been zero'd in the right way, or copying
	 * small bits of fragments into special area in reading process, etc.)
	 */
	fs = &un->un_ud->ud_fs;
	if (fs->fs_bsize < vm_page_size) {
		/*
		 * At WORST, this check should be done beforehand.
		 * At best, we should have code to deal with it.
		 */
		printf("ufs_pager: we are in some serious trouble...\n");
		memory_object_data_error(memory_control, offset, length,
					 error);
		return;
	}
	/*
	 * If this is the last page of the file, we may have to deal with
	 * frags; we definitely have to worry about bzeroing trailing part
	 * of page.
	 */
	if (offset + vm_page_size >= un->i_size) {
		buffer = 0;
		error = vm_allocate(mach_task_self(), &buffer, vm_page_size,
				    TRUE);
		if (error) {
			printf("ufs_pager: can't alloc page!\n");
			memory_object_data_error(memory_control, offset,
						 length, error);
			return;
		}
/***************************************************/
/*{char *x=(char*)buffer;int j;for(j=0;j<vm_page_size;j++)x[j]='~';}*/
/***************************************************/
		ufs_pager_cthread_busy();
		error = fill_page(un, offset, buffer, un->i_size - offset);
		ufs_pager_cthread_active();
		if (error == -1) {
  DP(128,("memory_object_data_request: data unavailable (fill page)\n"));
			memory_object_data_unavailable(memory_control, offset,
						       length);
		} else if (error) {
  DP(128,("memory_object_data_request: data error (fill page)\n"));
			memory_object_data_error(memory_control, offset,
						 length, error);
		} else {
  DP(128,("memory_object_data_request: data provided (fill page)\n"));
			memory_object_data_provided(memory_control, offset,
						    buffer, vm_page_size,
						    VM_PROT_NONE);
		}
		vm_deallocate(mach_task_self(), buffer, vm_page_size);
		return;
	}
	off = blkoff(fs, offset); /* important when pagesize < blocksize */
	lbn = lblkno(fs, offset);
	block_size = blksize(fs, un, lbn);
	ufs_pager_cthread_busy();
	error = sbmap(un, lbn, &d);
	ufs_pager_cthread_active();
	if (error) {
		printf("ufs_pager: sbmap error %d?\n", error);
		memory_object_data_error(memory_control, offset, length,
					 error);
		return;
	}
	if (d == 0) {
		/*
		 * This seems like a waste, especially for readers.
		 * However, it's probably pretty uncommon. (Is it?)
		 *
		 * ----- this often also happens for writers,
		 * ----- at least in the current scheme.
		 */
  DP(128,("memory_object_data_request: data unavailable (after sbmap)\n"));
		memory_object_data_unavailable(memory_control, offset, length);
		return;
	}
	dev_offset = 512 * fsbtodb(fs, d) + off; /* 512 ?! XXX */
	if (dev_offset + block_size > un->un_ud->ud_size) {
		printf("ufs_pagein: read past end of device!\n");
		printf("dev_offset=0x%x, block_size=0x%x, ud_size=0x%x\n",
		       dev_offset, block_size, un->un_ud->ud_size);
		printf("offset=0x%x, lbn=%x0x, off=0x%x\n",offset,lbn,off);
		printf("d = 0x%x, fs->fs_fsbtodb = 0x%x\n",d, fs->fs_fsbtodb);
		memory_object_data_error(memory_control, offset, length,
					 error);
		return;
	}
	if (block_size < vm_page_size && offset + vm_page_size < un->i_size) {
		printf("ufs_pager: fragment not at end of file?\n");
		memory_object_data_error(memory_control, offset, length,
					 error);
		return;
	}
	/*
	 * We can return a complete page.
	 *
	 * Read-ahead? Should check it here, and generate before.
	 *
	 * It would be nice to allow this page to be stolen if
	 * we know that this page will be written...
	 */
	map_file_data_provided(&un->un_ud->ud_map_info, memory_control, offset,
			       dev_offset, vm_page_size, VM_PROT_NONE);
}

memory_object_data_unlock(memory_object, memory_control, offset, length,
			  desired_access)
	mach_port_t memory_object;
	mach_port_t memory_control;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	struct unode *un;
	int error;
	vm_offset_t buffer;
	vm_size_t bsize;
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	lbn;
	daddr_t			d;
	vm_size_t		block_size;
	vm_offset_t		dev_offset;

	DP(8,("memory_object_data_unlock(%x, %x, %x, %x, %x)\n",memory_object, memory_control, offset, length, desired_access));
	un = un_by_memory_object(memory_object);
	if (! un) {
		printf("data_unlock: invalid memory_object\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	if ((offset % vm_page_size) != 0) {
		printf("data_unlock: unaligned offset\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	if (length != vm_page_size) {
		printf("data_unlock: size not vm_page_size\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	memory_object_lock_request(memory_control, offset, length,
				   FALSE, FALSE, VM_PROT_NONE, MACH_PORT_NULL);
}

memory_object_data_write(memory_object, memory_control, offset, data, length)
	mach_port_t memory_object;
	mach_port_t memory_control;
	vm_offset_t offset;
	vm_offset_t data;
	int length;
{
	struct unode *un;
struct uhack *uhk;

	DP(16,("memory_object_data_write(%x, %x, %x, %x, %x)\n",memory_object, memory_control, offset, data, length));
	un = un_by_memory_object(memory_object);
	if (! un) {
		printf("ufs_data_write: invalid memory_object\n");
		return;
	}
	dprintf("ufs_data_write: %o.%d + 0x%x\n",
	       un->un_ud->ud_dev, un->un_ino, offset);



	uhk = (struct uhack *) malloc(sizeof(*uhk));
	uhk->uh_offset = offset;
	uhk->uh_data = data;
	uhk->uh_next = un->un_hack;
	un->un_hack = uhk;
}

memory_object_lock_completed(memory_object, memory_control, offset, length)
	mach_port_t memory_object;
	mach_port_t memory_control;
	vm_offset_t offset;
	vm_size_t length;
{
	printf("ufs_pager: lock_completed called!\n");
}

memory_object_copy(old_memory_object, old_memory_control, offset, length,
		   new_memory_object)
	memory_object_t old_memory_object;
	memory_object_control_t old_memory_control;
	vm_offset_t offset;
	vm_size_t length;
	memory_object_t new_memory_object;
{
	printf("ufs_pager: copy called!\n");
}

memory_object_terminate(memory_object, memory_control, memory_object_name)
	mach_port_t memory_object;
	mach_port_t memory_control;
	mach_port_t memory_object_name;
{

	struct unode *un;
	int refs;

	DP(32,("memory_object_terminate(%x, %x, %x)\n",memory_object, memory_control, memory_object_name));
	(void) mach_port_mod_refs(mach_task_self(), memory_object_name,
				  MACH_PORT_RIGHT_RECEIVE, -1);

	un = un_by_memory_object(memory_object);
	if (! un) {
		printf("ufs_object_terminate: invalid memory_object\n");
		return;
	}
	refs = un->un_pager_refs;
	un->un_pager_refs = 0;

	(void) mach_port_mod_refs(mach_task_self(), memory_control,
				  MACH_PORT_RIGHT_SEND, -refs);

	(void) mach_port_mod_refs(mach_task_self(), memory_control,
				  MACH_PORT_RIGHT_RECEIVE, -1);

}

mach_port_t	ufs_pager_port_set		= MACH_PORT_NULL;
int		ufs_pager_kthread_min		= 1;
int		ufs_pager_kthread_max		= 2;
int		ufs_pager_cthread_min		= 1;
int		ufs_pager_cthread_current	= 0;
struct mutex	ufs_pager_cthread_mutex		= MUTEX_INITIALIZER;
extern int	ufs_pager_loop();

int ufs_pager_init()
{
	kern_return_t error;

	error = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
		&ufs_pager_port_set);
	if (error) {
		mach_error("ufs_pager_init: port_allocate", error);
		panic("ufs_pager_init");
	}
	cthread_set_kernel_limit(cthread_kernel_limit() + ufs_pager_kthread_max);
	cthread_detach(cthread_fork((any_t (*)()) ufs_pager_loop,
				    (any_t) 0));
}

void
ufs_pager_cthread_busy()
{
	cthread_msg_busy(ufs_pager_port_set,
			 ufs_pager_kthread_min,
			 ufs_pager_kthread_max);
	mutex_lock(&ufs_pager_cthread_mutex);
	if (--ufs_pager_cthread_current < ufs_pager_cthread_min) {
	    mutex_unlock(&ufs_pager_cthread_mutex);
	    cthread_detach(cthread_fork((any_t (*)()) ufs_pager_loop,
					(any_t) 0));
	} else {
	    mutex_unlock(&ufs_pager_cthread_mutex);
	}
}

void
ufs_pager_cthread_active()
{
	cthread_msg_active(ufs_pager_port_set,
			   ufs_pager_kthread_min,
			   ufs_pager_kthread_max);
	mutex_lock(&ufs_pager_cthread_mutex);
	++ufs_pager_cthread_current;
	mutex_unlock(&ufs_pager_cthread_mutex);
}

int
ufs_pager_loop()
{
	struct ux_task *ut;

	kern_return_t ret;
	union request_msg {
	    mach_msg_header_t	hdr;
	    mig_reply_header_t	death_pill;
	    char		space[8192];
	} msg_buffer, out;

	char	name[64];

	sprintf(name, "ufs_pager thread %x", cthread_self());
	cthread_set_name(cthread_self(), name);

	ufs_pager_cthread_active();
	do {
	    ret = cthread_mach_msg(&msg_buffer, MACH_RCV_MSG,
				   0, sizeof msg_buffer, ufs_pager_port_set,
				   MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL,
				   ufs_pager_kthread_min,
				   ufs_pager_kthread_max);
	    if (ret != MACH_MSG_SUCCESS)
		panic("ufs_pager_loop: receive", ret);
	    if (!memory_object_server(&msg_buffer, &out))
		{}

	} while (1);
}

ufs_getpager(un)
	struct unode *un;
{
	int error;
	mach_port_t pager;

	if (un->un_fn.fn_map_info.mi_pager) {
		return 0;
	}
	if (! un->un_fn.fn_maymap) {
		return EINVAL;
	}
	error = mach_port_allocate_name(mach_task_self(),
					MACH_PORT_RIGHT_RECEIVE,
					(mach_port_t)un);
	if (error) {
		return error;
	}
	pager = (mach_port_t)un;

	error = mach_port_insert_right(mach_task_self(), pager, pager,
				       MACH_MSG_TYPE_MAKE_SEND);
	if (error) {
		return error;
	}

	error = mach_port_move_member(mach_task_self(), pager,
				      ufs_pager_port_set);
	if (error) {
		return error;
	}
	un->un_fn.fn_map_info.mi_pager = pager;
	un->un_pager_refs = 0;
	return 0;
}

/*
 *  XXX
 *  rw flag?
 */
bsd_map(fn)
	struct fnode *fn;
{
	int error;

	if (! fn->fn_maymap) {
		return EINVAL;
	}
	if (! fn->fn_map_info.mi_pager) {
		error = FOP_GETPAGER(fn);
		if (error) {
			return error;
		}
	}
	fn->fn_map_info.mi_read = TRUE;
	fn->fn_map_info.mi_write = TRUE;
	return KERN_SUCCESS;
}
