#ifndef __nubus_pri_h__

int mac68k_bus_space_probe(vm_offset_t offset, int sz);
void	setfault(long *faultbuf);

int
mac68k_bus_space_probe(vm_offset_t offset, int sz)
//	bus_space_tag_t t;
//	bus_space_handle_t bsh;
//	bus_size_t offset;
{
	int i;
#if 0
	label_t faultbuf;

	/* Nubus shouldn't ever fault on PPC, should it? */
	nofault = &faultbuf;
	if (setjmp(nofault)) {
		nofault = (label_t *)0;
		return (0);
	}
#else
	jmp_buf_t faultbuf;

	if (_setjmp(&faultbuf)) {
		memset((void *)&faultbuf, 0, sizeof(faultbuf));
		return(0);
	}
#endif

	switch (sz) {
	case 1:
		i = inb(offset);
		break;
	case 2:
		i = inw(offset);
		break;
	case 4:
		i = inl(offset);
		break;
	case 8:
	default:
		panic("bus_space_probe: unsupported data size %d\n", sz);
		/* NOTREACHED */
	}

#if 0
	nofault = (label_t *)0;
	return (1);
#else
	// setfault(NULL);

	// return (!((int)faultbuf));
	return 1;
#endif
}

#if 0
void	setfault(long *faultbuf)
{
extern long *global_fault;

global_fault=faultbuf;
}
#endif

#define __nubus_pri_h__
#endif // __nubus_pri_h__
