/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <mach/mach_interface.h>
#include <mach/vm_attributes.h>

#include <osfmach3/assert.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/server_thread.h>

#include <linux/tty.h>

/*
 * The format of "screen_info" is strange, and due to early
 * i386-setup code. This is just enough to make the console
 * code think we're on a EGA+ colour display.
 */
struct screen_info screen_info = {
	0, 0,			/* orig-x, orig-y */
	{ 0, 0 },		/* unused */
	0,			/* orig-video-page */
	0,			/* orig-video-mode */
	80,			/* orig-video-cols */
	0,			/* unused [short] */
	0,			/* ega_bx */
	0,			/* unused [short] */
	25,			/* orig-video-lines */
	0,			/* isVGA */
	16			/* video points */
};

int
get_cpuinfo(char *buffer)
{
	char *model;
#ifdef	CONFIG_OSFMACH3_DEBUG
	extern cpu_type_t cpu_type;
#endif	/* CONFIG_OSFMACH3_DEBUG */
	extern cpu_subtype_t cpu_subtype;

	ASSERT(cpu_type == CPU_TYPE_HPPA);
	switch (cpu_subtype) {
	    case CPU_SUBTYPE_HPPA_705:
		model = "705";
		break;
	    case CPU_SUBTYPE_HPPA_710:
		model = "710";
		break;
	    case CPU_SUBTYPE_HPPA_712:
		model = "712";
		break;
	    case CPU_SUBTYPE_HPPA_715:
		model = "715";
		break;
	    case CPU_SUBTYPE_HPPA_720:
		model = "720";
		break;
	    case CPU_SUBTYPE_HPPA_725:
		model = "725";
		break;
	    case CPU_SUBTYPE_HPPA_730:
		model = "730";
		break;
	    case CPU_SUBTYPE_HPPA_750:
		model = "750";
		break;
	    case CPU_SUBTYPE_HPPA_770:
		model = "770";
		break;
	    case CPU_SUBTYPE_HPPA_777:
		model = "777";
		break;
	    default:
		model = "unknown";
		break;
	}
	return sprintf(buffer, "HP9000 serie %s\n", model);
}

asmlinkage int sys_ioperm(unsigned long from, unsigned long num, int on)
{
	printk("sys_ioperm not implemented\n");
	return -EIO;
}

void
flush_instruction_cache_range(
	struct mm_struct	*mm,
	unsigned long		start,
	unsigned long		end)
{
#if 0
	/*
	 * If we have a split I/D cache, we need to maintain cache
	 * coherency after writing some instructions.
	 */
	vm_machine_attribute_val_t	val;
	mach_port_t			mach_task;
	vm_address_t			mach_start;
	vm_size_t			mach_size;
	kern_return_t			kr;

	mach_task = mm->mm_mach_task->mach_task_port;
	mach_start = (vm_address_t) start;
	mach_size = (vm_size_t) (end - start);
	val = MATTR_VAL_CACHE_FLUSH;
	kr = vm_machine_attribute(mach_task, mach_start, mach_size,
				  MATTR_CACHE, &val);
	if (kr != KERN_SUCCESS) {
		MACH3_DEBUG(1, kr,
			    ("flush_instruction_cache_range"
			     "(%p,0x%lx,0x%lx): "
			     "vm_machine_attribute(0x%x,0x%x,0x%x)",
			     mm, start, end,
			     mach_task, mach_start, mach_size));
	}
#else
	flush_cache(current->osfmach3.thread->regs_ptr->state.sr4, start, end);
#endif
}

