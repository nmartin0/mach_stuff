/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 *  5-Oct-93  Alessandro (af) at Carnegie-Mellon University
 *	Created and first checkin.
 *
 * $Log$
 *
 *    Author:	Alessandro Forin, Carnegie Mellon University
 *    Date:	Jan 1993
 */

#include "a.out.h"
#include "mips_reloc.h"
#include <sys/file.h>
#include <alpha/coff.h>
#include <stdio.h>

char *malloc();

main(argc,argv)
     char **argv;
{
  int f;

  f = open(argv[1], O_RDONLY, 0);
  printit(f);
  close(f);
}

printit(f)
     int f;
{
  struct {
    struct filehdr f;
    struct aouthdr a;
  } header;
  struct scnhdr scn;
  int i, off;

  if (read(f, &header, sizeof(header)) != sizeof(header))
    exit(1);
  off = sizeof(header);
  printf("%d sections in file, gp_value = %lx\n",
	 header.f.f_nscns, header.a.gp_value);

  for (i = 0; i < header.f.f_nscns; i++) {
    read(f, &scn, sizeof(scn));
    printrelocs(f, &scn);
    off += sizeof(scn);
    lseek(f, (long)off, 0);
  }
}

printrelocs(f, scnp)
     struct scnhdr *scnp;
{
  int i;
  struct reloc *r;
  char *dat;

  printf("Section %8.8s:\n", scnp->s_name);
  printf("\tpaddr %lx vaddr %lx size %lx ptr %lx\n",
	 scnp->s_paddr, scnp->s_vaddr, scnp->s_size, scnp->s_scnptr);
  printf("\t%d relocs @ %lx  [%x]\n",
	 scnp->s_nreloc, scnp->s_relptr, scnp->s_flags);

  dat = malloc(scnp->s_size);
  lseek(f, scnp->s_scnptr, 0);
  if (read(f, dat, scnp->s_size) != scnp->s_size) exit(2);

  i = scnp->s_nreloc * sizeof(*r);
  r = (struct reloc *) malloc( i );
  lseek(f, scnp->s_relptr, 0);
  if (read(f, r, i) != i) exit(2);

  for (i = 0; i < scnp->s_nreloc; i++) {
    int *mem = (int *)(dat + r[i].r_vaddr - scnp->s_vaddr);
    print_a_reloc(r[i]);
    if ((long)mem & 0x3) printf("\t[??]\n");
    else printf("\t[%x]\n", *mem);
  }

  free(r); free(dat);
}

print_a_reloc(r)
     struct reloc r;
{
  static char *r_sn_name[] = {
    "NULL",
    "TEXT",
    "RDATA",
    "DATA",
    "SDATA",
    "SBSS",
    "BSS",
    "INIT",
    "LIT8",
    "LIT4",
    "XDATA",
    "PDATA",
    "FINI",
    "LITA",
    "ABS",
  };

  static char *r_type_name[] = {
    "ABS",
    "REFLONG",
    "REFQUAD",
    "GPREL32",
    "LITERAL",
    "LITUSE",
    "GPDISP",
    "BRADDR",
    "HINT",
    "SREL16",
    "SREL32",
    "SREL64",
    "OP_PUSH",
    "OP_STORE",
    "OP_PSUB",
    "OP_PRSHIFT",
  };

  if (r.r_extern || (r.r_symndx > MAX_R_SN))
    printf("\t    %lx: %x %s %d %x",
	   r.r_vaddr, r.r_symndx, r_type_name[r.r_type],
	   r.r_offset, r.r_size);
  else
    printf("\t    %lx: %s %s %d %x",
	   r.r_vaddr, r_sn_name[r.r_symndx], r_type_name[r.r_type],
	   r.r_offset, r.r_size);
  fflush(stdout);
}

