/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#ifndef _STRING_H_
#define _STRING_H_

#define STR_DOT					0
#define STR_PART_TYPE			1
#define STR_SPACE				2
#define STR_DEBUGGER			3

char *getstr(int);
int strcmp(char *, char *);
int strlen(char *);
void strcpy(char *, char *);
void strcat(char *, char *);
char *strchr(char *, char);

#endif /* _STRING_H_ */