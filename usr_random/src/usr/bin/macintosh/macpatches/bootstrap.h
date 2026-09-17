/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

typedef struct {
    mach_msg_header_t	head;
    mach_msg_type_t	MasterDevicePortType;
    mach_port_t		MasterDevicePort;
    mach_msg_type_t	HostPrivPortType;
    mach_port_t		HostPrivPort;
} bootstrap_msg_t;
