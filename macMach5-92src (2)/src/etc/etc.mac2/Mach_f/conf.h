#ifndef _CONF_H_
#define _CONF_H_

#define resID_conf 128

#define CONF_ENABLE		0x1
#define CONF_SINGLE		0x2
#define CONF_ALTERNATE	0x4
#define CONF_DEBUG		0x8

typedef struct {
	short	LoadDev1;
	short	RootDev1;
	short	LoadDev2;
	short	RootDev2;
	short	TimeOffset;
	short	Flags;
	long	KernelSpace;
	char	LoadFile1[256];
	char	LoadFile2[256];
} **ConfHandle;

#endif /* _CONF_H_ */