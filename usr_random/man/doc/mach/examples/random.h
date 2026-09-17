#ifndef	_random
#define	_random

/* Module random */

#include <mach/kern_return.h>
#if	(defined(__STDC__) || defined(c_plusplus)) || defined(LINTLIBRARY)
#include <mach/port.h>
#include <mach/message.h>
#endif

#ifndef	mig_external
#define mig_external extern
#endif

mig_external void init_random
#if	(defined(__STDC__) || defined(c_plusplus))
    (port_t rep_port);
#else
    ();
#endif
#include <mach/std_types.h>
#include "random_types.h"

/* Routine get_random */
mig_external kern_return_t get_random
#if	defined(LINTLIBRARY)
    (server_port, num)
	port_t server_port;
	int *num;
{ return get_random(server_port, num); }
#else
#if	(defined(__STDC__) || defined(c_plusplus))
(
	port_t server_port,
	int *num
);
#else
    ();
#endif
#endif

/* Routine get_secret */
mig_external kern_return_t get_secret
#if	defined(LINTLIBRARY)
    (server_port, password)
	port_t server_port;
	string25 password;
{ return get_secret(server_port, password); }
#else
#if	(defined(__STDC__) || defined(c_plusplus))
(
	port_t server_port,
	string25 password
);
#else
    ();
#endif
#endif

#endif	_random
