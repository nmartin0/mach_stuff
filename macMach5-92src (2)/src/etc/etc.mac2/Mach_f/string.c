/* standard string functions, string constant lookup */

#include <Resources.h>
#include <Memory.h>

#include "string.h"
#include "resID.h"

typedef struct {
	char msg[64];
} cstr;

char *getstr(n)
int n;
{
	Handle h;
	cstr *m;
	
	h = GetResource('cstr', resID_cstr_STRS);
	m = (cstr *)(*h);
	return ((char *)&m[n]);
}

int strcmp(s1, s2)
register char *s1, *s2;
{
	while (*s1 == *s2++) if (*s1++ == 0) return 0;
	return 1;
}

int strlen(s)
register char *s;
{
	register n;

	n = 0;
	while (*s++) n++;
	return n;
}

void strcpy(s1, s2)
register char *s1, *s2;
{
	while (*s2) *s1++ = *s2++;
	*s1 = 0;
}

void strcat(s1, s2)
register char *s1, *s2;
{
	while (*s1) *s1++;
	strcpy(s1, s2);
}

char *strchr(s, c)
register char *s;
register char c;
{
	while (*s && (*s != c)) s++;
	return *s ? s : 0;
}

