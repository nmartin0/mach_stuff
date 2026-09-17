/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#ifndef _INIT_DIALOG_H_
#define _INIT_DIALOG_H_

typedef struct {
	DialogPtr		dp;
	VBLTask			vtask;
	int				count;
	unsigned long	num, denom;
	int				DisablePrimary;
	int				DisableAlternate;
	int				DefaultItem;
} DialogVars;

int INIT_dialog(DialogVars *);
void ProgrssBar(DialogVars *, unsigned long, unsigned long);

#endif /* _INIT_DIALOG_H_ */