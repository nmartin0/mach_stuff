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
 * $Log:	dos_fs.c,v $
 * Revision 2.5  92/05/22  15:59:38  grm
 * 	Aesthetic change.
 * 	[92/05/21            grm]
 * 
 * Revision 2.4  92/02/02  23:02:37  rvb
 * 	Fixed the build_ufs_path problem with '/' as DOSROOT.
 * 	[92/01/29            grm]
 * 
 * Revision 2.3  91/12/06  15:29:30  grm
 * 	Added code to find out what the psp of the process which
 * 	initializes the device drivers in config.sys was.  This seems to
 * 	be undefined ([sarcasm] surprise surprise :-)).
 * 	[91/12/06            grm]
 * 
 * Revision 2.2  91/12/05  16:42:23  grm
 * 	Added some initialization code for relative and absolute cluster
 * 	offsets within the sft.  Added the debug_dump_sft procedure which
 * 	dumps out the sft for a given file handle.  Fixed the MS-Write
 * 	network drive bug by checking if my_drive == the file's
 * 	sft_device_info's number.  This insidious bug could be the fault
 * 	of the MS-Write program and not us.  It somehow corrupts it's
 * 	copy of the file's cds.  This was a nasty one!  Ifdefed out the
 * 	afs get_disk_space code.
 * 	[91/12/04            grm]
 * 	Added support for the DOSROOT functionality.
 * 	Fixed the set_directory_name so that a trailing
 * 	backslash wouldn't be the last character except
 * 	for the root directory in the cds.
 * 	[91/08/09  19:54:24  grm]
 * 
 * 	Corrected the code so that you can use different
 * 	find_first/next lists.
 * 	[91/07/16  17:50:46  grm]
 * 
 * 	Changed so that can be used with Dos v4.0 and 5.00
 * 	[91/06/28  18:55:03  grm]
 * 
 * 	Initialization fixes.  Several small fixes.
 * 	[91/06/14  11:57:29  grm]
 * 
 * 	New Copyright.  Fixed get_disk_space.
 * 	[91/05/28  15:16:08  grm]
 * 
 * 	Exports lol, cds, and sda to other modules.
 * 	[91/04/30  13:49:20  grm]
 * 
 * 	Works with version 1.1 of machfs.sys and
 * 	mfsini.exe.  Fixed make_dir.
 * 	[91/04/30  13:45:51  grm]
 * 
 * 	Created.
 * 	[91/03/26  19:25:05  grm]
 * 
 */

#include "base.h"
#include "bios.h"

#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/errno.h>

#include <mach/message.h>
#include <mach/exception.h>

#include "dos.h"

/* these universal globals defined here (externed in dos.h) */
boolean_t mach_fs_enabled = FALSE;

#define INSTALLATION_CHECK	0x0
#define	REMOVE_DIRECTORY	0x1
#define	REMOVE_DIRECTORY_2	0x2
#define	MAKE_DIRECTORY		0x3
#define	MAKE_DIRECTORY_2	0x4
#define	SET_CURRENT_DIRECTORY	0x5
#define	CLOSE_FILE		0x6
#define	COMMIT_FILE		0x7
#define	READ_FILE		0x8
#define	WRITE_FILE		0x9
#define	LOCK_FILE_REGION	0xa
#define	UNLOCK_FILE_REGION	0xb
#define	GET_DISK_SPACE		0xc
#define	SET_FILE_ATTRIBUTES	0xe
#define	GET_FILE_ATTRIBUTES	0xf
#define RENAME_FILE		0x11
#define	DELETE_FILE		0x13
#define	OPEN_EXISTING_FILE	0x16
#define	CREATE_TRUNCATE_FILE	0x17
#define	CREATE_TRUNCATE_NO_DIR	0x18
#define	FIND_FIRST		0x1b
#define	FIND_NEXT		0x1c
#define	CLOSE_ALL		0x1d
#define	CONTROL_REDIRECT	0x1e
#define	FLUSH_ALL_DISK_BUFFERS	0x20
#define	SEND_FROM_EOF		0x21
#define	PROCESS_TERMINATED	0x22
#define	QUALIFY_FILENAME	0x23
#define MULTIPURPOSE_OPEN	0x2e	/* Used in DOS 4.0+ */
#define UNDOCUMENTED_FUNCTION_2	0x25	/* Used in DOS 4.0+ */

#define EOS		'\0'
#define	SLASH		'/'
#define BACKSLASH	'\\'

/* dos_disk.c */
struct dir_ent * get_dir();
void auspr();

lol_t	lol;
cds_t	cds;
sda_t	sda;
u_short	com_psp = NULL;

u_char		last_drive;
u_char		my_drive;
int		dos_major;
int		dos_minor;

extern char * dos_root;
extern int dos_root_len;

/* initialize 'em to 3.1 to 3.3 */

int sdb_drive_letter_off	= 0x0;
int sdb_template_name_off	= 0x1;
int sdb_template_ext_off	= 0x9;
int sdb_attribute_off		= 0xc;
int sdb_dir_entry_off		= 0xd;
int sdb_p_cluster_off		= 0xf;
int sdb_file_name_off		= 0x15;
int sdb_file_ext_off		= 0x1d;
int sdb_file_attr_off		= 0x20;
int sdb_file_time_off		= 0x2b;
int sdb_file_date_off		= 0x2d;
int sdb_file_st_cluster_off 	= 0x2f;
int sdb_file_size_off		= 0x31;
		    
int sft_handle_cnt_off 		= 0x0;
int sft_open_mode_off  		= 0x2;
int sft_attribute_byte_off	= 0x4;
int sft_device_info_off  	= 0x5;
int sft_dev_drive_ptr_off	= 0x7;
int sft_fd_off			= 0xb;
int sft_start_cluster_off	= 0xb;
int sft_time_off		= 0xd;
int sft_date_off		= 0xf;
int sft_size_off		= 0x11;
int sft_position_off		= 0x15;
int sft_rel_cluster_off		= 0x19;
int sft_abs_cluster_off		= 0x1b;
int sft_directory_sector_off	= 0x1d;
int sft_directory_entry_off	= 0x1f;
int sft_name_off		= 0x20;
int sft_ext_off			= 0x28;

int cds_record_size 		= 0x51;
int cds_current_path_off	= 0x0;
int cds_flags_off		= 0x43;
		    
int sda_current_dta_off		= 0xc;
int sda_cur_psp_off		= 0x10;
int sda_filename1_off		= 0x92;
int sda_filename2_off		= 0x112;
int sda_sdb_off			= 0x192;
int sda_cds_off			= 0x26c;
int sda_search_attribute_off	= 0x23a;
int sda_open_mode_off		= 0x23b;
int sda_rename_source_off	= 0x2b8;

int lol_cdsfarptr_off		= 0x16;
int lol_last_drive_off		= 0x21;

/*
 * These offsets only meaningful for DOS 4 or greater:
 */
int sda_ext_act_off		= 0x2dd;
int sda_ext_attr_off		= 0x2df;
int sda_ext_mode_off		= 0x2e1;

void init_dos_offsets(ver)
	int ver;
{
	Debug0((dbg_fd,"dos_fs: using dos version = %d.\n", ver));
	switch(ver) {
	    case DOSVER_31_33: {
		    sdb_drive_letter_off	= 0x0;
		    sdb_template_name_off	= 0x1;
		    sdb_template_ext_off	= 0x9;
		    sdb_attribute_off		= 0xc;
		    sdb_dir_entry_off		= 0xd;
		    sdb_p_cluster_off		= 0xf;
		    sdb_file_name_off		= 0x15;
		    sdb_file_ext_off		= 0x1d;
		    sdb_file_attr_off		= 0x20;
		    sdb_file_time_off		= 0x2b;
		    sdb_file_date_off		= 0x2d;
		    sdb_file_st_cluster_off 	= 0x2f;
		    sdb_file_size_off		= 0x31;
		    
		    sft_handle_cnt_off 		= 0x0;
		    sft_open_mode_off  		= 0x2;
		    sft_attribute_byte_off	= 0x4;
		    sft_device_info_off  	= 0x5;
		    sft_dev_drive_ptr_off	= 0x7;
		    sft_fd_off			= 0xb;
		    sft_start_cluster_off	= 0xb;
		    sft_time_off		= 0xd;
		    sft_date_off		= 0xf;
		    sft_size_off		= 0x11;
		    sft_position_off		= 0x15;
		    sft_rel_cluster_off		= 0x19;
		    sft_abs_cluster_off		= 0x1b;
		    sft_directory_sector_off	= 0x1d;
		    sft_directory_entry_off	= 0x1f;
		    sft_name_off		= 0x20;
		    sft_ext_off			= 0x28;

		    cds_record_size 		= 0x51;
		    cds_current_path_off	= 0x0;
		    cds_flags_off		= 0x43;
		    
		    sda_current_dta_off		= 0xc;
		    sda_cur_psp_off		= 0x10;
		    sda_filename1_off		= 0x92;
		    sda_filename2_off		= 0x112;
		    sda_sdb_off			= 0x192;
		    sda_cds_off			= 0x26c;
		    sda_search_attribute_off	= 0x23a;
		    sda_open_mode_off		= 0x23b;
		    sda_rename_source_off	= 0x2b8;

		    lol_cdsfarptr_off		= 0x16;
		    lol_last_drive_off		= 0x21;
		    break;
	    }
	    case DOSVER_50:
	    case DOSVER_41: {
		    sdb_drive_letter_off	= 0x0;
		    sdb_template_name_off	= 0x1;
		    sdb_template_ext_off	= 0x9;
		    sdb_attribute_off		= 0xc;
		    sdb_dir_entry_off		= 0xd;
		    sdb_p_cluster_off		= 0xf;
		    sdb_file_name_off		= 0x15;
		    sdb_file_ext_off		= 0x1d;
		    sdb_file_attr_off		= 0x20;
		    sdb_file_time_off		= 0x2b;
		    sdb_file_date_off		= 0x2d;
		    sdb_file_st_cluster_off 	= 0x2f;
		    sdb_file_size_off		= 0x31;
		    
/* same */	    sft_handle_cnt_off 		= 0x0;
		    sft_open_mode_off  		= 0x2;
		    sft_attribute_byte_off	= 0x4;
		    sft_device_info_off  	= 0x5;
		    sft_dev_drive_ptr_off	= 0x7;
		    sft_fd_off			= 0xb;
		    sft_start_cluster_off	= 0xb;
		    sft_time_off		= 0xd;
		    sft_date_off		= 0xf;
		    sft_size_off		= 0x11;
		    sft_position_off		= 0x15;
		    sft_rel_cluster_off		= 0x19;
		    sft_abs_cluster_off		= 0x1b;
		    sft_directory_sector_off	= 0x1d;
		    sft_directory_entry_off	= 0x1f;
		    sft_name_off		= 0x20;
		    sft_ext_off			= 0x28;

/* done */	    cds_record_size 		= 0x58;
		    cds_current_path_off	= 0x0;
		    cds_flags_off		= 0x43;
		    
/* done */	    sda_current_dta_off		= 0xc;
		    sda_cur_psp_off		= 0x10;
		    sda_filename1_off		= 0x9e;
		    sda_filename2_off		= 0x11e;
		    sda_sdb_off			= 0x19e;
		    sda_cds_off			= 0x282;
		    sda_search_attribute_off	= 0x24d;
		    sda_open_mode_off		= 0x24e;
		    sda_ext_act_off		= 0x2dd;
		    sda_ext_attr_off		= 0x2df;
		    sda_ext_mode_off		= 0x2e1;
		    sda_rename_source_off	= 0x300;

/* same */	    lol_cdsfarptr_off		= 0x16;
		    lol_last_drive_off		= 0x21;

		    break;
	    }
	    default: {

	    }
	}
}


void init_dos_side()
{
	mach_fs_enabled = TRUE;
}

int
dos_read(fd, data, cnt)
int fd;
char * data;
int cnt;
{
	int ret;
	int total = 0;
	char buff[4096];
	while (cnt > 0) {
		int amount = cnt > 4096 ? 4096 : cnt;
		ret = read(fd, buff, amount);
		if (ret < 0) return (ret);
		if (ret < amount) {
			bcopy(buff, data, ret);
			return (total + ret);
		}
		bcopy(buff, data, amount);
		cnt   -= amount;
		data  += amount;	
		total += amount;
	}
	return (total);
}

int
dos_write(fd, data, cnt)
int fd;
char * data;
int cnt;
{
	int ret;
	int total = 0;
	char buff[4096];
	while (cnt > 0) {
		int amount = cnt > 4096 ? 4096 : cnt;
		bcopy(data, buff, amount);
		ret = write(fd, buff, amount);
		if (ret < 0) return (ret);
		cnt   -= amount;
		data  += amount;	
		total += amount;
	}
	return (total);
}

void calculate_dos_pointers()
{
	far_t cdsfarptr;

	cdsfarptr = lol_cdsfarptr(lol);
	cds = (cds_t) Addr_8086(cdsfarptr.segment, cdsfarptr.offset);

	cds = (cds_t) (((int) cds) + (cds_record_size*(my_drive-1)));
	Debug0((dbg_fd,"Calculated DOS Information:\n"));
	Debug0((dbg_fd,"  lol = 0x%x, sda = 0x%x\n",lol, sda));
	Debug0((dbg_fd,"  cdsfar = %x, %x\n",cdsfarptr.segment,
					     cdsfarptr.offset));
	cds_flags(cds) |= (1<<15);
	cds_flags(cds) |= (1<<14);

	{
		u_short psp;
		u_char * ptr;

		psp = sda_cur_psp(sda);
		ptr = (u_char *)Addr_8086(psp, 0);
		com_psp = psp_parent_psp(ptr);
	}
}

boolean_t dos_fs_dev(state)
	state_t * state;
{
	int dos_ver;
	Debug0((dbg_fd,"Mach fs operation: 0x%x\n",state->eax));

	if (WORD(state->eax) == 0x500) {
		if (!mach_fs_enabled) {
			return(UNCHANGED);
		}
		lol = (lol_t) Addr(state, es, ebx);
		sda = (sda_t) Addr(state, ds, esi);
		dos_major = LOW(state->ecx);
		dos_minor = HIGH(state->ecx);
		Debug0((dbg_fd,"dos_fs: dos_major:minor = 0x%x:%x.\n",
			dos_major, dos_minor));
		if ((dos_major == 3) && (dos_minor > 9) && (dos_minor < 31)) {
			dos_ver = DOSVER_31_33;
		}else if ((dos_major == 4) && (dos_minor >= 0) && (dos_minor <= 1)) {
			dos_ver = DOSVER_41;
		}else if ((dos_major == 5) && (dos_minor == 0)) {
			dos_ver = DOSVER_50;
		}else{
			dos_ver = NULL;
		}
		init_dos_offsets(dos_ver);
		calculate_dos_pointers();
		/*
		 * For mfsini.exe v1.1+ just in case.
		 */
		SETWORD(&(state->eax), 1);
	}

	if (WORD(state->eax) == 0) {
		u_char * ptr;
		ptr = (u_char *)Addr_8086(state->es, state->edi) + 22;
		my_drive = *ptr + 1;
		init_dos_side();
		/*
		 * So that machfs.sys v1.1+ will know that
		 * we're running Mach too.
		 */
		SETWORD(&(state->eax), 1);
		return(UNCHANGED);
	}
	
	return (UNCHANGED);
}

void time_to_dos(clock, date, time)
	time_t * clock;
	u_short * date;
	u_short * time;
{
	struct tm * tm;

	tm = localtime(clock);

	*date = ((((tm->tm_year - 80)&0x1f) << 9) |
		 (((tm->tm_mon + 1) & 0xf) << 5) |
		 (tm->tm_mday & 0x1f));

	*time = (((tm->tm_hour & 0x1f) << 0xb) |
		 ((tm->tm_min & 0x3f) << 5));
}

int strip_char(ptr, ch)
	char * ptr;
	char ch;
{
	int len = 0;
	char * wptr;
	char * rptr;

	wptr = ptr;
	rptr = ptr;

	while (*rptr != EOS) {
		if (*rptr == ch) {
			rptr++;
		}else{
			if (wptr != rptr)
				*wptr = *rptr;
			wptr++;
			rptr++;
			len++;
		}
	}
	*wptr = EOS;

	return(len);
}

void path_to_ufs(ufs, path)
	char * ufs;
	char * path;
{
	char ch;
	int len = 0;
	char * wptr = ufs;
	char * rptr = path;

	while ((ch = *rptr) != EOS) {
		switch (ch) {
			case ' ':
				rptr++;
				continue;
			case BACKSLASH:
				*wptr = SLASH;
				break;
			default:
				if (isalpha(ch) && isupper(ch)) {
					*wptr = tolower(ch);
				} else  *wptr = ch;
				break;
		}
		wptr++;	rptr++;	len++;
	}
	*wptr = EOS;

	if (ufs[len] == '.')
		ufs[len] = EOS;

	Debug0((dbg_fd,"dos_gen: path_to_ufs '%s'\n",ufs));
}

void build_ufs_path(ufs, path)
	char * ufs;
	char * path;
{
	strcpy(ufs, dos_root);

	Debug0((dbg_fd,"dos_fs: build_ufs_path '%s' '%s'\n",
		ufs, path));

	if (path[0] == BACKSLASH)
		path++;
	path_to_ufs(ufs + dos_root_len, path);

	Debug0((dbg_fd,"dos_fs: build_ufs_path '%s'\n",ufs));
}


/*
 * function: file_file
 *
 * Finds file fpath using stat.
 * If file exists under given name, it returns true.
 * This routine also checks for uppercase and lowercase
 * versions of last component of filename.  It returns
 * the stat structure modified appropriately and converts
 * the string to the matching case or to uppercase if there
 * is no match.
 */
boolean_t find_file(fpath, st)
	char * fpath;
	struct stat * st;
{
	int i;
	int len = 0;

	/* Check original name */
	Debug0((dbg_fd,"Find file trying '%s'\n",fpath));
    	if(stat(fpath, st) == 0) return(TRUE);

	/* Restore to lower case */
	for (i = 0; fpath[i] != EOS; i++) {
	    	if (isalpha(fpath[i]) && isupper(fpath[i]))
			fpath[i] = (char)tolower(fpath[i]);
		len++;
	}
	Debug0((dbg_fd,"Find file trying '%s'\n",fpath));
    	if(stat(fpath, st) == 0) return(TRUE);

	/* Force each component from the end to have upper case */
	i = len-1;
	while (i >= 0) {
		/* Check upper case version of component */
		for (; i >= 0; i--) {
			if (fpath[i] == SLASH) {
				i--;
				break;
			}
		    	if (isalpha(fpath[i]) && islower(fpath[i]))
				fpath[i] = (char)toupper(fpath[i]);
		}
		Debug0((dbg_fd,"Find file trying '%s'\n",fpath));
	    	if(stat(fpath, st) == 0) return(TRUE);
	}

	/* Restore to lower case */
	for (i = 0; fpath[i] != EOS; i++) {
	    	if (isalpha(fpath[i]) && isupper(fpath[i]))
			fpath[i] = (char)tolower(fpath[i]);
	}
	return (FALSE);
}

boolean_t compare(fname, fext, mname, mext)
	char * fname;
	char * fext;
	char * mname;
	char * mext;
{
	int i;

        Debug0((dbg_fd,"dos_gen: compare '%.8s'.'%.3s' to '%.8s'.'%.3s'\n",
			mname, mext, fname, fext));
	/* match name first */
	for(i=0;i<8;i++) {
		if(mname[i] == '?') {
			i++;
			continue;
		}
		if(mname[i] == ' ') {
			if (fname[i] == ' ') {
				break;
			}else{
				return(FALSE);
			}
		}
		if(mname[i] == '*') {
			break;
		}
		if (isalpha(mname[i]) && isalpha(fname[i])) {
			char x = isupper(mname[i]) ?
					mname[i] : toupper(mname[i]);
			char y = isupper(fname[i]) ?
					fname[i] : toupper(fname[i]);
			if (x != y) return(FALSE);
		} else if(mname[i] != fname[i]) {
			return(FALSE);
		}
	}
	/* if got here then name matches */
	/* match ext next */
	for(i=0;i<3;i++) {
		if(mext[i] == '?') {
			i++;
			continue;
		}
		if(mext[i] == ' ') {
			if (fext[i] == ' ') {
				break;
			}else{
				return(FALSE);
			}
		}
		if(mext[i] == '*') {
			break;
		}
		if (isalpha(mext[i]) && isalpha(fext[i])) {
			char x = isupper(mext[i]) ?
					mext[i] : toupper(mext[i]);
			char y = isupper(fext[i]) ?
					fext[i] : toupper(fext[i]);
			if (x != y) return(FALSE);
		} else if(mext[i] != fext[i]) {
			return(FALSE);
		}
	}
	return(TRUE);
}

struct dir_ent * match_filename_prune_list(list, name, ext)
	struct dir_ent * list;
	char * name;
	char * ext;
{
	int num_quest;
	u_char nq[13];
	int num_ast;
	u_char na[2];
	int i;
	struct dir_ent * last_ptr;
	struct dir_ent * tmp_ptr;
	struct dir_ent * first_ptr;

	/* special case checks */
	if ((strncmp(name, "????????", 8) == 0) &&
	    (strncmp(ext, "???", 3) == 0))
		return(list);

	/* more special case checks */
	if ((strncmp(name, "????????", 8) == 0) &&
	    (strncmp(ext, "   ", 3) == 0))
		return(list);

	first_ptr = NULL;
	last_ptr = NULL;

	while(list != NULL) {
		if(compare(list->name, list->ext, name, ext)) {
			if (first_ptr == NULL) {
				first_ptr = list;
			}
			last_ptr = list;
			tmp_ptr = list->next;
			list = tmp_ptr;
		}else{
			last_ptr->next = list->next;
			tmp_ptr = list->next;
			free(list);
			list = tmp_ptr;
		}
	}
	return(first_ptr);
}

#define HLIST_STACK_SIZE 32
struct dir_ent * hlist = NULL;
struct dir_ent * hlist_stack[HLIST_STACK_SIZE];
int hlist_stack_indx = 0;

boolean_t
hlist_push(hlist)
struct dir_ent * hlist;
{
	Debug0((dbg_fd,"hlist_push: %x\n",hlist_stack_indx));
	if (hlist_stack_indx >= HLIST_STACK_SIZE) {
		return (FALSE);
	} else {
		hlist_stack[hlist_stack_indx] = hlist;
		hlist_stack_indx++;
	}
	return (TRUE);
}

struct dir_ent *
hlist_pop()
{
	Debug0((dbg_fd,"hlist_pop: %x\n",hlist_stack_indx));
	if (hlist_stack_indx <= 0) return ((struct dir_ent *)NULL);
	hlist_stack_indx--;
	return (hlist_stack[hlist_stack_indx]);
}

void debug_dump_sft ( handle )
	char handle;
{
	u_short * ptr;
	u_char * sptr;
	int sftn;
	
	ptr = (u_short *)(FARPTR((far_t *)(lol + 0x4)));

	fprintf(dbg_fd,"SFT: han = %x, sftptr = %x\n",handle, ptr);

	/* Assume 3.1 or 3.3 Dos */
	sftn = handle;
	while (TRUE) {
		if ((*ptr == 0xffff) && (ptr[2] < sftn)) {
			fprintf(dbg_fd,"handle invalid.\n");
			break;
		}
		if (ptr[2] > sftn) {
			sptr = (u_char *)&ptr[3];
			while (sftn--)
				sptr += 0x35;	/* dos 3.1 3.3 */
			fprintf(dbg_fd,"handle_count = %x\n",
				sft_handle_cnt(sptr));
			fprintf(dbg_fd,"open_mode = %x\n",
				sft_open_mode(sptr));
			fprintf(dbg_fd,"attribute byte = %x\n",
				sft_attribute_byte(sptr));
			fprintf(dbg_fd,"device_info = %x\n",
				sft_device_info(sptr));
			fprintf(dbg_fd,"dev_drive_ptr = %x\n",
				sft_dev_drive_ptr(sptr));
			fprintf(dbg_fd,"starting cluster = %x\n",
				sft_start_cluster(sptr));
			fprintf(dbg_fd,"file time = %x\n",
				sft_time(sptr));
			fprintf(dbg_fd,"file date = %x\n",
				sft_date(sptr));
			fprintf(dbg_fd,"file size = %x\n",
				sft_size(sptr));
			fprintf(dbg_fd,"pos = %x\n",
				sft_position(sptr));
			fprintf(dbg_fd,"rel cluster = %x\n",
				sft_rel_cluster(sptr));
			fprintf(dbg_fd,"abs cluster = %x\n",
				sft_abs_cluster(sptr));
			fprintf(dbg_fd,"dir sector = %x\n",
				sft_directory_sector(sptr));
			fprintf(dbg_fd,"dir entry = %x\n",
				sft_directory_entry(sptr));
			fprintf(dbg_fd,"name = %.8s\n",
				sft_name(sptr));
			fprintf(dbg_fd,"ext = %.3s\n",
				sft_ext(sptr));
			return;
		}
		sftn -= ptr[2];
		ptr = (u_short *)Addr_8086(ptr[1],ptr[0]);
	}
}

boolean_t dos_fs_redirect(state)
	state_t * state;
{
	char * 	filename1;
	char * 	filename2;
	char * 	dta;
	u_char	attr;
	int	mode;
	int	fd;
	int	cnt;
	int ret = REDIRECT;
	cds_t	my_cds;
	sft_t  	sft;
	sdb_t	sdb;
	static boolean_t pointers_calculated = FALSE;
	static	boolean_t find_in_progress = FALSE;
    	int i;
    	int bs_pos;
    	char fname[8];
    	char fext[3];
    	char fpath[256];
    	char buf[256];
    	struct dir_ent * tmp;
	struct stat st;

	if(!mach_fs_enabled)
		return(REDIRECT);

	if (!pointers_calculated) {
		/* calculate_dos_pointers(); */
		pointers_calculated = TRUE;
	}

	my_cds = sda_cds(sda);

	Debug0((dbg_fd,"sda %x\n",sda));
	Debug0((dbg_fd,"cds %x\n",cds));

	sft = (u_char *) Addr(state, es, edi);

	/* Check for Write's bug! */
	if ((LOW(state->eax) == READ_FILE) &&
	    ((sft_device_info(sft) & 0x1f) == my_drive - 1)) {
		/* go on through */
	}else if ((!find_in_progress) && (my_cds != cds)) {
		Debug0((dbg_fd,"Not my cds: %x '%s' '%s'\n", my_cds, sda_filename1(sda),
			Addr(state,ds,esi)));
		return(REDIRECT);				
	}
	
	filename1 = sda_filename1(sda);
	filename2 = sda_filename2(sda);
	sdb = sda_sdb(sda);
	dta = sda_current_dta(sda);

	Debug0((dbg_fd,"CDS current path: %s\n",cds_current_path(cds)));
	Debug0((dbg_fd,"Filename1 %s\n",filename1));
	Debug0((dbg_fd,"Filename2 %s\n",filename2));
	Debug0((dbg_fd,"sft %x\n",sft));
	Debug0((dbg_fd,"dta %x\n",dta));
	fflush(NULL);
	
	switch(LOW(state->eax)) {
		case INSTALLATION_CHECK:	/* 0x00 */
			Debug0((dbg_fd,"Installation check\n"));
			return(TRUE);
		case REMOVE_DIRECTORY:		/* 0x01 */
		case REMOVE_DIRECTORY_2:	/* 0x02 */
			Debug0((dbg_fd,"Remove Directory\n"));
			build_ufs_path(fpath, filename1+2);
			if (find_file(fpath, &st)) {
			    if (rmdir(fpath,0755) != 0) {
				SETWORD(&(state->eax), PATH_NOT_FOUND);
			    	return(FALSE);
			    }				
			} else {
				SETWORD(&(state->eax), PATH_NOT_FOUND);
			    	return(FALSE);
			}
			return (TRUE);
		case MAKE_DIRECTORY:		/* 0x03 */
		case MAKE_DIRECTORY_2:		/* 0x04 */
			Debug0((dbg_fd,"Make Directory\n"));
			build_ufs_path(fpath, filename1 + 2);
			if (find_file(fpath, &st)) {
				Debug0((dbg_fd,"make failed already dir or file '%s'\n",
					fpath));
				SETWORD(&(state->eax), ACCESS_DENIED);
				return(FALSE);
			}
			if (mkdir(fpath,0755) != 0) {
			    	for(i=0,bs_pos = 0;fpath[i] != EOS;i++) {
					if (fpath[i] == SLASH)
					bs_pos = i;
			    	}
				strncpy(buf, fpath, bs_pos);
				buf[bs_pos] = EOS;
				find_file(buf, &st);
				strncpy(fpath, buf, bs_pos);
				Debug0((dbg_fd,"trying '%s'\n", fpath));
				if (mkdir(fpath,0755) != 0) {
					Debug0((dbg_fd,"make failed '%s'\n",
						fpath));
					SETWORD(&(state->eax),PATH_NOT_FOUND);
					return(FALSE);
				}
		    	}
			return (TRUE);
		case SET_CURRENT_DIRECTORY:	/* 0x05 */
			build_ufs_path(fpath, filename1 + 2);

		    	/* Try the given path */
		   	if(!find_file(fpath, &st)) {
				SETWORD(&(state->eax), PATH_NOT_FOUND);
				return(FALSE);
			}
		 	if (!(st.st_mode & S_IFDIR)) {
				SETWORD(&(state->eax), PATH_NOT_FOUND);
				Debug0((dbg_fd,"Set Directory %s ",fpath));
				Debug0((dbg_fd," not found.\n"));
			    	return(FALSE);
		    	}else{
				int i = strlen(filename1);
				/* Take the trailing slash off path if present */
				if (filename1[i - 1] == BACKSLASH)
					filename1[i - 1] = EOS;
				Debug0((dbg_fd,"Set Directory %s\n",fpath));
			}
			return(TRUE);
		case CLOSE_FILE:		/* 0x06 */
			fd  = sft_fd(sft);
			Debug0((dbg_fd,"Close file %x\n",fd));
			Debug0((dbg_fd,"Handle cnt %d\n",
						sft_handle_cnt(sft)));
			sft_handle_cnt(sft)--;
			if (sft_handle_cnt(sft) > 0) {
				Debug0((dbg_fd,"Still more handles\n"));
				return(TRUE);
			} else if (close(fd) != 0) {
				Debug0((dbg_fd,"Close file fails\n"));
				return(FALSE);
		    	}else{
				Debug0((dbg_fd,"Close file succeeds\n"));
				return(TRUE);
		    	}
		case READ_FILE: {		/* 0x08 */
			int return_val;
			cnt = WORD(state->ecx);
			fd  = sft_fd(sft);
			Debug0((dbg_fd,"Read file fd=%x, dta=%x,cnt=%d\n",
					fd, dta, cnt));
			Debug0((dbg_fd,"Read file pos = %d\n",
					sft_position(sft)));
			Debug0((dbg_fd,"Handle cnt %d\n",
						sft_handle_cnt(sft)));
			lseek(fd, sft_position(sft), L_SET);
		    	ret = dos_read(fd, dta, cnt);
			if (ret < 0) {
				return(FALSE);
		    	}else if (ret < cnt) {
			    	SETWORD(&(state->ecx), ret);
				return_val = TRUE;
		    	}else{
				SETWORD(&(state->ecx), cnt);
				return_val = TRUE;
		    	}
		    	sft_position(sft) += ret;
			sft_abs_cluster(sft) = 0x174a;	/* XXX a test */
			Debug0((dbg_fd,"File data %c %c %c\n",
					dta[0], dta[1], dta[2]));
			Debug0((dbg_fd,"Read file pos after = %d\n",
					sft_position(sft)));
			return(return_val);
		}
		case WRITE_FILE:		/* 0x09 */
			Debug0((dbg_fd,"Write file\n"));
			if (us_debug_level > Debug_Level_0) fflush(dbg_fd);
			cnt = WORD(state->ecx);
			fd  = sft_fd(sft);
			lseek(fd, sft_position(sft), L_SET);
		    	if (cnt == 0) {
				Debug0((dbg_fd,"write cnt = 0\n"));
			    	SETWORD(&(state->ecx), 0);
				return(TRUE);
		    	}
			Debug0((dbg_fd,"dta = %x, cnt = %x\n",dta,cnt));
			if (us_debug_level > Debug_Level_0) fflush(dbg_fd);
		    	ret = dos_write(fd, dta, cnt);
			Debug0((dbg_fd,"write operation done,ret=%x\n",ret));
			if (us_debug_level > Debug_Level_0) fflush(dbg_fd);
		    	if (ret < 0) return(FALSE);
		    	SETWORD(&(state->ecx), ret);
		    	sft_position(sft) += ret;
			sft_abs_cluster(sft) = 0x174a;	/* XXX a test */
			if (us_debug_level > Debug_Level_0) fflush(dbg_fd);
			return(TRUE);
		case GET_DISK_SPACE: {		/* 0x0c */
#ifdef USE_DF_AND_AFS_STUFF
			int free, tot;
			Debug0((dbg_fd,"Get Disk Space\n"));
			build_ufs_path(fpath,cds_current_path(cds)+2);
			if (find_file(fpath, &st)) {
				if (get_disk_space(fpath,&free,&tot)){
					int spc = 1;
					int bps = 512;
					int tmpf, tmpt;

					if ((tot > 256*256) || (free > 256*256)) {
						tmpf = free * bps * spc;
						tmpt = tot * bps * spc;

						spc = 8;
						bps = 1024;

						free = (tmpf / bps) / spc;
						tot = (tmpt / bps) / spc;
					}

					SETWORD(&(state->eax),spc);
					SETWORD(&(state->edx),free);
					SETWORD(&(state->ecx),bps);
					SETWORD(&(state->ebx),tot);
					Debug0((dbg_fd,
						"%d, %d, %d, %d\n",free,tot,bps,spc));
				
					return(TRUE);
				}else{
					Debug0((dbg_fd, "no ret gds\n"));
				}
			}
#endif /* USE_DF_AND_AFS_STUFF */
			break;
		}
		case SET_FILE_ATTRIBUTES:	/* 0x0e */
			Debug0((dbg_fd,"Set File Attributes\n"));
			break;
		case GET_FILE_ATTRIBUTES:	/* 0x0f */
			Debug0((dbg_fd,"Get File Attributes\n"));
			build_ufs_path(fpath, filename1 + 2);
			if(!find_file(fpath, &st)) {
				Debug0((dbg_fd,"Get failed: '%s'\n",fpath));
				SETWORD(&(state->eax), FILE_NOT_FOUND);
			    	return(FALSE);
		    	}
		    	if (st.st_mode & S_IFDIR) {
				SETWORD(&(state->eax), DIRECTORY);
		   	} else {
				if (st.st_mode & S_IWRITE) {
					SETWORD(&(state->eax),
					    ARCHIVE_NEEDED|REGULAR_FILE);
				} else {
					SETWORD(&(state->eax),
				            ARCHIVE_NEEDED|READ_ONLY_FILE);
				}
			}
			return(TRUE);
		case RENAME_FILE:		/* 0x11 */
			Debug0((dbg_fd,"Rename file\n"));
			build_ufs_path(fpath, filename2 + 2);
		    	for(i=0,bs_pos = 0;fpath[i] != EOS;i++) {
				if (fpath[i] == SLASH)
					bs_pos = i;
		    	}
			strncpy(buf, fpath, bs_pos);
			buf[bs_pos] = EOS;
			find_file(buf, &st);
			strncpy(fpath, buf, bs_pos);

			build_ufs_path(buf, filename1 + 2);
			if (!find_file(buf, &st)) {
			    	Debug0((dbg_fd,"Rename '%s' error.\n",fpath));
			    	SETWORD(&(state->eax), PATH_NOT_FOUND);
			    	return(FALSE);
			}

			if (rename(buf, fpath) != 0) {
				SETWORD(&(state->eax), PATH_NOT_FOUND);
			    	return(FALSE);
		    	}else{
				Debug0((dbg_fd,"Rename file %s to %s\n",
					fpath, buf));
				return(TRUE);
		   	}
		case DELETE_FILE:		/* 0x13 */
			Debug0((dbg_fd,"Delete file\n"));
			build_ufs_path(fpath, filename1 + 2);
		    	for(i=0,bs_pos = 0;fpath[i] != EOS;i++) {
				if (fpath[i] == SLASH)
					bs_pos = i;
		    	}
		    	fpath[bs_pos] = EOS;
		    	auspr(fpath+bs_pos+1, fname, fext);
			if (bs_pos == 0) {
				bs_pos = -1;
				strcpy(fpath,"/");
			}

		    	free_list(hlist);
		    	hlist = match_filename_prune_list(
					get_dir(fpath), fname, fext);

		    	if (hlist == NULL) {
				build_ufs_path(fpath, filename1 + 2);
				if(!find_file(fpath, &st)) {
					SETWORD(&(state->eax),FILE_NOT_FOUND);
					return(FALSE);
				}
				if (unlink(fpath) != 0) {
					Debug0((dbg_fd,"Deleted %s\n",fpath));
					SETWORD(&(state->eax),FILE_NOT_FOUND);
					return(FALSE);
				}
			    	return(TRUE);
		    	}else while (hlist != NULL) {
				if (hlist->mode & S_IFDIR) {
					goto delete_next;
				}
				strncpy(fpath+bs_pos+1, hlist->name, 8);
				fpath[bs_pos] = SLASH;
				fpath[bs_pos+9] = '.';
				fpath[bs_pos+13] = EOS;
				strncpy(fpath+bs_pos+10, hlist->ext, 3);
				strip_char(fpath, ' ');
				cnt = strlen(fpath);
				if (fpath[cnt-1] == '.') 
					fpath[cnt-1] = EOS;
				if (find_file(fpath, &st)) {
			    		unlink(fpath);
					Debug0((dbg_fd,"Deleted %s\n",fpath));
				}
delete_next:
				tmp = hlist->next;
				free(hlist);
			    	hlist = tmp;
		    	}
			return(TRUE);
		case OPEN_EXISTING_FILE:	/* 0x16 */
		    	mode = sda_open_mode(sda) & 0x3;
			attr = *(u_short *)Addr(state,ss,uesp);
			Debug0((dbg_fd,"Open existing file\n"));
			Debug0((dbg_fd,"mode, attr %x, %x\n", mode, attr));

do_open_existing:
			build_ufs_path(fpath, filename1 + 2);
			if(!find_file(fpath, &st)) {
				Debug0((dbg_fd,"open failed: '%s'\n",fpath));
				SETWORD(&(state->eax), FILE_NOT_FOUND);
			    	return(FALSE);
		    	}

		    	if (st.st_mode & S_IFDIR) {
				Debug0((dbg_fd,"S_IFDIR: '%s'\n",fpath));
				SETWORD(&(state->eax), FILE_NOT_FOUND);
				return(FALSE);
		   	}
		    	if (mode == READ_ACC) {
				mode = O_RDONLY;
		    	}else if (mode == WRITE_ACC) {
				mode = O_WRONLY;
			}else if (mode == READ_WRITE_ACC) {
				mode = O_RDWR;
		    	}else{
				Ddebug0((dbg_fd,"Illegal access_mode 0x%x\n",
				    				mode));
			    	mode = O_RDONLY;
		    	}
		    	if ((fd = open(fpath,mode)) < 0) {
			    	Debug0((dbg_fd,"access denied:'%s'\n",fpath));
			    	SETWORD(&(state->eax), ACCESS_DENIED);
			    	return(FALSE);
		   	 }

		    	for(i=0,bs_pos = 0;fpath[i] != EOS;i++) {
				if (fpath[i] == SLASH)
					bs_pos = i;
		    	}

		    	auspr(fpath+bs_pos+1, 
			      sft_name(sft), 
			      sft_ext(sft));
			sft_open_mode(sft) = sda_open_mode(sda) & 0x7f;
			sft_dev_drive_ptr(sft) = NULL;
			sft_directory_entry(sft) = 0;
			sft_directory_sector(sft) = 0;
#ifdef NOTEST
			sft_attribute_byte(sft) = attr;
#else
			sft_attribute_byte(sft) = 0x20;
#endif NOTEST
			sft_device_info(sft) = my_drive-1 + (0x8040);
			time_to_dos(&st.st_mtime, 
				    &sft_date(sft), &sft_time(sft));
			sft_size(sft) = st.st_size;
			sft_position(sft) = 0;
			sft_fd(sft) = fd;
		    	Debug0((dbg_fd,"open succeeds: '%s' fd = 0x%x\n",fpath,fd));
		    	return(TRUE);
		case CREATE_TRUNCATE_FILE:	/* 0x17 */
			Debug0((dbg_fd,"Create truncate file\n"));
			attr = *(u_short *)Addr(state,ss,uesp);

do_create_truncate:
			build_ufs_path(fpath, filename1 + 2);
			if(find_file(fpath, &st)) {
				if (!(st.st_mode & S_IFREG)) {
					SETWORD(&(state->eax), ACCESS_DENIED);
					Debug0((dbg_fd,"access denied '%s'\n",
						fpath));
					return(FALSE);
				}
		    	}

		    	for(i=0,bs_pos = 0;fpath[i] != EOS;i++) {
				if (fpath[i] == SLASH)
					bs_pos = i;
		    	}

		    	if ((fd = open(fpath,(O_RDWR|O_CREAT|O_TRUNC),
					    		    0664)) < 0) {
				strncpy(buf, fpath, bs_pos);
				buf[bs_pos] = EOS;
				find_file(buf, &st);
				strncpy(fpath, buf, bs_pos);
				Debug0((dbg_fd,"trying '%s'\n", fpath));
			    	if ((fd = open(fpath,(O_RDWR|O_CREAT|O_TRUNC),
					  	  		0664)) < 0) {
					Debug0((dbg_fd,"access denied '%s'\n",
						fpath));
					SETWORD(&(state->eax), ACCESS_DENIED);
					return(FALSE);
				}
		    	}

		    	auspr(fpath+bs_pos+1, sft_name(sft), sft_ext(sft));
			sft_dev_drive_ptr(sft) = NULL;
			sft_open_mode(sft) = 0x1;
			sft_directory_entry(sft) = 0;
			sft_directory_sector(sft) = 0;
			sft_attribute_byte(sft) = attr;
			sft_device_info(sft) = my_drive-1 + (0x8040);
			time_to_dos(&st.st_mtime, &sft_date(sft), 
						  &sft_time(sft));
			sft_size(sft) = 1024;
			sft_position(sft) = 0;
			sft_fd(sft) = fd;
		    	Debug0((dbg_fd,"create succeeds: '%s' fd = 0x%x\n",fpath,fd));
		    	Debug0((dbg_fd,"size = %x\n",sft_size(sft)));
			return(TRUE);
		case FIND_FIRST:		/* 0x1b */
			Debug0((dbg_fd,"Find first\n"));
			attr = sda_search_attribute(sda);

			if (find_in_progress) {
				if (!hlist_push(hlist)) 
					free_list(hlist);
			} else {
			    	free_list(hlist);
			}
			hlist = NULL;

		    	Debug0((dbg_fd,"attr = 0x%x\n",attr));
			if (attr & VOLUME_LABEL) {
				/* do vol label XXX */
			    	Debug0((dbg_fd,"DO LABEL!!\n"));
			    	auspr("Mach FS", fname, fext);
				strncpy(sdb_file_name(sdb), fname, 8);
				strncpy(sdb_file_ext(sdb), fext, 3);
			    	strncpy(sdb_template_name(sdb), fname, 8);
			  	strncpy(sdb_template_ext(sdb), fext, 3);
				sdb_file_attr(sdb) = VOLUME_LABEL;
				find_in_progress = TRUE;
			    	return(TRUE);
		    	}

			build_ufs_path(fpath, filename1 + 2);	

		    	for(i=0,bs_pos = 0;fpath[i] != EOS;i++) {
				if (fpath[i] == SLASH)
					bs_pos = i;
		    	}
		    	fpath[bs_pos] = EOS;

		    	auspr(fpath+bs_pos+1, fname, fext);
		    	strncpy(sdb_template_name(sdb), fname, 8);
		    	strncpy(sdb_template_ext(sdb), fext, 3);
			sdb_attribute(sdb) = attr;
			sdb_drive_letter(sdb) = 0x80 + my_drive - 1;
			
			if (bs_pos == 0) strcpy(fpath,"/");
		    	hlist = match_filename_prune_list(
					get_dir(fpath), fname, fext);

find_again:
		    	if (hlist == NULL) {
				/* no matches or empty directory */
			    	Debug0((dbg_fd,"No more matches\n"));
			    	SETWORD(&(state->eax), NO_MORE_FILES);
				hlist = hlist_pop();
				if (hlist == NULL)
					find_in_progress = FALSE;
			    	return(FALSE);
		    	}else{
			    	Debug0((dbg_fd,"'%.8s'.'%.3s'\n",
				    hlist->name, hlist->ext));
				
				if(hlist->mode & S_IFDIR) {
					sdb_file_attr(sdb) = 
					    ARCHIVE_NEEDED|DIRECTORY;
					if (!(attr & DIRECTORY)) {
						tmp = hlist->next;
						free(hlist);
			    			hlist = tmp;
						goto find_again;
					}
				}else{
					sdb_file_attr(sdb) = 
					    ARCHIVE_NEEDED|REGULAR_FILE;
				}
				time_to_dos(&hlist->time, 
					    &sdb_file_date(sdb),
					    &sdb_file_time(sdb));
				sdb_file_size(sdb) = hlist->size;
				strncpy(sdb_file_name(sdb), hlist->name, 8);
				strncpy(sdb_file_ext(sdb), hlist->ext, 3);

			    	Debug0((dbg_fd,"'%.8s'.'%.3s'\n",
				    sdb_file_name(sdb), 
				    sdb_file_ext(sdb)));

				tmp = hlist->next;
				free(hlist);
			    	hlist = tmp;
		    	}
			find_in_progress = TRUE;
		    	return(TRUE);
		case FIND_NEXT:			/* 0x1c */
			Debug0((dbg_fd,"Find next\n"));
			attr = sdb_attribute(sdb);
			goto find_again;
		case CLOSE_ALL:			/* 0x1d */
			Debug0((dbg_fd,"Close All\n"));
			break;
		case FLUSH_ALL_DISK_BUFFERS:	/* 0x20 */
			Debug0((dbg_fd,"Flush Disk Buffers\n"));
			break;
		case SEND_FROM_EOF:		/* 0x21 */
			Debug0((dbg_fd,"Send From EOF\n"));
			break;
		case QUALIFY_FILENAME:		/* 0x23 */
			filename1 = (char *)Addr(state, ds, esi);
			Debug0((dbg_fd,"Qualify filename %s\n",filename1));
			break;
		case LOCK_FILE_REGION:		/* 0x0a */
			Debug0((dbg_fd,"Lock file region\n"));
			break;
		case UNLOCK_FILE_REGION:	/* 0x0b */
			Debug0((dbg_fd,"Unlock file region\n"));
			break;
		case CREATE_TRUNCATE_NO_DIR:	/* 0x18 */
			Debug0((dbg_fd,"Create truncate no dir\n"));
			break;
		case PROCESS_TERMINATED:	/* 0x22*/
			Debug0((dbg_fd,"Process terminated\n"));
			if (find_in_progress) {
				while (hlist != NULL) {
					free_list(hlist);
					hlist = hlist_pop();
				}
			      	find_in_progress = FALSE;
			}
			return (REDIRECT);
#ifdef	WRONG
			restore_mouse_state();
			restore_interrupt_vectors();
			/* XXX do programs do this? */
		    	restore_video_mode();
#endif	WRONG
			return(REDIRECT);
		case CONTROL_REDIRECT:		/* 0x1e */
			Debug0((dbg_fd,"Control redirect\n"));
			break;
		case COMMIT_FILE:		/* 0x07 */
			Debug0((dbg_fd,"Commit\n"));
			break;
		case MULTIPURPOSE_OPEN: {
			boolean_t file_exists;
			u_short action = sda_ext_act(sda);
			mode = sda_ext_mode(sda) & 0x7f;
			attr = *(u_short *)Addr(state,ss,uesp);
			Debug0((dbg_fd,"Multipurpose open file:\n"));
			Debug0((dbg_fd,"Mode, action, attr = %x, %x, %x\n",
					mode, action, attr));

			build_ufs_path(fpath, filename1 + 2);
			file_exists = find_file(fpath, &st);

			if (((action & 0x10) == 0) && !file_exists) {
				/* Fail if file does not exist */
				SETWORD(&(state->eax), FILE_NOT_FOUND);
				return(FALSE);
			}

			if (((action & 0xf) == 0) && file_exists) {
				/* Fail if file does exist */
				SETWORD(&(state->eax), FILE_ALREADY_EXISTS);
				return(FALSE);
			}


			if (((action & 0xf) == 1) && file_exists) {
				/* Open if does exist */
				SETWORD(&(state->ecx), 0x1);
				goto do_open_existing;
			}


			if (((action & 0xf) == 2) && file_exists ) {
				/* Replace if file exists */
				SETWORD(&(state->ecx), 0x3);
				goto do_create_truncate;	
			}


			if (((action & 0x10) != 0) && !file_exists ) {
				/* Replace if file exists */
				SETWORD(&(state->ecx), 0x2);
				goto do_create_truncate;	
			}


			Debug0((dbg_fd,"Multiopen failed: %x\n",
							LOW(state->eax)));
			/* Fail if file does exist */
			SETWORD(&(state->eax), FILE_NOT_FOUND);
			return (FALSE);
		}
		case UNDOCUMENTED_FUNCTION_2:
			Debug0((dbg_fd,"Undocumented function: %x\n",
							LOW(state->eax)));
			return(TRUE);
	    	default:
			Debug0((dbg_fd,"Undocumented function: %x\n",
							LOW(state->eax)));
			return(REDIRECT);
	}
	return(ret);
}
