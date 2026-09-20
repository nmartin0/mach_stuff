/*
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 *
 * Purpose:
 *	V86 DOS disk emulation routines
 *
 * HISTORY: 
 * $Log:	dos_disk.c,v $
 * Revision 2.2  91/12/05  16:42:14  grm
 * 	Added a closedir to cleanup dangling structures.
 * 	[91/07/16  17:48:34  grm]
 * 
 * 	New Copyright.  Added . and .. to the
 * 	directory list.
 * 	[91/05/28  15:14:25  grm]
 * 
 * 	Ch ch ch changes....
 * 	[91/03/26  19:24:23  grm]
 * 
 * 	Some changes.  Exec pseudo works.  This is
 * 	a back up checkin.
 * 	[91/03/01  14:42:59  grm]
 * 
 * 	Fixed some name.ext problems.
 * 	[91/02/12  17:10:16  grm]
 * 
 * 	Do cd's.
 * 	[91/02/06  18:36:55  grm]
 * 
 * 	Fancy dir.
 * 	[91/02/06  16:59:31  grm]
 * 
 * 	u: dir works.  not fancy yet.
 * 	[91/02/06  14:31:01  grm]
 * 
 * 	Created.
 * 	[91/02/01  13:30:19  grm]
 * 
 */

#include "base.h"
#include "bios.h"

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <sgtty.h>
#include <ctype.h>
#include <errno.h>

#include "dos.h"

struct dir_ent * make_entry()
{
	struct dir_ent * entry;

	entry = (struct dir_ent *)malloc(sizeof(struct dir_ent));
	entry->next = NULL;

	return(entry);
}

void free_list(list)
	struct dir_ent * list;
{
	struct dir_ent * next;

	if (list == NULL)
		return;

	while (list->next != NULL) {
		next = list->next;
		free(list);
		list = next;
	}
	free(list);
}

void dump_list(list)
	struct dir_ent * list;
{
	struct dir_ent * entry;

	entry = list;
	while (entry != NULL) {
		printf("%.8s . %.3s 0%o %d\n",entry->name, entry->ext,
		       entry->mode, entry->size);
		entry = entry->next;
	}
}

boolean_t extract_filename(filename, name, ext)
	char * filename;
	char * name;
	char * ext;
{
	int pos;
	int dec_found;
	boolean_t invalid;
	int end_pos;
	int slen;
	int flen;
	
	pos = 1;
	end_pos = 0;
	dec_found = 0;
	invalid = FALSE;
	while((pos < 13) && !invalid) {
		char ch = filename[pos];
		
		if (ch == 0) {
			end_pos = pos - 1;
			pos = 20;
			continue;
		}
		if (dec_found) {
			/* are there more than one .'s ? */
			if (ch == '.') {
				invalid = TRUE;
				continue;
			}
			/* is extension > 3 long ? */
			if (pos - dec_found > 3) {
				invalid = TRUE;
				continue;
			}
		}else{
			/* is filename > 8 long ? */
			if ((pos > 7) && (ch != '.')){
				invalid = TRUE;
				continue;
			}
		}
		switch(ch) {
		    case '.':
			dec_found = pos;
			break;
		    case '"':
		    case '/':
		    case '\\':
		    case '[':
		    case ']':
		    case ':':
		    case '<':
		    case '>':
		    case '+':
		    case '=':
		    case ';':
		    case ',':
			invalid = TRUE;
			break;
		    default:
			break;
		}
		pos++;
	}
	if (invalid)
		return(FALSE);
	
	if ((pos > 11) && (pos != 20))
		return(FALSE);
	
	if (dec_found == 0) {
		slen = end_pos+1;
	}else{
		slen = dec_found;
	}
	strncpy(name, filename, slen);
	if (slen < 8) {
		if((flen = 8 - slen) > 0)
			strncpy((name+slen),"        ",flen);
	}
	
	if (dec_found) {
		if (end_pos) {
			slen = end_pos - dec_found;
		}else{
			slen = 3;
		}
		strncpy(ext, (filename + dec_found + 1), slen);
		if(3-slen > 0)
			strncpy((ext+slen),"   ",3-slen);
	}else{
		strncpy(ext, "   ", 3);
	}

	for (pos = 0; pos < 8; pos++) {
		char ch = name[pos];
		if (isalpha(ch) && islower(ch)) name[pos] = toupper(ch);
	}

	for (pos = 0; pos < 3; pos++) {
		char ch = ext[pos];
		if (isalpha(ch) && islower(ch)) ext[pos] = toupper(ch);
	}
	
	return(TRUE);
}

struct dir_ent * get_dir(name)
	char * name;
{
	DIR * cur_dir;
	struct direct * cur_ent;
	struct dir_ent * dir_list;
	struct dir_ent * entry;
	struct stat sbuf;
	int pos;
	int dec_found;
	boolean_t invalid;
	int end_pos;
	int slen;
	int flen;
	char buf[256];
	char * sptr;
	char fname[8];
	char fext[3];
	
	(void) find_file(name,&sbuf);
	
	if ((cur_dir = opendir(name)) == NULL) {
		extern int errno;
		Debug0((dbg_fd,"couldn't open '%s' errno = %d\n",name,errno));
		return(NULL);
	}

	Debug0((dbg_fd,"get_dir opened '%s'\n",name));

	dir_list = NULL;
	entry = dir_list;

	while (cur_ent = readdir(cur_dir)) {
		if (cur_ent->d_ino == 0)
			continue;
		if (cur_ent->d_namlen > 13)
			continue;
		if (cur_ent->d_name[0] == '.') {
			if (cur_ent->d_namlen > 2)
				continue;
			if ((cur_ent->d_namlen == 2) && 
			    (cur_ent->d_name[1] != '.'))
				continue;
			strncpy(fname,"..",cur_ent->d_namlen);
			strncpy(fname+cur_ent->d_namlen, "        ",
				8-cur_ent->d_namlen);
			strncpy(fext,"   ",3);
		}else{
			if (!extract_filename(cur_ent->d_name,
					      fname, fext))
				continue;
		}

		if (entry == NULL) {
			entry = make_entry();
			dir_list = entry;
		}else{
			entry->next = make_entry();
			entry = entry->next;
		}
		entry->next = NULL;

		strncpy(entry->name, fname, 8);
		strncpy(entry->ext, fext, 8);

		strcpy(buf,name);
		slen = strlen(buf);
		sptr = buf + slen + 1;
		buf[slen] = '/';
		strcpy(sptr,cur_ent->d_name);
		
		if (!find_file(buf,&sbuf)) {
			Debug1((dbg_fd,"\rdos_disk: couldn't stat '%s'\n",buf));
			entry->mode = S_IFREG;
			entry->size = 0;
			entry->time = 0;
		}else{
			entry->mode = sbuf.st_mode;
			entry->size = sbuf.st_size;
			entry->time = sbuf.st_mtime;
		}

	}
	closedir(cur_dir);
	return(dir_list);
}

/*
 * Another useless specialized parsing routine!
 * Assumes that a legal string is passed in.
 */
void auspr(filestring, name, ext)
	char * filestring;
	char * name;
	char * ext;
{
	int pos = 0;
	int dot_pos = 0;
	int elen;

	Debug1((dbg_fd,"auspr '%s'\n",filestring));
	for(pos =0;;pos++) {
		if (filestring[pos] == '.') {
			dot_pos = pos;
			continue;
		}
		if (filestring[pos] == '\0')
			break;
	}
	
	if (dot_pos > 0) {
		strncpy(name, filestring, dot_pos);
		if (8-dot_pos > 0)
			strncpy(name + dot_pos, "        ", 8-dot_pos);
		elen = pos - dot_pos - 1;
		strncpy(ext, filestring + dot_pos + 1, elen);
		if (3 - elen > 0)
			strncpy(ext + elen, "   ", 3 - elen);
	}else{
		strncpy(name, filestring, pos);
		if (8 - pos > 0)
			strncpy(name + pos, "        ", 8 - pos);
		strncpy(ext, "   ", 3);
	}

	for (pos = 0; pos < 8; pos++) {
		char ch = name[pos];
		if (isalpha(ch) && islower(ch)) name[pos] = toupper(ch);
	}

	for (pos = 0; pos < 3; pos++) {
		char ch = ext[pos];
		if (isalpha(ch) && islower(ch)) ext[pos] = toupper(ch);
	}
}
