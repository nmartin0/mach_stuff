/*
 * Copyright (c) 1991 Carnegie Mellon University
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
 * HISTORY
 * $Log:	vmread.c,v $
 * Revision 2.2  91/12/05  16:45:34  grm
 * 	Created.
 * 	[91/12/05  14:34:10  grm]
 * 
 *
 *
 * The V86 Mode Monitor:
 *
 * Dump out vm in hex
 * code originally from dorr
 * modified by grm
 *
 */
#include <mach.h>

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#define MACH_CALL(x,y)	{int foo;if((foo=(x))!=KERN_SUCCESS){	\
			 mach_error(y,foo);exit(1);}}

#define EIP_MODE	1
#define CSIP_MODE	2

main(argc,argv)
	char **argv;
	
{
	int mode;
	int pid;
	task_t task;
	vm_address_t addr = 0, address;
	vm_size_t size= 100, read_amount;
	int i,j, error;
	int offset = 0;
	vm_address_t start;
	vm_size_t rsize;
	char addrstr[256];
	int cs;
	int ip;
	
	argc--, argv++;
	if (argc<1) {
		printf("Pid?\n");
		exit(1);
	}
	
	pid = atoi(argv[0]);
	
	argc--, argv++;
	if (argc<1) {
		addr = 0x00;
	} else {
		char * pos;
		
		strcpy(addrstr, *argv);
		
		pos = (char *)strchr(addrstr, ':');
		if (pos) {
			*(pos++) = '\0';
			sscanf(addrstr, "%x", &cs);
			sscanf(pos, "%x", &ip);
			mode = CSIP_MODE;
			addr = (vm_address_t)(((u_short)cs << 4) + (u_short)ip);
		}else{
			cs = 0;
			sscanf(addrstr, "%x", &ip);
			mode = EIP_MODE;
			addr = ip;
		}
	}
	
	argc--, argv++;
	if (argc>0) sscanf(argv[0],"%x",&size);
	
	argc--, argv++;
	if (argc>0) sscanf(argv[0],"%x",&offset);
	
	start = trunc_page(addr);
	rsize = round_page(size);
	printf("%d %x %x\n",pid,start,rsize);
	
	if (addr + size > start + rsize) {
		rsize += round_page(1);
	}
	
	/* this is 3.0 */
	task = task_by_pid(pid);
	
	MACH_CALL(error = vm_read(task,
				  start,
				  rsize,
				  &address,&read_amount),
		  "vm_read");
	
	if (error == KERN_PROTECTION_FAILURE) {
		MACH_CALL(error = vm_protect(task,
					     start,
					     rsize,
					     FALSE,
					     VM_PROT_READ),
			  "vm_protect");
		
		MACH_CALL(error = vm_read(task,
					  start,
					  rsize,
					  &address,&read_amount),
			  "vm_read");
		
		if (error != KERN_SUCCESS) {
			fprintf(stderr,"Failed re-protecting target region 0x%x %s\n",
				error, mach_error_string(error));
			exit(1);
		}
	}
	
	address += (addr - trunc_page(addr));
	
	for (j=0;j<size;j += 16) {
		if (mode == CSIP_MODE) {
			printf("0x%4.4x:%4.4x ",cs,ip);
			ip += 16;
			if (ip > 0xffff) {
				ip &= 0xffff;
				cs += 0x1000;
			}
		}else{
			printf("%07x ",(addr + j + offset));
		}
		for (i=0;i<4;i++) printf(" %4.4x",((unsigned short *)(address + j))[i] );
		printf(" ");
		for (i=4;i<8;i++) printf(" %4.4x",((unsigned short *)(address + j))[i] );
		printf(" ");
		for (i=0;i<16;i++) {
			char c = ((char *)(address + j))[i];
			(isprint(c) ? printf("%c",c) : printf("."));
		}
		printf("\n");
	}
	
}

