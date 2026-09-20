/*
 * Copyright (c) 1991 Carnegie Mellon University
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
 *
 * Purpose:
 * 	VGA graphics via assembly
 *
 * HISTORY: 
 * $Log:	bios_graphics.s,v $
 * Revision 2.2  91/12/05  16:40:17  grm
 * 	Created.
 * 	[91/06/28  18:03:35  grm]
 * 
 */
	.text
/*
 * void
 * vga_and_write(ptr, left, right, over)
 *		u_char * ptr;
 *		u_char left, right, over;
 */
	.align	2
	.globl	_vga_and_write
_vga_and_write:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edx
	pushl	%ebx
	pushl	%eax			/* do we need to save eax? */

	movw	$0x3cf, %edx		/* outb (GRAPHICS_DATA, 0xXX) */
	movb	$0x28, %al
	outb	%al, %dx

	movl	0x08(%ebp), %edx	/* ptr -> edx */

	movb	0x0c(%ebp), %bl		/* left -> bl */
	movb	(%edx), %al		/* dummy = *ptr */
	movb	%bl, (%edx)

	incl	%edx

	movb	0x10(%ebp), %bl		/* right -> bl */
	movb	(%edx), %al
	movb	%bl, (%edx)

	incl	%edx

	movb	0x14(%ebp), %bl		/* over -> bl */
	movb	(%edx), %al
	movb	%bl, (%edx)

	popl	%eax
	popl	%ebx
	popl	%edx
	popl	%ebp

	ret


/*
 * void
 * vga_xor_write(ptr, left, right, over)
 *		u_char * ptr;
 *		u_char left, right, over;
 */
	.align	2
	.globl	_vga_xor_write
_vga_xor_write:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edx
	pushl	%ebx
	pushl	%eax			/* do we need to save eax? */

	movw	$0x3cf, %edx		/* outb (GRAPHICS_DATA, 0xXX) */
	movb	$0x10, %al
	outb	%al, %dx

	movl	0x08(%ebp), %edx	/* ptr -> edx */

	movb	0x0c(%ebp), %bl		/* left -> bl */
	movb	(%edx), %al		/* prime VGA latch */
	movb	%bl, (%edx)

	incl	%edx

	movb	0x10(%ebp), %bl		/* right -> bl */
	movb	(%edx), %al		/* prime VGA latch */
	movb	%bl, (%edx)

	incl	%edx

	movb	0x14(%ebp), %bl		/* over -> bl */
	movb	(%edx), %al		/* prime VGA latch */
	movb	%bl, (%edx)

	popl	%eax
	popl	%ebx
	popl	%edx
	popl	%ebp

	ret
