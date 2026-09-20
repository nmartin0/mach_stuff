/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


#include <linux/kernel.h>

unsigned char
outb(unsigned char val,int port)
{
	panic("outb\n");
	return (unsigned char) 0;
}

unsigned char
inb(int port)
{
	panic("inb\n");
	return (unsigned char) 0;
}

