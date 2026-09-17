#ifndef _HPPA_SYSCALL_H
#define _HPPA_SYSCALL_H

#define _NR(n) #n
/***
#define _lisc(n) "li 0," _NR(n)
***/
/* __asm__ ("ldi     SYS_##name,%r22"); */
/* #define _ldisc(n,p) "ldi " _NR(n) _NR(p) */
#define _ldisc(n,p) "ldi " _NR(n) p
/* __asm__ (".label _r_##name"); */
#define _r_label(n) ".label " _NR(n)
/* __asm__ ("b,n     _r_##name"); */
#define _goto_label(n) "b,n " _NR(n)


#if 0
#define _syscall0(type,name) \
type name(void) \
{ \
register long __res __asm__ ("0") = SYS_##name; \
__asm__ __volatile__ ("sc" \
                      : "=g" (__res) \
                      : "0" (SYS_##name)); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
register long __res __asm__ ("0") = SYS_##name; \
__asm__ __volatile__ ("sc" \
                      : "=g" (__res) \
                      : "0" (SYS_##name)); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
register long __res __asm__ ("0") = SYS_##name; \
__asm__ __volatile__ ("sc" \
                      : "=g" (__res) \
                      : "0" (SYS_##name)); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
register long __res __asm__ ("0") = SYS_##name; \
__asm__ __volatile__ ("sc" \
                      : "=g" (__res) \
                      : "0" (SYS_##name)); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name (atype a, btype b, ctype c, dtype d) \
{ \
register long __res __asm__ ("0") = SYS_##name; \
__asm__ __volatile__ ("sc" \
                      : "=g" (__res) \
                      : "0" (SYS_##name)); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall5(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e) \
type name (atype a,btype b,ctype c,dtype d,etype e) \
{ \
register long __res __asm__ ("0") = SYS_##name; \
__asm__ __volatile__ ("sc" \
                      : "=g" (__res) \
                      : "0" (SYS_##name)); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}
#endif	/* 0 */
#if 0	/* [ppc] */

/* Note: this works for functions which are *not* inline */
#define __syscall(name) \
 __asm__ (_lisc(SYS_##name)); \
 __asm__ ("sc"); \
 __asm__ ("bns 10f"); \
 __asm__ ("mr 0,3"); \
 __asm__ ("lis 3,errno@ha"); \
 __asm__ ("stw 0,errno@l(3)"); \
 __asm__ ("li 3,-1"); \
 __asm__ ("10:"); 
#else
/* hppa */
#define __syscall(name) /*  while(1) ; */    \
 __asm__ (".import _hppa_seterrno");         \
 /* __asm__ (".label _r_##name"); */               \
 __asm__ (_r_label(_r_##name));              \
 __asm__ ("mtsp    %r0,%sr0");               \
 __asm__ ("ldil    L%0xc0000004,%r1");       \
 __asm__ ("ble     R%0xc0000004(%sr0,%r1)"); \
 /* __asm__ ("ldi     SYS_##name,%r22"); */        \
 __asm__ (_ldisc(SYS_##name,",%r22"));       \
                                             \
 __asm__ ("bv,n    0(%r2)");                 \
 /* __asm__ ("b,n     _r_##name"); */              \
 __asm__ (_goto_label(_r_##name));           \
 __asm__ ("b       _hppa_seterrno");         \
 __asm__ ("or      %r0,%r28,%arg0");         \
 
#endif

#define _syscall0(type,name) \
type name(void) \
{ \
	__syscall(name) \
}

#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
	__syscall(name) \
}

#define _syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
	__syscall(name) \
}

#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
	__syscall(name) \
}

#define _syscall4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name (atype a, btype b, ctype c, dtype d) \
{ \
	__syscall(name) \
}

#define _syscall5(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e) \
type name (atype a,btype b,ctype c,dtype d,etype e) \
{ \
	__syscall(name) \
}


#endif /* _HPPA_SYSCALL_H */

