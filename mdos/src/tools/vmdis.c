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
 * Dissassemble a block of instructions at a virtual address.
 *
 * HISTORY:
 * $Log:	vmdis.c,v $
 * Revision 2.2  91/12/05  16:45:24  grm
 * 	Changed copyrights.
 * 
 * 	Initial version grm (Gerald Malan) 90/03/09
 * 	[91/12/04            grm]
 * 
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <mach.h>

#define EIP_MODE	1
#define CSIP_MODE	2
#define PROT_MODE	3

#define MACH_CALL(x,y)	{int foo;if((foo=(x))!=KERN_SUCCESS){\
                         mach_error(y,foo);exit(1);}}

void dis_block (mode, address, cs, ip, size)
	int	mode;
	unsigned char * address;
	int	cs;
	int	ip;
	int	size;
{
	char outbuf[1024];
	unsigned char * eip;
	int numbytes;

	eip = address;
	while (eip < address + size) {
		switch (mode) {
		    case CSIP_MODE:
			fprintf(stdout, "dis: 0x%4.4x:%4.4x     ",cs,ip);
			numbytes = i386dis (mode, cs, ip, eip, outbuf);
			fprintf(stdout, "%s\n", outbuf);
			eip += numbytes;
			ip += numbytes;
			if (ip > 0xffff) {
				ip &= 0xffff;
				cs += 0x1000;
			}
			break;
		    case EIP_MODE:
		    case PROT_MODE:
		    default:
			fprintf(stdout, "dis: 0x%8.8x     ",ip);
			numbytes = i386dis (mode, cs, ip, eip, outbuf);
			fprintf(stdout, "%s\n", outbuf);
			eip += numbytes;
			ip += numbytes;
			break;
			
		}
	}
}

main(argc, argv)
	int argc;
	char **argv;
{
	int mode = 0;
	int offset;
	int pid;
	vm_size_t size, read_amount;
	kern_return_t error;
	vm_address_t addr, address, staddr, edaddr;
	char addrstr[256];
	int cs;
	int ip;
	
	task_t task;

	argc--, argv++;
	
	if ((argc > 1) && *argv[0] == '-') {
		if (strcmp(*argv, "-p") == 0) {
			mode = PROT_MODE;
			argc--; argv++;
		}
	}

	if (argc<1) {
		printf("usage: vmdis [-p] pid [addr] [size]\n");
		exit(1);
	}
	
	pid = atoi(*argv);
	argc-- , argv++;
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
			if (mode != PROT_MODE)
				mode = EIP_MODE;
			addr = ip;
		}
	}

	argc-- , argv++;
	if (argc<1) {
		size = 0x50;
	} else {
		sscanf(*argv, "%x", &size);
	}

	argc-- , argv++;
	if (argc<1) {
		offset = 0x00;
	} else {
		sscanf(*argv, "%x", &offset);
	}

	task = task_by_pid(pid);

	staddr = trunc_page(addr);
	edaddr = round_page(addr+size);
	MACH_CALL((vm_read(task, staddr, edaddr - staddr,
			&address,&read_amount)),"vm_read");


	dis_block (mode, address+(addr%vm_page_size), cs, ip, size);

}
