/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: uniproc.h,v $
 * Revision 1.1.2.4  1997/10/15  16:34:55  barbou
 * 	When a threads working for a user process tries to get the
 * 	"uniproc_mutex", make it take the "uniproc_antechamber_mutex" first
 * 	to reduce contention on the "uniproc_mutex" and let system threads
 * 	take it first.
 * 	[1997/10/15  16:20:56  barbou]
 *
 * Revision 1.1.2.3  1997/01/28  13:27:45  barbou
 * 	Added definitions for server_thread_blocking/unblocking (inline for
 * 	non-DEBUG configurations).
 * 	[1997/01/28  13:16:05  barbou]
 * 
 * Revision 1.1.2.2  1996/09/10  18:02:21  barbou
 * 	Updated the usage of "current".
 * 	[96/09/10            barbou]
 * 
 * Revision 1.1.2.1  1996/09/09  16:58:53  barbou
 * 	Added prototype for holding_uniproc.
 * 	[96/09/02            barbou]
 * 
 * 	Created.
 * 	[1996/08/21  15:51:59  barbou]
 * 
 * $EndLog$
 */

#ifndef	_OSFMACH3_UNIPROC_H_
#define _OSFMACH3_UNIPROC_H_

#include <linux/autoconf.h>

#include <cthreads.h>
#include <osfmach3/server_thread.h>

#include <linux/sched.h>

extern boolean_t		use_antechamber_mutex;
extern struct mutex		uniproc_antechamber_mutex;
extern struct mutex		uniproc_mutex;

#if	CONFIG_OSFMACH3_DEBUG

extern void uniproc_enter(void), uniproc_exit(void);
extern void uniproc_has_entered(void), uniproc_will_exit(void);
extern void uniproc_switch_to(struct task_struct *old_task,
			      struct task_struct *new_task);
extern void set_current_task(struct task_struct *current_task);
extern struct task_struct *get_current_task(void);

extern void uniproc_preemptible(void);
extern void uniproc_unpreemptible(void);

extern void server_thread_blocking(boolean_t preemptible);
extern void server_thread_unblocking(boolean_t preemptible);

#else	/* CONFIG_OSFMACH3_DEBUG */

static __inline__ struct task_struct
*get_current_task(void)
{
	struct server_thread_priv_data	*priv_datap;

	priv_datap = server_thread_get_priv_data(cthread_self());
	return priv_datap->current_task;
}

static __inline__ void
uniproc_has_entered(void)
{
	current_set[smp_processor_id()] = get_current_task();
}

static __inline__ void
uniproc_enter(void)
{
	if (use_antechamber_mutex) {
		struct server_thread_priv_data	*priv_datap;

		priv_datap = server_thread_get_priv_data(cthread_self());
		if (priv_datap->current_task &&
		    priv_datap->current_task->mm == &init_mm) {
			/*
			 * We're working for a system thread: carry on.
			 */
			mutex_lock(&uniproc_mutex);
		} else {
			/*
			 * We're working for a user thread: we don't want
			 * loads of user threads contending with system
			 * threads on the uniproc mutex, so take the
			 * uniproc_antechamber_mutex first to ensure that
			 * only one user thread will be contending with
			 * system threads at a given time.
			 */
			mutex_lock(&uniproc_antechamber_mutex);
			mutex_lock(&uniproc_mutex);
			mutex_unlock(&uniproc_antechamber_mutex);
		}
	} else {
		mutex_lock(&uniproc_mutex);
	}
	uniproc_has_entered();
}

static __inline__ void
set_current_task(
	struct task_struct *current_task)
{
	struct server_thread_priv_data	*priv_datap;

	priv_datap = server_thread_get_priv_data(cthread_self());
	priv_datap->current_task = current_task;
}

static __inline__ void
uniproc_change_current(
	struct task_struct *old_task,
	struct task_struct *new_task)
{
	current_set[smp_processor_id()] = new_task;
	set_current_task(current);
}

static __inline__ void
uniproc_switch_to(
	struct task_struct *old_task,
	struct task_struct *new_task)
{
	uniproc_change_current(old_task, new_task);
}

static __inline__ void
uniproc_will_exit(void)
{
	set_current_task(current);
	current_set[smp_processor_id()] = (struct task_struct *) NULL;
}

static __inline__ void
uniproc_exit(void)
{
	uniproc_will_exit();
	mutex_unlock(&uniproc_mutex);
}

static __inline__ void
uniproc_preemptible(void)
{
	uniproc_exit();
}

static __inline__ void
uniproc_unpreemptible(void)
{
	uniproc_enter();
}

static __inline__ void
server_thread_blocking(boolean_t preemptible)
{
	uniproc_exit();
}
static __inline__ void
server_thread_unblocking(boolean_t preemptible)
{
	uniproc_enter();
}

#endif	/* CONFIG_OSFMACH3_DEBUG */

extern boolean_t holding_uniproc(void);

#endif	/*_OSFMACH3_UNIPROC_H_*/
