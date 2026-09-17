/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: macro_help.h,v $
 * Revision 1.1.2.1  1996/09/09  16:57:52  barbou
 * 	Created.
 * 	[1996/08/21  15:18:41  barbou]
 *
 * $EndLog$
 */

/*
 *	File:	osfmach3/macro_help.h
 *
 *	Provide help in making lint-free macro routines
 *
 */

#ifndef	_OSFMACH3_MACRO_HELP_H_
#define _OSFMACH3_MACRO_HELP_H_

#include <mach/boolean.h>

#define		NEVER		FALSE
#define		ALWAYS		TRUE

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (NEVER)

#define		MACRO_RETURN	if (ALWAYS) return

#endif	/*_OSFMACH3_MACRO_HELP_H_*/
