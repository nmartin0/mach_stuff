/* Define the machine-dependent type `jmp_buf'.  preliminary hppa version.  */

#if 0	/* [ppc] */
typedef struct
  {
    long int __misc[3];       /* GPR1, GPR2, LR */
    long int __gregs[32-14];  /* GPR14..GPR31 */
    double   __fpregs[32-14]; /* FPR14..FPR31 */
  } __jmp_buf[1];
#else
typedef int __jmp_buf[64];
#endif
  
