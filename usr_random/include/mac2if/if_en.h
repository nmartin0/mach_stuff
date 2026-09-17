/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	if_en.h,v $
 * Revision 2.2  91/09/12  16:49:29  bohman
 * 	Created.
 * 	[91/09/11  16:18:08  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2if/if_en.h
 */

typedef unsigned char	ether_address_t[6];

typedef struct {
    ether_address_t	dest;
    ether_address_t	src;
    unsigned short	type;
} ether_header_t;

#define EN_ENBL_PROTO		1

typedef unsigned	en_enbl_proto_t;

#define EN_ADD_MULTI		2
#define EN_DEL_MULTI		3

typedef unsigned	en_address_status_t[2];

#define EN_ADDRESS_STATUS_COUNT	2

