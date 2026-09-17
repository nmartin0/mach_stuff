/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * pmk1.1
 */


/*
 * What is usually in libs.
 * the speed is really not important here...
 */

void *
memset(void *dst, int c, unsigned int length)
{
	unsigned char *pt = (unsigned char *)dst;

	while(length--) 
		*pt++ = (unsigned char)c;

	return dst;
}

void
bzero(void *dst, unsigned int length)
{
	unsigned char *pt = (unsigned char *)dst;

	while(length--) 
		*pt++ = (unsigned char)0;
}

void *
memcpy(void *dst, const void *src, unsigned int length)
{
	unsigned char *ptdest = (unsigned char *)dst;
	unsigned char *ptsrc = (unsigned char *)dst;

	while(length--) 
		*ptdest++ = *ptsrc++;

	return dst;
}

char *
strncpy(char *dst, const char *src, unsigned int length)
{
	int i;
	unsigned char *pt = (unsigned char *)dst;

	for (i = 0; i < length; i++)
		if ((*pt++ = *src++) == '\0') {
			while (++i < length)
				*pt++ = '\0';
			return dst;
		}

	return dst;
}

void
bcopy(const void *dst, void *src, unsigned int length)
{
	unsigned char *ptdest = (unsigned char *)dst;
	unsigned char *ptsrc = (unsigned char *)dst;

	while(length--) 
		*ptdest++ = *ptsrc++;
}
