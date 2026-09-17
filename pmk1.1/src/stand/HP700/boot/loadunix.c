/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * pmk1.1
 */

#include "stand.h"
#include "saio.h"
#include <machine/exec.h>

char *argv[3];

void
copyunix(int howto, int devtype, struct file *file)
{
	register int result;
	register char *addr;
	struct header hdr;
	struct som_exec_auxhdr som;

	result = read_file(file, 0, &hdr, sizeof(struct header));
	if (result) {
		printf("Short read on header\n");
		return;
	}
		
	result = read_file(file, hdr.aux_header_location, &som, sizeof(som));
	if (result) {
		printf("Short read on SOM header \n");
		return;
	}

	addr = (char *) som.exec_tmem;
	printf("text (0x%x) at 0x%x\n", som.exec_tsize, addr);

	result = read_file(file, som.exec_tfile, addr, som.exec_tsize);
	if(result) {
		printf("Short read on text (0x%x 0x%x 0x%x)\n", 
		       som.exec_tsize, som.exec_tmem, som.exec_tfile);
		return;
	}

	addr = (char *) som.exec_dmem;
	printf("data (0x%x) at 0x%x\n", som.exec_dsize, addr);

	result = read_file(file, som.exec_dfile, addr, som.exec_dsize);
	if(result) {
		printf("Short read on data (0x%x 0x%x 0x%x)\n", 
		       som.exec_dsize, som.exec_dmem, som.exec_dfile);
		return;
	}

	fcacheall();
	execute(som.exec_entry, argv, 2);
}


