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

extern void	_ReadDateTime(), _SetDateTime();
extern void	_InitUtil(), _WriteParam();
extern void	_ReadXPRam(), _WriteXPRam();
extern void	_InsTime(), _RmvTime(), _PrimeTime();
extern void	_MemoryDispatch();

extern void	_Dequeue(), _Enqueue();

extern void	_SoundDispatch();
extern void	_SndDisposeChannel(), _SndAddModifier(), _SndDoCommand();
extern void	_SndDoImmediate(), _SndPlay(), _SndControl();
extern void	_SndNewChannel(); 

/*
 * Support for A-line trap emulation
 */

typedef struct emul_trap {
    void		(*routine)();
    unsigned short	trap;
} *emul_trap_t;

struct emul_trap	os_traps[] = {
    { _WriteParam,	0x0038 },
    { _ReadDateTime,	0x0039 },
    { _SetDateTime,	0x003a },
    { _InitUtil,	0x003f },
    { _ReadXPRam,	0x0051 },
    { _WriteXPRam,	0x0052 },
    { _InsTime,		0x0058 },
    { _RmvTime,		0x0059 },
    { _PrimeTime,	0x005a },
    { _MemoryDispatch,	0x005c },
    { 0,		0x0000 }
};

struct emul_trap	tbox_traps[] = {
    { _SoundDispatch,	0x0000 },
    { _SndDisposeChannel,
	  		0x0001 },
    { _SndAddModifier,	0x0002 },
    { _SndDoCommand,	0x0003 },
    { _SndDoImmediate,	0x0004 },
    { _SndPlay,		0x0005 },
    { _SndControl,	0x0006 },
    { _SndNewChannel,	0x0007 },
    { _Dequeue,		0x016e },
    { _Enqueue,		0x016f },
    { 0,		0x0000 }
};

void
emulation_init()
{
    (void) Emulate(mach_thread_self());

    (void) inst_patch_init();

    (void) emul_patch_init();

    (void) emul_init_os();
   
    (void) emul_init_tbox();

    (void) emul_device_init();
}

void
emul_init_os()
{
    register	emul_trap_t	t = os_traps;

    while (t->routine) {
	asm("movl %0,a0; movw %1,d0; .word 0xa247"
	    :
	    : "rm" (t->routine), "rm" (t->trap)
	    : "d0", "d1", "d2", "a0", "a1");
	t++;
    }
}

void
emul_init_tbox()
{
    register	emul_trap_t	t = tbox_traps;

    while (t->routine) {
	asm("movl %0,a0; movw %1,d0; .word 0xa647"
	    :
	    : "rm" (t->routine), "rm" (t->trap)
	    : "d0", "d1", "d2", "a0", "a1");
	t++;
    }
}
