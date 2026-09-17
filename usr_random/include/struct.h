/*	struct.h	4.1	83/05/03	*/

/*
 * access to information relating to the fields of a structure
 */

#ifndef _STRUCT_H_
#define _STRUCT_H_

#define	fldoff(str, fld)	((int)&(((struct str *)0)->fld))
#define	fldsiz(str, fld)	(sizeof(((struct str *)0)->fld))
#define	strbase(str, ptr, fld)	((struct str *)((char *)(ptr)-fldoff(str, fld)))

#endif /* _STRUCT_H_ */
