#ifndef _I386_JMP_BUF_H
#define _I386_JMP_BUF_H

/* Define the machine-dependent type `jmp_buf'.  Intel 386 version.  */

#include <sys/cdefs.h>

typedef struct
  {
    long int __bx, __si, __di;
    __ptr_t __bp;
    __ptr_t __sp;
    __ptr_t __pc;
#ifdef __SVR4_I386_ABI_L1__
    unsigned long pad1[4];
#endif
  } __jmp_buf[1];

#endif /* _I386_JMP_BUF_H */
