/*
 * Copyright (c) 1992, 1991 Carnegie Mellon University
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
 *
 * HISTORY:
 * $Log:	mdos.c,v $
 * Revision 2.3  92/07/01  14:25:04  grm
 * 	Replaced a.out.h inclusion with i386/exec.h (jvt@kampi.hut.fi).
 * 	[92/06/03            grm]
 * 
 * Revision 2.2  92/04/29  16:31:28  grm
 * 	Debug and a bit of cleanup.
 * 
 * 	Initial version grm (Gerald Malan) 2/?/90
 * 	Load a given program at a given address into the current image.
 * 	[92/04/27            rvb]
 * 
 *
 */

#include <mach.h>
#include <stdio.h>
#include <i386/exec.h>
#include <sys/file.h>
#include <sys/types.h>
#include <i386/vmparam.h>

#ifndef	Load_Addr
#define Load_Addr	0x1000000
#endif
#ifndef	Load_Name
#define Load_Name	"mdos.mon"
#endif

#define	LOADER_PAGE_SIZE	(4096)
#define loader_round_page(x)	((vm_offset_t)((((vm_offset_t)(x)) \
						+ LOADER_PAGE_SIZE - 1) \
					& ~(LOADER_PAGE_SIZE-1)))
#define loader_trunc_page(x)	((vm_offset_t)(((vm_offset_t)(x)) \
					& ~(LOADER_PAGE_SIZE-1)))

int verbose = 0;
char name[256];

main(argc, argv)
	int argc;
	char ** argv;
{
	struct exec	ex;
	int		fd;
	vm_offset_t	addr;
	vm_size_t	size;
	vm_offset_t     text_start;
	vm_size_t	text_size;
	vm_offset_t	data_start;
	vm_size_t	data_size;
	vm_size_t	total_size;
	int		ret, i;
	vm_offset_t	load_address;
	char		buffer[256];
	char *		lpath =(char *)getenv("LPATH");

	strcpy(name, Load_Name);
	
	load_address = Load_Addr;

	if (verbose)
		printf("Loading file \"%s\" at 0x%x\n", name, load_address);

	if ((fd = openp(lpath, name, buffer, O_RDONLY, 0)) < 0) {
		printf("Loader: open error.\n");
		exit(1);
	}

	ret = read(fd, &ex, sizeof(struct exec));

	if (ret != sizeof(struct exec)) {
		printf("Loader: read exec error.\n");
		exit(1);
	}

	text_start = load_address;
	text_size = loader_round_page(ex.a_text + sizeof(struct exec));
	data_start = text_start + text_size;
	data_size = loader_round_page(ex.a_data + ex.a_bss);
	total_size = round_page(text_size + data_size);

	if (verbose) {
		printf("magic number = 0x%x\n", ex.a_magic);
		printf("text_start = 0x%x rounded text_size = 0x%x\n", text_start, text_size);
		printf("data_start = 0x%x rounded data_size = 0x%x\n", data_start, data_size);
		printf("actual data_size = 0x%x actual bss_size = 0x%x\n", ex.a_data, ex.a_bss);
		printf("goto_addr = 0x%x\n", ex.a_entry);
	}

#ifdef LOOK_AT_MAGIC
	if (ex.a_magic != 0413) {
		printf("Loader: not 0413 magic number\n");
		exit(1);
	}
#endif /* LOOK_AT_MAGIC */

	if (vm_deallocate(mach_task_self(), load_address, total_size)
	    != KERN_SUCCESS) {
		printf("Loader: couldn't deallocate memory.\n");
		exit(1);
	}

	/*
	 * Allocate memory for text and data.
	 */
	addr = sbrk(0);			/* where are we */
	size = text_size + ex.a_data + ex.a_bss;
	if (verbose)
		printf("current addr = %x, adding = 0x%x\n", addr, text_start + size);
	addr = sbrk(text_start + size - addr);
	addr = sbrk(0);			/* and now where are we */
	if (addr != text_start + size) {
		printf("Loader: Error sbrk'ing image.\n");
		exit(1);
	}
	if (verbose)
		printf("end of data now at = %x\n", addr);


	/*
	 * Read the text into memory.
	 */
	lseek(fd, 0, L_SET);
	if ((ret = read(fd, (char *) text_start, text_size)) != text_size) {
		printf("Loader: Error reading text.\n");
		exit(1);
	}

	/*
	 * Protect the text as read & execute only.
	 */
	vm_protect(mach_task_self(), text_start, round_page(ex.a_text),
		   FALSE, VM_PROT_READ|VM_PROT_EXECUTE);

	/*
	 * Read the data into memory.
	 */
	lseek(fd, (ex.a_text + sizeof(struct exec)) , L_SET);
	if ((ret = read(fd, (char *) data_start, ex.a_data)) != ex.a_data) {
		printf("Loader: Error reading data.\n");
		exit(1);
	}

	/* BSS already allocated. */

	/* And now a little magic */
	{ int __fake__;
		asm volatile("movl %1, %%esp; jmp %2" :
			 "=g" (__fake__) :
			 "g" (argv - 1), "g" (ex.a_entry));
	}
	/* never return */
}
