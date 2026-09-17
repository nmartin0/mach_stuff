/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: a.out.h,v $
 * Revision 1.1.2.1  1997/02/27  11:10:49  bruel
 * 	First revision
 * 	[1997/02/27  11:01:13  bruel]
 *
 * $EndLog$
 */

#ifndef __MACHINE_A_OUT_H__
#define __MACHINE_A_OUT_H__

#include <mach/machine/vm_param.h>
#ifdef __KERNEL_
#include <asm/processor.h>
#endif /* __KERNEL__ */

struct exec
{
  unsigned long a_info;		/* Use macros N_MAGIC, etc for access */
  unsigned a_text;		/* length of text, in bytes */
  unsigned a_data;		/* length of data, in bytes */
  unsigned a_bss;		/* length of uninitialized data area for file, in bytes */
  unsigned a_syms;		/* length of symbol table data in file, in bytes */
  unsigned a_entry;		/* start address */
  unsigned a_trsize;		/* length of relocation info for text, in bytes */
  unsigned a_drsize;		/* length of relocation info for data, in bytes */
};

#define N_TRSIZE(a)	((a).a_trsize)
#define N_DRSIZE(a)	((a).a_drsize)
#define N_SYMSIZE(a)	((a).a_syms)

#ifdef __KERNEL__

#define STACK_TOP	TASK_SIZE

#endif

#endif
