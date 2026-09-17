#ifndef	_macserver_user_
#define	_macserver_user_

/* Module macserver */

#include <mach/kern_return.h>
#if	(defined(__STDC__) || defined(c_plusplus)) || defined(LINTLIBRARY)
#include <mach/port.h>
#include <mach/message.h>
#endif

#include <mach/std_types.h>
#include <mach/mach_types.h>

/* Routine KernelObjects */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t KernelObjects
#if	defined(LINTLIBRARY)
    (request_port, mac32bit, low_object, low_offset, low_size, high_object, high_offset, high_size)
	mach_port_t request_port;
	boolean_t *mac32bit;
	mach_port_t *low_object;
	vm_offset_t *low_offset;
	vm_size_t *low_size;
	mach_port_t *high_object;
	vm_offset_t *high_offset;
	vm_size_t *high_size;
{ return KernelObjects(request_port, mac32bit, low_object, low_offset, low_size, high_object, high_offset, high_size); }
#else
#if	(defined(__STDC__) || defined(c_plusplus))
(
	mach_port_t request_port,
	boolean_t *mac32bit,
	mach_port_t *low_object,
	vm_offset_t *low_offset,
	vm_size_t *low_size,
	mach_port_t *high_object,
	vm_offset_t *high_offset,
	vm_size_t *high_size
);
#else
    ();
#endif
#endif

/* Routine DeviceMaster */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t DeviceMaster
#if	defined(LINTLIBRARY)
    (request_port, master_port)
	mach_port_t request_port;
	mach_port_t *master_port;
{ return DeviceMaster(request_port, master_port); }
#else
#if	(defined(__STDC__) || defined(c_plusplus))
(
	mach_port_t request_port,
	mach_port_t *master_port
);
#else
    ();
#endif
#endif

/* Routine Emulate */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t Emulate
#if	defined(LINTLIBRARY)
    (request_port)
	mach_port_t request_port;
{ return Emulate(request_port); }
#else
#if	(defined(__STDC__) || defined(c_plusplus))
(
	mach_port_t request_port
);
#else
    ();
#endif
#endif

/* Routine HostPriv */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t HostPriv
#if	defined(LINTLIBRARY)
    (request_port, host_priv)
	mach_port_t request_port;
	mach_port_t *host_priv;
{ return HostPriv(request_port, host_priv); }
#else
#if	(defined(__STDC__) || defined(c_plusplus))
(
	mach_port_t request_port,
	mach_port_t *host_priv
);
#else
    ();
#endif
#endif

#endif	_macserver_user_
