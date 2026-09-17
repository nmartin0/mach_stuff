/* 
 * HISTORY
 * $Log:	regmagic.h,v $
 * Revision 2.2  91/10/14  16:47:44  berman
 * 	Created for updated make from bww's merge.
 * 	[91/10/14  16:19:33  berman]
 * 
 */
/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	MAGIC	0234
