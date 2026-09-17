/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: kkt.h,v $
 * Revision 1.1.4.3  1996/07/31  20:44:19  paire
 * 	Merged with changes from 1.1.4.2
 * 	[1996/07/31  20:32:14  paire]
 *
 * 	Merged with changes from 1.1.4.2
 * 	[1996/07/31  14:50:23  paire]
 *
 * 	Merged with nmk20b7_shared (1.1.2.1)
 * 	[96/06/07            paire]
 *
 * Revision 1.1.2.1  1996/05/10  16:36:46  ferranti
 * 	First version. Adapted from intel version.
 * 	[1996/05/10  16:19:43  ferranti]
 * 
 * $EndLog$
 */

#ifndef _HP_PA_KKT_H_
#define _HP_PA_KKT_H_

#include <norma_scsi.h>
#include <dipc_xkern.h>

#if     NORMA_SCSI
#define	splkkt	splbio
#elif   DIPC_XKERN
/*
 * With DIPC/KKT/x-kernel, the whole protocol stack 
 * is executed in thread context. Thus, splkkt results 
 * in a no-op.
 */
#include <cpus.h>
#if     NCPUS > 1

#if     defined(__GNUC__)
#include <kern/spl.h>
#include <hp_pa/cpu_number.h>

#warning This code has been compiled but not tested

extern int curr_ipl[];
extern spl_t __inline__ splkkt(void);

extern spl_t __inline__ splkkt(void) {
	spl_t   ss;
	disable_preemption();
	ss = (spl_t)curr_ipl[cpu_number()];
	enable_preemption();
	return  ss;
}
#else   /* __GNUC__ */
#error  Not implemented
#endif  /* __GNUC__ */

#else   /* NCPUS == 1 */

extern spl_t get_spl(void);
#define	splkkt()	get_spl()

#endif  /* NCPUS == 1 */
#endif  /* DIPC_XKERN */

typedef struct transport {
	struct request_block 	*next;
#if	NORMA_SCSI
	void *kkte;
#else	/* NORMA_SCSI */
        vm_offset_t save_address;
        vm_size_t save_size;
        boolean_t save_sg;
#endif	/* NORMA_SCSI */
} transport_t;

#endif  /* _HP_PA_KKT_H_ */
