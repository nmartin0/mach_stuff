/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <net/checksum.h>

/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
unsigned int
csum_partial(const unsigned char *addr, int count, unsigned int sum)
{
	register const u_char *w = addr;
	register long result = 0;

	if(count && ((unsigned int) addr & 1)) {
		result = *w << 8;
		w++;
		count--;
	}

	while (count > 1)  {
		result += *(unsigned short *)w;
		w = w + sizeof(short);
		count -= 2;
	}

	if(count) 
		result += (*(unsigned char *)w) << 8;

	if(sum) {
		w = (u_char *)&sum;
		result += *(unsigned short *)w;
		w = w + sizeof(short);
		result += *(unsigned short *)w;
	}

	return result;
}

/*
 * the same as csum_partial, but copies from src while it
 * checksums
 *
 * here even more important to align src and dst on a 32-bit boundary
 */

unsigned int
csum_partial_copy(const char *src, char *dst, int len, int sum)
{
	/*
	 * The whole idea is to do the copy and the checksum at
	 * the same time, but we do it the easy way now.
	 *
	 * At least csum on the source, not destination, for cache
	 * reasons..
	 */
	sum = csum_partial(src, len, sum);
	memcpy(dst, src, len);
	return sum;
}

/*
 *  C Version of the Internet checksum.
 */
unsigned short
ip_compute_csum(unsigned char *addr, int count)
{
	register u_char *w = addr;
	register long result = 0;

	if(count && ((unsigned int) addr & 1)) {
		result = *w << 8;
		w++;
		count--;
	}

	while (count > 1)  {
		result += *(unsigned short *)w;
		w = w + sizeof(short);
		count -= 2;
	}

	if(count)
		result += (*(unsigned char *)w) << 8;

	return csum_fold(result);
}

unsigned short
ip_fast_csum(unsigned char *addr, unsigned int count)
{
	register u_char *w = addr;
	register long result = 0;

	count = count * 4;

	if(count && ((unsigned int) addr & 1)) {
		result = *w << 8;
		w++;
		count--;
	}

	while (count > 1)  {
		result += *(unsigned short *)w;
		w = w + sizeof(short);
		count -= 2;
	}
		
	if(count)
		result += (*(unsigned char *)w) << 8;

	return csum_fold(result);
}

unsigned short
csum_tcpudp_magic(unsigned long saddr, unsigned long daddr, unsigned short len, unsigned short proto, unsigned int sum)
{
	register unsigned long result = 0;
	register u_char *w;

	w = (u_char *)&saddr;
	result += *(unsigned short *)w;
	w = w + sizeof(short);
	result += *(unsigned short *)w;

	w = (u_char *)&daddr;
	result += *(unsigned short *)w;
	w = w + sizeof(short);
	result += *(unsigned short *)w;

	w = (u_char *)&sum;
	result += *(unsigned short *)w;
	w = w + sizeof(short);
	result += *(unsigned short *)w;

	w = (u_char *)&len;
	result += *(unsigned short *)w;

	w = (u_char *)&proto;
	result += *(unsigned short *)w;

	return csum_fold(result);
}

unsigned int
csum_partial_copy_fromuser(
	const char *src,
	char *dst,
	int len,
	int sum)
{
	memcpy_fromfs(dst, src, len);
	return csum_partial(dst, len, sum);
}


