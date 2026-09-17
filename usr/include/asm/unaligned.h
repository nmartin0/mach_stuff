/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: unaligned.h,v $
 * Revision 1.1.2.2  1997/07/29  14:03:03  bruel
 * 	HP PA doesn't resolve unaligned refs itself.
 * 	[97/07/29            bruel]
 *
 * Revision 1.1.2.1  1997/02/27  11:10:12  bruel
 * 	First revision
 * 	[1997/02/27  11:00:52  bruel]
 * 
 * $EndLog$
 */

#ifndef __MACHINE_UNALIGNED_H
#define __MACHINE_UNALIGNED_H

#define get_unaligned(ptr) \
  ({ __typeof__(*(ptr)) __tmp; memcpy(&__tmp, (ptr), sizeof(*(ptr))); __tmp; })

#define put_unaligned(val, ptr)				\
  ({ __typeof__(*(ptr)) __tmp = (val);			\
     memcpy((ptr), &__tmp, sizeof(*(ptr)));		\
     (void)0; })

#endif
