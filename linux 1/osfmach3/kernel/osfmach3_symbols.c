/*
 * Copyright (c) 1991-1998 Open Software Foundation, Inc. 
 *  
 * 
 */
/*
 * MkLinux
 *
 * Fri May 22 1998 Tom Rini <trini@kernel.crashing.org>
 *	- new file (hold all symbols for osfmach3 arches)
 *	- not sure if some of these symbols are needed on hppa or x86
 *	  but were missing for ppc, so..
 */

#include <linux/config.h>
#include <linux/module.h>

#include <linux/string.h>
#include <asm/segment.h>

extern int bad_user_access_length(void);
extern void *__get_instruction_pointer(void);
extern void dump_thread(void);
extern void __ashrdi3(void);

#include <linux/locks.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ppp.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/unistd.h>
#include <asm/checksum.h>
#ifdef CONFIG_SERIAL_MODULE
#include <osfmach3/device_reply_hdlr.h>
#include <osfmach3/sched.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/server_thread.h>
#include <osfmach3/mach_init.h>
#include <osfmach3/uniproc.h>
#include <osfmach3/device_utils.h>
#endif /* CONFIG_SERIAL_MODULE */

static struct symbol_table osfmach3_syms = {
#include <linux/symtab_begin.h>
/* General symbols, many modules depend on a few of these... */
	X(atomic_add),
	X(atomic_sub),
	X(atomic_inc_return),
	X(atomic_dec_return),
	X(bmap),
	X(blkdev_open),
	X(blkdev_release),
	X(block_fsync),
	X(block_read),
	X(block_write),
	X(current_set),
	X(clear_bit),
	X(cthread_self),
	X(cthread_set_data),
	X(cthread_data),
	X(cthread_exit),
	X(cthread_set_name),
	X(cthread_detach),
	X(dev_tint),
	X(dump_thread),
	X(dev_alloc_skb),
	X(dev_kfree_skb),
	X(ether_setup),
	X(eth_type_trans),
	X(expand_stack),
	X(find_first_zero_bit),
	X(find_next_zero_bit),
	X(ffz),
	X(force_sig),
	X(getblk),
	X(inb),
	X(inode_pager_uncache),
	X(ip_fast_csum),
	X(iput),
	X(invalidate_inode_pages),
	X(kfree_skb),
	X(kernel_thread),
	X(ll_rw_block),
	X(mark_buffer_uptodate),
	X(netif_rx),
	X(outb),
	X(__osfmach3_yield),
	X(register_blkdev),
	X(register_netdev),
	X(refile_buffer),
	X(set_writetime),
	X(start_thread),
	X(set_bit),
	X(unregister_netdev),
	X(udelay),
	X(xchg_u32),
	X(__ashrdi3),
	X(__brelse),
	X(__get_instruction_pointer),
	X(__wait_on_buffer),

#ifdef CONFIG_SERIAL_MODULE /* Other OSF-spec modules might need some, but
				no others exist yet.. */
	X(server_thread_start),
	X(server_security_token),
	X(mach3_debug_msg),
	X(mach3_debug),
	X(dev_tint),
	X(dump_thread),
	X(dev_alloc_skb),
	X(dev_kfree_skb),
	X(device_open),
	X(device_set_status),
	X(device_get_status),
	X(device_read_inband),
	X(device_reply_register),
	X(device_reply_deregister),
	X(device_server_port),
	X(device_close),
	X(mutex_unlock_solid),
	X(mutex_lock_solid),
	X(serv_device_write_async),
	X(free_area_page_dirty),
	X(task),
	X(uniproc_antechamber_mutex),
	X(uniproc_mutex),
	X(spin_unlock),
	X(spin_try_lock),
	X(mach_port_destroy),
	X(init_mm),
	X(mach_task_self_),
	X(use_antechamber_mutex),
#endif /* CONFIG_SERIAL_MODULE */

#include <linux/symtab_end.h>
};

void export_osfmach3_symbols(void)
{
	register_symtab(&osfmach3_syms);
}
