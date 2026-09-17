/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <mach/mach_types.h>
#include <mach/mig_errors.h>
#include <mach/msg_type.h>
#include <mach/memory_object.h>
#include <strings.h>

kern_return_t memory_object_change_completed
(
  mach_port_t memory_object,
  mach_msg_type_name_t memory_objectPoly,
  boolean_t may_cache,
  memory_object_copy_strategy_t copy_strategy
)
{
  return 0;
}

