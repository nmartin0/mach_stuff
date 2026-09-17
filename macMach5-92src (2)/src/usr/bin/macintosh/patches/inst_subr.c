/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

#include <mach.h>

#include <th.h>
#include <inst.h>
#include <access.h>

/*
 * Privileged instruction emulation
 * support.
 */

/*
 * Return the current
 * value of the PC
 */
#define PC_VALUE	(state->frame->f_normal.f_pc)

/*
 * Return the next
 * word from the instruction
 * stream and store it in
 * the variable x.
 */
#define FETCH_INST(x)	FETCH_INCR(PC_VALUE, &(x), unsigned short)

#define FETCH_BYTE(p, x)	FETCH((p), &(x), unsigned char)
#define STORE_BYTE(p, x)	STORE((p), &(x), unsigned char)

#define FETCH_WORD(p, x)	FETCH((p), &(x), unsigned short)
#define STORE_WORD(p, x)	STORE((p), &(x), unsigned short)

#define FETCH_LONG(p, x)	FETCH((p), &(x), unsigned long)
#define STORE_LONG(p, x)	STORE((p), &(x), unsigned long)

extern	inst_andi_sr(), inst_eori_sr(), inst_ori_sr();
extern	inst_move_f_sr(), inst_move_t_sr();
extern	inst_cpRESTORE(), inst_cpSAVE();
extern	inst_rte();
int	inst_error();

static struct inst_table {
    int			(*action)();
    unsigned short	code;
    unsigned short	mask;
    boolean_t		patch;
} inst_table[] = {
    { inst_move_f_sr,	0x40C0,	0xFFC0, TRUE	},
    { inst_move_t_sr,	0x46C0, 0xFFC0, TRUE	},
    { inst_ori_sr,	0x007C, 0xFFFF, TRUE	},
    { inst_cpSAVE,	0xF100, 0xF1C0, FALSE	},
    { inst_cpRESTORE,	0xF140, 0xF1C0, FALSE	},
    { inst_rte,		0x4E73, 0xFFFF, FALSE	},
    { inst_andi_sr,	0x027C, 0xFFFF, TRUE	},
    { inst_eori_sr,	0x0A7C, 0xFFFF, TRUE	},
    { inst_error,	0x0000, 0x0000, FALSE	}	/* end of table */
};

inst_error()
{
    return (INST_ERROR);
}

unsigned inst_count;

kern_return_t
priv_inst(thread)
{
    inst_state_t		state;
    thread_state_regs_t		regs;
    thread_state_frame_t	frame;
    thread_state_fpframe_t	fp_frame;
    register			inst_result;

    state.thread = thread;

    state.regs = &regs;
    state.frame = &frame;
    state.fp_frame = &fp_frame;

    state.frame_count = THREAD_STATE_FRAME_COUNT;
    if (thread_get_state(thread,
			 THREAD_STATE_FRAME,
			 state.frame, &state.frame_count) != KERN_SUCCESS)
	return (KERN_FAILURE);

    state.regs_count = state.fp_frame_count = 0;

    {
	register struct inst_table	*t = inst_table;
	unsigned short			inst;
    
	/*
	 * fetch the instruction
	 * word and increment the pc
	 */
	FETCH_INCR(state.frame->f_normal.f_pc, &inst, unsigned short);

	while (TRUE) {
	    if ((inst & t->mask) == t->code) {
		if (t->patch)
		    patch_inst(inst, t - inst_table, &state);

		inst_result = (*t->action)(inst, &state);
		break;
	    }

	    t++;
	}

	if (inst_result == INST_ERROR)
	    return (KERN_FAILURE);
    }
    inst_count++;

    if (inst_result == INST_DONE) {
	if (state.regs_count > 0)
	    (void) thread_set_state(thread,
				    THREAD_STATE_REGS,
				    state.regs, state.regs_count);

	if (state.fp_frame_count > 0)
	    (void) thread_set_state(thread,
				    THREAD_STATE_FPFRAME,
				    state.fp_frame, state.fp_frame_count);

	(void) thread_set_state(thread,
				THREAD_STATE_FRAME,
				state.frame, state.frame_count);
    }

    return (KERN_SUCCESS);
}

void
patched_inst(regs, frame)
thread_state_regs_t	regs;
thread_state_frame_t	frame;
{
    register th_t	th = th_self();
    inst_state_t	state;
    int			inst_result;

    if (th == (th_t)0)
	state.thread = mach_thread_self();
    else
	state.thread = th->thread;

    state.regs = &regs;
    state.regs_count = THREAD_STATE_REGS_COUNT;

    state.frame = &frame;
    state.frame_count = sizeof (normal_exception_frame_t) / sizeof (int);

    {
	register unsigned short		inst;
	int				index;

	inst = patch_inst_lookup(frame.f_normal.f_pc, &index);

	inst_result = (*(inst_table + index)->action)(inst, &state);

	if (inst_result != INST_DONE)
	    stop(frame.f_normal.f_pc, inst);
    }
}

void
get_thread_regs(state)
register inst_state_t	*state;
{
    if (state->regs_count == 0) {
	state->regs_count = THREAD_STATE_REGS_COUNT;
	(void) thread_get_state(state->thread,
				THREAD_STATE_REGS,
				state->regs, &state->regs_count);
    }
}

void
get_thread_fp_frame(state)
register inst_state_t	*state;
{
    if (state->fp_frame_count == 0) {
	state->fp_frame_count = THREAD_STATE_FPFRAME_COUNT;
	(void) thread_get_state(state->thread,
				THREAD_STATE_FPFRAME,
				state->fp_frame, &state->fp_frame_count);
    }
}

/*
 * fetch_operand is used for instruction
 * emulation.  it fetches the operand
 * based on the effective addressing
 * mode, and returns the operand value
 * in the memory cell pointed at by opnd.
 */
fetch_operand(opnd, mode, reg, size, state)
register struct cell		*opnd;
register			mode, reg, size;
register inst_state_t		*state;
{
    switch (mode) {
      case 0:
      case 1:
	/*
	 * Register Direct modes
	 */
	{
	    register struct cell	*greg;
	    
	    if (mode == 0)
		greg = (struct cell *)&state->regs->r_dreg[reg];
	    else
		greg = (struct cell *)&state->regs->r_areg[reg];
	    
	    switch (size) {
	      case OPSIZE_BYTE:
		opnd->c_lsbyte = greg->c_lsbyte;
		break;
		
	      case OPSIZE_WORD:
		opnd->c_loword = greg->c_loword;
		break;
		
	      case OPSIZE_LONG:
		opnd->c_longword = greg->c_longword;
		break;
	    }
	    break;
	}
	
      default:
	/*
	 * Immediate mode
	 */
	if (mode == 7 && reg == 4) {
	    switch (size) {
	      case OPSIZE_BYTE:
	      case OPSIZE_WORD:
		FETCH_INST(opnd->c_loword);
		break;
		
	      case OPSIZE_LONG:
		FETCH_INST(opnd->c_hiword);
		FETCH_INST(opnd->c_loword);
		break;
		
	      case OPSIZE_DBLONG:
		FETCH_INST(opnd->c_hiword);
		FETCH_INST(opnd->c_loword);
		FETCH_INST((opnd+1)->c_hiword);
		FETCH_INST((opnd+1)->c_loword);
		break;
	    }
	}
	else {
	    /*
	     * All others
	     */
	    struct cell		*m;
	    
	    calc_ea(&m, mode, reg, size, state);
	    switch (size) {
	      case OPSIZE_BYTE:
		FETCH_BYTE(&m->c_byte, opnd->c_lsbyte);
		break;
		
	      case OPSIZE_WORD:
		FETCH_WORD(&m->c_word, opnd->c_loword);
		break;
		
	      case OPSIZE_LONG:
		FETCH_LONG(&m->c_longword, opnd->c_longword);
		break;
		
	      case OPSIZE_DBLONG:
		FETCH_LONG(&m->c_longword, opnd->c_longword);
		FETCH_LONG(&(m+1)->c_longword, (opnd+1)->c_longword);
		break;
	    }
	}
    }
}

/*
 * store_operand is used for instruction
 * emulation.  a pointer to the operand
 * is passed in opnd.  the operand is stored
 * based on the effective addressing mode.
 */
store_operand(opnd, mode, reg, size, state)
register struct cell		*opnd;
register			mode, reg, size;
register inst_state_t		*state;
{
    switch (mode) {
      case 0:
      case 1:
	/*
	 * Register Direct modes
	 */
	{
	    register struct cell	*greg;
	    
	    if (mode == 0)
		greg = (struct cell *)&state->regs->r_dreg[reg];
	    else
		greg = (struct cell *)&state->regs->r_areg[reg];
	    
	    switch (size) {
	      case OPSIZE_BYTE:
		greg->c_lsbyte = opnd->c_lsbyte;
		break;
		
	      case OPSIZE_WORD:
		greg->c_loword = opnd->c_loword;
		break;
		
	      case OPSIZE_LONG:
		greg->c_longword = opnd->c_longword;
		break;
	    }
	    break;
	}
	
      default:
	/*
	 * All others
	 */
	{
	    struct cell		*m;
	    
	    calc_ea(&m, mode, reg, size, state);
	    switch (size) {
	      case OPSIZE_BYTE:
		STORE_BYTE(&m->c_byte, opnd->c_lsbyte);
		break;
		
	      case OPSIZE_WORD:
		STORE_WORD(&m->c_word, opnd->c_loword);
		break;
		
	      case OPSIZE_LONG:
		STORE_LONG(&m->c_longword, opnd->c_longword);
		break;
		
	      case OPSIZE_DBLONG:
		STORE_LONG(&m->c_longword, opnd->c_longword);
		STORE_LONG(&(m+1)->c_longword, (opnd+1)->c_longword);
		break;
	    }
	}
    }
}

/*
 * calc_ea is used to calculate the
 * address of an operand in memory
 * for all cases except immediate data.
 * the address of the operand is returned
 * to the cell pointer pointed at by aea.
 */
calc_ea(aea, mode, reg, size, state)
struct cell			**aea;
int				mode, size;
register			reg;
register inst_state_t		*state;
{
    register struct exten_full	*extf;
    register struct exten_brief *extb;
    register unsigned long	temp, index;
    register			e_pc;
    unsigned long		indir;
    unsigned short		exten;
    struct cell			disp;
    
    switch (mode) {
	/*
	 * Address register indirect
	 */
      case 2:
	*aea = (struct cell *)state->regs->r_areg[reg];
	break;
	
	/*
	 * Address register indirect
	 * with postincrement
	 */
      case 3:
	*aea = (struct cell *)state->regs->r_areg[reg];
	switch (size) {
	  case OPSIZE_BYTE:
	    state->regs->r_areg[reg]++;
	    break;
	    
	  case OPSIZE_WORD:
	    state->regs->r_areg[reg] += 2;
	    break;
	    
	  case OPSIZE_LONG:
	    state->regs->r_areg[reg] += 4;
	    break;
	    
	  case OPSIZE_DBLONG:
	    state->regs->r_areg[reg] += 8;
	    break;
	}
	break;
	
	/*
	 * Address register indirect
	 * with predecrement
	 */
      case 4:
	switch (size) {
	  case OPSIZE_BYTE:
	    state->regs->r_areg[reg]--;
	    break;
	    
	  case OPSIZE_WORD:
	    state->regs->r_areg[reg] -= 2;
	    break;
	    
	  case OPSIZE_LONG:
	    state->regs->r_areg[reg] -= 4;
	    break;
	    
	  case OPSIZE_DBLONG:
	    state->regs->r_areg[reg] -= 8;
	    break;
	}
	*aea = (struct cell *)state->regs->r_areg[reg];
	break;
	
	/*
	 * Address register indirect
	 * with displacment
	 */
      case 5:
	temp = state->regs->r_areg[reg];
	FETCH_INST(disp.c_loword);
	temp += (short)disp.c_loword;
	*aea = (struct cell *)temp;
	break;
	
	/*
	 * Address register indirect
	 * with index
	 */
      case 6:
      PCspecial:
	e_pc = PC_VALUE;
	FETCH_INST(exten);
	extb = (struct exten_brief *)&exten;
	if (extb->ext_full == 0) {
	    /*
	     * Index with 8 bit displacement
	     */
	    if (mode == 7)
		temp = e_pc;
	    else
		temp = state->regs->r_areg[reg];
	    temp += (char)(extb->ext_disp);
	    index = (extb->ext_iregtyp?
		     state->regs->r_areg[reg]: state->regs->r_dreg[reg]);
	    if (extb->ext_isize == 0)
		index = (short)index;
	    if (extb->ext_scale)
		index *= (1<<extb->ext_scale);
	    temp += index;
	}
	else {
	    /*
	     * Index with base and outer
	     * displacements
	     */
	    register flags = 0;
#define F_MEMIND  0x01
#define F_PRENDX  0x02
#define F_OUTDISP 0x04
	    temp = 0;
	    extf = (struct exten_full *)&exten;
	    if (extf->ext_isel)
		flags |= F_MEMIND;
	    if ((flags&F_MEMIND) && extf->ext_isel < 4)
		flags |= F_PRENDX;
	    if ((extf->ext_isel&3) > 1)
		flags |= F_OUTDISP;
	    if (extf->ext_bs == 0) {
		if (mode == 7)
		    temp = e_pc;
		else
		    temp = state->regs->r_areg[reg];
	    }
	    switch (extf->ext_bdsize) {
	      case 2:
		FETCH_INST(disp.c_loword);
		temp += (short)disp.c_loword;
		break;
		
	      case 3:
		FETCH_INST(disp.c_hiword);
		FETCH_INST(disp.c_loword);
		temp += disp.c_longword;
		break;
	    }
	    if ((flags&(F_MEMIND|F_PRENDX)) == F_MEMIND) {
		FETCH_LONG(temp, indir);
		temp = indir;
	    }
	    if (extf->ext_is == 0) {
		index = (extf->ext_iregtyp? state->regs->r_areg[reg]: state->regs->r_dreg[reg]);
		if (extf->ext_isize == 0)
		    index = (short)(index&0xffff);
		if (extf->ext_scale)
		    index *= (1<<extf->ext_scale);
		temp += index;
	    }
	    if ((flags&(F_MEMIND|F_PRENDX)) == (F_MEMIND|F_PRENDX)) {
		FETCH_LONG(temp, indir);
		temp = indir;
	    }
	    if (flags&F_OUTDISP) {
		switch (extf->ext_isel&3) {
		  case 2:
		    FETCH_INST(disp.c_loword);
		    temp += (short)disp.c_loword;
		    break;
		    
		  case 3:
		    FETCH_INST(disp.c_hiword);
		    FETCH_INST(disp.c_loword);
		    temp += disp.c_longword;
		    break;
		}
	    }
	}
#undef F_MEMIND
#undef F_PRENDX
#undef F_OUTDISP
	*aea = (struct cell *)temp;
	break;
	
      case 7:
	switch (reg) {
	  case 0:
	    FETCH_INST(disp.c_loword);
	    temp = (short)disp.c_loword;
	    *aea = (struct cell *)temp;
	    break;
	    
	  case 1:
	    FETCH_INST(disp.c_hiword);
	    FETCH_INST(disp.c_loword);
	    *aea = (struct cell *)disp.c_longword;
	    break;
	    
	  case 2:
	    temp = PC_VALUE;
	    FETCH_INST(disp.c_loword);
	    temp += (short)disp.c_loword;
	    *aea = (struct cell *)temp;
	    break;
	    
	  case 3:
	    goto PCspecial;
	}
	break;
    }
}

fetch_frestore_operand(state, mode, reg)
register inst_state_t	*state;
register		mode, reg;
{
    register fp_frame_t	*fp = state->fp_frame;
    struct cell		*m;

    calc_ea(&m, mode, reg, OPSIZE_LONG, state);
    FETCH(&m->c_longword, fp, unsigned long);
    FETCH_N((m+1), &fp->fpf_data, fp->fpf_size);
    if (mode == 3)
	state->regs->r_areg[reg] += fp->fpf_size;
}

store_fsave_operand(state, mode, reg)
register inst_state_t	*state;
register		mode, reg;
{
    register fp_frame_t	*fp = state->fp_frame;
    struct cell		*m;

    if (mode == 4)
	state->regs->r_areg[reg] -= (fp->fpf_size+4);
    calc_ea(&m, mode, reg, OPSIZE_NONE, state);
    STORE_N(m, fp, fp->fpf_size+4);
}

sign_extend(opnd, size)
register struct cell	*opnd;
register		size;
{
    switch (size) {
      case OPSIZE_BYTE:
	opnd->c_longword = (char)opnd->c_lsbyte;
	break;
	
      case OPSIZE_WORD:
	opnd->c_longword = (short)opnd->c_loword;
	break;
    }
}
