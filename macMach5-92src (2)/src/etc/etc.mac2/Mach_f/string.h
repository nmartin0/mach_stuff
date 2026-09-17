#ifndef _STRING_H_
#define _STRING_H_

#define STR_DOT					0
#define STR_PART_TYPE			1
#define STR_SPACE				2
#define STR_DEBUGGER			3

char *getstr(int);
int strcmp(char *, char *);
int strlen(char *);
void strcpy(char *, char *);
void strcat(char *, char *);
char *strchr(char *, char);

#endif /* _STRING_H_ */