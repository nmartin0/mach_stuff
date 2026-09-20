/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * HISTORY
 * $Log:	lock.s,v $
 * Revision 2.6  91/05/14  17:58:13  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:20:54  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:38:54  mrt]
 * 
 * Revision 2.4  90/01/22  23:09:49  af
 * 	Conditionalized timing functions.
 * 	[90/01/20  17:34:21  af]
 * 
 * Revision 2.3  89/12/08  19:49:24  rwd
 * 	Reflected rwd's name changes.
 * 	[89/12/06            af]
 * 
 * Revision 2.2  89/11/29  14:19:08  af
 * 	New name for tas, fixed timing functions.
 * 	[89/10/28  12:11:22  af]
 * 
 * 	Turned into a piece of literate programming.
 * 	Therefore:
 * 		Copyright (c) 1989 Alessandro Forin
 * 	[89/07/16            af]
 * 
 * 	Created.
 * 	[89/07/06            af]
 * 
 */
/*
 * File:    pmax/lock.s

		Test-And-Set instructions for MIPS.


				Abstract

   	    The MIPS instruction set does not include any
	    interlocked instruction to provide synchronization
	    in parallel programs.  We provide two different
	    implementations of a non-blocking lock/unlock pair
	    despite this limitation.

 
   1. Introduction

   There are basically two possible types of solutions.   Either
   we invoke help from the Operating System or we find some clever
   algorithm that does not require any intelocked instruction.
   Obviously, a solution in the second class looks more appealing
   for performance reasons.  It is unlikely that a solution in the
   first class could avoid e.g. trapping in the OS kernel which is
   usually a very expensive operation.  Note, however, that MIPS
   provides a pretty fast trapping mechanism so things are not
   all black-and-white and some measurements might be necessary
   in order to asses what the best choice is.

   The first approach includes, for instance, providing some
   proper addition to the MIPS instruction set via software emulation.
   The idea is to use some invalid instruction opcode which therefore
   generates an Illegal Instruction trap.  The OS handler for this trap
   checks for the special opcode, and in case it changes the state
   of the user program so that the proper semantic of our pseudo-instruction
   is realized.
   The second approach means a trip to the library to browse through
   the literature to see whether some published algorithm does have
   the special property we need.  Or, inventing a new algorithm if
   no suitable one can be found.

   The first idea requires a minumum amount of work so it was quickly
   realized. It is therefore described at the beginning of this file,
   in Chapter 2.  The second idea was also pursued with success, albeit
   in a little more time. Chapter 3 describes how we found a (presumably) 
   new algorithm with the necessary properties by modifying an existing one.


   2. Sofware Emulation

   Since we decide to go for emulation, we can chose basically anything
   we please. All is needed, however, is a simple non-blocking lock and
   the obvious choice is therefore a Test-And-Set instruction. To make
   things as simple as possible (speed comes first) the semantic is the
   following. TAS operates on a single register which holds the address
   of the lock. The previous content of the lock will be returned in that
   same register, and the lock itself will contain some non-zero value.
   Since the compiler will never generate this instruction, we'll further
   restrict the instruction to only operate on one particular register,
   register "a0".

   Implementing the user part is trivial. All we need is an assembly
   function that uses the new opcode that we will add to the MIPS
   instruction set.
 */
#include <mach/mips/asm.h>
#include <mach/mips/mips_instruction.h>
/*
   The C interface for this function is

	boolean_t
	spin_try_lock_sw(m)
	int * m;

   The function has a slightly different semantics than TAS: it will
   return a boolean value that indicates whether the lock was acquired
   or not.  If not, we'll presume that the user will retry after some
   appropriate delay.
 */
	.text
	.align	2
	.set	noreorder

LEAF(spin_try_lock)
	move	v0,a0		# preserve a0's content
	.word	op_tas		# do the TAS
	j	ra		# return whether the
	xor	v0,a0,v0	# lock was acquired
	END(spin_try_lock)

LEAF(spin_unlock)
	j	ra
	sw	zero,0(a0)
END(spin_unlock)

	.set	reorder
/*
   We make use here of one more piece of information: the value
   that TAS puts in the lock is the address of the lock itself.
   This makes things fit into four instructions, but adding a
   branch instruction would only waste one extra cycle and buy
   more generality.  A truly general purpose implementation would
   also follow a test&TAS scheme, by testing the content of the
   lock before executing the (expensive) TAS instruction.
   In our case though, it is known that the CThreads wrapping
   for this function already does that before calling the function
   itself.  The unlock primitive is quite strightforward.  Note
   only that the existing code in the CThreads package already
   makes machine-independent assumptions about the value (and size!)
   of a lock, so there really is no choice here.

   Now we need only to implement the instruction in the kernel, after
   deciding which opcode we should employ.
   Note that the class of "special" opcodes is the most suitable
   one since it already includes instructions like syscall and break
   that do not fit in any other general category.  Curiously enough, in that
   same class there is a sub-opcode for an instruction named "vcall"
   which is neither described in [Kane 88] which is our reference book
   for the MIPS instruction set, nor appears to do anything but generate
   an Illegal Instruction exception when tested on a DECStation 3100.
   Our choice then is the very next sub-opcode

#define op_tas		0x0F

   which we add to the original "inst.h" file.

   The next thing is to write a trap handler for the Illegal Instruction
   trap.  Currently the VEC_trap() handler is in charge of handling this
   exception too. By modifying the dispatch table appropriately

extern VEC_syscall(), VEC_cpfault(), VEC_trap(), VEC_int(), VEC_tlbmod();
extern VEC_tlbmiss(), VEC_breakpoint(), VEC_addrerr(), VEC_ibe(), VEC_dbe();
extern VEC_ill_instr();
extern VEC_unexp();
int  (*causevec[16])() = {
	-*  0: EXC_INT	 *-		VEC_int,
	-*  1: EXC_MOD	 *-		VEC_tlbmod,
	-*  2: EXC_RMISS *-		VEC_tlbmiss,
	-*  3: EXC_WMISS *-		VEC_tlbmiss,
	-*  4: EXC_RADE	 *-		VEC_addrerr,
	-*  5: EXC_WADE	 *-		VEC_addrerr,
	-*  6: EXC_IBE	 *-		VEC_ibe,
	-*  7: EXC_DBE	 *-		VEC_dbe,
	-*  8: EXC_SYSCALL *-	 	VEC_syscall,
	-*  9: EXC_BREAK *-		VEC_breakpoint,
	-* 10: EXC_ILL	 *-		VEC_ill_instr,
	-* 11: EXC_CPU	 *-		VEC_cpfault,
	-* 12: EXC_OV	 *-		VEC_trap,
	-* 13: undefined *-		VEC_unexp,
	-* 14: undefined *-		VEC_unexp,
	-* 15: undefined *-		VEC_unexp
};

  we can easily add our new VEC_ill_instr() handler, which we'll
  add to the kernel's file of low-level operations.
  The handler proper is as follows.

VECTOR(VEC_ill_instr, M_EXCEPT)
	.set	reorder

     Check whether this is an emulated instruction or not.

	lw	ra,EF_EPC*4(sp)		# get user's PC
	li	a2,op_tas
	lw	a0,0(ra)		# get the instruction proper
	bne	a0,a2,truly_ill		# is this our TAS

     Now that it is clear that it is precisely a TAS instruction,
     all we need to do is to keep interrupts disabled while we emulate
     it, and guard against faults.

	lw	a0,EF_A0*4(sp)		# Get address of lock
					# Protect against malicious uses, like
	bgt	a0,zero,addr_ok		# sneaking into kernel space
	sw	a0,EF_BADVADDR*4(sp)	# Give an exception to
	li	a1,SEXC_ILL		# the sneaker.
	b	truly_ill
addr_ok:

    Use the nofault trick e.g. if we fault on a bad
    user address we'll endup (by magic) in uaerror() below

	li	a2,NF_USERACC
	sw	a2,nofault

    Now it should be safe to proceed.
    There is a subtle semantic question about a TAS
    if it fails: does it actually write or not ?
    For instance, should a user that does a tas
    on e.g. his protected text segment get a bus error or not ?
    Both choices are easy to provide, so we'll define a 
    compile time switch to choose among them.

	lw	a2,0(a0)		# 	.. TEST ..
#ifdef	tas_may_not_write
	bne	a2,zero,1f		# 	.. AND ..
	sw	a0,0(a0)		# 	.. SET
#else	tas_may_not_write

    The default behaviour is indeed to perform the write 
    anyways, which should make it easier for users to 
    debug their mistakes.
    
	sw	a0,0(a0)		# 	.. AND SET
#endif	tas_may_not_write

1:	sw	a2,EF_A0*4(sp)		# return the result still in a0
	sw	zero,nofault

    All done, increment PC and return

	addiu	ra,4			# increment user's PC
	sw	ra,EF_EPC*4(sp)
	b	exception_exit
truly_ill:
	move	a0,sp
	b	VEC_trap
	END(VEC_ill_instr)

LEAF(uaerror)

     User gave a bad address.  Give him a SEGV in trap()
     that stems from a write miss.

	li	a1,SEXC_SEGV
	li	a3,EXC_WMISS
	sw	a3,EF_CAUSE*4(sp)
	b	truly_ill
	END(uaerror)

   2.1 Performance

   The user side of the primitive costs 4 cycles. This is easy
   to see since a call to this function (after checking that
   the lock is free) will find the lock in the cache, so we'll
   take precisely one cycle per instruction.
   The kernel part is instead much more complicated.  There we
   have to consider the average effects of page faults, misses
   in the TLB etc etc.  The simplest and most effective measure
   is therefore an actual run of some test program.  We did that 
   by repeatedly calling the spin_try_lock_sw() function
   and the result is about 9 microseconds per call, or 138 cycles
   on a DECStation 3100.  Clearly the costs on the user side are
   a minor fraction of the total cost.


   3 A New Algorithm   

   A quick look at the literature produced an interesting finding.
   Lamport's article [Lamport 88] on formal correctness proofs of
   parallel programs contains a small fragment of code which is
   very interesting.  The example at page ??? provides precisely
   mutual exclusion among N processes and without requiring any
   hardware support other that atomic load and store operations 
   on memory, which are available on all sensible memory systems.

   There are limits in the code, however, that will have to be 
   overcome.  For instance, as described the code will block
   forever all of the processes but the one that acquires the lock,
   should conflict arise.  We are not overly concerned about
   issues such as liveness and fairness: it is most certainly possible
   to prove that some scheduling sequences might theoretically
   generate livelocks, but these in practice will not arise
   because of the large number of factors that affect scheduling 
   of user programs on a real multiprocessor.
   Moreover, our initial target machines are all uniprocessor.

   Our first attempt at modifying the code as follows fails.

	1	x = me
	2	if (y != -1) return FAIL
	3	y = me
	4	if (x != me) return FAIL
	5	return SUCCESS

   The idea came from flying on an oversold flight to Pittsburgh:
   line 1 is doing the check-in at the gate, line 4 is actually
   checking the seat on the airplane.  It took six hours to that
   flight to get from NewYork to Pittsburgh, and it was not
   the algorithm's fault ! But we had time for thinking..

   The failure sequence for the algorithm (which does happen about
   2 in a million times on a DECStation 3100 running Mach) is   

	A12 B1 A34

  where A and B denote two different processes.  This sequence
  will leave the lock busy but return FAIL to both processes.
  Note that there is still the guarantee, proved by Lamport,
  that only one process at a time will ever get to execute 5.
  This is because the basic idea is that the last process to
  take the reservation at line 1 is the favored one to acquire
  the lock.  Unfair maybe, but works. Even on airplanes!
  Therefore the following variation over the original code
  should do the trick for us.

	1	x = me
	2	if (y != -1) return FAIL
	3	z = me
	4	if (x != me) return FAIL
	5	y = me
	6	if (z != me) return FAIL
	7	return SUCCESS

  Note that we introduced one more variable, which basically
  acts as the "reservation for the reservation".  So in our
  airline methaphor we'd go to the ticket counter to check
  our seats (line 1-2), and maybe change flight if it is oversold.
  Then we'd go to the gate for control (lines 3-4)
  and finally get to our seat in the airplane (lines 5-6).
  We can only, however, make guarantees for our computer programs
  and not for airline companies, so Caveat User!

  As far as the unlocking primitive we will stick to the above
  function.  This is because of the above mentioned constrain
  that CThreads imposes e.g. that a free lock be an integer of
  size 32 bits and value 0.  This has also another subtle implication
  on our algorithm:  We cannot just "return FAIL" at lines 4 and 6.
  If we did so we would live the lock free allright, but the cover
  code would notice that that some reserve field is non-zero and
  mistake that for a busy lock!  Since those failures are due to
  a collision between two processes in the attempt to acquire an
  otherwise free lock, it is certainly within the semantics of
  "try to acquire the lock" that we only give up when the lock 
  is really busy.  Our fix therefore is to substitute the return 
  statements with a jump back to line 1, which is what the cover
  code should have done anyways.
  
  Coding the function in assembly is quite straightforward,
  except for noting that we have to (a) fit everything in a
  32 bit word and (b) make a 32 bit "0" the value of an
  empty lock.  The first requirement is easy to accomodate,
  splitting the word as follows:

	   16       8       8
	-------------------------
	|   y   |   x   |   z   |
	-------------------------

  This is because MIPS provides both 8/16/32 bit load and store
  operations.
  The second requirement is also easy, since the values of the
  extra x and z fields do not really matter as long as we do not
  allow "0" to be the identifier of any of the processes.

  So for our purposes a lock can be defined as

	typedef struct {
		unsigned int
			busy : 16,
			res1 : 8,
			res2 : 8;
	} spin;

 */
#	define busy		0
#	define res1		2
#	define res2		3

	.align	2
	.set	noreorder
/*
  Here is our final and preferred implementation for

	boolean_t
	spin_try_lock(m)
		spin * m;

 */
LEAF(spin_try_lock_unsafe)
#define my_id		t0

  # We need to generate an ID on the fly. Using the stack pointer is
  # ok, but note that we can only extract 8bits from it. We think that
  # the uppermost 2nd and 3rd nibbles should be a reliable choice, but
  # it still is a quite arbitrary one.

	sll	my_id,sp,4
	srl	my_id,my_id,24
again:
	sb	my_id,res1(a0)		# line 1
	lhu	t1,busy(a0)		# line 2
	nop
	beq	t1,zero,1f
	nop
	bne	t1,my_id,fail
1:

  # The delay slot in this branch can be filled by the
  # next instruction.

	sb	my_id,res2(a0)		# line 3
	lbu	t1,res1(a0)		# line 4
	nop
	bne	t1,my_id,again

  # Ditto

	sh	my_id,busy(a0)		# line 5
	lbu	t1,res2(a0)		# line 6
	nop
	bne	t1,my_id,again
	nop

pass:					# line 7
	j	ra
	li	v0,1
fail:
	j	ra
	move	v0,zero
#undef my_id
	END(spin_try_lock_sw)
/*

   3.1 Performance

   Should be 14 stright cycles in the good outcome,
   since we do 3 loads and 3 stores but on the same
   cache line, actually the same word. The cache should
   not miss, because as explained above the value of the
   lock was tested right before invoking this function.
   In reality, the cache is write-through rather than
   write-back and moreover our 3 store instructions are
   nicely separated by 2 intervening instructions each.
   Therefore the MIPS write-buffer will not always be
   able to shrink them into one single store to memory.
   The actually measured number is infact 35 cycles in
   the good outcome and 19 on a busy lock.

   This still compares very well with the 138 cycles
   of the "firmware" solution.  Moreover, in case the 
   lock is actually busy this last implementation takes
   less time. On collisions the number of (nominal) cycles is
   14 + 7N + 11M, depending on where the collision is detected.
   Testing on the DECStation 3100 reveals that collisions are
   quite rare (less than one in a million).  A true multiprocessor
   machine is then needed to asses their frequency and relevance
   when processes are really executing concurrently.


   4 Conclusions

   The problem of providing a mutual exclusion primitive
   without hardware support was analyzed.  Two different 
   approaches, namely provide support in the Operating 
   System or find a user-level suitable algorithm, were both
   explored successfully.  Two real-word working solutions
   were described in detail, and their performance compared.
   A new algorithm, derived from one by Lamport, was presented,
   implemented on the MIPS processor, and tested.
   The algorithmic solution proves to be about 4 times faster
   that the one that receives support from the OS.


   5. Bibliography

   [Kane 88]
	Gerry Kane
	"mips RISC Architecture"
	Prentice Hall, NJ, 1988

   [Lamport 88]
	Leslie Lamport
	"Control Predicatres are Better than Dummy Variables
	 for Reasoning about Program Control"
	ACM TOPLAS, Vol. 10-2, April 1988, pp. 267-281
 */
#ifdef	TIMEIT
	.set	noreorder
#define FRAME (4*4)
NESTED(time_spin_try_lock_sw, FRAME, s0)
	move	s0,ra

	move	a2,a0
1:	move	a0,a2
	sw	zero,0(a0)
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	jal	spin_try_lock_sw
	subu	a1,a1,1
	bne	a1,zero,1b
	nop

	move	ra,s0
	j	ra
	END(time_spin_try_lock_sw)

NESTED(time_spin_try_lock, FRAME, s0)
	move	s0,ra

	move	a2,a0
1:	jal	spin_try_lock
	move	a0,a2
	sw	zero,0(a0)
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	subu	a1,a1,1
	bne	a1,zero,1b
	nop

	move	ra,s0
	j	ra
	END(time_spin_try_lock)

#endif	TIMEIT
