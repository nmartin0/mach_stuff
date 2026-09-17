/* Tell xm-mips.h not to do anything about alloca.  */
#define MIPS_OVERRIDE_ALLOCA

#include "xm-mips.h"

#define USG

#define bcopy(a,b,c) memcpy (b,a,c)
#define bzero(a,b) memset (a,0,b)
#define bcmp(a,b,c) memcmp (a,b,c)

#ifdef __GNUC__
/* If compiling with GCC, use the built-in alloca.  */
#define alloca(n) __builtin_alloca (n)
#else
/* The normal irix compiler requires alloca.h or alloca doesn't work.  */
#include <alloca.h>
#endif
