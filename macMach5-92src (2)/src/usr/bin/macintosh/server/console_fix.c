#include <stdio.h>
#include <sys/ioctl.h>

/* When BREAK is set for the console, the next attempt to read from or write
 * to the console will first cause the console to reset.
 * See /usr/src/mach_servers/ux/server/uxkern/tty_io.c
 * See /usr/src/mach_kernel/kernel/mac2dev/console.c
 */

int console;

void console_fix(void)
{
  if (console = !strcmp(ttyname(0), "/dev/console"))
    (void)ioctl(1, TIOCSBRK, (char *)0);
}
