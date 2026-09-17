/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#ifndef _HFS_H_
#define _HFS_H_

typedef ParamBlockRec HFSparam;

int HFSopen(HFSparam *, char *);
long HFSread(HFSparam *, char *, long);

#endif /* _HFS_H_ */