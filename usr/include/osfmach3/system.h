/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: system.h,v $
 * Revision 1.1.2.1  1996/09/09  16:58:49  barbou
 * 	Created.
 * 	[1996/08/21  14:35:19  barbou]
 *
 * $EndLog$
 */

#ifndef __OSFMACH3_SYSTEM_H
#define __OSFMACH3_SYSTEM_H

#include <asm/segment.h>

#define sti()			while (0)
#define cli()			while (0)
#define save_flags(x)		(x) = 0
#define restore_flags(x)	(x) = 0

#endif	/* __OSFMACH3_SYSTEM_H */
