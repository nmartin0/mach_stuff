/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


#include <linux/autoconf.h>

#include <linux/module.h>
#include <linux/smp.h>

extern put_user_byte(char, char *);

static struct symbol_table arch_symbol_table = {
#include <linux/symtab_begin.h>
	/* platform dependent support */
#include <linux/symtab_end.h>
};

void arch_syms_export(void)
{
	register_symtab(&arch_symbol_table);
}



