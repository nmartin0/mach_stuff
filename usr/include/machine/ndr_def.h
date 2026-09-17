/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: ndr_def.h,v $
 * Revision 1.1.9.2  1995/08/21  20:31:09  devrcs
 * 	endian is big.
 * 	[95/06/23            bruel]
 *
 * Revision 1.1.9.1  1995/03/15  17:40:13  bruel
 * 	hppa merge
 * 	[1995/03/15  09:28:39  bruel]
 * 
 * Revision 1.1.2.1  1994/02/11  13:39:14  bruel
 * 	Created.
 * 	[93/11/23            bruel]
 * 
 * $EndLog$
 */

#include <mach/ndr.h>

NDR_record_t NDR_record = {
	0,			/* mig_reserved */
	0,			/* mig_reserved */
	0,			/* mig_reserved */
	NDR_PROTOCOL_2_0,		
	NDR_INT_BIG_ENDIAN,
	NDR_CHAR_ASCII,
	NDR_FLOAT_IEEE,
	0,
};
