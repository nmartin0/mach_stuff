/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: param.h,v $
 * Revision 1.1.2.1  1997/02/27  11:10:27  bruel
 * 	First revision
 * 	[1997/02/27  11:01:00  bruel]
 *
 * $EndLog$
 */

#ifndef _ASM_OSFMACH3_MACHINE_PARAM_H
#define _ASM_OSFMACH3_MACHINE_PARAM_H

#ifndef HZ
#define HZ 100
#endif

#define EXEC_PAGESIZE	4096

#ifndef NGROUPS
#define NGROUPS		32
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#endif	/* _ASM_OSFMACH3_MACHINE_PARAM_H */
