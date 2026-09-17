/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

/*
 * XXX hack
 * Redefine IOCTLs because the IOC* definitions are not exported by
 * the micro-kernel, and they are different from the Linux definitions...
 * Hard-code the value here as a quick work-around. This should be undone
 * when the kernel interface for getting disk parameters gets standardized
 * and correctly exported.
 */
#define	OSFMACH3_IOCPARM_MASK	0x1fff		/* parameter length, at most 13 bits */
#define	OSFMACH3_IOC_VOID	0x20000000	/* no parameters */
#define	OSFMACH3_IOC_OUT	0x40000000	/* copy out parameters */
#define	OSFMACH3_IOC_IN		0x80000000	/* copy in parameters */
#define	OSFMACH3_IOC_INOUT	(OSFMACH3_IOC_IN|OSFMACH3_IOC_OUT)

#define _OSFMACH3_IOC(inout,group,num,len) \
	(inout | ((len & OSFMACH3_IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define	_OSFMACH3_IO(g,n)	_OSFMACH3_IOC(OSFMACH3_IOC_VOID, (g), (n), 0)
#define	_OSFMACH3_IOR(g,n,t)	_OSFMACH3_IOC(OSFMACH3_IOC_OUT,	(g), (n), sizeof(t))
#define	_OSFMACH3_IOW(g,n,t)	_OSFMACH3_IOC(OSFMACH3_IOC_IN, (g), (n), sizeof(t))
#define	_OSFMACH3_IOWR(g,n,t)	_OSFMACH3_IOC(OSFMACH3_IOC_INOUT, (g), (n), sizeof(t))

