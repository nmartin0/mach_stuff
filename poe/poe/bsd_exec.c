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
 * $Log:	bsd_exec.c,v $
 * Revision 2.7  94/03/25  18:27:32  mrt
 * 	Changed some print statements.
 * 	[94/03/25            mrt]
 * 
 * Revision 2.6  92/02/02  13:01:34  rpd
 * 	Fixed vm_read argument types for sun3.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.5  91/12/19  20:27:02  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  90/12/04  21:54:54  rpd
 * 	Added set_ux_task_name, called by Bsd1_execve.
 * 	[90/11/25            rpd]
 * 
 * Revision 2.3  90/09/27  13:53:46  rwd
 * 	Zero bss that resides in mapped data area.
 * 	[90/09/10            rwd]
 * 
 * Revision 2.2  90/09/08  00:06:48  rwd
 * 	Remove mi_pager == 1 hack.
 * 	[90/09/04            rwd]
 * 	Converted exec to XUX22 version which keeps args in emulator.
 * 	[90/07/14            rwd]
 * 
 */
/*
 *	File:	./bsd_exec.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <bsd_msg.h>
#include <fnode.h>
#include <ux_user.h>
#include <errno.h>
#include <loader_info.h>

vm_prot_t data_prot = (VM_PROT_READ | VM_PROT_WRITE);
vm_prot_t text_prot = (VM_PROT_READ | VM_PROT_EXECUTE);

static
map_region(ut, fn, fn_offset, vm_offset, size, prot)
	struct ux_task		*ut;
	struct fnode		*fn;
	vm_offset_t		fn_offset;
	vm_offset_t		vm_offset;
	vm_size_t		size;
	vm_prot_t		prot;	/* XXX should protect ! */
{
	vm_offset_t		data;
	vm_size_t		rsize;
	int			error;
	int			resid;
	
	dprintf("map_region(%x, %x, %x, %x, %x, %x)\n",ut, fn, fn_offset, vm_offset, size, prot);
	rsize = trunc_page(size);
	
	if (vm_offset != round_page(vm_offset)) {
		/*
		 *  In this case, need to copy. Yuck.
		 */
		printf("map_region: region start 0x%x not page-alligned\n",
		       vm_offset);
		return KERN_FAILURE;
	}
	if (rsize > 0) {
		if (fn->fn_maymap) {
			error = FOP_GETPAGER(fn);
			if (error) {
				return error;
			}
			error = vm_map(ut->ut_task, &vm_offset, rsize, 0,
				       FALSE, fn->fn_map_info.mi_pager,
				       fn_offset, TRUE,
				       prot, VM_PROT_ALL, VM_INHERIT_COPY);
			if (error) {
				printf("map_region: vm_map error %x\n",error);
				return error;
			}
		} else {
			dprintf("map_region: reading\n");
			data = 0;
			error = vm_allocate(mach_task_self(), &data, rsize,
					    TRUE);
			if (error) {
				return error;
			}
			error = FOP_READ(fn, fn_offset, data, size, &resid);
			if (error) {
				return error;
			}
			if (resid) {
				return ENOEXEC;
			}
			error = vm_allocate(ut->ut_task, &vm_offset, rsize,
					    FALSE);
			if (error) {
				return error;
			}
			error = vm_write(ut->ut_task, vm_offset, data, rsize);
			if (error) {
				return error;
			}
			vm_deallocate(mach_task_self(), data, rsize);
		}
	}
	vm_offset += (size - rsize);
	fn_offset += (size - rsize);
	size -= rsize;
	if (size == 0) {
		return 0;
	}
printf("bsd_exec:map_region: how weird, addr=0x%x, prot=0x%x\n", vm_offset, prot);
	data = 0;
	error = vm_allocate(mach_task_self(), &data, vm_page_size, TRUE);
	if (error) {
		return error;
	}
	error = FOP_READ(fn, fn_offset, data, size, &resid);
	if (error) {
		return error;
	}
	if (resid) {
		return ENOEXEC;
	}
	error = vm_allocate(ut->ut_task, &vm_offset, vm_page_size, FALSE);
	if (error) {
		return error;
	}
	error = vm_write(ut->ut_task, vm_offset, data, vm_page_size);
	if (error) {
		return error;
	}
	vm_deallocate(mach_task_self(), data, vm_page_size);
	return 0;
}

load_program_file(ut, fn, lp)
	struct ux_task		*ut;
	struct fnode		*fn;
	struct loader_info	*lp;
{
	int			error;
	vm_offset_t		bss_page_start;
	vm_offset_t		bss_page_end;

	/* XXX worry about contiguous text & data! */
	dprintf("load_program_file: map text\n");
	error = map_region(ut, fn, lp->text_offset, lp->text_start,
			    lp->text_size, text_prot);
	if (error) {
		return error;
	}
	dprintf("load_program_file: map data\n");
	error = map_region(ut, fn, lp->data_offset, lp->data_start,
			    round_page(lp->data_size), data_prot);
	if (error) {
		return error;
	}
	ut->ut_break = round_page(lp->data_start + lp->data_size +
				  lp->bss_size);
	if (lp->bss_size == 0) {
		return 0;
	}
	bss_page_start = round_page(lp->data_start + lp->data_size);
	if (bss_page_start > lp->data_start + lp->data_size) {
		/*
		 * Must read in page from user, zero the appropriate
		 * part of it, and write it back.
		 */
		pointer_t	kernel_addr;
		vm_offset_t	page_start;
		vm_size_t	size;
		vm_offset_t	zero_start;

		page_start = trunc_page(lp->data_start + lp->data_size);
		zero_start = lp->data_start + lp->data_size - page_start;
		size = vm_page_size;

		error = vm_read(ut->ut_task,
				page_start,
				vm_page_size,
				&kernel_addr,
				(unsigned int *) &size);
		if (error == KERN_SUCCESS) {
		    bzero((char *)(kernel_addr + zero_start),
				   size - zero_start);
		    error = vm_write(ut->ut_task,
				     page_start,
				     kernel_addr,
				     size);
		    (void) vm_deallocate(mach_task_self(), kernel_addr, size);
		}
	}
	if (error)
		return error;
	bss_page_end = ut->ut_break;
	if (bss_page_start < bss_page_end) {
	    error = vm_allocate(ut->ut_task, &bss_page_start,
				bss_page_end - bss_page_start, FALSE);
	    if (error) {
		return error;
	    }
	}
	return 0;
}

/*
 * Only used by spawn to set init's arguments.
 */

load_program_args(user_task, na, new_arg_addr, argstrings, nc, ne)
	mach_port_t	user_task;
	int		na;
	vm_offset_t	*new_arg_addr;	/* OUT */
	char		*argstrings;
	int		nc;
	int		ne;
{
	int			i;
	int			arg_len;
	char		*xp;

	kern_return_t		result;

	vm_offset_t	u_arg_start;    /* user start of arg list block */
	vm_offset_t	k_arg_start;    /* kernel start of arg list block */
	vm_offset_t	u_arg_page_start;/* user start of args, page-aligned */
	vm_size_t	arg_page_size;	/* page_aligned size of args */
	vm_offset_t	k_arg_page_start; /* kern start of args, pg-aligned */

	char **	k_ap;	/* kernel arglist address */
	char *	u_cp;	/* user argument string address */
	char *	k_cp;	/* kernel argument string address */

	/*
	 * Calculate the size of the argument list.
	 * Add space for:
	 *    argc
	 *    pointers to arguments
	 *    trailing 0 pointer
	 *    pointers to environment variables
	 *    trailing 0 pointer
	 *    and align to integer boundary
	 */
	arg_len = nc;
	arg_len += sizeof(int) + (2 + na + ne) * sizeof(char *);
	arg_len = (arg_len + (sizeof(int) - 1)) & ~(sizeof(int)-1);
	/*
	 * Get address of argument list in user space
	 */
	u_arg_start = (STACK_END - arg_len) & ~(sizeof(int)-1);

	/*
	 * Round to page boundaries, and allocate kernel copy
	 */
	u_arg_page_start = trunc_page(u_arg_start);
	arg_page_size = (vm_size_t)(round_page(u_arg_start + arg_len)
				    - u_arg_page_start);

	result = vm_allocate(mach_task_self(), &k_arg_page_start,
			     arg_page_size, TRUE);
	if (result) return result;

	/*
	 * Set up addresses corresponding to user pointers
	 * in the kernel block
	 */
	k_arg_start = k_arg_page_start + (u_arg_start - u_arg_page_start);

	k_ap = (char **)k_arg_start;

	/*
	 * Start the strings after the arg-count and pointers
	 */
	u_cp = (char *)u_arg_start + (na + ne) * sizeof(char *)
				    + 2 * sizeof(char *)
				    + sizeof(int);
	k_cp = (char *)k_arg_start + (na + ne) * sizeof(char *)
				    + 2 * sizeof(char *)
				    + sizeof(int);

	/*
	 * first the argument count
	 */
	*k_ap++ = (char *)na;

	/*
	 * Then the strings and string pointers for each argument
	 */
	xp = argstrings;
	for (i = 0; i < na; i++) {
	    *k_ap++ = u_cp + (xp - argstrings);
	    xp += strlen(xp) + 1;
	}
	*k_ap++ = 0; /* end of argv */
	for (i = 0; i < ne; i++) {
	    *k_ap++ = u_cp + (xp - argstrings);
	    xp += strlen(xp) + 1;
	}
	*k_ap++ = 0; /* end of envp */
	bcopy(argstrings, k_cp, nc);

	/*
	 * Now write all of this to user space.
	 */
	result = vm_write(user_task,
			u_arg_page_start,
			k_arg_page_start,
			arg_page_size);
	if (result) return result;

	result = vm_deallocate(mach_task_self(),
			     k_arg_page_start,
			     arg_page_size);
	if (result) return result;

	if (new_arg_addr)
	    *new_arg_addr = u_arg_start;

	return (0);
}

int
Bsd1_execve(proc_port, interrupt,
	   fname, fname_count,
	   cfname,
	   cfarg,
	   entry,
	   entry_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	char		*cfname;	/* OUT */
	char		*cfarg;		/* OUT */
	int		*entry;		/* pointer to OUT array */
	unsigned int	*entry_count;	/* OUT */
{
	int error;
	struct ux_task *ut;
	struct loader_info li;
	extern int silent;
	struct fnode *fn;
	vm_offset_t stack_start = STACK_END - STACK_SIZE;

	error = ut_lookup("bsd_exec", proc_port, &ut, interrupt);
	if (error) {
		return error;
	}
	cfname[0] = '\0';
	cfarg[0] = '\0';

	dprintf("bsd1_exec(%s)\n", fname);
	error = bsd_lookup(ut->ut_cdir, fname, &fn, TRUE);
	if (error) {
		return error;
	}

/*XXX check prot bits? eg:

     [EACCES]       The new process file is not an ordinary file.

     [EACCES]       The new process file mode denies execute
                    permission.
*/
	error = ex_get_header(fn, &li);
	if (error) {
		return error;
	}

/*
 * XXXXX beyond this point we should KILL the process instead
 * XXXXX of returning an error!
 */

	error = machine_deallocate_for_exec(ut->ut_task);
	if (error) {
		printf("bsd1_execve: returning error from mach_deallocate\n");
		return ENOMEM;
	}

	error = load_program_file(ut, fn, &li);
	if (error) {
		return error;
	}

	set_entry_address(&li, entry, entry_count);
	sig_doexec(ut);

	set_ux_task_name(ut, fname);
	dprintf("bsd1_exec(%s): sucessful return\n",fname);

	return KERN_SUCCESS;
}

set_ux_task_name(ut, fname)
	struct ux_task *ut;
	char *fname;
{
	char *name, *p;

	/* find name component in path */

	name = fname;
	for (p = fname; *p != '\0';)
		if (*p++ == '/')
			name = p;

	/* ut->ut_comm must remain null-terminated */

	strncpy(ut->ut_comm, name, sizeof ut->ut_comm - 1);
}
