/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

#include "prio.h"
#include "inst.h"
#include "access.h"

#include <mac2/psl.h>

/*
 * Privileged instruction emulation.
 */

#define FATAL()								\
{									\
    return (INST_ERROR);						\
}

#define RETRY()								\
{									\
    return (INST_RETRY);						\
}

#define INST_ACTION_0_EXTEN(nnn)					\
inst_##nnn##(inst, state)	      					\
unsigned short				inst;  				\
register inst_state_t			*state;				\
{                                        				\
    register struct status_reg		*sr =				\
	(struct status_reg *)&state->frame->f_normal.f_sr;		\
    {

#define INST_ACTION_1_EXTEN(nnn)   					\
inst_##nnn##(inst, state)						\
unsigned short				inst;  				\
register inst_state_t			*state;				\
{									\
    register struct status_reg		*sr =				\
	(struct status_reg *)&state->frame->f_normal.f_sr;		\
    unsigned short			exten;				\
    FETCH_INST(exten);							\
    {

#define INST_EMUL(xxx) \
    { xxx; }

#define INST_CC(xxx) \
    { xxx; }

#define INST_END() } return (INST_DONE); }

#define DEFINE_CELL(name)	\
    struct cell		name;	\
    name.c_longword = 0

#define DEFINE_MULT_CELL(name, nelt)	\
    struct cell		name[nelt];	\
    {					\
	register _n;			\
					\
	for (_n = 0; _n < nelt; _n++)	\
	    name[_n].c_longword = 0;	\
    }

static unsigned		ipl_to_prio_map[] = {
    PRIO_NONE,		/* IPL 0 */
    PRIO_LOW,		/* IPL 1 */
    PRIO_HIGH,		/* IPL 2 */
    PRIO_HIGH,		/* IPL 3 */
    PRIO_HIGH,		/* IPL 4 */
    PRIO_HIGH,		/* IPL 5 */
    PRIO_HIGH,		/* IPL 6 */
    PRIO_MAX,		/* IPL 7 */
};

#define inst_set_prio(x, thread) \
{								\
    if (prio_set_thread(thread,					\
			ipl_to_prio_map[((struct status_reg *)	\
					 &(x))->sr_ipl]) < 0)	\
					     RETRY();		\
}

static unsigned		prio_to_ipl_map[] = {
    SR_IPL0,		/* PRIO_NONE */
    SR_IPL1,		/* PRIO_LOW */
    SR_IPL2,		/* PRIO_HIGH */
    SR_IPL7,		/* PRIO_MAX */
};

#define inst_get_prio(x, thread) \
{						\
    if (((x) = prio_get_thread(thread)) < 0)	\
	RETRY();				\
\
    (x) = prio_to_ipl_map[(x)];			\
}

#define UPDATE_r_SR() \
    ((struct status_reg *)&r_SR)->sr_cc = state->frame->f_normal.f_sr

/*
 * The current
 * value of the PC
 */
#define PC_VALUE	(state->frame->f_normal.f_pc)

/*
 * Return the next
 * word from the instruction
 * stream.
 */
#define FETCH_INST(x)	FETCH_INCR(PC_VALUE, &(x), unsigned short)

/*
 * Instruction emulation
 * routines.
 */

INST_ACTION_1_EXTEN(andi_sr)
     unsigned short		r_SR = 0;
     INST_EMUL(UPDATE_r_SR();
	       r_SR &= exten;
	       inst_set_prio(r_SR, state->thread);
	       INST_CC(sr->sr_cc = r_SR));
INST_END()

INST_ACTION_1_EXTEN(eori_sr)
     unsigned short		r_SR = 0;
     INST_EMUL(UPDATE_r_SR();
	       r_SR ^= (exten&~SR_SUPR);
	       inst_set_prio(r_SR, state->thread);
	       INST_CC(sr->sr_cc = r_SR));
INST_END()

INST_ACTION_1_EXTEN(ori_sr)
     unsigned short		r_SR = 0;
     INST_EMUL(UPDATE_r_SR();
	       r_SR |= (exten&~SR_SUPR);
	       inst_set_prio(r_SR, state->thread);
	       INST_CC(sr->sr_cc = r_SR));
INST_END()

INST_ACTION_0_EXTEN(move_f_sr)
     struct {
	 unsigned short :10,
                    mode:3,
                     reg:3;
     } *i = (typeof (i))&inst;
     unsigned short		r_SR = 0;
     register	ipl;
     INST_EMUL(DEFINE_CELL(data);
	       UPDATE_r_SR();
	       inst_get_prio(ipl, state->thread);
	       r_SR |= ipl;
	       data.c_loword = r_SR;
	       get_thread_regs(state);
	       store_operand(&data,
			     i->mode, i->reg,
			     OPSIZE_WORD, state));
INST_END()

INST_ACTION_0_EXTEN(move_t_sr)
     struct {
	 unsigned short :10,
                    mode:3,
                     reg:3;
     } *i = (typeof (i))&inst;
     unsigned short		r_SR = 0;
     INST_EMUL(DEFINE_CELL(data);
	       get_thread_regs(state);
	       fetch_operand(&data,
			     i->mode, i->reg,
			     OPSIZE_WORD, state);
	       r_SR = (data.c_loword&~SR_SUPR);
	       inst_set_prio(r_SR, state->thread);
	       INST_CC(sr->sr_cc = r_SR));
INST_END()

INST_ACTION_0_EXTEN(move_usp)
     struct {
	 unsigned short :12,
                     dir:1,
                     reg:3;
     } *i = (typeof (i))&inst;
     FATAL();
INST_END()

INST_ACTION_1_EXTEN(movec)
     struct {
	 unsigned short :15,
                     dir:1;
     } *i = (typeof (i))&inst;
     struct {
	 unsigned short gregtyp:1,
                           greg:3,
                           creg:12;
     } *e = (typeof (e))&exten;
     FATAL();
INST_END()

INST_ACTION_1_EXTEN(moves)
     struct {
	 unsigned short :8,
                    size:2,
                    mode:3,
                     reg:3;
     } *i = (typeof (i))&inst;
     struct {
	 unsigned short gregtyp:1,
                           greg:3,
                            dir:1,
	                       :11;
     } *e = (typeof (e))&exten;
     FATAL();
INST_END()

INST_ACTION_0_EXTEN(reset)
     FATAL();
INST_END()

INST_ACTION_0_EXTEN(rte)
     INST_EMUL(generic_exception_frame_t	*rframe;
	       get_thread_regs(state);
	       rframe = (generic_exception_frame_t *)state->regs->r_sp;
	       switch (rframe->f_normal.f_fmt) {
		 case STKFMT_SHORT_BUSERR:
		   bcopy(state->regs->r_sp,
			 state->frame,
			 SHORT_BUSERR_EXCEPTION_FRAME_SIZE);
		   state->regs->r_sp += SHORT_BUSERR_EXCEPTION_FRAME_SIZE;
		   state->frame_count = SHORT_BUSERR_EXCEPTION_FRAME_SIZE
		       / sizeof (int);
		   break;

		 case STKFMT_LONG_BUSERR:
		   bcopy(state->regs->r_sp,
			 state->frame,
			 LONG_BUSERR_EXCEPTION_FRAME_SIZE);
		   state->regs->r_sp += LONG_BUSERR_EXCEPTION_FRAME_SIZE;
		   state->frame_count = LONG_BUSERR_EXCEPTION_FRAME_SIZE
		       / sizeof (int);
		   break;

		 default:
		   FATAL();
	       });
INST_END()

INST_ACTION_1_EXTEN(stop)
     FATAL();
INST_END()

INST_ACTION_0_EXTEN(cpBcc)
     struct {
	 unsigned short :4,
                    cpid:3,
	                :2,
                    size:1,
                      cc:6;
     } *i = (typeof (i))&inst;
     FATAL();
INST_END()

INST_ACTION_1_EXTEN(cpDBcc)
     struct {
	 unsigned short :4,
                    cpid:3,
	                :6,
                    dreg:3;
     } *i = (typeof (i))&inst;
     struct {
	 unsigned short :10,
                      cc:6;
     } *e = (typeof (e))&exten;
     FATAL();
INST_END()

INST_ACTION_1_EXTEN(cpGEN)
     struct {
	 unsigned short :4,
                    cpid:3,
	                :3,
                    mode:3,
                     reg:3;
     } *i = (typeof (i))&inst;
     struct {
	 unsigned short op:3,
                     subop:3,
	                  :10;
     } *e = (typeof (e))&exten;
     FATAL();
INST_END()

INST_ACTION_0_EXTEN(cpRESTORE)
     struct {
	 unsigned short :4,
                    cpid:3,
	                :3,
                    mode:3,
                     reg:3;
     } *i = (typeof (i))&inst;
     if (i->cpid == 0) {
	 INST_EMUL(FATAL());
     } else if (i->cpid == 1) {
	 INST_EMUL(get_thread_regs(state);
		   fetch_frestore_operand(state, i->mode, i->reg);
		   state->fp_frame_count =
		   (state->fp_frame->fpf_size
		    + sizeof (state->fp_frame->fpf_format) / sizeof (int)));
     } else
        FATAL();
INST_END()

INST_ACTION_0_EXTEN(cpSAVE)
     struct {
	 unsigned short :4,
                    cpid:3,
	                :3,
                    mode:3,
                     reg:3;
     } *i = (typeof (i))&inst;
     if (i->cpid == 0) {
	 INST_EMUL(FATAL());
     } else if (i->cpid == 1) {
	 INST_EMUL(get_thread_regs(state);
		   get_thread_fp_frame(state);
		   store_fsave_operand(state, i->mode, i->reg);
		   state->fp_frame_count = 0);
     } else
        FATAL();
INST_END()

INST_ACTION_1_EXTEN(cpScc)
     struct {
	 unsigned short :4,
                    cpid:3,
	                :3,
                    mode:3,
                     reg:3;
     } *i = (typeof (i))&inst;
     struct {
	 unsigned short :10,
                      cc:6;
     } *e = (typeof (e))&exten;
     FATAL();
INST_END()

INST_ACTION_1_EXTEN(cpTRAPcc)
     struct {
	 unsigned short :4,
                    cpid:3,
	                :7,
                      op:3;
     } *i = (typeof (i))&inst;
     struct {
	 unsigned short :10,
                      cc:6;
     } *e = (typeof (e))&exten;
     FATAL();
INST_END()
