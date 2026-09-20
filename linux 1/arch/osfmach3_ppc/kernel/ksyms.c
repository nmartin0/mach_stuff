/*
 * Copyright (c) 1991-1998 Open Software Foundation, Inc. 
 *  
 * 
 */
/*
 * MkLinux
 *
 * Fri May 22 1998 Tom Rini <trini@kernel.crashing.org>
 *	- fixed up previous changes
 */

#include <linux/config.h>

#include <linux/module.h>
#include <linux/smp.h>

#ifdef CONFIG_OSFMACH3
#include <linux/string.h>
#include <asm/segment.h>
#endif /* CONFIG_OSFMACH3 */

extern void put_user_byte(char, char *);
extern unsigned long __mem_base;

/* For Linux/PPC module compatability */
extern void _enable_interrupts(void);
extern void _disable_interrupts(void);
extern void __save_flags(void);
extern void __restore_flags(void);
extern void cli(void);
extern void sti(void);
extern void adb_mouse_interrupt_hook(void);
extern int misc_register, misc_deregister;
extern int bad_user_access_length(void);
extern int console_loglevel(void);

static struct symbol_table arch_symbol_table = {
#include <linux/symtab_begin.h>
	/* platform dependent support */
	X(put_user_byte),
#ifdef	CONFIG_OSFMACH3
	X(bad_user_access_length),
	X(strcat),
	X(strcmp),
	X(strcpy),
	X(strlen),
	X(strncmp),
	X(strncpy),
	X(strnlen),
	X(strstr),
	X(strtok),
	X(strchr),
	X(memcmp),
	X(memmove),
	X(memcpy),
	X(memset),
	X(memscan),
#endif	/* CONFIG OSFMACH3 */
	X(__mem_base),
	X(_enable_interrupts),
	X(_disable_interrupts),
	X(__save_flags),
	X(__restore_flags),
	X(__put_user),
	X(__get_user),
	X(cli),
	X(sti),
	X(get_ds),
	X(set_fs),
#if defined(CONFIG_MACMOUSE) || defined(CONFIG_MACMOUSE_MODULE)
	X(console_loglevel),
	X(adb_mouse_interrupt_hook),
	X(misc_register),
	X(misc_deregister),
#endif
#include <linux/symtab_end.h>
};

void arch_syms_export(void)
{
	register_symtab(&arch_symbol_table);
}
