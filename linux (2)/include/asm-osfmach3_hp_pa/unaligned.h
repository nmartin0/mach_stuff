/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
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
