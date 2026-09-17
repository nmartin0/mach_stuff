/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#ifndef _BOOT_H_
#define _BOOT_H_

int CheckLoad(short device, char *file);
void LoadMach(DialogVars *);

#endif /* _BOOT_H_ */